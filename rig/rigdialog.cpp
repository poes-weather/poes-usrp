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
#include <QtGlobal>
#include <QListWidget>
#include <QProcess>
#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>

#include "rigdialog.h"
#include "ui_rigdialog.h"
#include "rotorpindialog.h"
#include "mainwindow.h"
#include "utils.h"
#include "rig.h"
#include "jrkconfdialog.h"
#include "azeldialog.h"

//---------------------------------------------------------------------------
RigDialog::RigDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::RigDialog)
{

    m_ui->setupUi(this);
    setLayout(m_ui->mainLayout);

    rotorPins = new RotorPinDialog(this);

    mw = (MainWindow *) parent;

    rig = mw->getRig();

    flags = 256; // prevent rotor from turning

    // downconverter
    m_ui->lband_lo->setValue(rig->dc_lo_freq[DC_LO_L_BAND]);
    m_ui->sband_lo->setValue(rig->dc_lo_freq[DC_LO_S_BAND]);
    m_ui->cband_lo->setValue(rig->dc_lo_freq[DC_LO_C_BAND]);
    m_ui->xband_lo->setValue(rig->dc_lo_freq[DC_LO_X_BAND]);

    // usrp
    m_ui->autorecordCb->setChecked(rig->autorecord());

    // pass thresholds    
    m_ui->enableThresholds->setChecked(rig->passthresholds());

    m_ui->thresholdTypeCb->setCurrentIndex((int) rig->threshold);
    m_ui->aos_spinBox->setValue(rig->aos_elev);
    m_ui->passEl_spinBox->setValue(rig->pass_elev);
    m_ui->los_spinBox->setValue(rig->los_elev);

    // rotor
    m_ui->enableRotor->setChecked(rig->rotor->enable());
    // disable rotor so the thread wont turn it while tweaking its settings
    rotorEnabled = rig->rotor->enable();
    rig->rotor->enable(false);

    m_ui->rotorTypeCb->setCurrentIndex((int) rig->rotor->rotor_type);
    m_ui->commtypeCb->setCurrentIndex((int) rig->rotor->commtype);
    m_ui->hostEd->setText(rig->rotor->host);
    m_ui->hostPortEd->setValue(rig->rotor->port);

    // rotor limits
    m_ui->maxAzSb->setValue(rig->rotor->az_max);
    m_ui->minAzSb->setValue(rig->rotor->az_min);
    m_ui->maxElSb->setValue(rig->rotor->el_max);
    m_ui->minElSb->setValue(rig->rotor->el_min);
    setRotorControlLimits();

    // rotor parking
    m_ui->parkCb->setChecked(rig->rotor->parkingEnabled());
    m_ui->parkElSp->setValue(rig->rotor->parkEl);
    m_ui->parkAzSp->setValue(rig->rotor->parkAz);

    // control
    rig->rotor->openPort();

    m_ui->az_dial->setValue(rig->rotor->getAzimuth());
    m_ui->az_spinBox->setValue(rig->rotor->getAzimuth());
    m_ui->el_dial->setValue(rig->rotor->getElevation());
    m_ui->el_spinBox->setValue(rig->rotor->getElevation());

    // stepper rotor settings
    m_ui->rotorPort->setValue(rig->rotor->stepper->rotor_address);
    m_ui->rotorDelay->setValue(rig->rotor->stepper->rotor_speed);
    setRotorSteps(m_ui->azStepCb, rig->rotor->stepper->rotor_spr_az);
    setRotorSteps(m_ui->elStepCb, rig->rotor->stepper->rotor_spr_el);
    m_ui->azRatiospinBox->setValue(rig->rotor->stepper->rotor_az_ratio);
    m_ui->elRatiospinBox->setValue(rig->rotor->stepper->rotor_el_ratio);

    m_ui->checkBoxElCCW->setChecked(rig->rotor->stepper->rotor_el_pin[2] & R_ROTOR_CCW ? true:false);
    m_ui->checkBoxAzCCW->setChecked(rig->rotor->stepper->rotor_az_pin[2] & R_ROTOR_CCW ? true:false);

    // gs-232b rotor settings
    m_ui->serialPortEd->setText(rig->rotor->gs232b->deviceId);
    m_ui->gsSpeedCb->setCurrentIndex((int) rig->rotor->gs232b->speed - 1);

    // Monstrum rotor settings
    m_ui->monstrumPort->setText(rig->rotor->monster->deviceId);

    // AlphaSpid rotor settings
    m_ui->spidPorted->setText(rig->rotor->spid->deviceId);

    // Pololu Jrk settings
    m_ui->jrkAzPortEd->setText(rig->rotor->jrk->az_deviceId);
    m_ui->jrkAzIdEd->setValue(rig->rotor->jrk->az_id);
    m_ui->jrkElPortEd->setText(rig->rotor->jrk->el_deviceId);
    m_ui->jrkElIdEd->setValue(rig->rotor->jrk->el_id);

#if 0
    // Oak USB
    m_ui->enableOakSensor->setChecked(rig->flags&R_OAK_ENABLE ? true:false);
    m_ui->oakhiddev->setText(rig->oak_device);

#if !defined(Q_OS_UNIX)
    m_ui->oakReadBtn->setVisible(false);
#endif
#endif

    enableControls();

    /*  TODO: document flags bitmap

     flags   &1 =
             &2 =
            &16 = smooth rotor control
           &256 =

    */

    flags = 0;
    flags |= rig->rotor->rotor_type == RotorType_JRK ? 16:0;
}

