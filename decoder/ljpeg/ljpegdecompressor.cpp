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
#include "ljpegcomponent.h"
#include "ljpegdecompressor.h"
#include "ljpeghuffmantable.h"
#include "ljpegreader.h"
#include "plist.h"

//---------------------------------------------------------------------------
TLJPEGDecompressor::TLJPEGDecompressor(FILE *_fp)
{
   comp_info = new PList;
   cur_comp_info = new PList;
   huffmantables = new PList;

   reader = new TLJPEGReader(this, _fp);

   reset();
}

//---------------------------------------------------------------------------
TLJPEGDecompressor::~TLJPEGDecompressor(void)
{
   delete reader;

   reset();

   delete comp_info;
   delete cur_comp_info;
   delete huffmantables;
}

//---------------------------------------------------------------------------
void TLJPEGDecompressor::reset(void)
{
 TLJPEGComponent *c;
 THuffmanTable *table;

   cur_comp_info->Flush();

   while((c = (TLJPEGComponent *) comp_info->Last()) != NULL) {
      comp_info->Delete(c);
      delete c;
   }

   while((table = (THuffmanTable *) huffmantables->Last()) != NULL) {
      huffmantables->Delete(table);
      delete table;
   }

   restart_interval = 0;
   decompress = false;
}

//---------------------------------------------------------------------------
TLJPEGComponent *TLJPEGDecompressor::getCompById(quint8 id)
{
 TLJPEGComponent *c;
 int i;

   for(i=0; i<comp_info->Count; i++) {
      c = (TLJPEGComponent *) comp_info->ItemAt(i);
      if(c->id == id)
         return c;
   }

 return NULL;
}

//---------------------------------------------------------------------------
THuffmanTable *TLJPEGDecompressor::getHuffmanTable(quint8 id, quint8 table_type)
{
 THuffmanTable *c;
 int i;

   for(i=0; i<huffmantables->Count; i++) {
      c = (THuffmanTable *) huffmantables->ItemAt(i);
      if(c->id == id && c->table_type == table_type)
         return c;
   }

 return NULL;
}

//---------------------------------------------------------------------------
bool TLJPEGDecompressor::readHeader(void)
{
   return reader->readHeader();
}

//---------------------------------------------------------------------------
bool TLJPEGDecompressor::init(void)
{
 THuffmanTable *table;
 TLJPEGComponent *comp;
 int i;

   if(decompress) // already done
      return true;

   for(i=0; i<cur_comp_info->Count; i++) {
      comp = (TLJPEGComponent *) cur_comp_info->ItemAt(i);

      table = (THuffmanTable *) huffmantables->ItemAt(comp->dc_tbl_no);
      if(table == NULL)
         return false;

      if(!table->init())
         return false;
   }

   return decompress;
}

//---------------------------------------------------------------------------
