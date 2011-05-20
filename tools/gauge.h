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

#ifndef GAUGE_H
#define GAUGE_H

#include <QWidget>
#include <QBrush>
#include <QFont>
#include <QPen>


//---------------------------------------------------------------------------
typedef enum
{
    Azimuth_GaugeType,
    Elevation_GaugeType,
    MinMax_GaugeType,

} GaugeType;

//---------------------------------------------------------------------------
class TGauge : public QWidget
{
    Q_OBJECT

public:
    explicit TGauge(GaugeType type, QWidget *parent = 0, double minValue = 0, double maxValue = 0);
    ~TGauge(void);

    void setValue(double new_value);

signals:
    void renderGauge();

public slots:
    // void render(void);

protected:
    void paintEvent(QPaintEvent *event);


private:
    GaugeType gaugetype;
    double    min_value, max_value, value;

    QPointF cp;
    qreal   radius;

};

#endif // GAUGE_H
