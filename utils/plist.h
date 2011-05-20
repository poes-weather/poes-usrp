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
*/
//---------------------------------------------------------------------------
#ifndef PListH
#define PListH

//---------------------------------------------------------------------------
class DUMMY {
   public:
      DUMMY();
      ~DUMMY();
      DUMMY(void *owner, void *obj);

      class PList *Owner;
      DUMMY *next;
      void  *Obj;
};

//---------------------------------------------------------------------------
class PList {
   public:
      PList();
      ~PList();
      int    Add(void *item, int mode=0);
      void   *SetItem(void *item, int index);
      int    Delete(void *item);
      int    Delete(int index);
      void   Flush(void);

      int    IndexOf(void *item);
      void   *ItemAt(int index);
      void   *First(void);
      void   *Last(void);

      class  DUMMY *Items;
      int    Count;

   protected:

};
#endif
