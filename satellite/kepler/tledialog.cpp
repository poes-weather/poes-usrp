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
#include <QtNetwork>
#include <QHttp>
#include <QUrl>
#include <QMessageBox>
#include <QFileDialog>
#include <stdio.h>

#include "tledialog.h"
#include "ui_tledialog.h"

#include "mainwindow.h"
#include "Satellite.h"
#include "satutil.h"
#include "plist.h"
#include "station.h"


//---------------------------------------------------------------------------
tledialog::tledialog(PList *list, TStation *_qth, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::tledialog)
{
    m_ui->setupUi(this);
    setLayout(m_ui->gridLayout_2);


    http = new QHttp(this);
    connect(http, SIGNAL(done(bool)), this, SLOT(saveTLE()));

    satList = new PList;
    satListptr = list;
    qth = _qth;

    tlepath = ((MainWindow *) parent)->getTLEPath();
    tlearcpath = ((MainWindow *) parent)->getTLEPath(1);
}

//---------------------------------------------------------------------------
tledialog::~tledialog()
{
    delete m_ui;

    if(http->state() != QHttp::Unconnected)
       http->close();

    clearSatList(satList, 1);
}

//---------------------------------------------------------------------------
void tledialog::changeEvent(QEvent *e)
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
void tledialog::on_downloadBtn_clicked()
{
    if(m_ui->urlCb->currentText().isEmpty() || http->hasPendingRequests())
        return;

    QUrl url(m_ui->urlCb->currentText());
    if(!url.isValid()) {
        QMessageBox::critical(this, "Invalid URL", url.errorString());
        return;
    }
    if(url.host().isEmpty()) {
       QMessageBox::critical(this, "No host in URL!", "Missing host name!");
       return;
    }
    if(url.path().isEmpty()) {
       QMessageBox::critical(this, "No path in URL!", "Missing path to file!");
       return;
    }

    qDebug(url.host().toStdString().c_str());
    qDebug(url.path().toStdString().c_str());

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_ui->downloadBtn->setEnabled(false);
    m_ui->fileBtn->setEnabled(false);
    m_ui->buttonBox->setEnabled(false);

    http->setHost(url.host());
    http->get(url.path());
}

//---------------------------------------------------------------------------
void tledialog::saveTLE()
{
 QString file;
 FILE *fp;
 int i;

    m_ui->downloadBtn->setEnabled(true);
    m_ui->fileBtn->setEnabled(true);
    m_ui->buttonBox->setEnabled(true);

    QApplication::restoreOverrideCursor();

    if(http->error() != QHttp::NoError) {
       QMessageBox::critical(this, "HTTP Error!", http->errorString());
       return;
    }

    QUrl url(m_ui->urlCb->currentText());
    QStringList list = url.path().split("/");

    file = tlepath + "/" + list.last();

    fp = fopen(file.toStdString().c_str(), "w");
    if(!fp) {
       QMessageBox::critical(this, "Failed to save TLE file!", file);
       return;
    }

    QByteArray array = http->readAll();
    for(i=0;i<array.count(); i++)
       fprintf(fp, "%c", array.at(i));    

    fclose(fp);

    if(readTLE(file) > 0)
       addToListWidget();
}

//---------------------------------------------------------------------------
int tledialog::readTLE(const QString &filename)
{
 FILE *fp;
 int count;

    fp = fopen(filename.toStdString().c_str(), "r");
    if(!fp) {
       QMessageBox::critical(this, "Failed to open TLE file!", filename);
       return 0;
    }

    count = ReadTLE(fp, satList);

    fclose(fp);

    if(count <= 0) {
       QMessageBox::critical(this, "No valid satellites found in TLE file!", filename);

       return 0;
    }

  return count;
}

//---------------------------------------------------------------------------
void tledialog::on_fileBtn_clicked()
{
 int i, count=0;


  QStringList files = QFileDialog::getOpenFileNames(
                          this,
                          "Select one or more TLE files to open",
                          tlepath,
                          "TLE Files (*.txt *.tle);;Any File (*.*)");

  for(i=0; i<files.count(); i++)
     count += readTLE(files.at(i));

  if(count > 0)
     addToListWidget();
}

