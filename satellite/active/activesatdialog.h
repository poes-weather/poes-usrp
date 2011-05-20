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
#ifndef ACTIVESATDIALOG_H
#define ACTIVESATDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class ActiveSatDialog;
}

class QListWidgetItem;
class QTextEdit;
class QStringList;
class QString;
class QProcess;
class MainWindow;
class PList;
class TSat;
class TextWindow;

//---------------------------------------------------------------------------
class ActiveSatDialog : public QDialog {
    Q_OBJECT
public:
    ActiveSatDialog(QWidget *parent = 0);
    ~ActiveSatDialog();

protected:
    void changeEvent(QEvent *e);
    double downconvert(TSat *sat, int mode=0);
    void keyPressEvent(QKeyEvent *event);

    QStringList getArguments(QTextEdit *lineEdit);

    TSat *getSelectedSat(void);
    void stop_usrp(void);

    bool testRXscript(TSat *sat, int mode=0);
    bool testPostRXscript(TSat *sat, int mode=0);

private:
    Ui::ActiveSatDialog *m_ui;
    MainWindow *mw;
    PList *satList;
    int flags;

    TextWindow *terminal;
    QProcess *usrp;

private slots:
    void on_testAllSatScriptButton_clicked();
    void on_testprocscriptButton_clicked();
    void usrp_error();
    void usrp_info();

    void on_downconvertCb_clicked();
    void on_testrxscriptButton_clicked();
    void on_postrxscriptButton_clicked();
    void on_rxscriptButton_clicked();
    void on_defaultprocScriptArgsBtn_clicked();
    void on_defaultrecScriptArgsBtn_clicked();
    void on_buttonBox_accepted();
    void on_satListWidget_itemChanged(QListWidgetItem* item);
    void on_satListWidget_itemClicked(QListWidgetItem* item);
    void on_applyButton_clicked();
};

#endif // ACTIVESATDIALOG_H
