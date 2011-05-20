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
#include <stdlib.h>
#include <string.h>

#include "ljpeghuffmantable.h"


unsigned int HBitMask[] = {
   0xffffffff, 0x7fffffff, 0x3fffffff, 0x1fffffff,
   0x0fffffff, 0x07ffffff, 0x03ffffff, 0x01ffffff,
   0x00ffffff, 0x007fffff, 0x003fffff, 0x001fffff,
   0x000fffff, 0x0007ffff, 0x0003ffff, 0x0001ffff,
   0x0000ffff, 0x00007fff, 0x00003fff, 0x00001fff,
   0x00000fff, 0x000007ff, 0x000003ff, 0x000001ff,
   0x000000ff, 0x0000007f, 0x0000003f, 0x0000001f,
   0x0000000f, 0x00000007, 0x00000003, 0x00000001};

//---------------------------------------------------------------------------
THuffmanTable::THuffmanTable(void)
{
   id = 0xff;
   table_type = 0xff;

   reset();
}

//---------------------------------------------------------------------------
THuffmanTable::THuffmanTable(quint8 _id, quint8 _table_type)
{
   id = _id;
   table_type = _table_type;

   bits = NULL;
   huffval = NULL;

   reset();
}

//---------------------------------------------------------------------------
THuffmanTable::~THuffmanTable(void)
{
   if(bits != NULL)
      free(bits);
   if(huffval != NULL)
      free(huffval);

   if(mincode != NULL)
      free(mincode);
   if(maxcode != NULL)
      free(maxcode);
   if(valptr != NULL)
      free(valptr);
   if(numbits != NULL)
      free(numbits);
   if(value != NULL)
      free(value);
}

//---------------------------------------------------------------------------
void THuffmanTable::reset(void)
{
   bits = (quint8 *) malloc(HUFFMAN_BITS_SIZE);
   huffval = (quint8 *) malloc(HUFFMAN_VAL_SIZE);

   memset(bits, 0, HUFFMAN_BITS_SIZE);
   memset(huffval, 0, HUFFMAN_VAL_SIZE);

   mincode = NULL;
   maxcode = NULL;
   valptr  = NULL;
   numbits = NULL;
   value   = NULL;

   inited = false;
}

//---------------------------------------------------------------------------
bool THuffmanTable::init(void)
{   
   if(mincode == NULL)
      mincode = (quint16 *) malloc(HUFFMAN_BITS_SIZE * sizeof(quint16));
   if(maxcode == NULL)
      maxcode = (int *) malloc((HUFFMAN_BITS_SIZE + 1) * sizeof(int));
   if(valptr == NULL)
      valptr = (short *) malloc(HUFFMAN_BITS_SIZE * sizeof(short));
   if(numbits == NULL)
      numbits = (int *) malloc(HUFFMAN_VAL_SIZE * sizeof(int));
   if(value == NULL)
      value = (int *) malloc(HUFFMAN_VAL_SIZE * sizeof(int));

   inited = generatetables();

 return inited;
}

//---------------------------------------------------------------------------
bool THuffmanTable::generatetables(void)
{
 char    *huffsize;
 quint16 code, *huffcode;
 int     size, _value, ll, ul;
 int     p, i, l, lastp, si;

   if(inited)
      return true;

   if(mincode == NULL || maxcode == NULL || valptr == NULL ||
      numbits == NULL || value == NULL)
      return false;

   huffsize = (char *) malloc((HUFFMAN_VAL_SIZE + 1) * sizeof(char));
   if(huffsize == NULL)
      return false;

   huffcode = (quint16 *) malloc((HUFFMAN_VAL_SIZE + 1) * sizeof(quint16));
   if(huffcode == NULL) {
      free(huffsize);
      return false;
   }
   /*
    * Figure C.1: make table of Huffman code length for each symbol
    * Note that this is in code-length order.
    */
   p = 0;
   for(l=1; l<HUFFMAN_BITS_SIZE; l++)
      for(i=1; i<=(int)bits[l]; i++)
         huffsize[p++] = (char)l;

   huffsize[p] = 0;
   lastp = p;

   /*
    * Figure C.2: generate the codes themselves
    * Note that this is in code-length order.
    */
   code = 0;
   si = huffsize[0];
   p = 0;
   while(huffsize[p]) {
      while(((int) huffsize[p]) == si) {
         huffcode[p++] = code;
         code++;
      }
      code <<= 1;
      si++;
   }

   /*
    * Figure F.15: generate decoding tables
    */
   p = 0;
   for(l=1; l<HUFFMAN_BITS_SIZE; l++) {
      if(bits[l]) {
         valptr[l] = p;
         mincode[l] = huffcode[p];
         p += bits[l];
         maxcode[l] = huffcode[p - 1];
      }
      else
         maxcode[l] = -1;
   }

   /*
    * We put in this value to ensure HuffDecode terminates.
    */
   maxcode[HUFFMAN_BITS_SIZE] = 0xFFFFFL;

   /*
    * Build the numbits, value lookup tables.
    * These table allow us to gather 8 bits from the bits stream,
    * and immediately lookup the size and value of the huffman codes.
    * If size is zero, it means that more than 8 bits are in the huffman
    * code (this happens about 3-4% of the time).
    */
   memset(numbits, 0, sizeof(HUFFMAN_VAL_SIZE * sizeof(int)));
   for(p=0; p<lastp; p++) {
      size = huffsize[p];
      if(size <= 8) {
         _value = huffval[p];
         code = huffcode[p];
         ll = code << (8 - size);
         if(size < 8)
            ul = ll | HBitMask[24 + size];
         else
            ul = ll;

         for(i=ll; i<=ul; i++) {
            numbits[i] = size;
            value[i] = _value;
         }
       }
   }

   free(huffsize);
   free(huffcode);

 return true;
}
