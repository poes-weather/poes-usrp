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
#include <QString>
#include <QDateTime>
#include <stdlib.h>

#include "Satellite.h"
#include "satutil.h"
#include "plist.h"

//---------------------------------------------------------------------------
int ReadTLE(FILE *fp, PList *list)
{
 TSat *tmpsat, *sat;
 char *name, *line1, *line2;
 int  count = 0;

  if(fp == NULL || list == NULL)
      return 0;

  name  = (char *) malloc(TLE_STRLEN + 1);
  line1 = (char *) malloc(TLE_STRLEN + 1);
  line2 = (char *) malloc(TLE_STRLEN + 1);

  tmpsat = new TSat;

  while(!feof(fp)) {
     name[0]  = '\0';
     line1[0] = '\0';
     line2[0] = '\0';

     tmpsat->Zero();

     if(!fgets(name, TLE_STRLEN, fp) || *name == '#' || strlen(name) < 2)
         continue;

     if(!fgets(line1, TLE_STRLEN, fp))
        break;
     if(*line1 != '1')
        continue;
     if(!fgets(line2, TLE_STRLEN, fp) || *line2 != '2')
        break;
     if(!tmpsat->TLEKepCheck(name, line1, line2))
        continue;

     sat = getSat(list, tmpsat->name);
     if(sat) {
         if(tmpsat->GetKeplerIssuedDateTime() > sat->GetKeplerIssuedDateTime())
           sat->TLEKepCheck(tmpsat->name, tmpsat->line1, tmpsat->line2);
     }
     else
        list->Add(new TSat(tmpsat));

     count++;
  }

  free(name);
  free(line1);
  free(line2);
  delete tmpsat;

 return count;
}

//---------------------------------------------------------------------------
TSat *getSat(PList *list, const QString &name)
{
 TSat *sat, *rc = NULL;
 int i;

 if(list == NULL || name.isEmpty())
     return NULL;

 for(i=0; i<list->Count && rc == NULL; i++) {
    sat = (TSat *) list->ItemAt(i);
    if(sat->name == name)
        rc = sat;
 }

 return rc;
}

//---------------------------------------------------------------------------
// flags&1 = delete list
void clearSatList(PList *list, int flags)
{
 TSat *sat;

  if(list == NULL)
      return;

  while((sat = (TSat *)list->Last())) {
      list->Delete(sat);
      delete sat;
  }

  if(flags&1) {
      delete list;
      list = NULL;
  }
}
