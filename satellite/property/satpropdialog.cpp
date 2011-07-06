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
#include <QKeyEvent>
#include <string.h>

#include "satpropdialog.h"
#include "ui_satpropdialog.h"
#include "plist.h"
#include "Satellite.h"
#include "satutil.h"
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

    selsat = NULL;
}

//---------------------------------------------------------------------------
SatPropDialog::~SatPropDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------
int SatPropDialog::countSelected(QListWidget *lw)
{
    QListWidgetItem* item;
    int i, n = 0;

    for(i=0; i<lw->count(); i++) {
        item = (QListWidgetItem *) lw->item(i);
        if(item->isSelected())
            n++;
    }

    return n;
}

//---------------------------------------------------------------------------
int SatPropDialog::finditem(QListWidget *lw, const QString& str)
{
    QListWidgetItem* item;
    int i;

    for(i=0; i<lw->count(); i++) {
        item = (QListWidgetItem *) lw->item(i);
        if(item->text() == str)
            return i;
    }

    return -1;
}

//---------------------------------------------------------------------------
void SatPropDialog::on_satlistWidget_itemClicked(QListWidgetItem* item)
{
    QListWidgetItem *item2;
    TRGBConf *rc;
    PList    *plist;
    QString  str;
    int      i;

    i = countSelected(ui->satlistWidget);
    if(i <= 1)
        ui->rgblv->clear();

    selsat = getSat(satlist, item->text());
    if(!selsat)
        return;

    plist = selsat->sat_props->rgblist;
    for(i=0; i<plist->Count; i++) {
        rc = (TRGBConf *) plist->ItemAt(i);
        str = rc->name();
        if(finditem(ui->rgblv, str) < 0)
            ui->rgblv->addItem(str);
    }

    // select the first one
    rc = (TRGBConf *) plist->ItemAt(0);
    if(rc) {
        i = finditem(ui->rgblv, rc->name());
        if(i >= 0) {
            item2 = ui->rgblv->item(i);
            item2->setSelected(true);
            on_rgblv_itemClicked(item2);
        }

    }
}
//---------------------------------------------------------------------------
//
//                  RGB Settings
//
//---------------------------------------------------------------------------
void SatPropDialog::on_rgblv_itemClicked(QListWidgetItem* item)
{
    if(!selsat)
        return;

    TRGBConf *rc = selsat->sat_props->get_rgb(item->text());
    if(!rc)
        return;

    ui->rgbnameEd->setText(item->text());
    int *rgb = rc->rgb_ch();
    ui->rgb_r->setValue(rgb[0]);
    ui->rgb_g->setValue(rgb[1]);
    ui->rgb_b->setValue(rgb[2]);
}

//---------------------------------------------------------------------------
void SatPropDialog::on_applyRGB_clicked()
{
    QListWidgetItem *item, *rgbitem;
    TRGBConf *rc = NULL;
    TSat *sat;
    int i, j;

    if(ui->rgbnameEd->text().isEmpty())
        return;

    // apply to all selected satellites the same selected rgb settings
    for(i=0; i<ui->satlistWidget->count(); i++) {
        item = ui->satlistWidget->item(i);
        if(!item->isSelected())
            continue;

        sat = getSat(satlist, item->text());
        if(!sat)
            continue;

        if(rc == NULL) {
            // get the selected rgb setting and modify it
            for(j=0; j<ui->rgblv->count(); j++) {
                rgbitem = ui->rgblv->item(j);
                if(rgbitem->isSelected()) {
                    rc = sat->sat_props->get_rgb(rgbitem->text());
                    if(rc)
                        break;
                }
            }

            if(rc == NULL)
                return; // corrupt...

            rgbitem->setText(ui->rgbnameEd->text());
            rc->name(ui->rgbnameEd->text());
        }

        sat->sat_props->add_rgb(ui->rgbnameEd->text(),
                                ui->rgb_r->value(),
                                ui->rgb_g->value(),
                                ui->rgb_b->value());
    }
}

//---------------------------------------------------------------------------
void SatPropDialog::on_addRGBBtn_clicked()
{
    QListWidgetItem *item;
    TRGBConf *rc;
    TSat *sat;
    int i;

    if(ui->rgbnameEd->text().isEmpty())
        return;

    // apply to all selected satellites the same new rgb settings
    for(i=0; i<ui->satlistWidget->count(); i++) {
        item = ui->satlistWidget->item(i);
        if(!item->isSelected())
            continue;

        sat = getSat(satlist, item->text());
        if(!sat)
            continue;

        rc = sat->sat_props->get_rgb(ui->rgbnameEd->text());
        if(rc)
            rc->rgb_ch(ui->rgb_r->value(),
                       ui->rgb_g->value(),
                       ui->rgb_b->value());
        else
            sat->sat_props->add_rgb(ui->rgbnameEd->text(),
                                    ui->rgb_r->value(),
                                    ui->rgb_g->value(),
                                    ui->rgb_b->value());
    }

    if(finditem(ui->rgblv, ui->rgbnameEd->text()) < 0)
        ui->rgblv->addItem(ui->rgbnameEd->text());

    // select it
    i = finditem(ui->rgblv, ui->rgbnameEd->text());
    if(i >= 0) {
        item = ui->rgblv->item(i);
        item->setSelected(true);
        on_rgblv_itemClicked(item);
    }
}

//---------------------------------------------------------------------------
void SatPropDialog::keyPressEvent(QKeyEvent *event)
{
    QListWidgetItem *item, *rgbitem;
    TSat *sat;
    int i, j;

     if(event->key() != Qt::Key_Delete) {
         QWidget::keyPressEvent(event);
         return;
     }

     // get the selected rgb setting and delete it from all selected satellites
     for(j=0; j<ui->rgblv->count(); j++) {
         rgbitem = ui->rgblv->item(j);
         if(!rgbitem->isSelected())
             continue;

         for(i=0; i<ui->satlistWidget->count(); i++) {
             item = ui->satlistWidget->item(i);
             if(!item->isSelected())
                 continue;
             sat = getSat(satlist, item->text());
             if(sat)
                 sat->sat_props->del_rgb(rgbitem->text());
         }

         ui->rgblv->removeItemWidget(rgbitem);
         delete rgbitem;
     }
}

//---------------------------------------------------------------------------
void SatPropDialog::on_defaultrgbBtn_clicked()
{
    QListWidgetItem *item, *selitem = NULL;
    TSat *sat;
    int i;

    for(i=0; i<ui->satlistWidget->count(); i++) {
        item = ui->satlistWidget->item(i);
        if(!item->isSelected())
            continue;

        selitem = item;
        sat = getSat(satlist, item->text());
        if(!sat)
            continue;

        sat->sat_props->add_rgb_defaults();
    }

    if(selitem)
        on_satlistWidget_itemClicked(selitem);
}

//---------------------------------------------------------------------------

