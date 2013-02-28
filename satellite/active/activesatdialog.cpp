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
#include <QDesktopServices>
#include <QProcess>
#include <QUrl>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <string.h>
#include <math.h>

#include "activesatdialog.h"
#include "ui_activesatdialog.h"
#include "textwindow.h"

#include "mainwindow.h"
#include "plist.h"
#include "Satellite.h"
#include "satutil.h"
#include "settings.h"
#include "utils.h"

//---------------------------------------------------------------------------
ActiveSatDialog::ActiveSatDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ActiveSatDialog)
{
 QListWidgetItem *item;
 PList *list;
 TSat  *sat, *sat2;
 int i;

    flags = 1; // don't trigger events

    m_ui->setupUi(this);
    setLayout(m_ui->mainLayout);

    mw = (MainWindow *) parent;
    list = mw->getSatList();

    satList = new PList;
    m_ui->satListWidget->setSortingEnabled(false);

    for(i=0; i<list->Count; i++) {
        sat = (TSat *) list->ItemAt(i);
        item = new QListWidgetItem(sat->name, m_ui->satListWidget);
        item->setCheckState(sat->isActive() ? Qt::Checked:Qt::Unchecked);
        m_ui->satListWidget->addItem(item);

        sat2 = new TSat(sat);
        satList->Add(sat2);
    }

    m_ui->satListWidget->sortItems();
    m_ui->vhf_freqlabel->setText("");

    flags = 0; // trigger events

    if(m_ui->satListWidget->count()) {
        m_ui->satListWidget->item(0)->setSelected(true);
        on_satListWidget_itemClicked(m_ui->satListWidget->item(0));
    }


    terminal = new TextWindow("GNU Radio Terminal", this);

    usrp = new QProcess(this);

   connect(usrp, SIGNAL(readyReadStandardError()), this, SLOT(usrp_error()));
   connect(usrp, SIGNAL(readyReadStandardOutput()), this, SLOT(usrp_info()));
}

//---------------------------------------------------------------------------
ActiveSatDialog::~ActiveSatDialog()
{
    delete m_ui;

    stop_usrp();
    delete usrp;

    delete terminal;

    clearSatList(satList, 1);
}

//---------------------------------------------------------------------------
void ActiveSatDialog::stop_usrp(void)
{
    if(usrp->Running) {
        usrp->kill();
        usrp->waitForFinished();
    }
}

