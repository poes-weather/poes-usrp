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
#include "fy1hrptblock.h"
#include "block.h"

//---------------------------------------------------------------------------
/*

 Feng-Yun 1C/D
 http://www.sat.dundee.ac.uk/hrptformat.html

 Supported satellites: Feng-Yun 1C and Feng-Yun 1D

Band No         Spectral Range (µm)     Primary use of data

 1               0.58 - 0.68 (VIS)       Daytime clouds, ice and snow, vegetation
 2               0.84 - 0.89 (VNIR)      Daytime clouds, vegetation, water
 3               3.55 - 3.93 (MWIR)      Heat source, night cloud
 4               10.3 - 11.3 (TIR)       SST, day/night clouds
 5               11.5 - 12.5 (TIR)       SST, day/night clouds
 6               1.58 - 1.64 (SWIR)      Soil humidity, provision of ice/snow cover distinguishing capability
 7               0.43 - 0.48 (VIS)       Ocean color
 8               0.48 - 0.53 (VIS)       Ocean color
 9               0.53 - 0.58 (VIS)       Ocean color
10               0.90 - 0.965 (VNIR)     Water vapor

*/

//---------------------------------------------------------------------------

const int FY1_HRPT_BLOCK_SIZE   = 22180; // words
const int FY1_HRPT_NUM_CHANNELS = 10;
const int FY1_HRPT_SCAN_WIDTH   = 2048;  // words, one image scan
const int FY1_HRPT_SCAN_SIZE    = 20480; // words, width * channels
const int FY1_HRPT_IMAGE_START  = 1600;  // offset words from frame sync

#define FY1_HRPT_SYNC_SIZE 6
static const quint16 FY1_HRPT_SYNC[FY1_HRPT_SYNC_SIZE] = {
  0x0284, 0x016F, 0x035C, 0x019D, 0x020F, 0x0095
};

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
TFY1HRPT::TFY1HRPT(TBlock *_block)
{
  block = _block;

  sync_found = false;

  scanLine = NULL;
  fp = NULL;
}

//---------------------------------------------------------------------------
TFY1HRPT::~TFY1HRPT(void)
{
  if(scanLine)
     free(scanLine);
}

//---------------------------------------------------------------------------
bool TFY1HRPT::init(void)
{
  if(block == NULL)
     return false;

  block->setFrames(0);
  block->setFirstFrameSyncPos(-1);
  block->setLittleEndian(true); // USRP default format

  if(scanLine == NULL)
     scanLine = (quint16 *) malloc(FY1_HRPT_SCAN_SIZE << 1); // 20480 bytes

  fp = block->getHandle();
  if(countFrames() <= 0) {
     // retry using different endian
     block->setLittleEndian(!block->isLittleEndian());
     countFrames();
  }

  return check(1);
}

//---------------------------------------------------------------------------
// flags&1 = check found data
bool TFY1HRPT::check(int flags)
{
  // check allocation and file pointer status
  if(block == NULL || fp == NULL || scanLine == NULL)
     return false;

  // check found stuff
  if(flags&1) {
     if(block->getFrames() <= 0 || block->getFirstFrameSyncPos() < 0)
        return false;
  }

  return true;
}

//---------------------------------------------------------------------------
int TFY1HRPT::countFrames(void)
{
 long int syncSize, firstFrameSyncPos;
 int frames;

  if(!check())
     return 0; // fatal error

  if(check(1)) // already done
     return block->getFrames();

  block->gotoStart();

  syncSize = FY1_HRPT_SYNC_SIZE << 1; // 12 bytes
  firstFrameSyncPos = -1;
  frames = 0;
  sync_found = false;

  while(findFrameSync()) {
     // we just read 6 words, FY1_HRPT_SYNC_SIZE
     if(frames == 0)
        firstFrameSyncPos = ftell(fp) - syncSize;

     ++frames;

     // hop to next frame
     if(fseek(fp, (FY1_HRPT_BLOCK_SIZE << 1) - syncSize, SEEK_CUR) != 0)
        break;

     sync_found = true;
  }

  block->setFrames(frames);
  block->setFirstFrameSyncPos(firstFrameSyncPos);

 return frames;
}

