/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite imagery.
    Copyright (C) 2010 Free Software Foundation, Inc.

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

#ifndef GPSDIALOG_H
#define GPSDIALOG_H

#include <QDialog>
#include "qextserialport.h"

namespace Ui {
    class GPSDialog;
}

class TGPS;
class MainWindow;

//---------------------------------------------------------------------------
class GPSDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GPSDialog(QString ini, QWidget *parent = 0);
    ~GPSDialog();

protected:
    BaudRateType baudrate(void);
    FlowType flowtype(void);

    void writeSettings(void);
    void readSettings(void);


private:
    Ui::GPSDialog *ui;

    TGPS       *gps;
    QString    iniFile;
    MainWindow *mw;

private slots:
    void onGPSDialog_finished(int result);
    void on_startStopButton_clicked();
    void gpsDataAvailable();
};

#endif // GPSDIALOG_H
