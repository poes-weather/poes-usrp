/*
    HRPT-Decoder, a software for processing NOAA-POES high resolution weather satellite images.
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

    ------------------------------------------------------------------------
    Lossless JPEG decompression derived from work by:

    Copyright (C) 1991, 1992, Thomas G. Lane.
    Part of the Independent JPEG Group's software.
    See the file Copyright for more details.

    Copyright (c) 1993 Brian C. Smith, The Regents of the University
    of California
    All rights reserved.

    Copyright (c) 1994 Kongji Huang and Brian C. Smith.
    Cornell University
    All rights reserved.
 */
//---------------------------------------------------------------------------
#ifndef LJPEGREADER_H
#define LJPEGREADER_H

#include <QtGlobal>
#include <stdio.h>
#include <stdlib.h>
#include "ljpeg.h"


//---------------------------------------------------------------------------
class TLJPEGDecompressor;

class TLJPEGReader
{
 public:
    TLJPEGReader(TLJPEGDecompressor *_dc, FILE *_fp);
    ~TLJPEGReader(void);

    bool readHeader(void);

    bool readbyte(quint8 *value);
    bool readword(quint16 *value);
    bool readbytes(quint8 *buff, size_t bytes, size_t maxlen = LJPEG_BUF_SIZE);

protected:
    bool       readMarkers(void);
    JPEGMarker readNextMarker(void);
    void       skipMarker(void);
    void       reset(void);

    bool readSOF(JPEGMarker marker);
    bool readSOS(void);
    bool readDHT(void);
    bool readDRI(void);

 private:
    TLJPEGDecompressor *dc;
    FILE *fp;
    quint8 *ljpegbuff;

    long int headerSize;

};

#endif // LJPEGREADER_H
