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
#include <stdlib.h>

#include "mn1lrptblock.h"
#include "block.h"
#include "ReedSolomon.h"


const int MN1LRPT_BLOCK_SIZE    = 256;  // undecoded rs decoded size in bytes
const int MN1LRPT_RS_BLOCK_SIZE = 1024; // rs decoded size in bytes
const int MN1LRPT_NUM_CHANNELS  = 6;
const int MN1LRPT_SCAN_SIZE     = 20480;
const int MN1LRPT_SCAN_WIDTH    = 1536;


#define MN1LRPT_SYNC_SIZE 4
static const quint8 MN1LRPT_SYNC[MN1LRPT_SYNC_SIZE] = {
  0x1A, 0xCF, 0xFC, 0x1D
};
// 1ACFFC1D

//---------------------------------------------------------------------------
TMN1LRPT::TMN1LRPT(TBlock *_block)
{
  block      = _block;

  scanLine   = NULL;
  rsData     = NULL;
  fp         = NULL;
  syncOffset = 0;

  rawData  = (quint8 *) malloc(MN1LRPT_BLOCK_SIZE * sizeof(quint8)); // holds the undecoded frame
  rsData   = (quint8 *) malloc(MN1LRPT_RS_BLOCK_SIZE * sizeof(quint8)); // holds the rs decoded frame
  scanLine = (quint8 *) malloc(10); // MN1LRPT_SCAN_WIDTH * MN1LRPT_NUM_CHANNELS * sizeof(quint8));

  rs = new CReedSolomon(8,                      // int BitsPerSymbol
                        16,                     // int CorrectableErrors
                        112,                    // int mo
                        11,                     // int poa
                        0,                      // int VirtualFill
                        4,                      // int Interleave
                        MN1LRPT_SYNC_SIZE,      // int FrameSyncLength
                        1                       // int mode, dual
                       );
}

//---------------------------------------------------------------------------
TMN1LRPT::~TMN1LRPT(void)
{
  if(scanLine)
     free(scanLine);
  if(rsData)
     free(rsData);
  if(rawData)
     free(rawData);

  delete rs;
}

//---------------------------------------------------------------------------
bool TMN1LRPT::init(void)
{
 int frames;

  if(block == NULL)
     return false;

  block->setFrames(0);
  block->setFirstFrameSyncPos(-1);
  syncOffset = 0;

  fp = block->getHandle();
  frames = countFrames();

#if 0
//  if(check(1) && fseek(fp, block->getFirstFrameSyncPos() + (MN1LRPT_BLOCK_SIZE * 100), SEEK_SET) == 0) {
  if(check(1) && fseek(fp, 0L, SEEK_SET) == 0) {
     for(int i=0;  i<1000; i++) {
        if(fread(rsData, MN1LRPT_BLOCK_SIZE, 1, fp) == 1) {
           // qDebug("RS decode frame: %d", i);
           if(rs->Decode(rsData))
              qDebug("RS decode OK! Frame: %d", i);
           /*
           else
              qDebug("RS decode failed! Correctable errors: %d, Uncorrectable errors: %d", rs->CorrectableErrorsInFrame(), rs->UncorrectableErrorsInFrame());
              */
        }
     }
  }

#endif


  return check(1);
}

//---------------------------------------------------------------------------
// flags&1 = check found data
bool TMN1LRPT::check(int flags)
{
  // check allocation and file pointer status
  if(block == NULL || fp == NULL || scanLine == NULL ||
     rsData == NULL || rawData == NULL)
     return false;

  // check found stuff
  if(flags&1) {
     if(block->getFrames() <= 0 || block->getFirstFrameSyncPos() < 0 || syncOffset <= 0)
        return false;
  }

  return true;
}

//---------------------------------------------------------------------------
int TMN1LRPT::countFrames(void)
{
 long int firstFrameSyncPos;
 int frames;

  if(block == NULL || fp == NULL)
     return 0; // fatal error

  frames = block->getFrames();
  if(frames > 0) // already done
     return true;

  block->gotoStart();

  firstFrameSyncPos = -1;
  frames            = 0;
  syncOffset        = 0;

  while(findFrameSync()) {
     if(frames == 0)
        firstFrameSyncPos = ftell(fp) - MN1LRPT_SYNC_SIZE;
     else if(frames == 1)
        syncOffset = ftell(fp) - MN1LRPT_SYNC_SIZE - firstFrameSyncPos;

     ++frames;

     // hop to next frame
     if(frames > 1)
        if(fseek(fp, syncOffset - MN1LRPT_SYNC_SIZE, SEEK_CUR) != 0)
           break;
  }

  block->setFrames(frames);
  block->setFirstFrameSyncPos(firstFrameSyncPos);
  qDebug("Block size: %d", syncOffset);

 return frames;
}

