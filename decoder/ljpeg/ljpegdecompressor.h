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

#ifndef _LJPEG_DECOMPRESS_
#define _LJPEG_DECOMPRESS_

#include <QtGlobal>
#include <stdio.h>
#include "ljpeg.h"


//---------------------------------------------------------------------------


#if 0
/*
 * One of the following structures is created for each huffman coding
 * table.  We use the same structure for encoding and decoding, so there
 * may be some extra fields for encoding that aren't used in the decoding
 * and vice-versa.
 */
typedef struct HuffmanTable {
    /*
     * These two fields directly represent the contents of a JPEG DHT
     * marker
     */
    Uchar bits[17];
    Uchar huffval[256];

    /*
     * This field is used only during compression.  It's initialized
     * FALSE when the table is created, and set TRUE when it's been
     * output to the file.
     */
    int sentTable;

    /*
     * The remaining fields are computed from the above to allow more
     * efficient coding and decoding.  These fields should be considered
     * private to the Huffman compression & decompression modules.
     */
    Ushort ehufco[256];
    char ehufsi[256];

    Ushort mincode[17];
    int maxcode[18];
    short valptr[17];
    int numbits[256];
    int value[256];
} HuffmanTable;
#endif

//---------------------------------------------------------------------------

class TLJPEGComponent;
class THuffmanTable;
class TLJPEGReader;
class PList;

//---------------------------------------------------------------------------
class TLJPEGDecompressor
{
public:
   TLJPEGDecompressor(FILE *_fp);
   ~TLJPEGDecompressor(void);

   void reset();

   TLJPEGReader *reader;
   bool readHeader(void);
   bool init(void);

   // data read from SOFn
   quint16 image_width;
   quint16 image_height;
   quint8  sample_precision;
   PList   *comp_info;

   TLJPEGComponent *getCompById(quint8 id);

   // data read from SOS
   PList *cur_comp_info; // pointer to scan comp_info objects

   quint8 Ss;     /* start of spectral or predictor selection
                   *  0 for lossy processes,
                   *  1-7 according to predictor table
                   */
   quint8 Al;     /* successive approximation bit position
                   *  low or point transform
                   *  0 for lossy processes,
                   *  0 .. 15 for lossless processes
                   *  point transform parameter
                   */

   // data read from DHT
   THuffmanTable *getHuffmanTable(quint8 id, quint8 table_type);
   PList *huffmantables;

   // data read from DRI
   quint16 restart_interval; // MCUs per restart interval, or 0 for no restart

protected:
   bool decompress;              // true after readHeader and init is called successfully

   TMCU *mcuROW1, *mcuROW2;       // point to two rows of MCU for encoding & decoding




    /*
     * MCUmembership[i] indexes the i'th component of MCU into the
     * curCompInfo array.
     */
    short MCUmembership[10];

    /*
     * ptrs to Huffman coding tables, or NULL if not defined
     */
    // HuffmanTable *dcHuffTblPtrs[4];

    /*
     * In lossless JPEG, restart interval shall be an integer
     * multiple of the number of MCU in a MCU row.
     */
    int restartInRows; /*if > 0, MCU rows per restart interval; 0 = no restart*/

    /*
     * these fields are private data for the entropy decoder
     */
    int restartRowsToGo;	/* MCUs rows left in this restart interval */
    short nextRestartNum;	/* # of next RSTn marker (0..7) */

private:


};


#endif /* _LJPEG_DECOMPRESS_ */
