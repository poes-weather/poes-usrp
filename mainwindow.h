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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>


namespace Ui
{
    class MainWindow;
}


//---------------------------------------------------------------------------
class QSettings;
class QLabel;
class QImage;

class THRPT;
class TBlock;

class PList;
class TStation;
class TSat;
class TSettings;
class TRig;

class ImageWidget;
class TrackWidget;
class TrackThread;
class GPSDialog;

//---------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool    countSats(int flags=0);
    QString getConfPath(void);
    QString getTLEPath(int type=0);

    TrackThread *thread;

    TBlock *getBlock(void) { return block; }

    bool renderImage(void);

    TSat      *getNextSat(void);
    TSat      *getNextSatByName(const QString &name);
    TSettings *getSettings(void);
    TRig      *getRig(void);
    PList     *getSatList(void);
    TStation  *getQTH(void) { return qth; }

    void updateQTH(void);

private slots:
     void on_actionSplit_CADU_to_file_triggered();
     void on_actionGPS_triggered();
     void on_actionRig_triggered();
     void on_actionActive_satellites_triggered();
     void on_actionPredict_triggered();
     void on_actionOrbit_data_triggered();
     void on_actionKeplerian_elements_triggered();
     void on_actionGroundstation_triggered();
     void on_actionSave_As_triggered();
     void on_actionOpen_triggered();

     void on_actionClose_triggered();

     void on_actionProperties_triggered();

protected:
     void closeEvent(QCloseEvent *event);
     bool processData(const char *filename, int blockType);
     void setCaption(const QString &filename = 0);

     void writeSettings(void);
     void readSettings(void);
     void writeSatelliteSettings(void);
     void readSatelliteSettings(void);

     void createPaths(void);
     bool mkpath(const QString &path, int flags=0);

     QString   getImageFormats(void);

private:
    Ui::MainWindow *ui;
    QAction *exitAct;

    QString FileName;
    QLabel *imageLabel;
    QImage *blockImage;


    TBlock    *block;
    PList     *satList;
    TStation  *qth;
    TSettings *settings;
    TRig      *rig;
    GPSDialog *gps;
    TSat      *opensat;

    TrackWidget *trackWidget;
    ImageWidget  *imageWidget;

};

#endif // MAINWINDOW_H
