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
#ifndef SATPASSDIALOG_H
#define SATPASSDIALOG_H

#include <QtGui>
#include <QDate>
#include <stdio.h>

namespace Ui {
    class satpassdialog;
}

//---------------------------------------------------------------------------
class Obj_t
{
  public:
    Obj_t(void);
    Obj_t(QString n, double v) { name = n; value = v; }

    QString name;
    double  value;
};
//---------------------------------------------------------------------------

class QListWidgetItem;
class QDateTime;
class MainWindow;
class PList;

//---------------------------------------------------------------------------
class satpassdialog : public QDialog {
    Q_OBJECT
public:
    satpassdialog(PList *_satList, QWidget *parent = 0);
    ~satpassdialog();

protected:
    void changeEvent(QEvent *e);

    QDateTime getSelectedUTC(void);


private:
    Ui::satpassdialog *m_ui;
    PList *satList;
    MainWindow *mw;

private slots:
    void on_satListWidget_itemClicked(QListWidgetItem* item);
    void on_dateEdit_dateChanged(QDate date);
    void on_timecheckBox_clicked();
    void on_saveButton_clicked();
    void on_satListWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void on_activeSatBtn_clicked();
};

#endif // SATPASSDIALOG_H
