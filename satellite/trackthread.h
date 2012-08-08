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
#ifndef TRACKTHREAD_H
#define TRACKTHREAD_H

#include <QThread>
#include <QDateTimeEdit>
#include <stdio.h>
//---------------------------------------------------------------------------
#define     TF_STOP     1

class QLabel;
class QProcess;
class QDateTime;
class QStringList;
class MainWindow;
class TSat;
class TRig;
class TrackWidget;

//---------------------------------------------------------------------------
class TrackThread : public QThread
{
    Q_OBJECT

public:
    TrackThread(QObject *parent);
    ~TrackThread();

    void run();
    void stop();

signals:
    void setSatLabelColor(const QString &cl);
    void setSatLabelText(const QString &cl);
    void setTimeLabelText(const QString &cl);
    void setSunLabelColor(const QString &cl);
    void setSunLabelText(const QString &cl);
    void setMoonLabelColor(const QString &cl);
    void setMoonLabelText(const QString &cl);

protected:
    void stopProcess(QProcess *proc);
    bool procRunning(QProcess *proc);

    void initRotor(TRig *rig, TSat *sat);
    void moveTo(double az, double el);

private:
    TrackWidget *tw;
    MainWindow  *mw;
    TRig        *rig;
    TSat        *sat;
    QProcess    *rx_proc, *post_rx_proc;
    QStringList *proc_que;

    QLabel *satLabel, *timeLabel, *sunLabel, *moonLabel;

    QDateTime speed_dt;
    double prev_el, prev_az, sat_aos_azi;
    FILE *debug_fp;

    int flags;

};

#endif // TRACKTHREAD_H
