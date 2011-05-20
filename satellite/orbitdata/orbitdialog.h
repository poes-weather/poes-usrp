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
#ifndef ORBITDIALOG_H
#define ORBITDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class orbitdialog;
}
//---------------------------------------------------------------------------
class PList;
//---------------------------------------------------------------------------
class orbitdialog : public QDialog {
    Q_OBJECT
public:
    orbitdialog(PList *_satList, QWidget *parent = 0);
    ~orbitdialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::orbitdialog *m_ui;
    PList *satList;

private slots:
    void on_satListWidget_itemSelectionChanged();
};

#endif // ORBITDIALOG_H
