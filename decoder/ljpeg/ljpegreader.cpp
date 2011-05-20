/*
    HRPT-Decoder, a software for processing NOAA-POES high resolution weather satellite images.
    Copyright (C) 2009 Free Software Foundation, Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Email: <postmaster@poes-weather.com>
    Web: <http://www.poes-weather.com>

    ------------------------------------------------------------------------
    Lossless JPEG decompression derived from work by:

    Copyright (C) 1991, 1992, Thomas G. Lane.
    Part of the Independent JPEG Group's software.
    See the file Copyright for more details.

    Copyright (c) 1993 Brian C. Smith, The Regents of the University
    of California
    All rights reserved.

    Copyright (c) 1994 Kongji Huang and Brian C. Smith.
    Cornell University
    All rights reserved.
 */
//---------------------------------------------------------------------------
#include <stdlib.h>

#include "ljpegcomponent.h"
#include "ljpegdecompressor.h"
#include "ljpeghuffmantable.h"
#include "ljpegreader.h"
#include "plist.h"



//---------------------------------------------------------------------------
TLJPEGReader::TLJPEGReader(TLJPEGDecompressor *_dc, FILE *_fp)
{
   dc = _dc;
   fp = _fp;

   ljpegbuff = (quint8 *) malloc(LJPEG_BUF_SIZE);

   reset();
}

//---------------------------------------------------------------------------
TLJPEGReader::~TLJPEGReader(void)
{
   if(ljpegbuff != NULL)
      free(ljpegbuff);

}

//---------------------------------------------------------------------------
void TLJPEGReader::reset(void)
{
  // reset
  headerSize = 0;

  dc->reset();
}

//---------------------------------------------------------------------------
bool TLJPEGReader::readHeader(void)
{
 quint16 w;
 long int pos, pos2;

   if(fp == NULL || ljpegbuff == NULL)
      return false;

   // expect the file pointer to be at SOI
   if(!readword(&w) || w != 0xFFD8)
      return false;

   if(headerSize > 2) { // hop to image start
      if(fseek(fp, (headerSize - 2), SEEK_CUR) == 0)
         return true;
      else
         return false;
   }

   reset();

   pos = ftell(fp) - 2;
   if(pos < 0)
      return false;

   if(!readMarkers())
      return false;

   pos2 = ftell(fp);

   headerSize = (pos2 < pos ? pos:pos2) - pos;

   return headerSize <= 2 ? false:true;
}

//---------------------------------------------------------------------------
bool TLJPEGReader::readMarkers(void)
{
 JPEGMarker marker;
 quint8 flags, expectedflags;
 long int pos;

  // at the moment we are only interested in
  // SOF3, SOS, DHT and possibly DRI = (flags & 8)
  expectedflags = (1 | 2 | 4);
  flags = 0;

  while(true) {
     pos = ftell(fp);
     if(pos < 0)
        return false;

     marker = readNextMarker();

     // DCT compression is not supported
     switch(marker) {
        case M_SOF0: // baseline DCT Huffman, lossy
        case M_SOF1: // extended sequential DCT Huffman
        case M_SOF2:
        case M_SOF3: // spatial sequential lossless Huffman
        case M_SOF5:
        case M_SOF6:
        case M_SOF7:
        case M_SOF9:
        case M_SOF10:
        case M_SOF11:
        case M_SOF13:
        case M_SOF14:
        case M_SOF15:
        {
           if(!readSOF(marker))
              return false;
           flags |= 1;
        }
        break;

        case M_SOS: // start of scan header
        {
           if(!readSOS())
              return false;
           flags |= 2;
        }
        break;

        case M_DHT: // huffman table
           if(!readDHT())
              return false;
           flags |= 4;
        break;

        case M_DRI: // restart interval
           if(!readDRI())
              return false;
           flags |= 8;
        break;

        case M_EOI: // end of image
        {
           if((flags & expectedflags) == expectedflags) {
              // seek back to end of last found marker, MCU start
              if(fseek(fp, pos, SEEK_SET) != 0)
                 return false;
              else
                 return true;
           }
           else
              return false;
        }
        break;

        case M_JPG:
        case M_SOI: // duplicate or reached next frame?
        case M_JPG0:
        case M_JPG13:
        case M_DQT: // this is lossy
        case M_ERROR:
           return false;

        case M_DNL:
        case M_DAC:
        case M_COM:
        case M_APP0:
        case M_APP1:
        case M_APP2:
        case M_APP3:
        case M_APP4:
        case M_APP5:
        case M_APP6:
        case M_APP7:
        case M_APP8:
        case M_APP9:
        case M_APP10:
        case M_APP11:
        case M_APP12:
        case M_APP13:
        case M_APP14:
        case M_APP15:
           skipMarker();
        break;

        case M_RST0:
        case M_RST1:
        case M_RST2:
        case M_RST3:
        case M_RST4:
        case M_RST5:
        case M_RST6:
        case M_RST7:
        case M_TEM:
           // parameterless
        break;

        default:
           return false;
     }
  }
}

//---------------------------------------------------------------------------
JPEGMarker TLJPEGReader::readNextMarker(void)
{
 quint8 b;

   do {
      // skip any non-FF bytes
      do {
         if(!readbyte(&b))
            return M_ERROR;
      } while(b != 0xFF);

      // skip any duplicate FFs, extra FFs are legal
      do {
         if(!readbyte(&b))
            return M_ERROR;
      } while(b == 0xFF);

   } while(b == 0); // repeat if it was a stuffed FF00

   return ((JPEGMarker) b);
}

