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

#ifndef fy1hrptblockH
#define fy1hrptblockH

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

//---------------------------------------------------------------------------

class QImage;
class TBlock;

//---------------------------------------------------------------------------
class TFY1HRPT
{
 public:
    TFY1HRPT(TBlock *_block);
    ~TFY1HRPT(void);

    bool init(void);
    int  countFrames(void);
    int  getWidth(void);

    int  getNumChannels(void);

    bool readFrameScanLine(int frame_nr);
    bool frameToImage(int frame_nr, QImage *image);
    bool toImage(QImage *image);

    quint16 getPixel_16(int channel, int sample);
    quint8  getPixel_8(int channel, int sample);

    int Modes;

 protected:
    bool check(int flags=0);

    bool findFrameSync(void);
    bool sync_found;

 private:
    TBlock  *block;
    FILE    *fp;

    quint16 *scanLine;
};

//---------------------------------------------------------------------------
#endif
