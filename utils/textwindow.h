/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite imagery.
    Copyright (C) 2010 Free Software Foundation, Inc.

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

#ifndef TEXTWINDOW_H
#define TEXTWINDOW_H

#include <QDialog>

namespace Ui {
    class TextWindow;
}

class TextWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TextWindow(QString caption, QWidget *parent = 0);
    ~TextWindow();

    void addTextLine(QString txt);
    void clear(void);

private:
    Ui::TextWindow *ui;
};

#endif // TEXTWINDOW_H
