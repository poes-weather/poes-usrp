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
#ifndef JRKLUT_H
#define JRKLUT_H

class JrkLUT
{
public:
    JrkLUT(void);
    JrkLUT(unsigned short _target, double _degrees);
    ~JrkLUT();

    unsigned short target(void) { return target_; }
    void target(unsigned short _target);

    double degrees(void) { return degrees_; }
    void degrees(double _degrees);

protected:
    unsigned short target_;
    double degrees_;
};

#endif // JRKLUT_H
