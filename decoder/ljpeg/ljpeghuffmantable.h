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
#ifndef HUFFMANTABLE_H
#define HUFFMANTABLE_H

#include <QtGlobal>
#include "ljpeg.h"


class THuffmanTable
{
public:
   THuffmanTable(void);
   THuffmanTable(quint8 _id, quint8 _table_type);
   ~THuffmanTable(void);

   bool init(void);

   quint8 *bits, *huffval;

   quint8 id;
   quint8 table_type; // 0 = DC, 1 = AC entropy table

protected:
   void reset(void);
   bool generatetables(void);

private:
   quint16 *mincode;
   int     *maxcode;
   short   *valptr;
   int     *numbits, *value;

   bool    inited;

};

#endif // HUFFMANTABLE_H
