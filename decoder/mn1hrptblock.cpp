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

#include "mn1hrptblock.h"
#include "block.h"


const int MN1_HRPT_BLOCK_SIZE       = 256;   // size in bytes
const int MN1_HRPT_IMAGE_BLOCK_SIZE = 232;   // size in bytes
const int MN1_HRPT_BLOCKS_PER_SCAN  = 50;
const int MN1_HRPT_NUM_CHANNELS     = 6;
const int MN1_HRPT_CHANNEL_SIZE     = 5;     // in bytes, 40 bits -> 4 10 bit pixels per every channel
const int MN1_HRPT_SCAN_SIZE        = 11600; // 11550 bytes of image starting at byte 50, all 6 channels
const int MN1_HRPT_IMAGE_START      = 22;    // offset bytes from CADU sync start and second sync
const int MN1_HRPT_SCAN_WIDTH       = 1540;

#define MN1_HRPT_SYNC2_SIZE 8
static const quint8 MN1_HRPT_SYNC2[MN1_HRPT_SYNC2_SIZE] = {
  0x02, 0x18, 0xA7, 0xA3,
  0x92, 0xDD, 0x9A, 0xBF
};



//---------------------------------------------------------------------------
TMN1HRPT::TMN1HRPT(TBlock *_block)
{
  block      = _block;

  scanLine   = NULL;
  imgBlock   = NULL;
  fp         = NULL;

  testfp = NULL;

//  testfp = fopen("c:\\tmp\\frames.dat", "wb");

}

//---------------------------------------------------------------------------
TMN1HRPT::~TMN1HRPT(void)
{
  if(scanLine)
     free(scanLine);

  if(imgBlock)
     free(imgBlock);

  if(testfp != NULL)
     fclose(testfp);
}

//---------------------------------------------------------------------------
bool TMN1HRPT::init(void)
{
  if(block == NULL)
     return false;

  if(scanLine == NULL)
     scanLine = (quint8 *) malloc(MN1_HRPT_SCAN_SIZE * sizeof(quint8));
  if(imgBlock == NULL)
     imgBlock = (quint8 *) malloc(MN1_HRPT_IMAGE_BLOCK_SIZE * sizeof(quint8));

  block->setFrames(0);
  block->setFirstFrameSyncPos(-1);

  fp = block->getHandle();

  countFrames();

  return check(1);
}

//---------------------------------------------------------------------------
// flags&1 = check found data
bool TMN1HRPT::check(int flags)
{
  // check allocation and file pointer status
  if(block == NULL || fp == NULL || scanLine == NULL || imgBlock == NULL)
     return false;

  // check found stuff
  if(flags&1) {
     if(block->getFrames() <= 0 || block->getFirstFrameSyncPos() < 0)
        return false;
  }

  return true;
}

//---------------------------------------------------------------------------
long int TMN1HRPT::countFrames(void)
{
 long int frames, sync_pos, pos;

  if(block == NULL || fp == NULL)
     return 0; // fatal error

  frames = block->getFrames();
  if(frames > 0) // already done
     return frames;

  block->gotoStart();

  frames = 0;
  sync_pos = -1;
  block->Modes &= ~B_SYNC_FOUND;

  while(block->findCADUFrameSync()) {
     pos = ftell(fp) - CADU_SYNC_SIZE;
     block->Modes |= B_SYNC_FOUND;

     // set pointer at byte 22 from sync
     if(fseek(fp, MN1_HRPT_IMAGE_START - CADU_SYNC_SIZE, SEEK_CUR) != 0)
        break;

     if(findFrameSync2()) {
        if(frames == 0)
           sync_pos = pos;

        frames++;

        // set pointer at next CADU sync start + 50 frames
        pos  = MN1_HRPT_BLOCK_SIZE - MN1_HRPT_IMAGE_START - MN1_HRPT_SYNC2_SIZE;
        pos += (MN1_HRPT_BLOCK_SIZE * (MN1_HRPT_BLOCKS_PER_SCAN - 1));
     }
     else {
        // check next frame
        pos = MN1_HRPT_BLOCK_SIZE - (ftell(fp) - pos);
     }

     if(fseek(fp, pos, SEEK_CUR) != 0)
        break;

     // pos = ftell(fp);
  }

  block->setFrames(frames);
  block->setFirstFrameSyncPos(sync_pos);

 return frames;
}

//---------------------------------------------------------------------------
bool TMN1HRPT::findFrameSync2(void)
{
 quint8 ch;
 int    i;

  i = 0;
  while(fread(&ch, 1, 1, fp) == 1) {
     if(ch == MN1_HRPT_SYNC2[i])
        i++;
     else
        return false;

     if(i == MN1_HRPT_SYNC2_SIZE)
        return true;
  }

 return false;
}

//---------------------------------------------------------------------------
int TMN1HRPT::getWidth(void)
{
   return MN1_HRPT_SCAN_WIDTH;
}

//---------------------------------------------------------------------------
int TMN1HRPT::getHeight(void)
{
   if(!check(1))
      return 0;

 return block->getFrames();
}

//---------------------------------------------------------------------------
int TMN1HRPT::setImageType(int type)
{
   if((Block_ImageType) type < Gray_ImageType)
      type = (int) Gray_ImageType;
   else if((Block_ImageType) type > RGB_ImageType)
      type = (int) RGB_ImageType;

 return type;
}

//---------------------------------------------------------------------------
// zero based
int TMN1HRPT::setImageChannel(int channel)
{
    if(channel < 0)
        channel = 0;
    else if(channel >= MN1_HRPT_NUM_CHANNELS)
        channel = MN1_HRPT_NUM_CHANNELS - 1;

  return channel;
}

