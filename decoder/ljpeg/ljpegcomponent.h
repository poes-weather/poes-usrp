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
#ifndef LJPEGCOMPONENT_H
#define LJPEGCOMPONENT_H

#include <QtGlobal>

#define NUM_HUFF_TBLS        4	/* Huffman tables are numbered 0..3 */
#define NUM_ARITH_TBLS      16	/* Arith-coding tables are numbered 0..15 */
#define MAX_COMPS_IN_SCAN    4	/* JPEG limit on # of components in one scan */
#define MAX_SAMP_FACTOR      4	/* JPEG limit on sampling factors */

/*
 * The following structure stores basic information about one component.
 */

class TLJPEGComponent
{
public:
   TLJPEGComponent(void);
   ~TLJPEGComponent(void);

   /*
    * These values are fixed over the whole image.
    * They are read from the SOF marker.
    *
    * Downsampling is not normally used in lossless JPEG, although
    * it is permitted by the JPEG standard (DIS). We set all sampling
    * factors to 1 in this program.
    */

    quint8  id;
    quint8  h_samp_factor;
    quint8  v_samp_factor;

    // read from SOS
    quint8 comps_in_scan;
    quint8 dc_tbl_no; // Huffman table selector



};

#endif // LJPEGCOMPONENT_H
