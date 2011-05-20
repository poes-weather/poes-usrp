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
#ifndef MN1LRPTBLOCK_H
#define MN1LRPTBLOCK_H

#include <QtGlobal>
#include <stdio.h>

//---------------------------------------------------------------------------
class QImage;
class TBlock;
class CReedSolomon;

//---------------------------------------------------------------------------
class TMN1LRPT
{
public:
    TMN1LRPT(TBlock *_block);
    ~TMN1LRPT(void);

    bool init(void);
    int  countFrames(void);
    int  getWidth(void);

    int  setImageType(int type);
    int  setImageChannel(int channel);

    bool readFrameScanLine(int frame_nr);
    bool frameToImage(int frame_nr, QImage *image);
    bool toImage(QImage *image);

    quint8 getPixel(int channel, int sample);

    int Modes;

 protected:
    bool check(int flags=0);
    bool findFrameSync(void);

 private:
    TBlock  *block;
    int     syncOffset;
    FILE    *fp;

    quint8 *scanLine, *rsData, *rawData;
    CReedSolomon *rs;
};

//---------------------------------------------------------------------------
#endif // MN1LRPTBLOCK_H