//---------------------------------------------------------------------------
// frame_nr is zero based (0, 1, 2, ... frames - 1)
bool TMN1HRPT::readFrameScan(int frame_nr)
{
 long int pos, scanPos;
 int i;

  if(!check(1))
     return false;

  pos = ftell(fp);
  if(pos < 0)
     return false;

  scanPos = block->getFirstFrameSyncPos() + MN1_HRPT_IMAGE_START + (frame_nr * MN1_HRPT_BLOCK_SIZE * MN1_HRPT_BLOCKS_PER_SCAN);

  if(pos != scanPos) {
     if(pos > scanPos)
        fseek(fp, scanPos, SEEK_SET);
     else
        fseek(fp, scanPos - pos, SEEK_CUR);
  }

  pos = ftell(fp);

  memset(scanLine, 0, sizeof(quint8) * MN1_HRPT_SCAN_SIZE);

  pos = 0;
  for(i=0; i<MN1_HRPT_BLOCKS_PER_SCAN; i++) {
     if(fread(imgBlock, MN1_HRPT_IMAGE_BLOCK_SIZE, 1, fp) != 1)
        break;

     memcpy(scanLine + pos, imgBlock, MN1_HRPT_IMAGE_BLOCK_SIZE);
     pos += MN1_HRPT_IMAGE_BLOCK_SIZE;

     // we are now at bytepos 254 from CADU sync
     // hop to byte 22 in the next CADU

     if(fseek(fp, MN1_HRPT_IMAGE_START + 2, SEEK_CUR) != 0)
        break;
  }

  if(testfp != NULL)
     if((signed) fwrite(scanLine, 1, MN1_HRPT_SCAN_SIZE, testfp) != MN1_HRPT_SCAN_SIZE)
        return false;

 return i > 0 ? true:false;
}

//---------------------------------------------------------------------------
// returns a 16 bit pixel from a frame channel
// sample (0-3) and channel are zero based

/*

         1            2            3            4            5       byte index
    00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  8 x 5 bit in

    11 11 11 11  11 22 22 22  22 22 33 33  33 33 33 44  44 44 44 44  10 x 4 bit out
*/

quint16 TMN1HRPT::getPixel_16(int channel, int sample)
{
 quint16 pixel_16, tmp16;
 int     pos, shift;

  if(!check())
     return 0;

  pos = channel * MN1_HRPT_CHANNEL_SIZE + sample;

  // XXXX XXXX YYYY YYYY
  pixel_16 = (imgBlock[pos] << 8) | imgBlock[pos + 1];
  // 0000 00XX XXXX XXYY
  shift = 6 - (sample * 2);
  pixel_16 >>= shift;

  tmp16 = pixel_16 & 0x03ff;
  pixel_16 = tmp16;

#if 0

  if(block->isNorthBound())
     pos = channel + sample * HRPT_NUM_CHANNELS;
  else
     pos = channel + (HRPT_SCAN_WIDTH - sample - 1) * HRPT_NUM_CHANNELS;

  pixel = scanLine[pos] & 0x03ff;

#endif

  return pixel_16;
}

//---------------------------------------------------------------------------
// returns a 8 bit pixel from a frame channel
// sample (0-3) and channel are zero based
quint8 TMN1HRPT::getPixel_8(int channel, int sample)
{
  return SCALE16TO8(getPixel_16(channel, sample));
}

//---------------------------------------------------------------------------
// fills an 24 bpp image line of the selected channel
// scan_nr is zero based
bool TMN1HRPT::scanToImage(int scan_nr, QImage *image)
{
 Block_ImageType it;
 uchar *imagescan;
 quint8 r, g, b;
 long int pos;
 int i, j, y, ch, width;

 if(!readFrameScan(scan_nr))
    return false;

 if(block->isNorthBound())
    y = image->height() - scan_nr - 1;
 else
    y = scan_nr;

  imagescan = (uchar *) image->scanLine(y);
  if(imagescan == NULL)
     return false;

  it = block->getImageType();
  ch = block->getImageChannel();

  // width / 4 = 385 samples needed to produce one scanline
  width = (MN1_HRPT_SCAN_WIDTH >> 2);

  pos = 50;
  for(i=0; i<width; i++) {
     // copy all channels 5 bytes * 6 ch
     memcpy(imgBlock, scanLine + pos, MN1_HRPT_CHANNEL_SIZE * MN1_HRPT_NUM_CHANNELS);

     for(j=0; j<4; j++) {
        switch(it) {
           case Gray_ImageType:
              r = getPixel_8(ch, j);
              g = r;
              b = r;
           break;

           case RGB_ImageType:
              r = getPixel_8(2, j); // ch 3
              g = getPixel_8(1, j); // ch 2
              b = getPixel_8(0, j); // ch 1
           break;

           default:
              return false;
        }

        *imagescan++ = r;
        *imagescan++ = g;
        *imagescan++ = b;
     }

     pos += MN1_HRPT_CHANNEL_SIZE * MN1_HRPT_NUM_CHANNELS;
  }

 return true;
}

//---------------------------------------------------------------------------
bool TMN1HRPT::toImage(QImage *image)
{
 long int frames, y;

  if(!check(1) || image == NULL)
     return false;

  block->gotoStart();
  frames = block->getFrames();

  for(y=0; y<frames; y++) {
     if(!scanToImage(y, image))
        break;
  }

 return y > 0 ? true:false;
}

