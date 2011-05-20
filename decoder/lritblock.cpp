/*
    POES-Decoder, a software for processing POES high resolution weather satellite images.
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
*/
//---------------------------------------------------------------------------


#include <QImage>
#include <stdlib.h>

#include "block.h"
#include "cadu.h"
#include "lritblock.h"
#include "RiceDecompression.h"

#define nolibjpeg

#ifndef nolibjpeg
extern "C" {
   #include "decoder/libjpeg/jpeglib.h"
   #include "decoder/libjpeg/jerror.h"
}

   #include <setjmp.h>

#else

   #include "ljpegdecompressor.h"

#endif


//---------------------------------------------------------------------------
#define LRIT_READ_BUFF_SIZE       8192 // CP_PDU size
#define LRIT_PDU_PRIM_HDR_LEN       16
#define LRIT_IMG_STRUCT_LEN          9
#define LRIT_RICE_RECORD_LEN         7

//---------------------------------------------------------------------------
TLRIT::TLRIT(TBlock *_block)
{
  block = _block;
  cadu  = block->getCADU();

  scanLine = NULL;
  fp = NULL;

  readBuff = (quint8 *) malloc(LRIT_READ_BUFF_SIZE * sizeof(quint8));

  zero();
}

//---------------------------------------------------------------------------
TLRIT::~TLRIT(void)
{
  if(scanLine)
     free(scanLine);

  if(readBuff)
     free(readBuff);
}

//---------------------------------------------------------------------------
void TLRIT::zero(void)
{
  LRIT_IMAGE_START = 0;
  columns = 0;
  rows = 0;
  bpp = 0;
  compressionType = LRIT_No_Compression;

  riceFlags = 49;
  ricePixelsPerBlock = 16;
  riceScanLinesPerPacket = 1;

  if(block) {
     block->setFrames(0);
     block->setFirstFrameSyncPos(-1);
  }

  if(scanLine)
     free(scanLine);
  scanLine = NULL;
}

//---------------------------------------------------------------------------
// flags&1 = check found data
bool TLRIT::check(int flags)
{
  // check allocation and file pointer status
  if(block == NULL || cadu == NULL || fp == NULL || readBuff == NULL)
     return false;

  // check found stuff
  if(flags&1) {
     if(scanLine == NULL)
        return false;

     if(block->getFrames() <= 0 || block->getFirstFrameSyncPos() < 0)
        return false;
  }

  return true;
}

//---------------------------------------------------------------------------
bool TLRIT::init(void)
{
 int frames;

  if(block == NULL)
     return false;

  zero();

  fp = block->getHandle();
  frames = countFrames();

  // assume for now we got bpp = 8, GOES LRIT/HRIT
  if(frames > 0)
     scanLine = (quint8 *) malloc(columns * sizeof(quint8));

  return check(1);
}

//---------------------------------------------------------------------------
int TLRIT::countFrames(void)
{
 long int filepos, nextpos, firstFrameSyncPos;
 int frames, pdu_hdrlen;
 quint32 fieldLen;

   if(!check())
      return 0; // fatal error

   if(check(1)) // already done
      return block->getFrames();

   block->gotoStart();
   zero();

   firstFrameSyncPos = -1;
   frames = 0;

   while((pdu_hdrlen = read_PDU_PrimaryHeader(&fieldLen)) > 0) {
      filepos = ftell(fp);
      nextpos = filepos + fieldLen;
      qDebug("frame start filepos: 0x%x, next 0x%x",
             (unsigned int) (filepos - LRIT_PDU_PRIM_HDR_LEN),
             (unsigned int) nextpos);

      if(read_ImageStructureRecord()) {
         if(frames == 0) {
            firstFrameSyncPos = filepos - LRIT_PDU_PRIM_HDR_LEN;
            LRIT_IMAGE_START  = pdu_hdrlen;
            LRIT_BLOCK_SIZE   = fieldLen;
         }

         ++frames;
      }

      qDebug("filepos: 0x%x", (unsigned int) ftell(fp));

      // hop to next primary header
      if(fseek(fp, nextpos - ftell(fp) - LRIT_PDU_PRIM_HDR_LEN, SEEK_CUR) != 0)
         break;
   }

   block->setFrames(frames);
   block->setFirstFrameSyncPos(firstFrameSyncPos);

 return frames;
}