//---------------------------------------------------------------------------
RigDialog::~RigDialog()
{
    delete m_ui;
    delete rotorPins;
}

//---------------------------------------------------------------------------
void RigDialog::changeEvent(QEvent *e)
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
void RigDialog::enableControls(void)
{
    m_ui->stepper_label_1->setText(rig->rotor->getRotorName());

    bool enb = rig->rotor->rotor_type == RotorType_Stepper ? true:false;

    m_ui->checkBoxElCCW->setEnabled(enb);
    m_ui->checkBoxAzCCW->setEnabled(enb);

    if(rig->rotor->rotor_type == RotorType_Stepper ||
       rig->rotor->rotor_type == RotorType_JRK)
        m_ui->rotorZeroPosBtn->setEnabled(true);
    else
        m_ui->rotorZeroPosBtn->setEnabled(false);

    m_ui->rotorstopButton->setEnabled(!enb);
}

//---------------------------------------------------------------------------
void RigDialog::on_buttonBox_accepted()
{
    // downconverter
    rig->dc_lo_freq[DC_LO_L_BAND] = m_ui->lband_lo->value();
    rig->dc_lo_freq[DC_LO_S_BAND] = m_ui->sband_lo->value();
    rig->dc_lo_freq[DC_LO_C_BAND] = m_ui->cband_lo->value();
    rig->dc_lo_freq[DC_LO_X_BAND] = m_ui->xband_lo->value();

    // usrp
    rig->autorecord(m_ui->autorecordCb->isChecked());

    // pass thresholds
    rig->passthresholds(false);
    if(m_ui->passEl_spinBox->value() > m_ui->aos_spinBox->value() &&
       m_ui->passEl_spinBox->value() > m_ui->los_spinBox->value())
       rig->passthresholds(m_ui->enableThresholds->isChecked());

    rig->threshold = (PassThresholdType_t) m_ui->thresholdTypeCb->currentIndex();
    rig->aos_elev  = m_ui->aos_spinBox->value();
    rig->pass_elev = m_ui->passEl_spinBox->value();
    rig->los_elev  = m_ui->los_spinBox->value();

    if(rig->pass_elev < 1)
        rig->passthresholds(false);

#if 0
    // Oak
    rig->flags &= ~R_OAK_ENABLE;
    rig->oak_device = m_ui->oakhiddev->text();
    if(!rig->oak_device.isEmpty())
        rig->flags |= m_ui->enableOakSensor->isChecked() ? R_OAK_ENABLE:0;
#endif

    // rotor
    rig->rotor->closePort();
    applyRotorSettings();
    rig->rotor->enable(m_ui->enableRotor->isChecked());

    if(rig->rotor->enable())
        if(!rig->rotor->openPort())
            rig->rotor->enable(false);

}

//---------------------------------------------------------------------------
void RigDialog::on_buttonBox_rejected()
{
    rig->rotor->rig->rotor->enable(rotorEnabled);
}