//---------------------------------------------------------------------------
void ActiveSatDialog::changeEvent(QEvent *e)
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
double ActiveSatDialog::downconvert(TSat *sat, int /*mode*/)
{
 QString str;
 double freq1, freq2;

   freq1 = atof(sat->sat_scripts->downlink().toStdString().c_str());
   freq2 = sat->getDownlinkFreq(mw->getRig());
   qDebug("freq2: %f", freq2);

   if(freq1 != freq2) {
       str.sprintf("-> %g MHz", freq2);
       m_ui->vhf_freqlabel->setText(str);

       return freq2;
   }
   else {
       if(freq1 <= 0)
           m_ui->vhf_freqlabel->setText("???");
       else
           m_ui->vhf_freqlabel->setText("");

       return freq1;
  }
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_satListWidget_itemClicked(QListWidgetItem* item)
{
    TSatScript *script;
    QStringList sl;
    int i;

    TSat *sat = getSat(satList, item->text());

    if(!sat)
        return;

    script = sat->sat_scripts;

    // General
    m_ui->freqCb->lineEdit()->setText(script->downlink());
    m_ui->downconvertCb->setChecked(script->downconvert());
    downconvert(sat);

    // RX-Script
    m_ui->enableRXScriptCb->setChecked(script->rx_srcrip_enable());
    m_ui->rxscriptEd->setText(script->rx_script());

    sl = script->rx_script_args();
    m_ui->rxscriptargEd->clear();
    for(i=0; i<sl.count(); i++)
        m_ui->rxscriptargEd->append(sl.at(i));

    // Post RX-Script
    m_ui->enablePostProcScriptCb->setChecked(script->postproc_srcrip_enable());
    m_ui->postprocScriptEd->setText(script->postproc_script());

    sl = script->postproc_script_args();
    m_ui->postprocScriptArgEd->clear();
    for(i=0; i<sl.count(); i++)
        m_ui->postprocScriptArgEd->append(sl.at(i));

    m_ui->applyButton->setText("Apply " + QString(sat->name) + " settings");
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_downconvertCb_clicked()
{
    TSat *sat = getSelectedSat();

    if(sat) {
        on_applyButton_clicked();
        downconvert(sat);
    }
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_applyButton_clicked()
{
    QListWidgetItem *item;
    TSatScript *script;
    TSat *sat;

    item = getSelectedListItem(m_ui->satListWidget);
    if(item == NULL)
        return;

    sat = getSelectedSat();
    if(!sat)
        return;

    script = sat->sat_scripts;

    // General
    script->active(item->checkState() == Qt::Checked ? true:false);
    script->downlink(m_ui->freqCb->currentText());
    script->downconvert(m_ui->downconvertCb->isChecked());
    downconvert(sat);

    // RX-Script
    script->rx_srcrip_enable(m_ui->enableRXScriptCb->isChecked());
    script->rx_script(m_ui->rxscriptEd->text());
    script->rx_script_args(getArguments(m_ui->rxscriptargEd));

    // Post RX-Script
    script->postproc_srcrip_enable(m_ui->enablePostProcScriptCb->isChecked());
    script->postproc_script(m_ui->postprocScriptEd->text());
    script->postproc_script_args(getArguments(m_ui->postprocScriptArgEd));
}

//---------------------------------------------------------------------------
QStringList ActiveSatDialog::getArguments(QTextEdit *textEdit)
{
    QStringList sl, sl2;
    QString arg;
    int i;

    arg = textEdit->toPlainText();
    sl2 = arg.split("\n", QString::SkipEmptyParts);

    for(i=0; i<sl2.count(); i++) {
        arg = sl2.at(i);
        arg = arg.trimmed();
        if(!arg.isEmpty())
            sl.append(arg);
    }

#if 0
    textEdit->clear();
    for(i=0; i<sl.count(); i++)
        textEdit->append(sl.at(i));
#endif

    return sl;
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_satListWidget_itemChanged(QListWidgetItem* item)
{
 if(flags & 1)
     return;

 TSat *sat = getSat(satList, item->text());

  if(sat)
     sat->setActive(item->checkState() == Qt::Checked ? true:false);
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_buttonBox_accepted()
{
 PList *list;
 TSat  *sat, *sat2;
 int i;

    on_applyButton_clicked();

    list = mw->getSatList();

    for(i=0; i<satList->Count; i++) {
        sat = (TSat *) satList->ItemAt(i);               
        sat2 = getSat(list, sat->name);
        if(sat2)
            *sat2->sat_scripts = *sat->sat_scripts;
    }

}

//---------------------------------------------------------------------------
void ActiveSatDialog::keyPressEvent(QKeyEvent *event)
{
 QListWidgetItem *item;
 PList *list;
 TSat *sat;

  if(event->key() != Qt::Key_Delete) {
      QWidget::keyPressEvent(event);
      return;
  }

  item = getSelectedListItem(m_ui->satListWidget);
  if(item == NULL)
      return;

  list = mw->getSatList();
  sat = getSat(list, item->text());
  if(sat == NULL)
      m_ui->satListWidget->removeItemWidget(item);
  else if(QMessageBox::question(this, "Delete satellite?", sat->name,
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
      sat->sat_flags |= SAT_DELETE;
      m_ui->satListWidget->removeItemWidget(item);
  }
  else
      return;

  delete item;

  m_ui->satListWidget->sortItems();
}

//---------------------------------------------------------------------------
TSat *ActiveSatDialog::getSelectedSat(void)
{
    QListWidgetItem *item;

    item = getSelectedListItem(m_ui->satListWidget);
    if(item == NULL)
        return NULL;

    return getSat(satList, item->text());
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_defaultrecScriptArgsBtn_clicked()
{
    TSat *sat = getSelectedSat();

    if(!sat)
        return;

    QStringList sl = sat->sat_scripts->rx_default_script_args();

    m_ui->rxscriptargEd->clear();
    for(int i=0; i<sl.count(); i++)
        m_ui->rxscriptargEd->append(sl.at(i));
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_defaultprocScriptArgsBtn_clicked()
{
    TSat *sat = getSelectedSat();

    if(!sat)
        return;

    QStringList sl = sat->sat_scripts->postproc_default_script_args();

    m_ui->postprocScriptArgEd->clear();
    for(int i=0; i<sl.count(); i++)
        m_ui->postprocScriptArgEd->append(sl.at(i));
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_rxscriptButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Script File"),
                                                    m_ui->rxscriptEd->text(),
                                                    tr("Python files (*.py);;All files (*.*)"));
    if(!fileName.isEmpty())
        m_ui->rxscriptEd->setText(fileName);
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_postrxscriptButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Script File"),
                                                    m_ui->postprocScriptEd->text(),
                                                    tr("Python files (*.py);;All files (*.*)"));
    if(!fileName.isEmpty())
        m_ui->postprocScriptEd->setText(fileName);
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_testrxscriptButton_clicked()
{
    on_applyButton_clicked();
    testRXscript(getSelectedSat());
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_testprocscriptButton_clicked()
{
    on_applyButton_clicked();
    testPostRXscript(getSelectedSat());
}

//---------------------------------------------------------------------------
void ActiveSatDialog::usrp_error()
{
    // QMessageBox::critical(this, "An Error Occured!", usrp->readAllStandardError().data());

    if(terminal->isVisible())
        terminal->addTextLine(usrp->readAllStandardError().data());
}

//---------------------------------------------------------------------------
void ActiveSatDialog::usrp_info()
{
    // QMessageBox::information(this, "Information", usrp->readAllStandardOutput().data());

    if(terminal->isVisible())
        terminal->addTextLine(usrp->readAllStandardOutput().data());
}

//---------------------------------------------------------------------------
void ActiveSatDialog::on_testAllSatScriptButton_clicked()
{
    TSat *sat;
    int  i;

    on_applyButton_clicked();

    for(i=0; i<satList->Count; i++) {
        sat = (TSat *) satList->ItemAt(i);
        if(!sat->isActive())
            continue;

        if(sat->sat_scripts->rx_srcrip_enable())
            if(!testRXscript(sat, 1))
                return;

        if(sat->sat_scripts->postproc_srcrip_enable())
            if(!testPostRXscript(sat, 1))
                return;
    }

    QMessageBox::information(this, "No fatal script errors detected!", "Notice:\nEnabled scripts were not executed, only arguments were checked.");
}

//---------------------------------------------------------------------------
// mode & 1 = show error only
bool ActiveSatDialog::testRXscript(TSat *sat, int mode)
{
    QString cmd, name, command;
    bool    error;
    double  freq;

    if(!sat) {
        QMessageBox::information(this, "Info", "Select a satellite first!");
        return false;
    }

    freq = downconvert(sat);
    name.sprintf("%s: ", sat->name);

    command = sat->sat_scripts->get_rx_command(sat->name, freq, &error, 1);
    cmd = command;

    //qDebug("cmd test: %s\n", cmd.toStdString().c_str());

    if(!sat->sat_scripts->frames_filename().isEmpty())
        cmd += "\n\nFrames file:\n" + sat->sat_scripts->frames_filename();

    if(!sat->sat_scripts->baseband_filename().isEmpty())
        cmd += "\n\nBaseband file:\n" + sat->sat_scripts->baseband_filename();

    if(error)
        QMessageBox::critical(this, name + "An Error Occured!", cmd);
    else if(!(mode & 1)){
        if(QMessageBox::question(this, "Execute command?", "Command:\n" + cmd,
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

            cmd = sat->sat_scripts->get_rx_command(sat->name, freq, &error);

            if(error) {
                flags |= 1024;
                QMessageBox::critical(this, name + "An Error Occured!", cmd);
            }
            else {
                stop_usrp();

                if(!terminal->isVisible())
                    terminal->show();

                terminal->addTextLine("Command: " + cmd + "\n");

                usrp->start(command);
            }

        }
    }

    return !error;
}

//---------------------------------------------------------------------------
// mode & 1 = show error only
bool ActiveSatDialog::testPostRXscript(TSat *sat, int mode)
{
    QString cmd, name;
    bool    error;

    if(!sat) {
        QMessageBox::information(this, "Info", "Select a satellite first!");
        return false;
    }

    cmd = sat->sat_scripts->get_postproc_command(&error, 1);

    if(!sat->sat_scripts->frames_filename().isEmpty())
        cmd += "\n\nFrames file:\n" + sat->sat_scripts->frames_filename();

    if(!sat->sat_scripts->baseband_filename().isEmpty())
        cmd += "\n\nBaseband file:\n" + sat->sat_scripts->baseband_filename();

    name.sprintf("%s: ", sat->name);

    if(error)
        QMessageBox::critical(this, name + "An Error Occured!", cmd);
    else if(!(mode & 1)) {
        if(QMessageBox::question(this, "Execute command?", "Command:\n" + cmd,
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

            cmd = sat->sat_scripts->get_postproc_command(&error);
            if(error) {
                flags |= 1024;
                QMessageBox::critical(this, name + "An Error Occured!", cmd);
            }
            else {
                stop_usrp();

                if(!terminal->isVisible())
                    terminal->show();

                terminal->addTextLine("Command: " + cmd + "\n");

                usrp->start(cmd);
            }

        }
    }

    return !error;
}

//---------------------------------------------------------------------------
