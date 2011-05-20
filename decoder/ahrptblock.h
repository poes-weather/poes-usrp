/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite imagery.
    Copyright (C) 2010,2011 Free Software Foundation, Inc.

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

#ifndef AHRPTBLOCK_H
#define AHRPTBLOCK_H


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
class TCADU;


//---------------------------------------------------------------------------
class TAHRPT
{
 public:
    TAHRPT(TBlock *_block);
    ~TAHRPT(void);

    bool init(void);
    int  countFrames(void);
    int  getWidth(void);

    int  setImageType(int type);
    int  setImageChannel(int channel);

    bool readFrameScanLine(int frame_nr);
    bool frameToImage(int frame_nr, QImage *image);
    bool toImage(QImage *image);

    quint16 getPixel_16(int channel, int sample);
    quint8  getPixel_8(int channel, int sample);

    int Modes;

 protected:
    bool check(int flags=0);
    bool findFrameSync(void);
    long count_AVHRR_HR_frames(void);

#if 0
    const char *spacecraftname(quint8 scid);
    const char *vcidTypeStr(quint8 vcid);
    const char *apidTypeStr(quint16 apid);
#endif

 private:
    TBlock  *block;
    FILE    *fp;

    TCADU   *cadu;
    quint16 *scanLine;
};

//---------------------------------------------------------------------------
#endif // AHRPTBLOCK_H
