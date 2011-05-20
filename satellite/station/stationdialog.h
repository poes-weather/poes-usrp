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
#ifndef STATIONDIALOG_H
#define STATIONDIALOG_H

#include <QtGui/QDialog>
#include <QTreeWidgetItem>

namespace Ui {
    class StationDialog;
}

class QString;
class TStation;
//---------------------------------------------------------------------------
class StationDialog : public QDialog {
    Q_OBJECT
public:
    StationDialog(QString _inifile, TStation *qth, QWidget *parent = 0);
    ~StationDialog();

    TStation *pqth;

protected:
    void changeEvent(QEvent *e);
    bool checkInput(int flags=0);
    void readStations(void);
    void writeStations(void);

private:
    Ui::StationDialog *m_ui;
    QString inifile;

private slots:
    void on_delButton_clicked();
    void on_stationTree_itemClicked(QTreeWidgetItem* item, int column);
    void on_stationTree_itemDoubleClicked(QTreeWidgetItem* item, int column);
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
    void on_addButton_clicked();
};

#endif // STATIONDIALOG_H
