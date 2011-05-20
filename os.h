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

#ifndef OS_H
#define OS_H

#if defined(_WIN32) || defined(WIN32)
#  if !defined(__HRPT_WIN__)
#    if defined(_WIN32)
#      define __HRPT_WIN__ _WIN32
#    else
#      if defined(WIN32)
#        define __HRPT_WIN__ WIN32
#      endif
#    endif
#  endif
#endif

#if defined(_WIN64) || defined(WIN64)
#  if !defined(__HRPT_WIN__)
#    if defined(_WIN64)
#      define __HRPT_WIN__ _WIN64
#    else
#      if defined(WIN64)
#        define __HRPT_WIN__ WIN64
#      endif
#    endif
#  endif
#endif


#endif // OS_H
