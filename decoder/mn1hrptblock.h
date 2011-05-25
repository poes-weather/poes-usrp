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
#ifndef MN1_HRPT_BLOCK_H
#define MN1_HRPT_BLOCK_H

#include <QtGlobal>
#include <stdio.h>

//---------------------------------------------------------------------------
class QImage;
class TBlock;

//---------------------------------------------------------------------------
class TMN1HRPT
{
public:
    TMN1HRPT(TBlock *_block);
    ~TMN1HRPT(void);

    bool init(void);
    long int countFrames(void);
    int  getWidth(void);
    int  getHeight(void);

    int  setImageType(int type);
    int  setImageChannel(int channel);
    int  getNumChannels(void);

    bool readFrameScan(int frame_nr);
    bool scanToImage(int scan_nr, QImage *image);
    bool toImage(QImage *image);

    int Modes;

 protected:
    bool check(int flags=0);
    bool findFrameSync(void);
    bool findFrameSync2(void);

    quint16 getPixel_16(int channel, int sample);
    quint8  getPixel_8(int channel, int sample);

 private:
    TBlock  *block;
    FILE    *fp, *testfp;

    quint8 *scanLine, *imgBlock;
};

//---------------------------------------------------------------------------
#endif // MN1_HRPT_BLOCK_H
