/*
    HRPT-Decoder, a software for processing NOAA-POES hig resolution weather satellite images.
    Copyright (C) 2009,2010,2011 Free Software Foundation, Inc.

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
#ifndef BLOCK_H
#define BLOCK_H

#include <QtGlobal>
#include <QStringList>
#include <stdio.h>

#include "satprop.h"
#include "cadu.h"

//---------------------------------------------------------------------------
#define B_BYTESWAP          1   // little endian data
#define B_NORTHBOUND        2   // pass is northbound
#define B_SYNC_FOUND        4   // first sync found

//---------------------------------------------------------------------------
#define CADU_SYNC_SIZE       4

typedef enum BlockType_t
{
    Undefined_BlockType = -1,
    HRPT_BlockType = 0,
    FY1HRPT_BlockType,
    AHRPT_BlockType,
    MN1HRPT_BlockType,

    // semi supported
    LRPT_BlockType,
    MN1LRPT_BlockType,
    LRIT_GOES_BlockType,   // uncompressed
    LRIT_JPEG_BlockType,   // JPEG compressed
} Block_Type;
#define NUM_SUPPORTED_BLOCKS (LRIT_JPEG_BlockType + 1)

typedef enum Block_ImageType_t
{
    Channel_ImageType = 0,      // grayscale per channel
    RGB_ImageType,              // user defined RGB
    NDVI_ImageType,             // user defined NDVI
} Block_ImageType;


//---------------------------------------------------------------------------
#define SCALE16TO8(x) \
  ((quint8)                             \
   ((((quint16) x) & 0x03ff) >> 2))

#define SWAP16PTR(x) \
{                                       \
  quint8 tmp, *ptr = (quint8 *) x;      \
                                        \
  tmp = ptr[0];                         \
  ptr[0] = ptr[1];                      \
  ptr[1] = tmp;                         \
}


//---------------------------------------------------------------------------
class QImage;
class QString;
class TCADU;
class TSatProp;
class TRGBConf;
class TNDVI;

//---------------------------------------------------------------------------
class TBlock
{
 public:
    TBlock(void);
    ~TBlock(void);

    bool open(const char *filename);
    FILE *getHandle(void) { return fp; }
    void close(void);

    QString    getBlockTypeStr(int index, int flags=0);
    bool       setBlockType(Block_Type type);
    Block_Type getBlockType(void) { return blocktype; }

    TCADU *getCADU(void) { return cadu; }

    bool isCompressed(void);
    bool uncompress(const char *filename);

    void gotoStart(void);
    void setLittleEndian(bool on);
    bool isLittleEndian(void) { return Modes&B_BYTESWAP ? true:false; }

    long int countCADUFrames(long int block_size);
    bool findCADUFrameSync(void);

    long int getFrames(void) { return frames; }
    void setFrames(int count=0) { frames = count; }

    void setFirstFrameSyncPos(long int count=-1) { firstFrameSyncPos = count; }
    int  getFirstFrameSyncPos(void) { return firstFrameSyncPos; }
    
    //void setImageType(Block_ImageType type);
    void setImageType(int index);
    Block_ImageType getImageType(void) { return imagetype; }
    QStringList getImageTypes(void) const;

    void setNorthBound(bool on);
    bool isNorthBound(void) { return Modes&B_NORTHBOUND ? true:false; }

    void setImageChannel(int channel);
    int  getImageChannel(void) { return imageChannel; }
    int  getNumChannels(void);
    void checkSatProps(void);

    int  getWidth(void);
    int  getHeight(void);
    bool toImage(QImage *image);

    int  Modes;

    TSatProp *satprop;
    TRGBConf *rgbconf;
    TNDVI    *ndvi;

 protected:
    bool init(void);
    void freeBlock(void);
    void setMode(bool on, int flag);


 private:
    FILE *fp;
    int  imageChannel;
    long int frames, firstFrameSyncPos;

    Block_Type      blocktype;
    Block_ImageType imagetype;

    void *block; // pointer to hrpt, lrpt, lrit, etc
    TCADU *cadu;
};

#endif // BLOCK_H
