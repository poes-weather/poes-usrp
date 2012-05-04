/*
    HRPT-Decoder, a software for processing NOAA-POES hig resolution weather satellite images.
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
#include <QSettings>
#include "trackwidget.h"
#include "ui_trackwidget.h"

#include "trackthread.h"

#include "mainwindow.h"
#include "Satellite.h"
#include "settings.h"
#include "plist.h"

//---------------------------------------------------------------------------
TrackWidget::TrackWidget(QWidget *parent) :
    QDockWidget(parent),
    m_ui(new Ui::TrackWidget)
{
    m_ui->setupUi(this);

    mw  = (MainWindow *) parent;
    sat = NULL;

    thread = new TrackThread(this);

    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(visibilityChanged(bool)));
}

//---------------------------------------------------------------------------
TrackWidget::~TrackWidget()
{

    stopThread();
    delete thread;
    deleteSat();

    delete m_ui;
}

//---------------------------------------------------------------------------
void TrackWidget::deleteSat(void)
{
    if(sat)
        delete sat;
    sat = NULL;
}

//---------------------------------------------------------------------------
void TrackWidget::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
TSat *TrackWidget::getSatellite(void)
{
    if(sat)
        sat->CheckThresholds(mw->getRig());

    return sat;
}

//---------------------------------------------------------------------------
QLabel *TrackWidget::getSatLabel(void)
{
    return m_ui->satLabel;
}

//---------------------------------------------------------------------------
QLabel *TrackWidget::getTimeLabel(void)
{
    return m_ui->timeLabel;
}

//---------------------------------------------------------------------------
QLabel *TrackWidget::getSunLabel(void)
{
    return m_ui->sunLabel;
}

//---------------------------------------------------------------------------
QLabel *TrackWidget::getMoonLabel(void)
{
    return m_ui->moonLabel;
}

//---------------------------------------------------------------------------
TSat *TrackWidget::getNextSatellite(void)
{
 QString str;
 TSat    *_sat;

    if(m_ui->satcomboBox->currentIndex() <= 0) // automatic
       _sat = mw->getNextSat(sat ? sat->lostime:0);
    else {
       if(sat && sat->name == m_ui->satcomboBox->currentText())
           return sat;

       _sat = mw->getNextSatByName(m_ui->satcomboBox->currentText());
    }

    if(!_sat)
        return NULL;

    deleteSat();
    sat = new TSat(_sat);
    sat->CheckThresholds(mw->getRig());

    str.sprintf("TLE Issued Date: %s", sat->GetKeplerAge().toStdString().c_str());
    m_ui->tleageLabel->setText(str);

 return sat;
}

//---------------------------------------------------------------------------
void TrackWidget::visibilityChanged(bool visible)
{
    if(visible)
    {
        startThread();
        // qDebug("TrackWidget is Visible");
    }
    else {
        stopThread();
        // qDebug("TrackWidget closed");
    }
}

//---------------------------------------------------------------------------
void TrackWidget::restartThread(void)
{
   startThread();
}

//---------------------------------------------------------------------------
void TrackWidget::startThread(void)
{
     stopThread();

     if(isVisible())
        if(getNextSatellite())
           thread->start(QThread::IdlePriority);
}

//---------------------------------------------------------------------------
void TrackWidget::stopThread(void)
{
    if(thread->isRunning()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        thread->stop();
        thread->wait();
        deleteSat();

        QApplication::restoreOverrideCursor();
   }
}

//---------------------------------------------------------------------------
void TrackWidget::on_satcomboBox_currentIndexChanged(int index)
{
    index = index;

    if(!thread->isRunning() || !this->isVisible())
        return;

    startThread();
}

//---------------------------------------------------------------------------
void TrackWidget::updateSatCb(void)
{
 QString str;
 PList *list, *list2;
 TSat  *sat;
 int   i, index;

    stopThread();

    index = 0;
    str = m_ui->satcomboBox->currentText();

    m_ui->satcomboBox->clear();
    m_ui->satcomboBox->addItem("Next");

    list = mw->getSatList();
    list2 = new PList;
    for(i=0; i<list->Count; i++) {
        sat = (TSat *) list->ItemAt(i);
        if(sat->sat_flags&SAT_DELETE)
            list2->Add(sat);
        else if(sat->isActive()) {
            m_ui->satcomboBox->addItem(sat->name);
            if(str == sat->name)
                index = m_ui->satcomboBox->count() - 1;
        }
    }

    while((sat = (TSat *) list2->Last())) {
        list2->Delete(sat);
        list->Delete(sat);
        delete sat;
    }

    delete list2;

    if(this->isVisible()) {
        if(index != m_ui->satcomboBox->currentIndex())
            m_ui->satcomboBox->setCurrentIndex(index);

        startThread();
    }
}
