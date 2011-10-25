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

#include "azeldialog.h"
#include "ui_azeldialog.h"

AzElDialog::AzElDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AzElDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);
}

//---------------------------------------------------------------------------
AzElDialog::~AzElDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------
void AzElDialog::setAzEl(double Az, double El)
{
    ui->azSB->setValue(Az);
    ui->elSB->setValue(El);
}

//---------------------------------------------------------------------------
void AzElDialog::on_AzElDialog_accepted()
{

}

//---------------------------------------------------------------------------
void AzElDialog::setlimits(double min_value, double max_value, bool az)
{
    QDoubleSpinBox *sb = az ? ui->azSB:ui->elSB;

    sb->setMinimum(min_value);
    sb->setMaximum(max_value);
}

//---------------------------------------------------------------------------
double AzElDialog::azimuth(void)
{
    return ui->azSB->value();
}

//---------------------------------------------------------------------------
void AzElDialog::azimuth(double az)
{
    ui->azSB->setValue(az);
}

//---------------------------------------------------------------------------
double AzElDialog::elevation(void)
{
    return ui->elSB->value();
}

//---------------------------------------------------------------------------
void AzElDialog::elevation(double el)
{
    ui->elSB->setValue(el);
}
