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
#include <QLabel>
#include <QWidget>
#include <QProcess>
#include <math.h>

#include "trackthread.h"
#include "trackwidget.h"

#include "mainwindow.h"
#include "Satellite.h"
#include "rig.h"


//---------------------------------------------------------------------------
TrackThread::TrackThread(QObject *parent) : QThread(parent)
{
    tw  = (TrackWidget *) parent;
    mw  = (MainWindow *) tw->parent();
    rig = mw->getRig();

    sat = NULL;

    rx_proc      = new QProcess(this);
    post_rx_proc = new QProcess(this);
    proc_que     = new QStringList;

    satLabel = tw->getSatLabel();
    connect(this, SIGNAL(setSatLabelColor(const QString &)),
            satLabel, SLOT(setStyleSheet(const QString &)));
    connect(this, SIGNAL(setSatLabelText(const QString &)),
            satLabel, SLOT(setText(const QString &)));

    timeLabel = tw->getTimeLabel();
    connect(this, SIGNAL(setTimeLabelText(const QString &)),
            timeLabel, SLOT(setText(const QString &)));

    sunLabel = tw->getSunLabel();
    connect(this, SIGNAL(setSunLabelColor(const QString &)),
            sunLabel, SLOT(setStyleSheet(const QString &)));
    connect(this, SIGNAL(setSunLabelText(const QString &)),
            sunLabel, SLOT(setText(const QString &)));

    moonLabel = tw->getMoonLabel();
    connect(this, SIGNAL(setMoonLabelColor(const QString &)),
            moonLabel, SLOT(setStyleSheet(const QString &)));
    connect(this, SIGNAL(setMoonLabelText(const QString &)),
            moonLabel, SLOT(setText(const QString &)));
}

//---------------------------------------------------------------------------
TrackThread::~TrackThread()
{
    stopProcess(rx_proc);
    stopProcess(post_rx_proc);

    delete rx_proc;
    delete post_rx_proc;
    delete proc_que;
}

//---------------------------------------------------------------------------
void TrackThread::stop(void)
{
    if(isRunning())
        flags |= TF_STOP;
}