//---------------------------------------------------------------------------
// find an image data file type header and return the
// total size of the header in bytes
int TLRIT::read_PDU_PrimaryHeader(quint32 *fieldLen)
{
 int totalLen;
 quint64 tmp64;

   totalLen = 0;
   while(fread(readBuff, 1, LRIT_PDU_PRIM_HDR_LEN, fp) == LRIT_PDU_PRIM_HDR_LEN)
   {
      if(readBuff[0] != 0) // type must be zero if it is a image primary header
         return 0;

      totalLen = (readBuff[4] << 24) | (readBuff[5] << 16) | (readBuff[6] << 8) | readBuff[7];

      // field len is in bits, convert it to bytes
      tmp64 =  (((quint64) readBuff[ 8]) << 56) |
               (((quint64) readBuff[ 9]) << 48) |
               (((quint64) readBuff[10]) << 40) |
               (((quint64) readBuff[11]) << 32) |
               (((quint64) readBuff[12]) << 24) |
               (((quint64) readBuff[13]) << 16) |
               (((quint64) readBuff[14]) <<  8) |
                ((quint64) readBuff[15]);

      // include the header size
      *fieldLen = ((quint32) (tmp64 >> 3)) + totalLen;
      qDebug("field len %d bytes", *fieldLen);

      if(readBuff[3] != 0) { // not an image data type
         // hop to next header
         fseek(fp, *fieldLen - LRIT_PDU_PRIM_HDR_LEN, SEEK_CUR);
         totalLen = 0;
      }
      else
         break;
   }

 return totalLen;
}

//---------------------------------------------------------------------------
// the image structure record will come right after the primary header
bool TLRIT::read_ImageStructureRecord(void)
{
 bool rc = true;

   if(fread(readBuff, 1, LRIT_IMG_STRUCT_LEN, fp) != LRIT_IMG_STRUCT_LEN)
      return false;

   if(readBuff[0] != 1) // type must be one if it is an image structure record
      return false;

   bpp     = readBuff[3];
   columns = (readBuff[4] << 8) | readBuff[5];
   rows    = (readBuff[6] << 8) | readBuff[7];
   compressionType = (LRIT_CompressionType) readBuff[8];

   // read mission specific compression method parameters
   if(compressionType != LRIT_No_Compression) {
      switch(block->getBlockType()) {
         case LRIT_GOES_BlockType:
            rc = readRiceCompressionRecord();
         break;

         case LRIT_JPEG_BlockType:

         break;

         default:
            rc = false;
     }
   }

 return rc;
}

//---------------------------------------------------------------------------
// the rice compression record should come immediately after
// the image structure
bool TLRIT::readRiceCompressionRecord(void)
{
 quint8 ch;

   if(fread(&ch, 1, 1, fp) != 1)
      return false;

   if(ch != 0x83) { // not NOAA Rice record type 131
      riceFlags = 49;
      ricePixelsPerBlock = 16;
      riceScanLinesPerPacket = 1;
   }
   else {
      if(fread(readBuff, 1, LRIT_RICE_RECORD_LEN - 1, fp) != (LRIT_RICE_RECORD_LEN - 1))
         return false;

      riceFlags = (readBuff[2] << 8) | readBuff[3];
      ricePixelsPerBlock = readBuff[4];
      riceScanLinesPerPacket = readBuff[5];
   }

 return true;
}

//---------------------------------------------------------------------------
int TLRIT::getWidth(void)
{
   return columns;
}

//---------------------------------------------------------------------------
int TLRIT::getHeight(void)
{
   return (rows * (block != NULL ? block->getFrames():0));
}

//---------------------------------------------------------------------------
int TLRIT::setImageType(int type)
{
#if 0
   if((Block_ImageType) type < Gray_ImageType)
      type = (int) Gray_ImageType;
   else if((Block_ImageType) type > RGB_ImageType)
      type = (int) RGB_ImageType;
#endif

 return type;
}

//---------------------------------------------------------------------------
// zero based
int TLRIT::setImageChannel(int channel)
{
#if 0
   if(channel < 0)
        channel = 0;
    else if(channel >= HRPT_NUM_CHANNELS)
        channel = HRPT_NUM_CHANNELS - 1;
#endif

  return channel;
}

