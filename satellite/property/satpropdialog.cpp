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

#include "eviconfdialog.h"

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
int SatPropDialog::finditem(QComboBox *cb, const QString& str)
{
    return cb->findText(str);
}

//---------------------------------------------------------------------------
void SatPropDialog::on_satlistWidget_itemClicked(QListWidgetItem* item)
{
    TRGBConf *rc;
    TNDVI    *vi;
    PList    *plist;
    QString  str;
    int      i;

    i = countSelected(ui->satlistWidget);
    if(i <= 1) {
        ui->rgbCb->clear();
        ui->ndviCb->clear();
        ui->ndviimageCb->clear();
        ui->ndviimageCb->addItem("Near IR");
    }

    selsat = getSat(satlist, item->text());
    if(!selsat)
        return;

    // RGB Conf
    plist = selsat->sat_props->rgblist;
    for(i=0; i<plist->Count; i++) {
        rc = (TRGBConf *) plist->ItemAt(i);
        str = rc->name();
        if(ui->rgbCb->findText(str) < 0)
            ui->rgbCb->addItem(str);

        if(ui->ndviimageCb->findText(str) < 0)
            ui->ndviimageCb->addItem(str);
    }

    // select the first one for RGB tab
    rc = (TRGBConf *) plist->ItemAt(0);
    if(rc) {
        i = ui->rgbCb->findText(rc->name());
        if(i >= 0)
            ui->rgbCb->setCurrentIndex(i);
    }

    // NDVI Conf
    plist = selsat->sat_props->ndvilist;
    for(i=0; i<plist->Count; i++) {
        vi = (TNDVI *) plist->ItemAt(i);
        str = vi->name();
        if(ui->ndviCb->findText(str) < 0)
            ui->ndviCb->addItem(str);
    }

    vi = (TNDVI *) plist->ItemAt(0);
    if(vi) {
        i = ui->ndviCb->findText(vi->name());
        if(i >= 0)
            ui->ndviCb->setCurrentIndex(i);
    }

    // Decoder
    ui->derandCb->setChecked(selsat->sat_props->derandomize());
    ui->rsdecodeCb->setChecked(selsat->sat_props->rs_decode());
    ui->syncCheckCb->setChecked(selsat->sat_props->syncCheck());
}
//---------------------------------------------------------------------------
//
//                  RGB Settings
//
//---------------------------------------------------------------------------
void SatPropDialog::on_rgbCb_currentIndexChanged(int /*index*/)
{
    TRGBConf *rc = getRGBConf(ui->rgbCb->currentText());
    if(!rc)
        return;

    ui->rgbnameEd->setText(ui->rgbCb->currentText());
    int *rgb = rc->rgb_ch();
    ui->rgb_r->setValue(rgb[0]);
    ui->rgb_g->setValue(rgb[1]);
    ui->rgb_b->setValue(rgb[2]);
}

//---------------------------------------------------------------------------
void SatPropDialog::on_applyRGB_clicked()
{
    if(!ui->rgbCb->currentText().isEmpty())
        addRGB(ui->rgbnameEd->text(), ui->rgbCb->currentText());
}

//---------------------------------------------------------------------------
void SatPropDialog::on_addRGBBtn_clicked()
{
    addRGB(ui->rgbnameEd->text());
}

//---------------------------------------------------------------------------
TRGBConf *SatPropDialog::getRGBConf(const QString& name)
{
    QListWidgetItem *item;
    TSat *sat;
    TRGBConf *rc;
    int i;

    if(name.isEmpty())
        return NULL;

    rc = NULL;
    if(selsat)
        rc = selsat->sat_props->get_rgb(name);

    if(rc == NULL) {
        for(i=0; i<ui->satlistWidget->count(); i++) {
            item = ui->satlistWidget->item(i);
            if(!item->isSelected())
                continue;

            sat = getSat(satlist, item->text());
            if(!sat)
                continue;

            rc = sat->sat_props->get_rgb(name);
            if(rc)
                break;
        }
    }

    return rc;
}