//---------------------------------------------------------------------------
bool TFY1HRPT::findFrameSync(void)
{
 quint8  ch[2];
 quint16 w;
 int i;

#if 0

 // flush the sync and continue
 if(sync_found) {
     i = 0;
     while(fread(ch, sizeof(ch), 1, fp) == 1) {
         i++;

         if(i == FY1_HRPT_SYNC_SIZE)
            return true;
     }

     return false;

 }

#endif

  i = 0;
  while(fread(ch, sizeof(ch), 1, fp) == 1) {
     if(block->isLittleEndian())
        w = (ch[1] << 8) | ch[0];
     else
        w = (ch[0] << 8) | ch[1];
        
     if(w == FY1_HRPT_SYNC[i])
        i++;
     else
        i = 0;

     if(i == FY1_HRPT_SYNC_SIZE)
        return true;
  }

 return false;
}

//---------------------------------------------------------------------------
int TFY1HRPT::getWidth(void)
{
   return FY1_HRPT_SCAN_WIDTH;
}

//---------------------------------------------------------------------------
int TFY1HRPT::getNumChannels(void)
{
    return FY1_HRPT_NUM_CHANNELS;
}

//---------------------------------------------------------------------------
// frame_nr is zero based (0, 1, 2, ... frames - 1)
bool TFY1HRPT::readFrameScanLine(int frame_nr)
{
 long int pos, scanPos;
 int x;

  if(!check(1))
     return false;

  pos = ftell(fp);
  if(pos < 0)
     return false;

  scanPos = block->getFirstFrameSyncPos() + ((FY1_HRPT_IMAGE_START + frame_nr*FY1_HRPT_BLOCK_SIZE) << 1);

  if(pos != scanPos) {
     if(pos > scanPos)
        fseek(fp, scanPos, SEEK_SET);
     else
        fseek(fp, scanPos - pos, SEEK_CUR);
  }

  // todo: implement different packing features
  if(fread(scanLine, FY1_HRPT_SCAN_SIZE << 1, 1, fp) != 1)
     return false;

  if(!block->isLittleEndian())
     for(x=0; x < (FY1_HRPT_SCAN_WIDTH * FY1_HRPT_NUM_CHANNELS); x++)
        SWAP16PTR(&scanLine[x]);

 return true;
}

//---------------------------------------------------------------------------
// returns a 16 bit pixel from a frame channel
// sample and channel are zero based
quint16 TFY1HRPT::getPixel_16(int channel, int sample)
{
 quint16 pixel, pos;

  if(!check())
     return 0;

  if(!block->isNorthBound())
     pos = channel + sample * FY1_HRPT_NUM_CHANNELS;
  else
     pos = channel + (FY1_HRPT_SCAN_WIDTH - sample - 1) * FY1_HRPT_NUM_CHANNELS;

  pixel = scanLine[pos] & 0x03ff;

  return pixel;
}

//---------------------------------------------------------------------------
// returns a 8 bit pixel from a frame channel
// sample and channel are zero based
quint8 TFY1HRPT::getPixel_8(int channel, int sample)
{
  return SCALE16TO8(getPixel_16(channel, sample));
}

//---------------------------------------------------------------------------
// fills an 24 bpp image line of the selected channel
// frame_nr is zero based
bool TFY1HRPT::frameToImage(int frame_nr, QImage *image)
{
 uchar *imagescan, r, g, b;
 int x, y, *ch_rgb;

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

  switch(block->getImageType()) {
  case RGB_ImageType:
      ch_rgb = block->rgbconf->rgb_ch();
      break;

  default:
      break;
  }

  for(x=0; x<FY1_HRPT_SCAN_WIDTH; x++) {
      switch(block->getImageType()) {
      case Channel_ImageType:
          r = getPixel_8(block->getImageChannel(), x);
          g = r;
          b = r;
          break;

      case RGB_ImageType:
          r = getPixel_8(ch_rgb[0] - 1, x);
          g = getPixel_8(ch_rgb[1] - 1, x);
          b = getPixel_8(ch_rgb[2] - 1, x);
          break;

       default:
          return false;
     }

     *imagescan++ = r;
     *imagescan++ = g;
     *imagescan++ = b;
  }

 return true;
}

//---------------------------------------------------------------------------
bool TFY1HRPT::toImage(QImage *image)
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

 return true;
}