//---------------------------------------------------------------------------
bool TLRIT::uncompress(const char *filename)
{
#if 1

   filename = filename;
   qDebug("uncompression not enabled");

   return false;
#else

 /*

   We need to have access to the whole CP_PDU layer for rice decompression

 */

 CRiceDecompression *rice;
 long int filepos, len;
 int pdu_hdrlen, i, frames;
 FILE *out;

   if(!check(1) || filename == NULL)
      return false;

   // rice compression only at the moment
   if(compressionType != LRIT_Lossless_Compression)
      return false;

   if(fseek(fp, block->getFirstFrameSyncPos(), SEEK_SET) != 0)
      return false;

   out = fopen(filename, "ab");
   if(out == NULL)
      return false;
/*
        CRiceDecompression(
                int Mask,
                int BitsPerPixel,
                int PixelsPerBlock,
                int PixelsPerScanline,
                int ScanLinesPerPacket);


*/
   rice = new CRiceDecompression(riceFlags, bpp, ricePixelsPerBlock, columns, riceScanLinesPerPacket);
   frames = block->getFrames();
   i = 0;
   while(i < frames && (pdu_hdrlen = read_PDU_PrimaryHeader()) > 0) {
      i++;

      fwrite(readBuff, LRIT_PDU_PRIM_HDR_LEN, 1, out);
      len = LRIT_PDU_PRIM_HDR_LEN;

      filepos = ftell(fp) - LRIT_PDU_PRIM_HDR_LEN;
      if(fread(readBuff, 1, LRIT_IMG_STRUCT_LEN, fp) != LRIT_IMG_STRUCT_LEN)
         break;

      // change only the compression flag
      readBuff[8] = 0x00;
      fwrite(readBuff, LRIT_IMG_STRUCT_LEN, 1, out);
      len += LRIT_IMG_STRUCT_LEN;

      // write the rest of the header(s)
      len = pdu_hdrlen - len;
      if(len > 0) {
         if(fread(readBuff, 1, len, fp) != len)
            break;
         fwrite(readBuff, len, 1, out);
      }

      // uncomress the scanline
#if 1

      qDebug("rice uncompression not enabled!");
      break;

#else
      /*
         read the entire user data field in CP_PDU
         datalength is packet length - 1
      */

      if(fread(scanLine, 1, columns, fp) != columns)
         break;
      if(!rice->Decompress(scanLine, columns)) {
         qDebug("rice->decompress failed!");
         break;
      }

      fwrite(rice->Ptr(), rice->Size(), 1, out);
#endif
   }

   fclose(out);
   delete rice;

 return true;
#endif
}

//---------------------------------------------------------------------------
// frame_nr is zero based (0, 1, 2, ... frames - 1)
bool TLRIT::readFrameScanLine(int frame_nr, int row_nr)
{
#if 1
   row_nr = row_nr;
   frame_nr = frame_nr;
   return false;
#else
 long int pos, scanPos;

  if(!check(1))
     return false;

  pos = ftell(fp);
  if(pos < 0)
     return false;

  scanPos = block->getFirstFrameSyncPos() + (frame_nr * LRIT_IMAGE_START);

  if(pos != scanPos) {
     if(pos > scanPos)
        fseek(fp, scanPos, SEEK_SET);
     else
        fseek(fp, scanPos - pos, SEEK_CUR);
  }

  if(fread(scanLine, columns, 1, fp) != 1)
     return false;

 return true;
#endif
}

//---------------------------------------------------------------------------
bool TLRIT::toImage(QImage *image)
{
 int frames, y;

  if(!check(1))
     return false;

  block->gotoStart();
  frames = block->getFrames();

  for(y=0; y<frames; y++) {
     if(!frameToImage(y, image))
        break;
  }

 return y > 0 ? true:false;
}

