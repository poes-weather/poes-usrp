#ifndef UTILS_H
#define UTILS_H
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
class QDateTime;
class QTableWidget;
class QTime;
class QListWidget;
class QListWidgetItem;

#include <stdio.h>
#include <math.h>

#ifndef rint
#   define rint(X) floor((X)+0.5)
#endif

#ifndef ClipValue
#   define ClipValue(value, maxv, minv) ((double) ( \
       value < minv ? minv:(value > maxv ? maxv:value)))
#endif

#ifndef MAX
#   define MAX(a, b) (a > b ? a:b)
#endif

#ifndef MIN
#   define MIN(a, b) (a < b ? a:b)
#endif

static const double RTD = 180.0 / M_PI;
static const double DTR = M_PI / 180.0;


double GetStartTime(QDateTime utc, int mode=0);
char   *ReplaceChar(const char *buff, char *ret, const char tofind, const char newchar);
bool   SameDate(QDateTime dt1, QDateTime dt2);
bool   SearchHeader(const char *str, char *tmp, int tsize, FILE *fp, int mode=0);
void   clearGrid(QTableWidget *grid);
void   delay(unsigned int msec);

QListWidgetItem *getSelectedListItem(QListWidget *list);

//---------------------------------------------------------------------------
#endif // UTILS_H