//---------------------------------------------------------------------------
void RigDialog::on_listWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    m_ui->stackedWidget->setCurrentIndex(m_ui->listWidget->currentRow());
    current = current;
    previous = previous;
}

//---------------------------------------------------------------------------
void RigDialog::on_el_dial_sliderPressed()
{
    if(!(flags & 16))
        flags |= 2;
}

//---------------------------------------------------------------------------
void RigDialog::on_el_dial_sliderReleased()
{
    if(!(flags & 16))
        moveAntenna();

    flags &= ~2;
}

//---------------------------------------------------------------------------
void RigDialog::on_az_dial_sliderPressed()
{
    if(!(flags & 16))
        flags |= 2;
}

//---------------------------------------------------------------------------
void RigDialog::on_az_dial_sliderReleased()
{
    if(!(flags & 16))
        moveAntenna();

    flags &= ~2;
}

//---------------------------------------------------------------------------
void RigDialog::on_az_dial_valueChanged(int value)
{
    if(flags & 1)
        return;

    QString str;

    flags |= 1;

    str.sprintf("%s: %d", rig->rotor->isXY() ? "X":"Azimuth", value);
    m_ui->az_dial->setToolTip(str);
    m_ui->az_spinBox->setValue(value);

    if(!(flags & 2))
       moveAntenna();

    flags &= ~1;
}

//---------------------------------------------------------------------------
void RigDialog::on_az_spinBox_valueChanged(double value)
{
    if(flags & 1)
        return;

    QString str;

    flags |= 1;

    str.sprintf("%s: %.3f", rig->rotor->isXY() ? "X":"Azimuth", value);
    m_ui->az_dial->setToolTip(str);
    m_ui->az_dial->setValue((int) rint(m_ui->az_spinBox->value()));

    flags &= ~1;

    if(!(flags & 2))
       moveAntenna();
}

//---------------------------------------------------------------------------
void RigDialog::on_el_dial_valueChanged(int value)
{
    if(flags & 1)
        return;

    QString str;

    flags |= 1;

    str.sprintf("%s: %d", rig->rotor->isXY() ? "Y":"Elevation", value);
    m_ui->el_dial->setToolTip(str);
    m_ui->el_spinBox->setValue(value);

    flags &= ~1;

    if(!(flags & 2))
       moveAntenna();
}

//---------------------------------------------------------------------------
void RigDialog::on_el_spinBox_valueChanged(double value)
{
    if(flags & 1)
        return;

    QString str;

    flags |= 1;

    str.sprintf("%s: %.3f", rig->rotor->isXY() ? "Y":"Elevation", value);
    m_ui->el_dial->setToolTip(str);
    m_ui->el_dial->setValue((int) rint(m_ui->el_spinBox->value()));

    flags &= ~1;

    if(!(flags & 2))
        moveAntenna();
}

