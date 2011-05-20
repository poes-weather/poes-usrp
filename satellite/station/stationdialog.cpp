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
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <math.h>

#include "station.h"
#include "stationdialog.h"
#include "ui_stationdialog.h"

//---------------------------------------------------------------------------
StationDialog::StationDialog(QString _inifile, TStation *qth, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::StationDialog)
{
 QString str;

    m_ui->setupUi(this);
    setLayout(m_ui->gridLayout_2);

    pqth = qth;
    inifile = _inifile;

    str.sprintf("%g", pqth->lon()); m_ui->lonEdit->setText(str);
    str.sprintf("%g", pqth->lat()); m_ui->latEdit->setText(str);
    str.sprintf("%g", pqth->alt()); m_ui->altEdit->setText(str);
    m_ui->nameEdit->setText(pqth->name());

    readStations();
}

//---------------------------------------------------------------------------
StationDialog::~StationDialog()
{
    delete m_ui;
}

//---------------------------------------------------------------------------
void StationDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
void StationDialog::readStations(void)
{
    QFile file(inifile);
    if(!file.exists(inifile))
        return;

    QSettings reg(inifile, QSettings::IniFormat);

    QTreeWidgetItem *child;
    QTreeWidgetItem *item;
    QStringList     keys;
    QString         key, str;
    int j, i=0;

    keys << "Lon";
    keys << "Lat";
    keys << "Alt";

    while(true) {
        key.sprintf("Station-%d", i++);

        reg.beginGroup(key);

        str = reg.value("Name", "").toString();
        if(str.isEmpty())
            break;

        item = new QTreeWidgetItem(QTreeWidgetItem::DontShowIndicatorWhenChildless);
        item->setText(0, str);

        for(j=0; j<keys.count(); j++) {
            str = reg.value(keys.at(j), "Type: 1 unit").toString();
            child = new QTreeWidgetItem(QTreeWidgetItem::DontShowIndicatorWhenChildless);
            child->setText(0, str);
            item->addChild(child);
        }

        m_ui->stationTree->addTopLevelItem(item);

        reg.endGroup();
    }
}

//---------------------------------------------------------------------------
void StationDialog::writeStations(void)
{
    if(m_ui->stationTree->topLevelItemCount() == 0)
        return;

    QSettings       reg(inifile, QSettings::IniFormat);
    QTreeWidgetItem *child;
    QTreeWidgetItem *item;
    QStringList     keys;
    QString         key;
    int j, i = 0;

    QFile file(inifile);
    if(file.exists(inifile))
        file.remove();

    keys << "Lon";
    keys << "Lat";
    keys << "Alt";

    while((item = m_ui->stationTree->topLevelItem(i))) {
        key.sprintf("Station-%d", i++);

        reg.beginGroup(key);

        reg.setValue("Name", item->text(0));

        for(j=0; j<item->childCount(); j++) {
            child = item->child(j);
            reg.setValue(keys.at(j), child->text(0));
        }

        reg.endGroup();
    }
}