//---------------------------------------------------------------------------
void TrackThread::run()
{
    int          sat_state; // 0 = init, 1 = tracking, 2 = LOS, 3 = idle, 4 = reinit
    unsigned int rig_modes, loop_index;

    /*

     rig_modes bitmap

                                ========================
                                STATIC RIG & ROTOR MODES
                                ========================
                                  &1 = rotor is enabled
                                  &2 = rotor parking is enabled
                                  &4 = auto record
                                  &8 = use passthresholds


                                ===================
                                DYNAMIC ROTOR MODES
                                ===================
                                 &32 = rotor is inited, pointing at AOS
                                 &64 = rotor is parked
                                &128 = recording is inited and enabled
                                &256 = rx script executed
                                &512 = rx script inited

     */


    QDateTime  now;
    QString    cl_down = "color:rgb(0, 170, 255);";
    QString    cl_up   = "color:yellow;";
    QString    cl_style, proc_cmd, dt_str;
    bool       script_error;
    // long       l1, l2;
    double     v1, v2, post_proc_start_time;

    flags = 0;
    sat_state = 0;
    loop_index = 0;
    post_proc_start_time = 0;

    // init rig & rotor static modes
    rig_modes = 0;

    rig_modes |= (rig->rotor->enable() && rig->rotor->openPort()) ? 1:0;
    rig_modes |= rig->rotor->parkingEnabled() ? 2:0;
    rig_modes |= rig->autorecord() ? 4:0;
    rig_modes |= rig->passthresholds() ? 8:0;

    sat = tw->getSatellite();
    while(!(flags & TF_STOP)) {

        now = QDateTime::currentDateTime();                
        dt_str = now.toString("dddd, d MMMM yyyy, hh:mm:ss");
        emit(setTimeLabelText(dt_str));

        if(!sat) {
            if(!(sat = tw->getNextSatellite())) {
                emit(setSatLabelText("No active satellites found to track @ " + dt_str + ", terminating!"));

                break;
            }
        }

        sat->Track();

        // check every now and then if the post rx process can be stopped and deque
        if((loop_index % 20) == 0 && procRunning(post_rx_proc)) {
            v1 = (sat->daynum - post_proc_start_time) * 1440;
            if(v1 > 20) // it has been running over 20 mins, stop it!
                stopProcess(post_rx_proc);

            if(proc_que->count() && !procRunning(post_rx_proc)) {
                proc_cmd = proc_que->first();
                proc_que->removeFirst();

                post_rx_proc->start(proc_cmd);
                post_proc_start_time = sat->daynum;
            }
        }

        switch(sat_state) {
        case 0: // init state, loop here until satellite is at AOS
            {
                // check if it can be recorded, 128 | 4
                if((rig_modes & 4) && !(rig_modes & 128) && sat->CanRecord())
                    rig_modes |= 128;

                // init rotor
                if((rig_modes & 1) && !(rig_modes & 32)) {
                    if(!sat->CanRecord()) {
                        if(rig_modes & 2)
                            rig->rotor->park();

                        rig_modes |= 32 | 64; // do nothing
                    }
                    // antenna parking
                    else if(rig_modes & 2) {
                        // park antenna if the satellite is >15 min from AOS time
                        v1 = (rig_modes & 8) ? sat->rec_aostime:sat->aostime;
                        v2 = (v1 - sat->daynum) * 1440;
                        if(v2 > 15) {
                            if(!(rig_modes & 64)) {
                                rig->rotor->park();
                                rig_modes |= 64;
                            }
                        }
                        else {
                            // move it to AOS from parked position
                            rig_modes &= ~64;
                            rig_modes |= 32;
                        }
                    }
                    else
                        rig_modes |= 32;

                    if((rig_modes & 32) && !(rig_modes & 64))
                        initRotor(rig, sat);
                }

                // everything should now be inited, wait for it to rise
                sat_state = sat->sat_ele > 0 ? 1:0;
            }
            break;

        case 1: // satellite tracking- and second init state
            {
                // swing the dish
                //if((rig_modes & 1) && ((rig_modes & 128) || sat->CanRecord()))
                if(rig_modes & 1)
                    rig->rotor->moveTo(sat->sat_azi, sat->sat_ele);

                // start the rx script
                if((rig_modes & 128) && !(rig_modes & 512) && sat->CanStartRecording(rig)) {

                    stopProcess(rx_proc); // kill it if it is alive!
                    proc_cmd = sat->sat_scripts->get_rx_command(sat->name, sat->getDownlinkFreq(rig), &script_error);
                    if(!script_error) {
                        rx_proc->start(proc_cmd);
                        sat->SavePassinfo();
                        rig_modes |= 256;
                    }
                    else {
                        // make sure it wont be tested again until user corrects errors
                        sat->sat_scripts->rx_srcrip_enable(false);
                        sat->sat_scripts->postproc_srcrip_enable(false);

                        qDebug("Error: %s", sat->GetImageText(256).toStdString().c_str());
                        qDebug("Error: rx script %s had fatal errors, disabling scripts! %s:%d",
                               sat->sat_scripts->rx_script().toStdString().c_str(),
                               __FILE__, __LINE__);
                    }

                    rig_modes |= 512;
                }


                v1 = sat->get_range_rate();

                // satellite is receding, start checking if a new state can be set
                if(v1 > 0) {
                    if(sat->sat_ele <= 0)
                        sat_state = 2;
                    else {
                        if((rig_modes & 8) && sat->CanStopRecording())
                            sat_state = 2;
                    }
                }

            }
            break;

        case 2: // satellite receded below LOS, post RX process and start to deinitialize
            {
                if(rig_modes & 256) {
                    stopProcess(rx_proc); // dont check its pid, user might have killed it...

                    if(sat->sat_scripts->postproc_srcrip_enable()) {
                        proc_cmd = sat->sat_scripts->get_postproc_command(&script_error);

                        if(!script_error) {
                            if(!procRunning(post_rx_proc)) {
                                post_rx_proc->start(proc_cmd);
                                post_proc_start_time = sat->daynum;
                            }
                            else {
                                qDebug("Queuing: %s", sat->GetImageText(256).toStdString().c_str());

                                proc_que->append(proc_cmd);
                            }
                        }
                        else {
                            // make sure it wont be tested again until user corrects errors
                            sat->sat_scripts->rx_srcrip_enable(false);
                            sat->sat_scripts->postproc_srcrip_enable(false);

                            qDebug("Error: %s", sat->GetImageText(256).toStdString().c_str());
                            qDebug("Error: post rx script %s had fatal errors, disabling scripts! %s:%d",
                                   sat->sat_scripts->postproc_script().toStdString().c_str(),
                                   __FILE__, __LINE__);
                        }
                    }
                }

#if 1
                sat_state = 3;
#else
                // get next satellite pass
                l1 = sat->catnum;
                l2 = sat->orbitnum;

                if((sat = tw->getNextSatellite())) {
                    sat->Track();

                    // check if it is the same satellite
                    if(sat->sat_ele > 0 && sat->catnum == l1 && l2 == sat->orbitnum)
                        sat_state = 3;
                    else
                        sat_state = 4;
                }
                else
                    sat_state = -999;
#endif

            }
            break;

        case 3: // idle state, wait for satellite to reced below qth horizon
            {
                sat_state = sat->sat_ele > 0 ? 3:4;
            }
            break;

        case 4: // start from the beginning
            {

                if((sat = tw->getNextSatellite()))
                    sat->Track();

                sat_state = 0;

                // delete all bits except the static ones (1 | 2 | 4 | 8)
                rig_modes &= ~0xFFFFFFF0;
            }
            break;

        default:
            break;
        }

        if(!sat) { // fatal error
            qDebug("Error: No more active satellites found @ %s, terminating! %s:%d",
                   dt_str.toStdString().c_str(),
                   __FILE__, __LINE__);

            emit(setSatLabelText("No more active satellites found to track @ " + dt_str + ", terminating!"));

            break;
        }

        // satellite label
        cl_style = sat->sat_ele > 0 ? cl_up:cl_down;
        if(satLabel->styleSheet() != cl_style)
            emit(setSatLabelColor(cl_style));
        emit(setSatLabelText(sat->GetTrackStr(rig, rx_proc->pid() ? 1:0)));


        // update sun- and moon position every 10 sec
        if((loop_index % 20) == 0) {
            // sun label
            // use dusk elevation as up threshold
            cl_style = sat->sun_ele >= -6 ? cl_up:cl_down;
            if(sunLabel->styleSheet() != cl_style)
                emit(setSunLabelColor(cl_style));
            emit(setSunLabelText(sat->GetSunPos()));

            // moon label
            sat->FindMoon(sat->daynum);
            cl_style = sat->moon_ele > 0 ? cl_up:cl_down;
            if(moonLabel->styleSheet() != cl_style)
                emit(setMoonLabelColor(cl_style));
            emit(setMoonLabelText(sat->GetMoonPos()));
        }

        loop_index++;
        msleep(500);
    }

    flags |= TF_STOP;

    // stop the rx script so it wont fill the disk
    // let the post rx script run
    stopProcess(rx_proc);

  exit();
}