//---------------------------------------------------------------------------
void tledialog::addToListWidget(void)
{
 TSat *sat, *sat2;
 int  i;

    m_ui->downloadList->clear();

    for(i=0; i<satList->Count; i++) {
        sat = (TSat *)satList->ItemAt(i);
        m_ui->downloadList->addItem(sat->name);

        if(!iteminlist(m_ui->updateList, sat->name)) {
           sat2 = getSat(satListptr, sat->name);
           if(sat2 && sat2->isActive())
              m_ui->updateList->addItem(sat->name);
        }
    }
}

//---------------------------------------------------------------------------
void tledialog::on_addButton_clicked()
{
 QListWidgetItem *item;
 int i;

    for(i=0; i<m_ui->downloadList->count(); i++) {
        item = m_ui->downloadList->item(i);
        if(item->isSelected())
           if(!iteminlist(m_ui->updateList, item->text()))
              m_ui->updateList->addItem(item->text());
    }
}

//---------------------------------------------------------------------------
bool tledialog::iteminlist(QListWidget *lw, const QString &str)
{
 QListWidgetItem *item;
 int i;

    for(i=0; i<lw->count(); i++) {
        item = lw->item(i);
        if(str == item->text())
            return true;
    }

 return false;
}

//---------------------------------------------------------------------------
void tledialog::on_delButton_clicked()
{
 QList<QListWidgetItem *> list;
 QListWidgetItem *item;
 int i;

    list = m_ui->updateList->selectedItems();
    for(i=0; i<list.count(); i++) {
        item = (QListWidgetItem *) list.at(i);
        delete item;
    }
}

//---------------------------------------------------------------------------
void tledialog::updateSatTLE()
{
 QListWidgetItem *item;
 QStringList     sl;
 TSat            *sat, *newsat;
 int  i;

    // remove possible duplicates
    for(i=0; i<m_ui->updateList->count(); i++) {
        item = m_ui->updateList->item(i);
        if(sl.indexOf(item->text()) == -1)
            sl.append(item->text());
    }

    for(i=0; i<sl.count(); i++) {
        newsat = getSat(satList, sl.at(i));
        if(!newsat)
            continue;
        sat = getSat(satListptr, sl.at(i));
        if(!sat) {
            sat = new TSat(newsat);
            sat->setActive(true);
            satListptr->Add(sat);
        }
        else {
            // archivate previous TLE
            if(strcmp(newsat->line1, sat->line1))
                archivate(newsat);

            sat->TLEKepCheck(newsat->name, newsat->line1, newsat->line2);
        }

        sat->AssignObsInfo(qth);

        if(sat->IsGeostationary() || sat->Decayed(0))
            sat->setActive(false);
        else
            sat->setActive(true);
    }
}

//---------------------------------------------------------------------------
void tledialog::archivate(TSat *sat)
{
    QString str, file;
    FILE    *fp;

    str = sat->name;
    str.replace(" ", "-");
    file = tlearcpath + "/" + str + ".tle";

    if((fp = fopen(file.toStdString().c_str(), "r"))) {
        while(fgets(sat->tmpstr, TLE_LINELEN, fp)) {
            // check if line 1 is found
            if(*sat->tmpstr == '1' && !strcmp(sat->tmpstr, sat->line1)) {
                fclose(fp);

                return;
            }
        }
        fclose(fp);
    }

    if((fp = fopen(file.toStdString().c_str(), "a"))) {
        fprintf(fp, "%s\n", sat->name);
        fprintf(fp, "%s\n", sat->line1);
        fprintf(fp, "%s\n", sat->line2);

        fclose(fp);
    }
}

//---------------------------------------------------------------------------
void tledialog::on_buttonBox_accepted()
{
    updateSatTLE();
}

