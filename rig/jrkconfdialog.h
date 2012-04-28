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

#ifndef JRKCONFDIALOG_H
#define JRKCONFDIALOG_H

#include <QDialog>
#include "rig.h"

namespace Ui {
    class JrkConfDialog;
}

class JrkConfDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JrkConfDialog(TRotor *rotor_, bool azimuth_, QWidget *parent = 0);
    ~JrkConfDialog();

    double getMinDeg(void);
    double getMaxDeg(void);

private:
    Ui::JrkConfDialog *ui;
    TRotor *rotor;
    TJRK *jrk;
    bool azimuth;
    QString lut;

private slots:
    void on_buttonBox_accepted();
    void on_jrkStatusButton_clicked();
    void on_openButton_clicked();
};

#endif // JRKCONFDIALOG_H
