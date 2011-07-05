/*
    POES-USRP, a software for recording and decoding POES high resolution weather satellite images.
    Copyright (C) 2009-2011 Free Software Foundation, Inc.

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
#include <string.h>

#include "satpropdialog.h"
#include "ui_satpropdialog.h"
#include "plist.h"
#include "Satellite.h"
#include "satprop.h"


//---------------------------------------------------------------------------
SatPropDialog::SatPropDialog(PList *satList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SatPropDialog)
{
    TSat *sat;
    int i;

    ui->setupUi(this);
    setLayout(ui->maingridLayout);

    satlist = satList;

    for(i=0; i<satList->Count; i++) {
        sat = (TSat *) satList->ItemAt(i);
        ui->satlistWidget->addItem(sat->name);
    }
    ui->satlistWidget->sortItems();
}

//---------------------------------------------------------------------------
SatPropDialog::~SatPropDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------
TSat *SatPropDialog::getSat(QString name)
{
    TSat *sat;
    int i;

    for(i=0; i<satlist->Count; i++) {
        sat = (TSat *) satlist->ItemAt(i);
        if(strcmp(sat->name, name.toStdString().c_str()) == 0)
            return sat;
    }

    return NULL;
}

//---------------------------------------------------------------------------
void SatPropDialog::on_satlistWidget_itemClicked(QListWidgetItem* item)
{
    TSat *sat;
    int *rgb;

    sat = getSat(item->text());
    if(!sat)
        return;

    rgb = sat->sat_props->rgb_day();
    ui->day_rgb_r->setValue(rgb[0]);
    ui->day_rgb_g->setValue(rgb[1]);
    ui->day_rgb_b->setValue(rgb[2]);

    rgb = sat->sat_props->rgb_night();
    ui->night_rgb_r->setValue(rgb[0]);
    ui->night_rgb_g->setValue(rgb[1]);
    ui->night_rgb_b->setValue(rgb[2]);
}

//---------------------------------------------------------------------------
void SatPropDialog::on_applyRGB_clicked()
{
    QListWidgetItem* item;
    TSat *sat;
    int i;

    for(i=0; i<ui->satlistWidget->count(); i++) {
        item = ui->satlistWidget->item(i);
        if(!item->isSelected())
            continue;

        sat = getSat(item->text());
        if(!sat)
            continue;
        sat->sat_props->rgb_day(ui->day_rgb_r->value(),
                                ui->day_rgb_g->value(),
                                ui->day_rgb_b->value());
        sat->sat_props->rgb_night(ui->night_rgb_r->value(),
                                  ui->night_rgb_g->value(),
                                  ui->night_rgb_b->value());
    }

}