//---------------------------------------------------------------------------
void TrackThread::stopProcess(QProcess *proc)
{
    if(proc->pid()) {
        proc->kill();
        proc->waitForFinished();

        QDateTime now(QDateTime::currentDateTime());

        qDebug("Process stopped @ %s, %s:%d",
               now.toString().toStdString().c_str(),
               __FILE__, __LINE__);
    }
}

//---------------------------------------------------------------------------
// notice: it can also be in error state
bool TrackThread::procRunning(QProcess *proc)
{
    return proc->pid() ? true:false;
}

//---------------------------------------------------------------------------
void TrackThread::initRotor(TRig *rig, TSat *sat)
{
    double aos_az, los_az, sat_ele;
    int    *rotor_flags_ptr;

    sat_ele = sat->sat_ele;

    if(sat_ele >= 0)
        aos_az = sat->sat_azi;
    else {
        sat->daynum = rig->passthresholds() ? sat->rec_aostime:sat->aostime;
        sat->Calc();
        aos_az = sat->sat_azi;
    }

    if(rig->rotor->rotor_type == RotorType_GS232B)
        rotor_flags_ptr = &rig->rotor->gs232b->flags;
    else if(rig->rotor->rotor_type == RotorType_SPID)
        rotor_flags_ptr = &rig->rotor->spid->flags;
    else
        rotor_flags_ptr = NULL;

    if(rotor_flags_ptr) {
        *rotor_flags_ptr &= ~R_ROTOR_CCW;

        sat->daynum = rig->passthresholds() ? sat->rec_lostime:sat->lostime;
        sat->Calc();
        los_az = sat->sat_azi;

        if(sat->isNorthbound()) {
            if(aos_az < 180 && los_az > 270)                
                *rotor_flags_ptr |= R_ROTOR_CCW;
        }
        else {
            if(aos_az > 180 && los_az < 90)
                *rotor_flags_ptr |= R_ROTOR_CCW;
        }
    }

    rig->rotor->moveTo(aos_az, sat_ele < 0 ? 0:sat_ele);

    sat->Track();
}

//---------------------------------------------------------------------------
