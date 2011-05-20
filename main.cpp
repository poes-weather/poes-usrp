/*
    HRPT-Decoder, a software for processing NOAA-POES hig resolution weather satellite images.
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

#include <QtGui/QApplication>

#if defined(HAVE_IMAGE_PLUGINS)
# include <QtPlugin>

  Q_IMPORT_PLUGIN(qjpeg)
  Q_IMPORT_PLUGIN(qgif)
  Q_IMPORT_PLUGIN(qtiff)
  Q_IMPORT_PLUGIN(qmng)
#endif

#include "mainwindow.h"


int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(application);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
