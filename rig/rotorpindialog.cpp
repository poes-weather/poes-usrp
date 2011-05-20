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
#include "rotorpindialog.h"
#include "ui_rotorpindialog.h"

RotorPinDialog::RotorPinDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RotorPinDialog)
{
    ui->setupUi(this);
}

//---------------------------------------------------------------------------
RotorPinDialog::~RotorPinDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------
void RotorPinDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
void RotorPinDialog::setPin(bool az, int pin1, int pin2, bool cc)
{
    this->setWindowTitle(az ? "Azimuth pin setup" : "Elevation pin setup");
    ui->azelLabel->setText(az ? "Azimuth:":"Elevation:");

    ui->azelCb->setCurrentIndex(pin1);
    ui->dirCb->setCurrentIndex(pin2);
    ui->ccCb->setChecked(cc);
}

//---------------------------------------------------------------------------
int RotorPinDialog::getAzElPinIndex(void)
{
    return ui->azelCb->currentIndex();
}

//---------------------------------------------------------------------------
int RotorPinDialog::getDirPinIndex(void)
{
    return ui->dirCb->currentIndex();
}

//---------------------------------------------------------------------------
bool RotorPinDialog::getCC(void)
{
    return ui->ccCb->isChecked();
}