//---------------------------------------------------------------------------
// fills an 24 bpp image line of the selected channel
// frame_nr is zero based
bool TLRIT::frameToImage(int frame_nr, QImage *image)
{
 long int scanPos;
 bool rc;

  if(!check(1) || image == NULL)
     return false;

  scanPos = block->getFirstFrameSyncPos() + (LRIT_IMAGE_START + frame_nr*LRIT_BLOCK_SIZE);
  qDebug("lrit scanPos: 0x%x frame: %d", (unsigned int) scanPos, frame_nr);

  if(fseek(fp, scanPos - ftell(fp), SEEK_CUR) != 0)
     return false;

  if(isCompressed()) {
     switch(block->getBlockType()) {
        case LRIT_GOES_BlockType:
           rc = false; // not yet supported
        break;

        case LRIT_JPEG_BlockType:
           rc = readjpegcompressed(frame_nr, image);
        break;

        default:
           rc = false;
     }
  }
  else
     rc = readUncompressed(frame_nr, image);

 return rc;
}

//---------------------------------------------------------------------------
bool TLRIT::readUncompressed(int frame_nr, QImage *image)
{
 uchar *imagescan, r, g, b;
 int x, y, ypos;

  ypos = frame_nr*rows;

  for(y=ypos; y < (ypos + rows); y++) {
     if(fread(scanLine, columns, 1, fp) != 1)
        return false;

     imagescan = (uchar *) image->scanLine(y);
     if(imagescan == NULL)
        return false;

     for(x=0; x<columns; x++) {
        switch(block->getImageType()) {
           case Gray_ImageType:
              r = scanLine[x];
              g = r;
              b = r;
           break;

           default:
              return false;
        }

        *imagescan++ = r;
        *imagescan++ = g;
        *imagescan++ = b;
     }
  }

 return true;
}

//---------------------------------------------------------------------------
#ifdef nolibjpeg

//---------------------------------------------------------------------------
bool TLRIT::readjpegcompressed(int frame_nr, QImage *image)
{
 TLJPEGDecompressor *ljpeg;

   image = image;
   frame_nr = frame_nr;

   ljpeg = new TLJPEGDecompressor(fp);
   ljpeg->readHeader();

   delete ljpeg;

   return false;
}

#else

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
 char buffer[JMSG_LENGTH_MAX];

  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  // (*cinfo->err->output_message) (cinfo);

  /* Create the message */
  (*cinfo->err->format_message) (cinfo, buffer);

  qDebug("JPEG Library Error: %s", buffer);
  /* Send it to stderr, adding a newline */
  // fprintf(stderr, "%s\n", buffer);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

#if 1

void my_emit_message( j_common_ptr cinfo, int msg_level )
{
        struct jpeg_error_mgr *err = cinfo->err;

        msg_level = 1;
        cinfo->err->trace_level = 4;

        if( msg_level < 0 )
        {
                /* It's a warning message.  Since corrupt files may generate many warnings,
                 * the policy implemented here is to show only the first warning,
                 * unless trace_level >= 3.
                 */
                if( err->num_warnings == 0 || err->trace_level >= 3 )
                        (*err->output_message) (cinfo);

                /* Always count warnings in num_warnings. */
                err->num_warnings++;
        } else {
                /* It's a trace message.  Show it if trace_level >= msg_level. */
                if( err->trace_level >= msg_level )
                        (*err->output_message) (cinfo);
        }
}

#endif

METHODDEF(void)
output_message (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */

  (*cinfo->err->format_message) (cinfo, buffer);

  qDebug("%s", buffer);
  /* Send it to stderr, adding a newline */
  fprintf(stderr, "%s\n", buffer);
}