//---------------------------------------------------------------------------
void SatPropDialog::addRGB(const QString& name, const QString& oldname)
{
    QListWidgetItem *item;
    TRGBConf *rc, *rc2;
    TSat *sat;
    int i;

    if(name.isEmpty() || countSelected(ui->satlistWidget) == 0)
        return;

    rc = new TRGBConf(name,
                      ui->rgb_r->value(),
                      ui->rgb_g->value(),
                      ui->rgb_b->value());

    // apply to all selected satellites the same new rgb settings
    for(i=0; i<ui->satlistWidget->count(); i++) {
        item = ui->satlistWidget->item(i);
        if(!item->isSelected())
            continue;

        sat = getSat(satlist, item->text());
        if(!sat)
            continue;

        rc2 = sat->sat_props->get_rgb(oldname);
        if(rc2)
            *rc2 = *rc;
        else
            sat->sat_props->add_rgb(rc);
    }

    i = ui->rgbCb->findText(name);
    if(i < 0) {
        ui->rgbCb->addItem(name);
        i = ui->rgbCb->count() - 1;
    }
    ui->rgbCb->setCurrentIndex(i);

    if(ui->ndviimageCb->findText(name) < 0)
        ui->ndviimageCb->addItem(name);

    delete rc;
}

//---------------------------------------------------------------------------
void SatPropDialog::on_delrgbBtn_clicked()
{
    QListWidgetItem *item, *selitem = NULL;
    QString name = ui->rgbCb->currentText();
    TSat *sat;
    int i;

    if(name.isEmpty())
        return;

    for(i=0; i<ui->satlistWidget->count(); i++) {
        item = ui->satlistWidget->item(i);
        if(!item->isSelected())
            continue;

        selitem = item;
        sat = getSat(satlist, item->text());
        if(sat)
            sat->sat_props->del_rgb(name);
    }

    ui->rgbCb->removeItem(ui->rgbCb->currentIndex());

    if((i = ui->ndviCb->findText(name)) >= 0)
        ui->ndviCb->removeItem(i);

    if(selitem)
        on_satlistWidget_itemClicked(selitem);
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
//
//      NDVI
//
//---------------------------------------------------------------------------
TNDVI *SatPropDialog::getNDVI(const QString& name)
{
    QListWidgetItem *item;
    TSat *sat;
    TNDVI *vi;
    int i;

    if(name.isEmpty())
        return NULL;

    vi = NULL;
    if(selsat)
        vi = selsat->sat_props->get_ndvi(name);

    if(vi == NULL) {
        for(i=0; i<ui->satlistWidget->count(); i++) {
            item = ui->satlistWidget->item(i);
            if(!item->isSelected())
                continue;

            sat = getSat(satlist, item->text());
            if(!sat)
                continue;

            vi = sat->sat_props->get_ndvi(name);
            if(vi)
                break;
        }
    }

    return vi;
}

//---------------------------------------------------------------------------
void SatPropDialog::on_ndviCb_currentIndexChanged(int /*index*/)
{
    TNDVI *vi = getNDVI(ui->ndviCb->currentText());

    if(vi == NULL)
        return;

    ui->ndvinameEd->setText(vi->name());
    ui->ndvivisch->setValue(vi->vis_ch());
    ui->ndvinirch->setValue(vi->nir_ch());
    ui->minndvi->setValue(vi->minValue());
    ui->maxndvi->setValue(vi->maxValue());
    ui->ndviluted->setText(vi->lut());

    int i = 0;
    if(!vi->rgbName().isEmpty())
        i = ui->ndviimageCb->findText(vi->rgbName());

    ui->ndviimageCb->setCurrentIndex(i < 0 ? 0:i);
}

//---------------------------------------------------------------------------
void SatPropDialog::on_addndvBtn_clicked()
{
    addNDVI(ui->ndvinameEd->text());
}

//---------------------------------------------------------------------------
void SatPropDialog::on_applyndviBtn_clicked()
{
    addNDVI(ui->ndviCb->currentText());
}

//---------------------------------------------------------------------------
void SatPropDialog::addNDVI(const QString& name, const QString& oldname)
{
    QListWidgetItem *item;
    TSat            *sat;
    TNDVI           *ndvi, *vi;
    TRGBConf        *rc, *rc2;
    int             i;

    if(name.isEmpty() || countSelected(ui->satlistWidget) == 0)
        return;

    ndvi = new TNDVI;
    ndvi->name(name);
    ndvi->vis_ch(ui->ndvivisch->value());
    ndvi->nir_ch(ui->ndvinirch->value());
    ndvi->minValue(ui->minndvi->value());
    ndvi->maxValue(ui->maxndvi->value());
    ndvi->lut(ui->ndviluted->text());

    // make sure the satellite got the RGB setting
    // channel (gray) is first imagetype then come RGB types
    rc = NULL;
    if(ui->ndviimageCb->currentIndex() > 0)
        rc = getRGBConf(ui->ndviimageCb->currentText());

    ndvi->rgbName(rc == NULL ? "":rc->name());

    for(i=0; i<ui->satlistWidget->count(); i++) {
        item = ui->satlistWidget->item(i);
        if(!item->isSelected())
            continue;

        sat = getSat(satlist, item->text());
        if(!sat)
            continue;

        if(rc) {
            rc2 = sat->sat_props->get_rgb(rc->name());

            if(rc2 == NULL) {
                rc2 = new TRGBConf(*rc);
                sat->sat_props->rgblist->Add(rc2);
            }
        }

        vi = sat->sat_props->get_ndvi(oldname);
        if(vi)
            *vi = *ndvi;
        else
            sat->sat_props->add_ndvi(ndvi);
    }

    i = ui->ndviCb->findText(ndvi->name());
    if(i < 0) {
        ui->ndviCb->addItem(ndvi->name());
        i = ui->ndviCb->count() - 1;
    }

    if(i != ui->ndviCb->currentIndex())
        ui->ndviCb->setCurrentIndex(i);

    delete ndvi;
}

//---------------------------------------------------------------------------
void SatPropDialog::on_delndviBtn_clicked()
{
    QListWidgetItem *item, *selitem = NULL;
    TSat *sat;
    int i;

    if(ui->ndviCb->count() == 0 || countSelected(ui->satlistWidget) == 0)
        return;

    for(i=0; i<ui->satlistWidget->count(); i++) {
        item = ui->satlistWidget->item(i);
        if(!item->isSelected())
            continue;

        selitem = item;
        sat = getSat(satlist, item->text());
        if(sat)
            sat->sat_props->del_ndvi(ui->ndviCb->currentText());
    }

    ui->ndviCb->removeItem(ui->ndviCb->currentIndex());

    if(selitem)
        on_satlistWidget_itemClicked(selitem);
}

//---------------------------------------------------------------------------
//
//      Decoder options
//
//---------------------------------------------------------------------------
void SatPropDialog::on_applyDecoderBtn_clicked()
{
    QListWidgetItem *item;
    TSat *sat;
    int i;

    for(i=0; i<ui->satlistWidget->count(); i++) {
        item = ui->satlistWidget->item(i);
        if(!item->isSelected())
            continue;

        sat = getSat(satlist, item->text());
        if(!sat)
            continue;

        sat->sat_props->derandomize(ui->derandCb->isChecked());
        sat->sat_props->rs_decode(ui->rsdecodeCb->isChecked());
        sat->sat_props->syncCheck(ui->syncCheckCb->isChecked());
    }
}

//---------------------------------------------------------------------------
void SatPropDialog::on_confEVIBtn_clicked()
{
    EVIConfDialog dlg(this);

    dlg.exec();

}

//---------------------------------------------------------------------------
