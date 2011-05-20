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
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>

#include "satpassdialog.h"
#include "ui_satpassdialog.h"

#include "mainwindow.h"
#include "plist.h"
#include "Satellite.h"
#include "satutil.h"
#include "utils.h"
#include "rig.h"
#include "station.h"

#define AOS_COL_NR 2

//---------------------------------------------------------------------------
satpassdialog::satpassdialog(PList *_satList, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::satpassdialog)
{
 TSat *sat;
 int i, flags;
 double daynum = GetStartTime(QDateTime::currentDateTime().toUTC());;

    m_ui->setupUi(this);
    setLayout(m_ui->gridLayout);

    mw = (MainWindow *) parent;
    satList = _satList;

    flags = 0;
    for(i=0; i<satList->Count; i++) {
        sat = (TSat *) satList->ItemAt(i);

        sat->daynum = daynum;
        sat->Calc();
        if(sat->IsGeostationary() || sat->Decayed(0))
            continue;

        m_ui->satListWidget->addItem(sat->name);

        flags |= sat->isActive() ? 1:0;
    }

    m_ui->activeSatBtn->setVisible(flags ? true:false);
    m_ui->dateEdit->setDate(QDate::currentDate());

    if(flags)
        on_activeSatBtn_clicked();
    else if(m_ui->satListWidget->count())
        on_satListWidget_currentItemChanged(m_ui->satListWidget->item(0), NULL);
    else
        QMessageBox::critical(this, "No satellites found!", "Click menu Satellite->Keplerian elements to download.");
}

//---------------------------------------------------------------------------
satpassdialog::~satpassdialog()
{
    delete m_ui;
}

//---------------------------------------------------------------------------
void satpassdialog::changeEvent(QEvent *e)
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
void satpassdialog::on_activeSatBtn_clicked()
{
    QDateTime      utc  = getSelectedUTC();
    TRig           *rig = mw->getRig();
    TSat           *sat;
    int            i;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_ui->tableWidget->setSortingEnabled(false);
    clearGrid(m_ui->tableWidget);

    for(i=0; i<satList->Count; i++) {
        sat = (TSat *) satList->ItemAt(i);
        if(sat->isActive())
            sat->SatellitePasses(rig, m_ui->tableWidget, utc, 1);
    }

    m_ui->tableWidget->setSortingEnabled(true);
    m_ui->tableWidget->sortItems(AOS_COL_NR);

    QApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------
void satpassdialog::on_timecheckBox_clicked()
{
    QListWidgetItem *item = m_ui->satListWidget->currentItem();

    if(!item)
        on_activeSatBtn_clicked();
    else
        on_satListWidget_currentItemChanged(item, NULL);
}

//---------------------------------------------------------------------------
void satpassdialog::on_dateEdit_dateChanged(QDate /*date*/)
{
    on_activeSatBtn_clicked();
}

//---------------------------------------------------------------------------
QDateTime satpassdialog::getSelectedUTC(void)
{
    QDateTime local(m_ui->dateEdit->date());

    if(m_ui->timecheckBox->isChecked())
        local.setTime(QTime::currentTime());

#if 0
    QString str = local.toString();
    qDebug("%s, %s:%d", str.toStdString().c_str(), __FILE__, __LINE__);
#endif

 return local.toUTC();
}

//---------------------------------------------------------------------------
void satpassdialog::on_satListWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
 TSat *sat;

    if(!current || current->text().isEmpty())
        return;

    sat = getSat(satList, current->text());
    if(sat == NULL)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_ui->tableWidget->setSortingEnabled(false);
    sat->SatellitePasses(mw->getRig(), m_ui->tableWidget, getSelectedUTC());
    m_ui->tableWidget->setSortingEnabled(true);
    m_ui->tableWidget->sortItems(AOS_COL_NR);

    QApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------
void satpassdialog::on_satListWidget_itemClicked(QListWidgetItem* item)
{
    on_satListWidget_currentItemChanged(item, NULL);
}

//---------------------------------------------------------------------------
void satpassdialog::on_saveButton_clicked()
{
 QTableWidgetItem *item;
 TStation *qth = mw->getQTH();
 QDateTime dt = getSelectedUTC();
 QString str, fileName;
 FILE *fp;
 int row, col, width, i;

   dt = dt.toLocalTime();
   str.sprintf("%s/%s-satellite-passlist.txt",
               QDir::currentPath().toStdString().c_str(),
               dt.toString("yyyy-MM-dd").toStdString().c_str());

   fileName = QFileDialog::getSaveFileName(this, tr("Save Passlist"),
                                str,
                                tr("Text (*.txt)"));
   if(fileName.isEmpty())
       return;

   fp = fopen(fileName.toStdString().c_str(), "w");
   if(fp == NULL) {
       QMessageBox::critical(this, "Error: Failed to write to file!", fileName);
       return;
   }

   QApplication::setOverrideCursor(Qt::WaitCursor);

   fprintf(fp, "Satellite pass prediction @ station %s %g%s %g%s. Using local time.\n",
           qth->name().toStdString().c_str(),
           qth->lon(), qth->lon() < 0 ? "W":"E",
           qth->lat(), qth->lat() < 0 ? "S":"N");
   fprintf(fp, "Azimuth, longitude and latitude are calculated at satellite maximum elevation.\n");

   width = 0;
   for(row=0; row<m_ui->tableWidget->rowCount(); row++) {
       if((row % 10) == 0) {
           if(width == 0)
               fprintf(fp, "\n");
           else {
               for(i=0; i<width; i++)
                   fprintf(fp, "=");
               fprintf(fp, "\n");
           }

           for(col=i=0; col<m_ui->tableWidget->columnCount(); col++) {
               item = m_ui->tableWidget->horizontalHeaderItem(col);
               if(col < 2)
                   i += fprintf(fp, "%-25.25s", item->text().toStdString().c_str());
               else
                   i += fprintf(fp, "%-15.15s", item->text().toStdString().c_str());
           }

           if(width == 0)
               width = i;

           fprintf(fp, "\n");
           for(i=0; i<width; i++)
               fprintf(fp, "=");
           fprintf(fp, "\n");

       }

       for(col=0; col<m_ui->tableWidget->columnCount(); col++) {
           item = m_ui->tableWidget->item(row, col);

           if(col < 2)
               fprintf(fp, "%-25.25s", item->text().toStdString().c_str());
           else
               fprintf(fp, "%-15.15s", item->text().toStdString().c_str());
       }

       fprintf(fp, "\n");
   }


   fclose(fp);

   QApplication::restoreOverrideCursor();

   QMessageBox::information(this, "Information", "Satellite pass list saved to:\n\n" + fileName);
}

//---------------------------------------------------------------------------
