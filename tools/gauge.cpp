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
#include <QtGui>

#include "gauge.h"

//---------------------------------------------------------------------------
TGauge::TGauge(GaugeType type, QWidget *parent, double minValue, double maxValue) :
    QWidget(parent)
{
    //qDebug("w:%d h:%d", parent->width(), parent->height());

    setFixedSize(parent->width(), parent->height());
    cp.setX(width() / 2.0);
    cp.setY(height() / 2.0);
    radius = cp.x() - 20;

    gaugetype = type;

    switch(gaugetype) {
    case Azimuth_GaugeType:
        {
            min_value = 0;
            max_value = 360;
        }
        break;
    case Elevation_GaugeType:
        {
            min_value = 0;
            max_value = 180;
        }
        break;
    case MinMax_GaugeType:
        {
            min_value = minValue;
            max_value = maxValue;
        }
        break;
    default:
        {
            type = MinMax_GaugeType;
            min_value = 0;
            max_value = 100;
        }
    }

    value = 0;
}

//---------------------------------------------------------------------------
TGauge::~TGauge(void)
{
}

//---------------------------------------------------------------------------
void TGauge::setValue(double new_value)
{
    if(value == new_value)
        return;

    if(new_value < min_value)
        value = min_value;
    else if(new_value > max_value)
        value = max_value;
    else
        value = new_value;

    // qDebug("setvalue: %g", value);
    repaint();
}

//---------------------------------------------------------------------------
void TGauge::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter;

    // qDebug("paint value: %g", value);

    painter.begin(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(cp);

    painter.save();
    painter.rotate(value);


#if 1
    QPointF points[3];
    int w = 9;

    painter.setPen(QPen(Qt::transparent));

    // north left side
    points[0] = QPointF(-w, 0);
    points[1] = QPointF(0, -radius);
    points[2] = QPointF(0, 0);
    //painter.setBrush(QBrush(QColor(255, 105, 0)));
    painter.setBrush(QBrush(Qt::red));
    painter.drawPolygon(points, 3);

    // north right side
    points[0] = QPointF(w, 0);
    points[1] = QPointF(0, -radius);
    points[2] = QPointF(0, 0);
    painter.setBrush(QBrush(Qt::darkRed));
    painter.drawPolygon(points, 3);

    // south left side
    points[0] = QPointF(-w, 0);
    points[1] = QPointF(0, radius);
    points[2] = QPointF(0, 0);
    //painter.setBrush(QBrush(Qt::green));
    painter.setBrush(QBrush(Qt::blue));
    painter.drawPolygon(points, 3);

    // south right side
    points[0] = QPointF(w, 0);
    points[1] = QPointF(0, radius);
    points[2] = QPointF(0, 0);
    //painter.setBrush(QBrush(Qt::darkYellow));
    painter.setBrush(QBrush(Qt::darkBlue));
    painter.drawPolygon(points, 3);

#else
    painter.drawLine(0, 20, 0, -radius);
    painter.drawLine(0, -radius, -5, -radius + 12);
    painter.drawLine(0, -radius, 5, -radius + 12);

    painter.drawEllipse(-5, -5, 10, 10);
#endif



    painter.restore();

    painter.end();
}

//---------------------------------------------------------------------------