//---------------------------------------------------------------------------
// flags&1 = silent
bool StationDialog::checkInput(int flags)
{
    double lon, lat, alt;

    if(m_ui->nameEdit->text().isEmpty())
    {
        if(!(flags&1))
           QMessageBox::critical(this, "Station name", "Station name can't be empty!");

        return false;
    }

    lon = atof(m_ui->lonEdit->text().toStdString().c_str());
    if(m_ui->lonEdit->text().isEmpty() ||
       lon < -180 || lon > 180)
    {
        if(!(flags&1))
           QMessageBox::critical(this, "Longitude", "Longitude must be between -180 (West) and 180 (East) in decimal degrees!");

        return false;
    }

    lat = atof(m_ui->latEdit->text().toStdString().c_str());
    if(m_ui->latEdit->text().isEmpty() ||
       lat < -90 || lat > 90)
    {
        if(!(flags&1))
           QMessageBox::critical(this, "Latitude", "Latitude must be between -90 (South) and 90 (North) in decimal degrees!");

        return false;
    }

    alt = atof(m_ui->altEdit->text().toStdString().c_str());
    if(m_ui->altEdit->text().isEmpty())
    {
        if(!(flags&1))
           QMessageBox::critical(this, "Altitude", "Altitude is in metric and it can't be empty!");

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void StationDialog::on_buttonBox_accepted()
{
    if(checkInput())
    {
        pqth->lon(m_ui->lonEdit->text());
        pqth->lat(m_ui->latEdit->text());
        pqth->alt(m_ui->altEdit->text());
        pqth->name(m_ui->nameEdit->text());

        writeStations();

        accept();
    }
}

//---------------------------------------------------------------------------
void StationDialog::on_buttonBox_rejected()
{
    reject();
}

//---------------------------------------------------------------------------
void StationDialog::on_addButton_clicked()
{
 QTreeWidgetItem *child;
 QTreeWidgetItem *item;
 QString str;
 double v;

   if(!checkInput())
       return;

   QList<QTreeWidgetItem *> list = m_ui->stationTree->findItems(m_ui->nameEdit->text(), Qt::MatchExactly);
   if(list.count()) {
       item = (QTreeWidgetItem *)list.at(0);
       while(item->childCount())
           item->removeChild(item->child(0));
   }
   else {
       item = new QTreeWidgetItem(QTreeWidgetItem::DontShowIndicatorWhenChildless);
       item->setText(0, m_ui->nameEdit->text());
   }

   v = atof(m_ui->lonEdit->text().toStdString().c_str());
   str.sprintf("Longitude: %g %s", fabs(v), v < 0 ? "West":"East");
   child = new QTreeWidgetItem(QTreeWidgetItem::DontShowIndicatorWhenChildless);
   child->setText(0, str);
   item->addChild(child);

   v = atof(m_ui->latEdit->text().toStdString().c_str());
   str.sprintf("Latitude: %g %s", fabs(v), v < 0 ? "South":"North");
   child = new QTreeWidgetItem(QTreeWidgetItem::DontShowIndicatorWhenChildless);
   child->setText(0, str);
   item->addChild(child);

   v = atof(m_ui->altEdit->text().toStdString().c_str());
   str.sprintf("Altitude: %g meter", v);
   child = new QTreeWidgetItem(QTreeWidgetItem::DontShowIndicatorWhenChildless);
   child->setText(0, str);
   item->addChild(child);

   m_ui->stationTree->addTopLevelItem(item);
   item->setExpanded(true);
}

//---------------------------------------------------------------------------
void StationDialog::on_stationTree_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
 QTreeWidgetItem *child;
 QStringList list;

  column = column;

  while(!item->childCount())
      item = m_ui->stationTree->itemAbove(item);

  m_ui->nameEdit->setText(item->text(0));

  child = item->child(0);
  list = child->text(0).split(" ");
  m_ui->lonEdit->setText(list.at(1));

  list.clear();
  child = item->child(1);
  list = child->text(0).split(" ");
  m_ui->latEdit->setText(list.at(1));

  list.clear();
  child = item->child(2);
  list = child->text(0).split(" ");
  m_ui->altEdit->setText(list.at(1));
}

//---------------------------------------------------------------------------
void StationDialog::on_stationTree_itemClicked(QTreeWidgetItem* item, int column)
{
   column = column;
   item = item;

   m_ui->delButton->setEnabled(true);
}

//---------------------------------------------------------------------------
void StationDialog::on_delButton_clicked()
{
 QList<QTreeWidgetItem *> list = m_ui->stationTree->selectedItems();
 QTreeWidgetItem *item;
 int i;

   for(i=0; i<list.count(); i++) {
      item = (QTreeWidgetItem *)list.at(i);
      while(!item->childCount())
         item = m_ui->stationTree->itemAbove(item);
      while(item->childCount())
         item->removeChild(item->child(0));

      delete item;
   }

   if(!m_ui->stationTree->topLevelItemCount())
       m_ui->delButton->setEnabled(false);
}

