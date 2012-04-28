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
#include <QMessageBox>
#include <QFileDialog>

#include "jrkconfdialog.h"
#include "ui_jrkconfdialog.h"

//---------------------------------------------------------------------------
JrkConfDialog::JrkConfDialog(TRotor *rotor_, bool azimuth_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JrkConfDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);

    rotor   = rotor_;
    jrk     = rotor->jrk;
    azimuth = azimuth_;

    this->setWindowTitle(azimuth ? "Calibrate Jrk Azimuth":"Calibrate Jrk Elevation");

    ui->lutCb->setChecked(jrk->enableLUT(azimuth));
    lut = jrk->lutFile(azimuth);
}

//---------------------------------------------------------------------------
JrkConfDialog::~JrkConfDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------
void JrkConfDialog::on_jrkStatusButton_clicked()
{
    jrk->clearError(azimuth);

    QMessageBox::information(this, "Jrk " + QString(azimuth ? "Azimuth":"Elevation") + " Status", jrk->status(azimuth));
}

//---------------------------------------------------------------------------
void JrkConfDialog::on_buttonBox_accepted()
{
    jrk->enableLUT(azimuth, ui->lutCb->isChecked());
    jrk->lutFile(azimuth, lut);

    jrk->loadLUT(azimuth);
}

//---------------------------------------------------------------------------
void JrkConfDialog::on_openButton_clicked()
{
    QString inifile = QFileDialog::getOpenFileName(this, tr("Open lookup table file"), lut, "INI files (*.ini);;All files (*.*)");

    if(!inifile.isEmpty())
        lut = inifile;
}
