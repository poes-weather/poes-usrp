/*
    POES-USRP, a software for recording and decoding POES high resolution weather satellite images.
    Copyright (C) 2009-2011 Free Software Foundation, Inc.

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
#ifndef SATPROPDIALOG_H
#define SATPROPDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

//---------------------------------------------------------------------------
namespace Ui {
    class SatPropDialog;
}

class QListWidget;
class QComboBox;
class PList;
class TSat;
class TRGBConf;
class TNDVI;

//---------------------------------------------------------------------------
class SatPropDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SatPropDialog(PList *satList, QWidget *parent = 0);
    ~SatPropDialog();

protected:
    int countSelected(QListWidget *lw);
    int finditem(QListWidget *lw, const QString& str);
    int finditem(QComboBox *cb, const QString& str);

    TRGBConf *getRGBConf(const QString& name);
    TNDVI    *getNDVI(const QString& name);

    void addRGB(const QString& name, const QString& oldname = "");
    void addNDVI(const QString& name, const QString& oldname = "");


private slots:
    void on_satlistWidget_itemClicked(QListWidgetItem* item);

    void on_rgbCb_currentIndexChanged(int index);
    void on_applyRGB_clicked();
    void on_addRGBBtn_clicked();
    void on_defaultrgbBtn_clicked();

    void on_ndviCb_currentIndexChanged(int index);
    void on_addndvBtn_clicked();
    void on_delndviBtn_clicked();
    void on_applyndviBtn_clicked();


    void on_delrgbBtn_clicked();

    void on_applyDecoderBtn_clicked();

private:
    Ui::SatPropDialog *ui;

    PList *satlist;
    TSat  *selsat;
};

#endif // SATPROPDIALOG_H