//---------------------------------------------------------------------------
void RigDialog::moveAntenna(void)
{
 unsigned long wait_ms;

    if(flags & 256) // constructor or apply zero position
        return;

    if(!rig->rotor->isPortOpen()) {
       on_applyRotorBtn_clicked();
       if(!rig->rotor->isPortOpen())
           return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    wait_ms = rig->rotor->getRotationTime(m_ui->az_spinBox->value(), m_ui->el_spinBox->value());
    rig->rotor->moveTo(m_ui->az_spinBox->value(), m_ui->el_spinBox->value());

    //qDebug("Wait %d ms", (int) wait_ms);
    delay(wait_ms);

    QApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------
void RigDialog::on_rotorZeroPosBtn_clicked()
{
    AzElDialog *dlg = new AzElDialog(this);

    dlg->setlimits(rig->rotor->az_min, rig->rotor->az_max, true);
    dlg->setlimits(rig->rotor->el_min, rig->rotor->el_max, false);

    if(dlg->exec()) {
        flags |= (256 | 2);

        rig->rotor->setAzimuth(dlg->azimuth());
        rig->rotor->setElevation(dlg->elevation());

        m_ui->az_dial->setValue(dlg->azimuth());
        m_ui->el_dial->setValue(dlg->elevation());

        flags &= ~(256 | 2);
    }

    delete dlg;
}

//---------------------------------------------------------------------------
void RigDialog::on_rotorstopButton_clicked()
{
    rig->rotor->stopMotor();
}

//---------------------------------------------------------------------------

#if 0
void RigDialog::on_oakReadBtn_clicked()
{
#if defined(Q_OS_UNIX)

    if(!rig->openOak())
        return;

    flags |= 1;

    if(rig->readAzEl()) {
        //m_ui->az_dial->setValue((int) rint(rig->oakAz));
        //m_ui->el_dial->setValue((int) rint(rig->oakEl));
        m_ui->az_spinBox->setValue(rig->oakAz);
        m_ui->el_spinBox->setValue(rig->oakEl);
    }

    flags &= ~1;

#endif
}
#endif

//---------------------------------------------------------------------------
//
//  Satellite pass thresholds
//
//---------------------------------------------------------------------------
void RigDialog::on_thresholdTypeCb_currentIndexChanged(int index)
{
    if(index == 0) {
        m_ui->aosLabel->setText("AOS Elevation:");
        m_ui->losLabel->setText("LOS Elevation:");
    }
    else {
        m_ui->aosLabel->setText("North elevation:");
        m_ui->losLabel->setText("South elevation:");
    }
}

//---------------------------------------------------------------------------
//
//  Rotor settings
//
//---------------------------------------------------------------------------
void RigDialog::on_applyRotorBtn_clicked()
{
    bool rc;

    rig->rotor->closePort();
    applyRotorSettings();

    rc = rig->rotor->openPort();

    if(!rc) {
        QMessageBox::critical(this, "Error: Communication failure!", rig->rotor->getErrorString());

        rig->rotor->closePort();
    }
    else {
        flags |= 256;

        on_el_spinBox_valueChanged(rig->rotor->getElevation());
        m_ui->el_spinBox->setValue(rig->rotor->getElevation());
        on_az_spinBox_valueChanged(rig->rotor->getAzimuth());
        m_ui->az_spinBox->setValue(rig->rotor->getAzimuth());

        flags &= ~256;

        if(!(flags & 1024))
            QMessageBox::information(this, "Port is open", "Communication was successfully established!");
    }
}

//---------------------------------------------------------------------------
void RigDialog::applyRotorSettings(void)
{
    rig->rotor->rotor_type = (TRotorType_t) m_ui->rotorTypeCb->currentIndex();
    rig->rotor->enable(m_ui->enableRotor->isChecked());

    rig->rotor->commtype = (TCommType) m_ui->commtypeCb->currentIndex();
    rig->rotor->host = m_ui->hostEd->text();
    rig->rotor->port = m_ui->hostPortEd->value();

    // rotor limits
    rig->rotor->az_max = m_ui->maxAzSb->value();
    rig->rotor->el_max = m_ui->maxElSb->value();
    rig->rotor->az_min = m_ui->minAzSb->value();
    rig->rotor->el_min = m_ui->minElSb->value();

    setRotorControlLimits();
    flags &= ~16;
    flags |= rig->rotor->rotor_type == RotorType_JRK ? 16:0;


    // rotor parking
    rig->rotor->parkingEnabled(m_ui->parkCb->isChecked());
    rig->rotor->parkEl = m_ui->parkElSp->value();
    rig->rotor->parkAz = m_ui->parkAzSp->value();

    // gs-232b rotor settings
    rig->rotor->gs232b->deviceId = m_ui->serialPortEd->text();
    rig->rotor->gs232b->speed    = (TGS232B_Speed_t) (m_ui->gsSpeedCb->currentIndex() + 1);

    // alphaspid settings
    rig->rotor->spid->deviceId = m_ui->spidPorted->text();

    // monstrum
    rig->rotor->monster->deviceId = m_ui->monstrumPort->text();

    // stepper settings
    rig->rotor->stepper->rotor_address = m_ui->rotorPort->value();
    rig->rotor->stepper->rotor_speed   = m_ui->rotorDelay->value();
    rig->rotor->stepper->rotor_spr_az  = getRotorSteps(m_ui->azStepCb);
    rig->rotor->stepper->rotor_spr_el  = getRotorSteps(m_ui->elStepCb);
    rig->rotor->stepper->rotor_az_ratio = m_ui->azRatiospinBox->value();
    rig->rotor->stepper->rotor_el_ratio = m_ui->elRatiospinBox->value();

    // jrk settings
    rig->rotor->jrk->az_deviceId = m_ui->jrkAzPortEd->text();
    rig->rotor->jrk->az_id       = m_ui->jrkAzIdEd->value();
    rig->rotor->jrk->el_deviceId = m_ui->jrkElPortEd->text();
    rig->rotor->jrk->el_id       = m_ui->jrkElIdEd->value();

    if(rig->rotor->rotor_type == RotorType_Stepper) {
        m_ui->az_spinBox->setSingleStep((360.0 / (double) rig->rotor->stepper->rotor_spr_az) / ((double) rig->rotor->stepper->rotor_az_ratio));
        m_ui->el_spinBox->setSingleStep((360.0 / (double) rig->rotor->stepper->rotor_spr_el) / ((double) rig->rotor->stepper->rotor_el_ratio));
    }
    else if(rig->rotor->rotor_type == RotorType_SPID) {
        m_ui->az_spinBox->setSingleStep(rig->rotor->spid->PH == 2 ? 0.5:1);
        m_ui->el_spinBox->setSingleStep(rig->rotor->spid->PV == 2 ? 0.5:1);
    }
    else if(rig->rotor->rotor_type == RotorType_JRK ||
            rig->rotor->rotor_type == RotorType_Monstrum) {
        m_ui->az_spinBox->setSingleStep(0.01);
        m_ui->el_spinBox->setSingleStep(0.01);
    }
    else { // GS232b
        m_ui->az_spinBox->setSingleStep(1);
        m_ui->el_spinBox->setSingleStep(1);
    }
}

//---------------------------------------------------------------------------
// set control limits
void RigDialog::setRotorControlLimits(void)
{
    bool isxy = rig->rotor->isXY();

    m_ui->az_spinBox->setMaximum(isxy ? 180:rig->rotor->az_max);
    m_ui->az_spinBox->setMinimum(isxy ? 0:rig->rotor->az_min);
    m_ui->az_dial->setMaximum(rint(isxy ? 180:rig->rotor->az_max));
    m_ui->az_dial->setMinimum(rint(isxy ? 0:rig->rotor->az_min));

    m_ui->el_spinBox->setMaximum(isxy ? 180:rig->rotor->el_max);
    m_ui->el_spinBox->setMinimum(isxy ? 0:rig->rotor->el_min);
    m_ui->el_dial->setMaximum(rint(isxy ? 180:rig->rotor->el_max));
    m_ui->el_dial->setMinimum(rint(isxy ? 0:rig->rotor->el_min));

    m_ui->el_dial->setNotchTarget(isxy ? 0.5:1);
    m_ui->az_dial->setNotchTarget(isxy ? 0.5:3.6);
}

//---------------------------------------------------------------------------
void RigDialog::on_rotorTypeCb_currentIndexChanged(int index)
{
    rig->rotor->rotor_type = (TRotorType_t) index;
    rig->rotor->closePort();

    enableControls();

    QString str;

    str.sprintf("%s:", rig->rotor->isXY() ? "X":"Azimuth");
    m_ui->ctrlazlabel->setText(str);

    str.sprintf("%s:", rig->rotor->isXY() ? "Y":"Elevation");
    m_ui->ctrlelevlabel->setText(str);
}

//---------------------------------------------------------------------------
void RigDialog::setRotorSteps(QComboBox *cb, int steps)
{
 int index = 0;

    switch(steps) {
       case  200: index = 0; break;
       case  400: index = 1; break;
       case  800: index = 2; break;
       case 1600: index = 3; break;

       default:
           index = 0;
    }

    cb->setCurrentIndex(index);
}

//---------------------------------------------------------------------------
int RigDialog::getRotorSteps(QComboBox *cb)
{
    switch(cb->currentIndex()) {
       case 0: return  200;
       case 1: return  400;
       case 2: return  800;
       case 3: return 1600;
       default: return 200;
    }
}

//---------------------------------------------------------------------------
void RigDialog::on_rotorAzPinSetup_clicked()
{
    rotorPins->setPin(true,
                      rig->rotor->stepper->rotor_az_pin[0],
                      rig->rotor->stepper->rotor_az_pin[1],
                      rig->rotor->stepper->rotor_az_pin[2] & R_ROTOR_CCW ? true:false);

    if(rotorPins->exec()) {
        rig->rotor->stepper->rotor_az_pin[0] = rotorPins->getAzElPinIndex();
        rig->rotor->stepper->rotor_az_pin[1] = rotorPins->getDirPinIndex();

        rig->rotor->stepper->rotor_az_pin[2] &=  ~R_ROTOR_CCW;
        rig->rotor->stepper->rotor_az_pin[2] |= rotorPins->getCC() ? R_ROTOR_CCW:0;

        m_ui->checkBoxAzCCW->setChecked(rig->rotor->stepper->rotor_az_pin[2] & R_ROTOR_CCW ? true:false);
    }
}

//---------------------------------------------------------------------------
void RigDialog::on_rotorElPinSetup_clicked()
{
    rotorPins->setPin(false,
                      rig->rotor->stepper->rotor_el_pin[0],
                      rig->rotor->stepper->rotor_el_pin[1],
                      rig->rotor->stepper->rotor_el_pin[2] & R_ROTOR_CCW ? true:false);

    if(rotorPins->exec()) {
        rig->rotor->stepper->rotor_el_pin[0] = rotorPins->getAzElPinIndex();
        rig->rotor->stepper->rotor_el_pin[1] = rotorPins->getDirPinIndex();

        rig->rotor->stepper->rotor_el_pin[2] &= ~R_ROTOR_CCW;
        rig->rotor->stepper->rotor_el_pin[2] |= rotorPins->getCC() ? R_ROTOR_CCW:0;

        m_ui->checkBoxElCCW->setChecked(rig->rotor->stepper->rotor_el_pin[2] & R_ROTOR_CCW ? true:false);
    }

}

//---------------------------------------------------------------------------
void RigDialog::on_checkBoxElCCW_clicked()
{
    rig->rotor->stepper->rotor_el_pin[2] &= ~R_ROTOR_CCW;
    rig->rotor->stepper->rotor_el_pin[2] |= m_ui->checkBoxElCCW->isChecked() ? R_ROTOR_CCW:0;
}

//---------------------------------------------------------------------------
void RigDialog::on_checkBoxAzCCW_clicked()
{
    rig->rotor->stepper->rotor_az_pin[2] &= ~R_ROTOR_CCW;
    rig->rotor->stepper->rotor_az_pin[2] |= m_ui->checkBoxAzCCW->isChecked() ? R_ROTOR_CCW:0;
}

//---------------------------------------------------------------------------
void RigDialog::on_calibrateJrkAzButton_clicked()
{
    calibrateJrk(true);
}

//---------------------------------------------------------------------------
void RigDialog::on_calibrateJrkElButton_clicked()
{
    calibrateJrk(false);
}

//---------------------------------------------------------------------------
void RigDialog::calibrateJrk(bool azimuth)
{
    int id;

    rig->rotor->closePort();
    applyRotorSettings();

    id = azimuth ? rig->rotor->jrk->az_id:rig->rotor->jrk->el_id;

    if(id < 0)
        return;

    if(!rig->rotor->openPort()) {
        QMessageBox::critical(this, "Error: Communication failure!", rig->rotor->getErrorString());
        return;
    }

    JrkConfDialog dlg(rig->rotor, azimuth, this);
    if(dlg.exec()) {
        flags |= 1024;

        m_ui->maxAzSb->setValue(rig->rotor->az_max);
        m_ui->minAzSb->setValue(rig->rotor->az_min);

        m_ui->maxElSb->setValue(rig->rotor->el_max);
        m_ui->minElSb->setValue(rig->rotor->el_min);

        on_applyRotorBtn_clicked();

        flags &= ~1024;
    }
}

//---------------------------------------------------------------------------
void RigDialog::on_jrkClrErrorsBtn_clicked()
{
    if(!rig->rotor->isPortOpen())
        QMessageBox::critical(this, "Error: Communication is not open!", "Toggle Apply button");
    else
        rig->rotor->jrk->clearErrors();
}

//---------------------------------------------------------------------------
void RigDialog::on_jrkStatusBtn_clicked()
{
    QMessageBox::information(this, "Jrk Status", rig->rotor->getErrorString());
}

//---------------------------------------------------------------------------
void RigDialog::on_monstrumStatusBtn_clicked()
{
    QMessageBox::information(this, "Monstrum X-Y Status", rig->rotor->getStatusString());
}
