/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite imagery.
    Copyright (C) 2010,2011 Free Software Foundation, Inc.

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

#ifndef AZELDIALOG_H
#define AZELDIALOG_H

#include <QDialog>

namespace Ui {
    class AzElDialog;
}

class AzElDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AzElDialog(QWidget *parent = 0);
    ~AzElDialog();

    double azimuth(void);
    void   azimuth(double az);
    void   setlimits(double min_value, double max_value, bool az);
    void   setAzEl(double Az, double El);

    double elevation(void);
    void   elevation(double el);

private:
    Ui::AzElDialog *ui;

private slots:
    void on_AzElDialog_accepted();
};

#endif // AZELDIALOG_H
