/*
    HRPT-Decoder, a software for processing NOAA-POES hig resolution weather satellite images.
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
#include <QString>
#include "block.h"
#include "hrptblock.h"
#include "ahrptblock.h"
#include "mn1lrptblock.h"
#include "mn1hrptblock.h"
#include "fy1hrptblock.h"
#include "lritblock.h"

#if 0
static const quint8 CADU_SYNC[CADU_SYNC_SIZE] = {
  0x1A, 0xCF, 0xFC, 0x1D,
};
#endif

static const char *SUPPORTED_BLOCKS[NUM_SUPPORTED_BLOCKS] =
{
   "NOAA HRPT",
   "METEOR M-N1 LRPT",
   "METEOR M-N1 HRPT",
   "FENGYUN 1 HRPT",
   "MetOp AHRPT",
   "NOAA LRPT",
   "GOES LRIT/HRIT",
   "JPEG LRIT/HRIT"
};

//---------------------------------------------------------------------------
TBlock::TBlock(void)
{    
   fp = NULL;
   block = NULL;

   frames = 0;
   firstFrameSyncPos = -1;

   blocktype = Undefined_BlockType;
   imagetype = Gray_ImageType;
   imageChannel = 0; // zero based

   cadu = new TCADU;
}

//---------------------------------------------------------------------------
TBlock::~TBlock(void)
{
    close();
    freeBlock();

    delete cadu;
}

//---------------------------------------------------------------------------
void TBlock::freeBlock(void)
{
   if(!block)
      return;

   switch(blocktype) {
       case HRPT_BlockType:
          delete ((THRPT *) block);
       break;

       case AHRPT_BlockType:
          delete ((TAHRPT *) block);
       break;

       case MN1LRPT_BlockType:
          delete ((TMN1LRPT *) block);
       break;

       case MN1HRPT_BlockType:
          delete ((TMN1HRPT *) block);
       break;

       case FY1HRPT_BlockType:
          delete ((TFY1HRPT *) block);
       break;

       case LRIT_GOES_BlockType:
       case LRIT_JPEG_BlockType:
          delete ((TLRIT *) block);
       break;

       default:
       {
           // corrupt!!!
           block = NULL;
       }
   }


   block = NULL;
}

//---------------------------------------------------------------------------
void TBlock::close(void)
{
   if(fp)
      fclose(fp);
   fp = NULL;
}

//---------------------------------------------------------------------------
void TBlock::gotoStart(void)
{
   if(fp)
      fseek(fp, 0L, SEEK_SET);
}

//---------------------------------------------------------------------------
// flags&1 = filter format
QString TBlock::getBlockTypeStr(int index, int flags)
{
 QString str;

   if(index < 0 || index >= NUM_SUPPORTED_BLOCKS)
      str = "Unknown block type";
   else
      str.sprintf("%s%s", SUPPORTED_BLOCKS[index], flags&1 ? " (*)":"");

  return str;
}

//---------------------------------------------------------------------------
void TBlock::setMode(bool on, int flag)
{
   if(on)
      Modes |= flag;
   else
      Modes &= ~flag;
}

//---------------------------------------------------------------------------
void TBlock::setNorthBound(bool on)
{
   setMode(on, B_NORTHBOUND);
}

//---------------------------------------------------------------------------
void TBlock::setLittleEndian(bool on)
{
   setMode(on, B_BYTESWAP);
}

//---------------------------------------------------------------------------
bool TBlock::setBlockType(Block_Type type)
{
   if(blocktype != type)
       freeBlock();

   if(block)
       return true;

   cadu->reset();

   switch(type)
   {
       case HRPT_BlockType:
          block = (THRPT *) new THRPT(this);
       break;

       case AHRPT_BlockType:
       {
           // TODO
#if 1
           cadu->derandomize(true);
           cadu->reed_solomon(false);
#endif
           block = (TAHRPT *) new TAHRPT(this);

           break;
       }

       case MN1LRPT_BlockType:
          block = (TMN1LRPT *) new TMN1LRPT(this);
       break;

       case MN1HRPT_BlockType:
          block = (TMN1HRPT *) new TMN1HRPT(this);
       break;

       case FY1HRPT_BlockType:
          block = (TFY1HRPT *) new TFY1HRPT(this);
       break;

       case LRIT_GOES_BlockType:
       case LRIT_JPEG_BlockType:
          block = (TLRIT *) new TLRIT(this);
       break;

       default:
          return false;
   }

   if(block)
       blocktype = type;

 return block != NULL ? true:false;
}

//---------------------------------------------------------------------------
// setBlockType must be called before this function
bool TBlock::open(const char *filename)
{
   if(block == NULL || filename == NULL)
       return false;

   close();

   fp = fopen(filename, "rb");
   if(fp == NULL)
       return false;

   switch(blocktype) {
       case HRPT_BlockType:
          return ((THRPT *) block)->init();
       break;

       case AHRPT_BlockType:
          return ((TAHRPT *) block)->init();
       break;

       case MN1HRPT_BlockType:
          return ((TMN1HRPT *) block)->init();
       break;

       case MN1LRPT_BlockType:
          return ((TMN1LRPT *) block)->init();
       break;

       case FY1HRPT_BlockType:
          return ((TFY1HRPT *) block)->init();
       break;

       case LRIT_GOES_BlockType:
       case LRIT_JPEG_BlockType:
          return ((TLRIT *) block)->init();
       break;

       default:
          return false;
   }
}

//---------------------------------------------------------------------------
int TBlock::getWidth(void)
{
   if(!block)
      return 0;

   switch(blocktype) {
       case HRPT_BlockType:
          return ((THRPT *) block)->getWidth();
       break;

       case AHRPT_BlockType:
          return ((TAHRPT *) block)->getWidth();
       break;

       case MN1HRPT_BlockType:
          return ((TMN1HRPT *) block)->getWidth();
       break;

       case MN1LRPT_BlockType:
          return ((TMN1LRPT *) block)->getWidth();
       break;

       case FY1HRPT_BlockType:
          return ((TFY1HRPT *) block)->getWidth();
       break;

       case LRIT_GOES_BlockType:
       case LRIT_JPEG_BlockType:
          return ((TLRIT *) block)->getWidth();
       break;

       default:
          return 0;
   }
}

//---------------------------------------------------------------------------
int TBlock::getHeight(void)
{
   if(!block)
      return 0;

   switch(blocktype) {
       case HRPT_BlockType:
       case AHRPT_BlockType:
       case FY1HRPT_BlockType:
       case MN1LRPT_BlockType:
          return frames;
       break;

       case MN1HRPT_BlockType:
          return ((TMN1HRPT *) block)->getHeight();
       break;

       case LRIT_GOES_BlockType:
       case LRIT_JPEG_BlockType:
          return ((TLRIT *) block)->getHeight();
       break;

       default:
          return 0;
   }
}

//---------------------------------------------------------------------------
long int TBlock::countCADUFrames(long int block_size)
{
   if(!block || !fp)
      return 0;

   gotoStart();
   firstFrameSyncPos = -1;
   frames            = 0;
   Modes            &= ~B_SYNC_FOUND;

   while(findCADUFrameSync()) {
      Modes |= B_SYNC_FOUND;

      if(frames == 0)
         firstFrameSyncPos = ftell(fp) - CADU_SYNC_SIZE;

      ++frames;

      // hop to next frame
      if(frames > 1)
         if(fseek(fp, block_size - CADU_SYNC_SIZE, SEEK_CUR) != 0)
            break;
   }

 return frames;
}

//---------------------------------------------------------------------------
bool TBlock::findCADUFrameSync(void)
{
 quint8 ch;
 int    i;

 i = 0;
 if(Modes & B_SYNC_FOUND) {
     // dummy read
     while(fread(&ch, 1, 1, fp) == 1) {
        i++;

        if(i == CADU_SYNC_SIZE)
           return true;
     }

     return false;
 }

  while(fread(&ch, 1, 1, fp) == 1) {
     if(ch == CADU_SYNC[i])
        i++;
     else
        i = 0;

     if(i == CADU_SYNC_SIZE)
        return true;
  }

 return false;
}

//---------------------------------------------------------------------------
bool TBlock::isCompressed(void)
{
   if(!block)
      return false;

   switch(blocktype) {
       case AHRPT_BlockType:
       case HRPT_BlockType:
       case FY1HRPT_BlockType:
       case MN1LRPT_BlockType:
          return false;
       break;

       case LRIT_GOES_BlockType:
       case LRIT_JPEG_BlockType:
          return ((TLRIT *) block)->isCompressed();
       break;

       default:
          return false;
    }
}

//---------------------------------------------------------------------------
bool TBlock::uncompress(const char *filename)
{
   if(!block || filename == NULL)
      return false;

   switch(blocktype) {
       case AHRPT_BlockType:
       case HRPT_BlockType:
       case FY1HRPT_BlockType:
       case MN1LRPT_BlockType:
          return false;
       break;

       case LRIT_GOES_BlockType:
       case LRIT_JPEG_BlockType:
          return false; //((TLRIT *) block)->uncompress(filename);
       break;

       default:
          return false;
    }
}

//---------------------------------------------------------------------------
void TBlock::setImageChannel(int channel)
{
   if(!block)
      return;

   switch(blocktype) {
      case HRPT_BlockType:
         imageChannel = ((THRPT *) block)->setImageChannel(channel);
      break;

      case AHRPT_BlockType:
         imageChannel = ((TAHRPT *) block)->setImageChannel(channel);
      break;

      case MN1HRPT_BlockType:
         imageChannel = ((TMN1HRPT *) block)->setImageChannel(channel);
      break;

      case FY1HRPT_BlockType:
         imageChannel = ((TFY1HRPT *) block)->setImageChannel(channel);
      break;


      case MN1LRPT_BlockType:
         imageChannel = ((TMN1LRPT *) block)->setImageChannel(channel);
      break;

      default:
         return;
   }
}

//---------------------------------------------------------------------------
int TBlock::getNumChannels(void)
{
    int channels;

    if(!block)
       return 0;

    switch(blocktype) {
       case HRPT_BlockType:
          channels = ((THRPT *) block)->getNumChannels();
       break;

       case AHRPT_BlockType:
          channels = ((TAHRPT *) block)->getNumChannels();
       break;
#if 0
       case MN1HRPT_BlockType:
          channels = ((TMN1HRPT *) block)->getNumChannels();
       break;
#endif
       case FY1HRPT_BlockType:
          channels = ((TFY1HRPT *) block)->getNumChannels();
       break;

       default:
          channels = 0;
    }

    return channels;
}

//---------------------------------------------------------------------------
void TBlock::setImageType(Block_ImageType type)
{
   if(!block)
      return;

   switch(blocktype) {
      case HRPT_BlockType:
         imagetype = (Block_ImageType) ((THRPT *) block)->setImageType((int) type);
      break;

      case AHRPT_BlockType:
         imagetype = (Block_ImageType) ((TAHRPT *) block)->setImageType((int) type);
      break;

      case MN1HRPT_BlockType:
         imagetype = (Block_ImageType) ((TMN1HRPT *) block)->setImageType((int) type);
      break;

      case FY1HRPT_BlockType:
         imagetype = (Block_ImageType) ((TFY1HRPT *) block)->setImageType((int) type);
      break;

      case MN1LRPT_BlockType:
      case LRIT_JPEG_BlockType:
         imagetype = (Block_ImageType) ((TMN1LRPT *) block)->setImageType((int) type);
      break;

      default:
         return;
   }
}

//---------------------------------------------------------------------------
bool TBlock::toImage(QImage *image)
{
   if(!block || !image)
      return false;

   switch(blocktype) {
      case HRPT_BlockType:
         return ((THRPT *) block)->toImage(image);
      break;

      case AHRPT_BlockType:
         return ((TAHRPT *) block)->toImage(image);
      break;

      case MN1HRPT_BlockType:
         return ((TMN1HRPT *) block)->toImage(image);
      break;

      case FY1HRPT_BlockType:
         return ((TFY1HRPT *) block)->toImage(image);
      break;

      case MN1LRPT_BlockType:
         return ((TMN1LRPT *) block)->toImage(image);
      break;

      case LRIT_GOES_BlockType:
      case LRIT_JPEG_BlockType:
         return ((TLRIT *) block)->toImage(image);
      break;

      default:
         return false;
   }
}
//---------------------------------------------------------------------------
