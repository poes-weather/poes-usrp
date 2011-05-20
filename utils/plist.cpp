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

#include <stdio.h>
#include "plist.h"

//---------------------------------------------------------------------------

DUMMY::DUMMY()
{
 Owner=NULL;
 next=NULL;
 Obj=NULL;
}

//---------------------------------------------------------------------------
DUMMY::~DUMMY()
{
}

//---------------------------------------------------------------------------
DUMMY::DUMMY(void *owner, void *obj)
{
  Owner=(PList *)owner;
  next=NULL;
  if(Owner->Items) next=Owner->Items;
  Obj=obj;
  Owner->Items=this;
  Owner->Count+=1;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
PList::PList()
{
  Count=0;
  Items=NULL;
}

//---------------------------------------------------------------------------
PList::~PList()
{
  Flush();
}

//---------------------------------------------------------------------------
int PList::Add(void *item, int mode)
{
   mode = mode;
   new DUMMY(this, item);

 return Count;
}

//---------------------------------------------------------------------------
int PList::Delete(void *item)
{
 DUMMY *c, *prev;
 int   i=-1;

  c=prev=Items;
  while(c) {
     if(c->Obj==item) {
        if(c==Items) Items=c->next;
        else         prev->next=c->next; 
        delete c;
        i=Count--;
        break; }
     prev=c;
     c=c->next; }

 if(!Count || !Items)
    Items=NULL;

 return i;
}

//---------------------------------------------------------------------------
void PList::Flush(void)
{
 DUMMY *c, *c2;

  c=Items;
  while(c) {
     c2=c->next;
     Delete(c->Obj);
     c=c2; }

  Count=0;
  Items=NULL;
}

//---------------------------------------------------------------------------
int PList::Delete(int index)
{
 DUMMY *c;
 int i=Count-1;

  if(!Count || index<0 || index>i)
     return -1;

  for(c=Items; c; c=c->next, i--)
     if(i==index)
        return(Delete(c->Obj));

 return(-1);
}

//---------------------------------------------------------------------------
int PList::IndexOf(void *item)
{
 DUMMY *c;
 int index=Count-1;

  for(c=Items; c; c=c->next, index--)
     if(c->Obj==item)
        return index;

 return(-1);
}

//---------------------------------------------------------------------------
void *PList::ItemAt(int index)
{
 DUMMY *c;
 int i=Count-1;

  if(!Count || index<0 || index>i)
     return NULL;

  for(c=Items; c; c=c->next, i--)
     if(i==index)
        return c->Obj;

 return NULL;
}

//---------------------------------------------------------------------------
void *PList::SetItem(void *item, int index) // zero based
{
 void *c = ItemAt(index);

  if(c)
     c = item;
     
 return c;
}

//---------------------------------------------------------------------------
void *PList::First(void)
{
 return ItemAt(0);
}

//---------------------------------------------------------------------------
void *PList::Last(void)
{
  if(!Count)
     return NULL;
  else
     return ItemAt(Count-1);
}

