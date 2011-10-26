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
#ifndef RIGDIALOG_H
#define RIGDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class RigDialog;
}

class QListWidgetItem;
class QComboBox;
class RotorPinDialog;
class MainWindow;
class TRig;

//---------------------------------------------------------------------------
class RigDialog : public QDialog {
    Q_OBJECT
public:
    RigDialog(QWidget *parent = 0);
    ~RigDialog();

protected:
    void changeEvent(QEvent *e);
    void setRotorSteps(QComboBox *cb, int steps);
    int  getRotorSteps(QComboBox *cb);
    void applyRotorSettings(void);
    void moveAntenna(void);
    void enableControls(void);
    void setRotorControlLimits(void);
    void calibrateJrk(bool azimuth);


private:
    Ui::RigDialog *m_ui;
    MainWindow *mw;
    TRig *rig;
    RotorPinDialog *rotorPins;

    int flags; // &1 = trigger no events
    bool rotorEnabled;

private slots:

    void on_jrkStatusBtn_clicked();
    void on_jrkClrErrorsBtn_clicked();
    void on_calibrateJrkElButton_clicked();
    void on_calibrateJrkAzButton_clicked();
    void on_rotorstopButton_clicked();
    void on_rotorTypeCb_currentIndexChanged(int index);
    void on_applyRotorBtn_clicked();
    void on_az_spinBox_valueChanged(double value);
    void on_el_spinBox_valueChanged(double value);
    void on_checkBoxAzCCW_clicked();
    void on_checkBoxElCCW_clicked();
    void on_buttonBox_rejected();
    void on_rotorZeroPosBtn_clicked();
    void on_az_dial_sliderReleased();
    void on_az_dial_sliderPressed();
    void on_el_dial_sliderReleased();
    void on_el_dial_sliderPressed();
    void on_rotorElPinSetup_clicked();
    void on_rotorAzPinSetup_clicked();
    void on_thresholdTypeCb_currentIndexChanged(int index);
    void on_az_dial_valueChanged(int value);
    void on_el_dial_valueChanged(int value);
    void on_listWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void on_buttonBox_accepted();
    void on_monstrumStatusBtn_clicked();
    void on_wobbleBtn_clicked();
};

#endif // RIGDIALOG_H
