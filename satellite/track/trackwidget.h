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
#ifndef TRACKWIDGET_H
#define TRACKWIDGET_H

#include <QtGui/QDockWidget>
#include <QDateTimeEdit>
namespace Ui {
    class TrackWidget;
}
//---------------------------------------------------------------------------
class QLabel;
class TSat;
class MainWindow;
class TrackThread;

//---------------------------------------------------------------------------
class TrackWidget : public QDockWidget {
    Q_OBJECT
public:
    TrackWidget(QWidget *parent = 0);
    ~TrackWidget();

    TSat   *getNextSatellite(void);
    TSat   *getSatellite(void);
    QLabel *getSatLabel(void);
    QLabel *getTimeLabel(void);
    QLabel *getSunLabel(void);
    QLabel *getMoonLabel(void);
    int    trackIndex(void);

    void updateSatCb(void);
    void restartThread(void);


protected:
    void changeEvent(QEvent *e);
    void deleteSat(void);
    void stopThread(void);
    void startThread(void);


private:
    Ui::TrackWidget *m_ui;
    TSat *sat;
    MainWindow *mw;
    TrackThread *thread;

private slots:
    void on_satcomboBox_currentIndexChanged(int index);
    void visibilityChanged(bool visible);

};

#endif // TRACKWIDGET_H