//---------------------------------------------------------------------------
bool TLRIT::readjpegcompressed(int frame_nr, QImage *image)
{

 struct jpeg_decompress_struct dinfo;
 struct jpeg_compress_struct cinfo;
 JQUANT_TBL *quant_ptr;
 JHUFF_TBL *htblptr;

 struct my_error_mgr jerr;
 int jpeg_rc, i;

#if 1 // create JPEG

 struct jpeg_error_mgr cjerr;
 FILE * outfile;		/* target file */
 JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
 int row_stride;		/* physical row width in image buffer */
 int image_width = 1;
 int image_height = 1;

 /* Step 1: allocate and initialize JPEG compression object */

 /* We have to set up the error handler first, in case the initialization
  * step fails.  (Unlikely, but it could happen if you are out of memory.)
  * This routine fills in the contents of struct jerr, and returns jerr's
  * address which we place into the link field in cinfo.
  */
 cinfo.err = jpeg_std_error(&cjerr);
 /* Now we can initialize the JPEG compression object. */
 jpeg_create_compress(&cinfo);

 /* Step 2: specify data destination (eg, a file) */
 /* Note: steps 2 and 3 can be done in either order. */

 /* Here we use the library-supplied code to send compressed data to a
  * stdio stream.  You can also write your own code to do something else.
  * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
  * requires it in order to write binary files.
  */
 if ((outfile = fopen("c:\\tmp\\test.jpg", "wb")) == NULL) {
    return false;
 }
 jpeg_stdio_dest(&cinfo, outfile);

 /* Step 3: set parameters for compression */

 /* First we supply a description of the input image.
  * Four fields of the cinfo struct must be filled in:
  */
 cinfo.image_width = image_width; 	/* image width and height, in pixels */
 cinfo.image_height = image_height;
 cinfo.input_components = 1;		/* # of color components per pixel */
 cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
 /* Now use the library's routine to set default compression parameters.
  * (You must set at least cinfo.in_color_space before calling this,
  * since the defaults depend on the source color space.)
  */
 jpeg_set_defaults(&cinfo);
 /* Now you can set any non-default parameters you wish to.
  * Here we just illustrate the use of quality (quantization table) scaling:
  */
 jpeg_set_quality(&cinfo, 100, TRUE /* limit to baseline-JPEG values */);
 cinfo.write_JFIF_header = false;
 cinfo.arith_code = false;
 cinfo.progressive_mode = false;

 /* Step 4: Start compressor */

 /* TRUE ensures that we will write a complete interchange-JPEG file.
  * Pass TRUE unless you are very sure of what you're doing.
  */
 jpeg_start_compress(&cinfo, TRUE);

 /* Step 5: while (scan lines remain to be written) */
 /*           jpeg_write_scanlines(...); */

 /* Here we use the library's state variable cinfo.next_scanline as the
  * loop counter, so that we don't have to keep track ourselves.
  * To keep things simple, we pass one scanline per call; you can pass
  * more if you wish, though.
  */
 row_stride = image_width * cinfo.input_components; /* JSAMPLEs per row in image_buffer */

 memset(scanLine, 0, columns);

 while (cinfo.next_scanline < cinfo.image_height) {
   /* jpeg_write_scanlines expects an array of pointers to scanlines.
    * Here the array is only one element long, but you could pass
    * more than one scanline at a time if that's more convenient.
    */
   row_pointer[0] = & scanLine[cinfo.next_scanline * row_stride];
   (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
 }
 /* Step 6: Finish compression */

 jpeg_finish_compress(&cinfo);
 /* After finish_compress, we can close the output file. */
 fclose(outfile);

 /* Step 7: release JPEG compression object */

 /* This is an important step since it will release a good deal of memory. */
 jpeg_destroy_compress(&cinfo);

 /* And we're done! */
 return false;

#endif // create JPEG

#if 0
   qDebug("LRIT JPEG decomress start");

   dinfo.err = jpeg_std_error(&jerr.pub);

   jerr.pub.error_exit = my_error_exit;
   //jerr.pub.emit_message = my_emit_msg;
   /* Establish the setjmp return context for my_error_exit to use. */
   if(setjmp(jerr.setjmp_buffer)) {
     // If we get here, the JPEG code has signaled an error.

     jpeg_destroy_decompress(&dinfo);

     return false;
   }

   jpeg_create_decompress(&dinfo);
   qDebug("jpeg_create_decompress");


   jpeg_stdio_src(&dinfo, fp);
   qDebug("jpeg_stdio_src");

   dinfo.image_height = rows;
   dinfo.image_width = columns;
   dinfo.num_components = 3;
   dinfo.jpeg_color_space = JCS_RGB;

   jpeg_rc = jpeg_read_header(&dinfo, true);
   qDebug("jpeg_read_header: %d", jpeg_rc);

   if(jpeg_rc != JPEG_HEADER_OK) {
      jpeg_destroy_decompress(&dinfo);

      return false;
   }

   if(compressionType == LRIT_Lossless_Compression) {
#if 0
      // create a default DQT marker

      jpeg_create_compress(&cinfo);

      cinfo.input_components = 3;
      cinfo.in_color_space = JCS_RGB;

      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, 100, TRUE);
/*
      cinfo.image_height = dinfo.image_height;
      cinfo.image_width = dinfo.image_width;
      cinfo.input_components = dinfo.num_components;
      cinfo.in_color_space = dinfo.jpeg_color_space;
*/

      // copy the default DQT tables to the decompress struct
      i = 0;
      while(true) {
         if(cinfo.quant_tbl_ptrs[i] == NULL)
            break;
         if(dinfo.quant_tbl_ptrs[i] != NULL)
            break;

         dinfo.quant_tbl_ptrs[i] = jpeg_alloc_quant_table((j_common_ptr) &dinfo);
         memcpy(dinfo.quant_tbl_ptrs[i], cinfo.quant_tbl_ptrs[i], sizeof(JQUANT_TBL));

         i++;
      }

      // copy huffman tables
      i = 0;
      while(true) {

         if(cinfo.dc_huff_tbl_ptrs[i] == NULL)
            break;
         if(dinfo.dc_huff_tbl_ptrs[i] == NULL)
            dinfo.dc_huff_tbl_ptrs[i] = jpeg_alloc_huff_table((j_common_ptr) &dinfo);

         memcpy(&dinfo.dc_huff_tbl_ptrs[i], &cinfo.dc_huff_tbl_ptrs[i], sizeof(JHUFF_TBL));

         //memcpy(&(*dinfo.dc_huff_tbl_ptrs[i]).bits, &(*cinfo.dc_huff_tbl_ptrs[i]).bits, sizeof(*(dinfo.dc_huff_tbl_ptrs[i])->bits));
         //memcpy(&(*dinfo.dc_huff_tbl_ptrs[i]).huffval, &(*cinfo.dc_huff_tbl_ptrs[i]).huffval, sizeof(*(dinfo.dc_huff_tbl_ptrs[i])->huffval));

         i++;

       }

#if 0
      if(cinfo.comp_info != NULL && dinfo.comp_info == NULL) {
         dinfo.comp_info = (jpeg_component_info *) (dinfo.mem->alloc_small)
                             ((j_common_ptr) &dinfo, JPOOL_IMAGE,
                              cinfo.num_components * sizeof(jpeg_component_info));
         memcpy(dinfo.comp_info, cinfo.comp_info, cinfo.num_components * sizeof(jpeg_component_info));

      }
#endif

#if 0
      // copy huffman tables
      i = 0;
      while(true) {
         if(cinfo.ac_huff_tbl_ptrs[i] == NULL)
            break;
         if(dinfo.ac_huff_tbl_ptrs[i] == NULL)
            dinfo.ac_huff_tbl_ptrs[i] = jpeg_alloc_huff_table((j_common_ptr) &dinfo);

         //unsigned char **tmp = cinfo.ac_huff_tbl_ptrs[i]->bits;

         memcpy(&dinfo.ac_huff_tbl_ptrs[i], &cinfo.ac_huff_tbl_ptrs[i], sizeof(JHUFF_TBL));

         //memcpy(&(*dinfo.ac_huff_tbl_ptrs[i]).bits, &(*cinfo.ac_huff_tbl_ptrs[i]).bits, sizeof(*(dinfo.ac_huff_tbl_ptrs[i])->bits));
         //memcpy(&(*dinfo.ac_huff_tbl_ptrs[i]).huffval, &(*cinfo.ac_huff_tbl_ptrs[i]).huffval, sizeof(*(dinfo.ac_huff_tbl_ptrs[i])->huffval));

         i++;

       }

      // MEMCOPY((*htblptr)->bits, bits, SIZEOF((*htblptr)->bits));
       // MEMCOPY((*htblptr)->huffval, huffval, SIZEOF((*htblptr)->huffval));
#endif




      jpeg_destroy_compress(&cinfo);
#endif
   }


   qDebug("jpeg_set_quality");

#if 1
   jpeg_start_decompress(&dinfo);
   qDebug("jpeg_start_decompress");

   jpeg_finish_decompress(&dinfo);
   qDebug("jpeg_finish_decompress....OK!");
#endif

   jpeg_destroy_decompress(&dinfo);


 return false; // true;

#endif
}
#endif // nolibjpeg


//---------------------------------------------------------------------------
