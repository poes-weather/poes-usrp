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
#include "hrptblock.h"
#include "block.h"

//---------------------------------------------------------------------------
/*

 NOAA KLMN and Prime after year 2005
 http://www.ncdc.noaa.gov/oa/pod-guide/ncdc/docs/klm/html/c4/sec4-1.htm

 Supported satellites: NOAA-15, 16, 17, 18 and 19

 Channel         Center wavelength (µm)      Spectral Range FWHM (µm)            Typical use
  1                0.63  (VIS)                  0.58  -  0.68                    Daytime cloud and surface mapping
  2                0.862 (VNIR)                 0.725 -  1.00                    Land-water boundaries
  3a               1.61  (NIR)                  1.58  -  1.64 (day selectable)   Snow and ice detection
  3b               3.74  (IR-window)            3.55  -  3.93 (night selectable) Night cloud mapping, sea surface temperature
  4               10.80  (IR-window)           10.30  - 11.30                    Night cloud mapping, sea surface temperature
  5               12.00  (IR window)           11.50  - 12.50                    Sea surface temperature

 */
//---------------------------------------------------------------------------

const int HRPT_BLOCK_SIZE   = 11090; // words
const int HRPT_NUM_CHANNELS = 5;
const int HRPT_SCAN_WIDTH   = 2048;  // words, one image scan
const int HRPT_SCAN_SIZE    = 10240; // words, width * channels
const int HRPT_IMAGE_START  = 750;   // offset words from frame sync

#define HRPT_SYNC_SIZE 6
static const quint16 HRPT_SYNC[HRPT_SYNC_SIZE] = {
  0x0284, 0x016F, 0x035C, 0x019D, 0x020F, 0x0095
};

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
THRPT::THRPT(TBlock *_block)
{
  block = _block;

  sync_found = false;

  datatype = UNPACKED16BIT;
  scanLine = NULL;
  fp = NULL;
}

//---------------------------------------------------------------------------
THRPT::~THRPT(void)
{
  if(scanLine)
     free(scanLine);
}

//---------------------------------------------------------------------------
bool THRPT::init(void)
{
  if(block == NULL)
     return false;

  block->setFrames(0);
  block->setFirstFrameSyncPos(-1);
  block->setLittleEndian(true); // USRP default format

  if(scanLine == NULL)
     scanLine = (quint16 *) malloc(HRPT_SCAN_SIZE << 1); // 20480 bytes

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
bool THRPT::check(int flags)
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
int THRPT::countFrames(void)
{
 long int syncSize, firstFrameSyncPos;
 int frames;

  if(!check())
     return 0; // fatal error

  if(check(1)) // already done
     return block->getFrames();

  block->gotoStart();

  syncSize = HRPT_SYNC_SIZE << 1; // 12 bytes
  firstFrameSyncPos = -1;
  frames = 0;
  sync_found = false;

  while(findFrameSync()) {
     // we just read 6 words, HRPT_SYNC_SIZE
     if(frames == 0)
        firstFrameSyncPos = ftell(fp) - syncSize;

     ++frames;

     // hop to next frame
     if(fseek(fp, (HRPT_BLOCK_SIZE << 1) - syncSize, SEEK_CUR) != 0)
        break;

     sync_found = true;
  }

  block->setFrames(frames);
  block->setFirstFrameSyncPos(firstFrameSyncPos);

 return frames;
}

//---------------------------------------------------------------------------
bool THRPT::findFrameSync(void)
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

         if(i == HRPT_SYNC_SIZE)
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
        
     if(w == HRPT_SYNC[i])
        i++;
     else
        i = 0;

     if(i == HRPT_SYNC_SIZE)
        return true;
  }

 return false;
}

//---------------------------------------------------------------------------
int THRPT::getWidth(void)
{
   return HRPT_SCAN_WIDTH;
}

//---------------------------------------------------------------------------
int THRPT::setImageType(int type)
{
   if((Block_ImageType) type < Gray_ImageType)
      type = (int) Gray_ImageType;
   else if((Block_ImageType) type > RGB_ImageType)
      type = (int) RGB_ImageType;

 return type;
}

//---------------------------------------------------------------------------
// zero based
int THRPT::setImageChannel(int channel)
{
    if(channel < 0)
        channel = 0;
    else if(channel >= HRPT_NUM_CHANNELS)
        channel = HRPT_NUM_CHANNELS - 1;

  return channel;
}

//---------------------------------------------------------------------------
int THRPT::getNumChannels(void)
{
    return HRPT_NUM_CHANNELS;
}

//---------------------------------------------------------------------------
// frame_nr is zero based (0, 1, 2, ... frames - 1)
bool THRPT::readFrameScanLine(int frame_nr)
{
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
}

//---------------------------------------------------------------------------
// returns a 16 bit pixel from a frame channel
// sample and channel are zero based
quint16 THRPT::getPixel_16(int channel, int sample)
{
 quint16 pixel, pos;

  if(!check())
     return 0;

  if(!block->isNorthBound())
     pos = channel + sample * HRPT_NUM_CHANNELS; // left to right
  else
     pos = channel + (HRPT_SCAN_WIDTH - sample - 1) * HRPT_NUM_CHANNELS; // right to left

  pixel = scanLine[pos] & 0x03ff;

  return pixel;
}

//---------------------------------------------------------------------------
// returns a 8 bit pixel from a frame channel
// sample and channel are zero based
quint8 THRPT::getPixel_8(int channel, int sample)
{
  return SCALE16TO8(getPixel_16(channel, sample));
}

//---------------------------------------------------------------------------
// fills an 24 bpp image line of the selected channel
// frame_nr is zero based
bool THRPT::frameToImage(int frame_nr, QImage *image)
{
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
}

//---------------------------------------------------------------------------
bool THRPT::toImage(QImage *image)
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