//---------------------------------------------------------------------------
bool TMN1LRPT::findFrameSync(void)
{
 quint8 ch;
 int    i;

  i = 0;
  while(fread(&ch, 1, 1, fp) == 1) {
     if(ch == MN1LRPT_SYNC[i])
        i++;
     else
        i = 0;

     if(i == MN1LRPT_SYNC_SIZE)
        return true;
  }

 return false;
}

//---------------------------------------------------------------------------
int TMN1LRPT::getWidth(void)
{
   return MN1LRPT_SCAN_WIDTH;
}

//---------------------------------------------------------------------------
// frame_nr is zero based (0, 1, 2, ... frames - 1)
bool TMN1LRPT::readFrameScanLine(int frame_nr)
{
#if 0
 long int pos, scanPos;
 int x;

  if(!check(1))
     return false;

  pos = ftell(fp);
  if(pos < 0)
     return false;

  scanPos = block->getFirstFrameSyncPos() + ((HRPT_IMAGE_START + frame_nr*HRPT_BLOCK_SIZE) << 1);

  if(pos != scanPos) {
     if(pos > scanPos)
        fseek(fp, scanPos, SEEK_SET);
     else
        fseek(fp, scanPos - pos, SEEK_CUR);
  }

  // todo: implement different packing features
  if(fread(scanLine, HRPT_SCAN_SIZE << 1, 1, fp) != 1)
     return false;

  if(!block->isLittleEndian())
     for(x=0; x < (HRPT_SCAN_WIDTH * HRPT_NUM_CHANNELS); x++)
        SWAP16PTR(&scanLine[x]);

 return true;
#else

 frame_nr = frame_nr;

 return false;
#endif
}

//---------------------------------------------------------------------------
// returns a 8 bit pixel from a frame channel
// sample and channel are zero based
quint8 TMN1LRPT::getPixel(int channel, int sample)
{
#if 0
 quint16 pixel, pos;

  if(!check())
     return 0;

  if(block->isNorthBound())
     pos = channel + sample * HRPT_NUM_CHANNELS;
  else
     pos = channel + (HRPT_SCAN_WIDTH - sample - 1) * HRPT_NUM_CHANNELS;

  pixel = scanLine[pos] & 0x03ff;

  return pixel;
#else
  sample = sample;
  channel = channel;
  return 0;
#endif
}

//---------------------------------------------------------------------------
// fills an 24 bpp image line of the selected channel
// frame_nr is zero based
bool TMN1LRPT::frameToImage(int frame_nr, QImage *image)
{
#if 0
 uchar *imagescan, r, g, b;
 int x, y;

  if(!check(1) || image == NULL)
     return false;

  if(!readFrameScanLine(frame_nr))
     return false;

  if(block->isNorthBound())
     y = image->height() - frame_nr - 1;
  else
     y = frame_nr;

  imagescan = (uchar *) image->scanLine(y);
  if(imagescan == NULL)
     return false;

  for(x=0; x<HRPT_SCAN_WIDTH; x++) {
     switch(block->getImageType()) {
        case Gray_ImageType:
           r = getPixel_8(block->getImageChannel(), x);
           g = r;
           b = r;
        break;

        case RGB_ImageType:
           r = getPixel_8(1, x); // ch 2
           g = getPixel_8(0, x); // ch 1
           b = getPixel_8(3, x); // ch 4
        break;

        default:
           return false;
     }

     *imagescan++ = r;
     *imagescan++ = g;
     *imagescan++ = b;
  }

 return true;
#else
 frame_nr = frame_nr;
 image = image;
 return false;
#endif
}

//---------------------------------------------------------------------------
bool TMN1LRPT::toImage(QImage *image)
{
#if 0
 int frames, y;

  if(!check(1))
     return false;

  block->gotoStart();
  frames = block->getFrames();

  for(y=0; y<frames; y++) {
     if(!frameToImage(y, image))
        break;
  }

 return true;
#else
 image = image;
 return false;
#endif
}

