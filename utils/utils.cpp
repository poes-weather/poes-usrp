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
#include <QtGlobal>
#include <QDateTime>
#include <QTime>
#include <QTableWidget>
#include <QListWidget>

#include "utils.h"

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
#    include <unistd.h>
#    include <time.h>
#endif
#if defined(Q_OS_WIN32)
#    include <windows.h>
#endif
//---------------------------------------------------------------------------
void delay(unsigned int msec)
{
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
   usleep(msec * 1000);
#else
#  if defined(Q_OS_WIN32)
      Sleep(msec);
#  endif
#endif
}

//---------------------------------------------------------------------------
// Return the number of days since 31Dec79 00:00:00 UTC (daynum 0)
// mode&1 = add milliseconds
double GetStartTime(QDateTime utc, int mode)
{
 QDateTime utc_date;
 QTime     utc_time;
 double    daynum;

  utc_date = utc;

  QDateTime dec3179(QDate(1979, 12, 31));
  utc_time = utc_date.time();

  daynum = (double) (
            ((double)dec3179.daysTo(utc_date)) +
            ((double)utc_time.hour())   / 24.0 +
            ((double)utc_time.minute()) / 1440.0 +
            ((double)utc_time.second()) / 86400.0 );
#if 0

  QString str;
  str.sprintf("GetStartTime daynum: %f", daynum);
  qDebug(str.toStdString().c_str());

#endif


  if(mode&1)
     daynum += ((double)utc_time.msec()) / (86400.0*1.0e3);

  return daynum;
}

//---------------------------------------------------------------------------
char *ReplaceChar(const char *buff, char *ret, const char tofind, const char newchar)
{
 char c;
 int  i;

  for(i=0; i<(int)strlen(buff); i++) {
     if(buff[i]==tofind) c=newchar;
     else                c=buff[i];
     ret[i]=c; }
  ret[i]='\0';

 return ret;
}

//---------------------------------------------------------------------------
// mode&0 = from the beginning
//     &1 = current position
bool SearchHeader(const char *str, char *tmp, int tsize, FILE *fp, int mode)
{
 bool rc = false;
 int  i;

  if(!fp || !str || !tmp || !tsize)
     return false;

  if(!mode)
     fseek(fp, 0L, SEEK_SET);

  // typical format [NOAA 18]\n
  i = strlen(str) + 1; // include the line feed
  tsize = tsize > i ? i:tsize; // speed up

  while(!rc && fgets(tmp, tsize, fp)) {
     if(*tmp != *str) continue; // check first chars
     if(!strcmp(str, tmp))
        rc=true; }

 return rc;
}

//---------------------------------------------------------------------------
bool SameDate(QDateTime dt1, QDateTime dt2)
{
  return dt1.daysTo(dt2) == 0 ? true:false;
}

//---------------------------------------------------------------------------
void clearGrid(QTableWidget *grid)
{
 QTableWidgetItem *item;
 int  i, j;

  for(i=0; i<grid->rowCount(); i++)
      for(j=0; j<grid->columnCount(); j++) {
         item = grid->item(i, j);
         if(item)
             delete item;
     }

  grid->setRowCount(0);
}

//---------------------------------------------------------------------------
QListWidgetItem *getSelectedListItem(QListWidget *list)
{
QListWidgetItem *item;
int i;

   item = NULL;
   for(i=0; list->count() && item == NULL; i++)
       if(list->item(i)->isSelected())
           item = list->item(i);

 return item;
}
