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

    this->setWindowTitle(azimuth ? "Calibrate Azimuth":"Calibrate Elevation");

    ui->minDegrees->setMaximum(azimuth ? 360:180);
    ui->maxDegrees->setMaximum(azimuth ? 360:180);

    ui->minFeedback->setValue(azimuth ? jrk->az_min_fk:jrk->el_min_fk);
    ui->minDegrees->setValue(azimuth ? rotor->az_min:rotor->el_min);
    ui->maxFeedback->setValue(azimuth ? jrk->az_max_fk:jrk->el_max_fk);
    ui->maxDegrees->setValue(azimuth ? rotor->az_max:rotor->el_max);
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
    double *min_deg, *max_deg;
    int *min_fk, *max_fk;

    if(azimuth) {
        min_fk = &jrk->az_min_fk;
        max_fk = &jrk->az_max_fk;
        min_deg = &rotor->az_min;
        max_deg = &rotor->az_max;
    }
    else {
        min_fk = &jrk->el_min_fk;
        max_fk = &jrk->el_max_fk;
        min_deg = &rotor->el_min;
        max_deg = &rotor->el_max;
    }

    *min_fk = ui->minFeedback->value();
    *max_fk = ui->maxFeedback->value();
    *min_deg = ui->minDegrees->value();
    *max_deg = ui->maxDegrees->value();
}

//---------------------------------------------------------------------------
void JrkConfDialog::on_readMinFeedbackButton_clicked()
{
    quint16 data = jrk->readFeedback(azimuth);

    if(data == 0xffff)
        on_jrkStatusButton_clicked();
    else
        ui->minFeedback->setValue(data);
}

//---------------------------------------------------------------------------
void JrkConfDialog::on_readMaxFeedbackButton_clicked()
{
    quint16 data = jrk->readFeedback(azimuth);

    if(data == 0xffff)
        on_jrkStatusButton_clicked();
    else
        ui->maxFeedback->setValue((int)jrk->readFeedback(azimuth));
}
