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
#ifndef TLEDIALOG_H
#define TLEDIALOG_H

#include <QtGui/QDialog>
#include <QtGui>

namespace Ui {
    class tledialog;
}

//---------------------------------------------------------------------------
class QDropEvent;
class QHttp;
class QListWidget;
class QString;
class PList;
class TStation;
class TSat;

class tledialog : public QDialog {
    Q_OBJECT
public:
    tledialog(PList *list, TStation *_qth, QWidget *parent = 0);
    ~tledialog();

    void updateSatTLE(void);

protected:
    void changeEvent(QEvent *e);

    int  readTLE(const QString &filename);
    void archivate(TSat *sat);
    void addToListWidget(void);
    bool iteminlist(QListWidget *lw, const QString &str);

private:
    Ui::tledialog *m_ui;
    QHttp *http;
    PList *satList, *satListptr;
    TStation *qth;
    QString tlepath, tlearcpath;

private slots:
    void on_delButton_clicked();
    void on_buttonBox_accepted();
    void on_addButton_clicked();
    void on_fileBtn_clicked();
    void on_downloadBtn_clicked();
    void saveTLE();
};

#endif // TLEDIALOG_H
