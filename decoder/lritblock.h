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

#ifndef lritblockH
#define lritblockH

//---------------------------------------------------------------------------
//
//                      include's
//
//---------------------------------------------------------------------------
#include <QtGlobal>
#include <stdio.h>


//---------------------------------------------------------------------------
//
//                      typedef's
//
//---------------------------------------------------------------------------
typedef enum LRIT_CompressionType_t
{
   LRIT_No_Compression = 0,
   LRIT_Lossless_Compression,
   LRIT_Lossy_Compression
} LRIT_CompressionType;

//---------------------------------------------------------------------------
class QImage;
class TBlock;
class TCADU;



//---------------------------------------------------------------------------
class TLRIT
{
 public:
    TLRIT(TBlock *_block);
    ~TLRIT(void);

    bool init(void);
    int  countFrames(void);
    int  getWidth(void);
    int  getHeight(void);

    bool isCompressed(void) { return compressionType == LRIT_No_Compression ? false:true; }
    bool uncompress(const char *filename);

    int  setImageType(int type);
    int  setImageChannel(int channel);

    bool readFrameScanLine(int frame_nr, int row_nr);
    bool frameToImage(int frame_nr, QImage *image);
    bool toImage(QImage *image);


    int Modes;

 protected:
    bool check(int flags=0);
    void zero(void);

    int  read_PDU_PrimaryHeader(quint32 *fieldLen);
    bool read_ImageStructureRecord(void);
    bool readRiceCompressionRecord(void);

    bool readUncompressed(int frame_nr, QImage *image);
    bool readjpegcompressed(int frame_nr, QImage *image);

 private:
    TBlock  *block;
    TCADU   *cadu;
    FILE    *fp;

    quint8 *scanLine, *readBuff;
    int    bpp, columns, rows;
    quint32 LRIT_BLOCK_SIZE, LRIT_IMAGE_START;

    LRIT_CompressionType compressionType;

    int riceFlags, ricePixelsPerBlock, riceScanLinesPerPacket;
};

//---------------------------------------------------------------------------
#endif // lrit