//---------------------------------------------------------------------------
bool TLJPEGReader::readbyte(quint8 *value)
{
   *value = 0;

   if(fp != NULL)
      if(fread(value, 1, 1, fp) == 1)
         return true;

 return false;
}

//---------------------------------------------------------------------------
bool TLJPEGReader::readbytes(quint8 *buff, size_t bytes, size_t maxlen)
{
   if(fp != NULL && buff != NULL && bytes < maxlen && bytes != 0)
      if(fread(buff, 1, bytes, fp) == bytes)
         return true;

   return false;
}

//---------------------------------------------------------------------------
bool TLJPEGReader::readword(quint16 *value)
{
 quint8 b;

    *value = 0;

    if(readbyte(&b)) {
       *value = (b << 8);
       if(readbyte(&b)) {
          *value |= b;

          return true;
       }
    }

 return false;
}

//---------------------------------------------------------------------------
void TLJPEGReader::skipMarker(void)
{
 quint8  b;
 quint16 len;

   if(readword(&len))
      while(len--)
         if(fread(&b, 1, 1, fp) != 1)
            break;
}

//---------------------------------------------------------------------------
bool TLJPEGReader::readSOF(JPEGMarker marker)
{
 TLJPEGComponent *comp; 
 quint16 len;
 quint8 i, num_components;

   if(marker != M_SOF3)
      return false;

   if(!readword(&len) || len != 11)
      return false;

   if(!readbytes(ljpegbuff, len - 2))
      return false;

   dc->sample_precision = ljpegbuff[0];
   dc->image_height     = (ljpegbuff[1] << 8) | ljpegbuff[2];
   dc->image_width      = (ljpegbuff[3] << 8) | ljpegbuff[4];
   num_components       = ljpegbuff[5];

   if((int)dc->image_height <= 0 || (int)dc->image_width <= 0 || num_components > 4)
      return false; // fatal

   // Lossless JPEG specifies data precision to be from 2 to 16 bits/sample.
   if(dc->sample_precision < 2 || dc->sample_precision > 16)
      return false; // erroneous or not supported

   for(i=0; i<num_components; i++) {
      comp = (TLJPEGComponent *) dc->comp_info->ItemAt(i);
      if(comp == NULL) {
         comp = new TLJPEGComponent;
         dc->comp_info->Add(comp);
      }

      comp->id            = ljpegbuff[6];
      comp->h_samp_factor = (ljpegbuff[7] >> 4) & 0x0F;
      comp->v_samp_factor = ljpegbuff[7] & 0x0F;

      if(comp->h_samp_factor != 1 || comp->v_samp_factor != 1)
         return false; // downsampling not supported yet
   }

 return true;
}

//---------------------------------------------------------------------------
bool TLJPEGReader::readSOS(void)
{
 TLJPEGComponent *comp;
 quint16 len;
 quint8 cc;
 int n;

   if(!readword(&len) || len <= 2)
      return false;

   if(!readbytes(ljpegbuff, len - 2))
      return false;

   n = (int) ljpegbuff[0];
   if(len != (n * 2 + 6) || n < 1 || n > MAX_COMPS_IN_SCAN)
      return false;

   cc = ljpegbuff[1]; // component selector

   comp = dc->getCompById(cc);
   if(comp == NULL)
      return false;

   if(dc->cur_comp_info->IndexOf(comp) == -1)
      dc->cur_comp_info->Add(comp);

   comp->dc_tbl_no = (ljpegbuff[2] >> 4) & 0x0F;

   dc->Ss = ljpegbuff[3];
   dc->Al = ljpegbuff[5] & 0x0F;

 return true;
}

//---------------------------------------------------------------------------
bool TLJPEGReader::readDHT(void)
{
 THuffmanTable *table;
 quint16 len;
 quint8 id, type;
 int i, count, pos;

   if(!readword(&len) || len <= 18)
      return false;

   if(!readbytes(ljpegbuff, len - 2))
      return false;

   if(ljpegbuff[0] & 0x10) {
      // AC entropy table, not suported though...
      id = ljpegbuff[0] - 0x10;
      type = 1;
   }
   else {
      // DC entropy table
      id = ljpegbuff[0];
      type = 0;
   }

   if(id > 4)
      return false;

   table = dc->getHuffmanTable(id, type);
   if(table == NULL) {
      table = new THuffmanTable(id, type);

      if(table->bits == NULL || table->huffval == NULL) {
         delete table;
         return false;
      }

      dc->huffmantables->Add(table);
   }


   pos = 1;
   count = 0;
   for(i=0; i<HUFFMAN_BITS_SIZE; i++) {
      table->bits[i] = ljpegbuff[pos++];
      count += table->bits[i];
   }

   if(count >= HUFFMAN_VAL_SIZE)
      return false;

   for(i=0; i<count; i++)
      table->huffval[i] = ljpegbuff[pos++];

 return true;
}

//---------------------------------------------------------------------------
bool TLJPEGReader::readDRI(void)
{
 quint16 len;

 if(!readword(&len) || len != 4)
    return false;

 if(!readword(&dc->restart_interval))
    return false;

 return true;
}
