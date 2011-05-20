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
#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QtGui/QDockWidget>

namespace Ui {
    class ImageWidget;
}

//---------------------------------------------------------------------------
class MainWindow;
class QSize;
class QString;

//---------------------------------------------------------------------------
class ImageWidget : public QDockWidget {
    Q_OBJECT
public:
    ImageWidget(QWidget *parent = 0);
    ~ImageWidget();

    int   getChannel(void);
    bool  isNorthbound(void);
    int   getImageType(void);

    void setFrames(QString format, long int frames = 0);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ImageWidget *m_ui;
    MainWindow *mw;

private slots:
    void on_enhanceCb_currentIndexChanged(int index);
    void on_NorthboundCb_clicked();
    void on_channelSpinBox_valueChanged(int );
};

#endif // IMAGEWIDGET_H
