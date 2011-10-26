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
#include <QSettings>
#include <QFile>
#include <stdlib.h>
#include <math.h>

#include "jrkusb.h"
#include "usbdevice.h"
#include "utils.h"

//---------------------------------------------------------------------------
TJrkUSB::TJrkUSB(void)
{
    jrk = NULL;

    current_deg = 0;
    min_deg = 0;
    max_deg = 360;
    min_fb  = 0;
    max_fb  = 4095;

    iobuff = (unsigned char *) malloc(JRK_IO_BUF_SIZE + 1);
    memset(&vars, 0, sizeof(jrk_variables_t));
}

//---------------------------------------------------------------------------
TJrkUSB::~TJrkUSB(void)
{
    free(iobuff);
}

//---------------------------------------------------------------------------
bool TJrkUSB::setDevice(TUSBDevice *usbdev)
{
    jrk = usbdev;
    sn_ = "";

    if(!isOpen() || !readVariables())
        return false;

    sn_ = jrk->serialnumber();

#if 0
    min_fb = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_MINIMUM);
    max_fb = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_MAXIMUM);
#endif

    current_deg = feedbackToPos(vars.feedback);

    return true;
}

//---------------------------------------------------------------------------
bool TJrkUSB::isOpen(void)
{
    return (jrk && jrk->isOpen()) ? true:false;
}

//---------------------------------------------------------------------------
void TJrkUSB::readSettings(QSettings *reg)
{
    if(!isOpen() || sn_.isEmpty())
        return;

    reg->beginGroup(sn_);

    max_fb = reg->value("MaxFeedback", "4095").toInt();
    min_fb = reg->value("MinFeedback", "0").toInt();

#if 0
    max_deg = reg->value("MaxDegrees", "360").toDouble();
    min_deg = reg->value("MinDegrees", "0").toDouble();
#endif

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TJrkUSB::writeSettings(QSettings *reg)
{
    if(!isOpen() || sn_.isEmpty())
        return;

    reg->beginGroup(sn_);

    reg->setValue("MaxFeedback", max_fb);
    reg->setValue("MinFeedback", min_fb);

#if 0
    reg->setValue("MaxDegrees", max_deg);
    reg->setValue("MinDegrees", min_deg);
#endif

    reg->endGroup();
}

//---------------------------------------------------------------------------
bool TJrkUSB::readVariables(void)
{
    bool rc = false;

    if(isOpen())
        if(jrk->control_transfer(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_VARIABLES, 0, 0, (char *)iobuff, sizeof(jrk_variables_t)))
            rc = true;

    if(rc)
        vars = *(jrk_variables *) iobuff;

    return rc;
}

//---------------------------------------------------------------------------
void TJrkUSB::stop(void)
{
    if(isOpen())
        jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_MOTOR_OFF, 0, 0);
}

//---------------------------------------------------------------------------
void TJrkUSB::clearErrors(void)
{
    if(isOpen())
        jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_CLEAR_ERRORS, 0, 0);
}

//---------------------------------------------------------------------------
void TJrkUSB::errorStr(QStringList *sl)
{
    sl->clear();

    if(!readVariables()) {
        sl->append("Fatal Error: Unable to read variables");
        return;
    }

    unsigned short err = vars.errorFlagBits | vars.errorOccurredBits;

    if(err & 0x0001)
        sl->append("Awaiting command");
    if(err & 0x0002)
        sl->append("No power");
    if(err & 0x0004)
        sl->append("Motor driver error");
    if(err & 0x0008)
        sl->append("Input invalid (Pulse Width Input Mode only)");
    if(err & 0x0010)
        sl->append("Input disconnect");
    if(err & 0x0020)
        sl->append("Feedback disconnect");
    if(err & 0x0040)
        sl->append("Maximum current exceeded");
    if(err & 0x0080)
        sl->append("Serial signal error");
    if(err & 0x0100)
        sl->append("Serial overrun");
    if(err & 0x0200)
        sl->append("Serial RX buffer full");
    if(err & 0x0400)
        sl->append("Serial CRC error");
    if(err & 0x0800)
        sl->append("Serial protocol error");
    if(err & 0x1000)
        sl->append("Serial timeout error");
}

//---------------------------------------------------------------------------
void TJrkUSB::moveTo(double deg)
{
    double delta = fabs(currentPos() - deg);

    if(isOpen() && delta >= 0.01)
        if(setTarget(posToTarget(deg)))
            current_deg = deg;
}

//---------------------------------------------------------------------------
// mode & 1 = read variables
double TJrkUSB::readPos(int mode)
{
    if(mode & 1)
        readVariables();

    current_deg = feedbackToPos(vars.feedback);

    return current_deg;
}

//---------------------------------------------------------------------------
bool TJrkUSB::setTarget(unsigned short target)
{
    if(isOpen())
        if(jrk->control_write(JRK_REQUEST_SET_TYPE,
                              JRK_REQUEST_SET_TARGET,
                              (target & 0x0fff), 0))
            return true;

    return false;
}

//---------------------------------------------------------------------------
// mode & 1 = read variables
unsigned short TJrkUSB::target(int mode)
{
    if(mode & 1)
        readVariables();

    return vars.target;
}

//---------------------------------------------------------------------------
// mode & 1 = read variables
unsigned short TJrkUSB::feedback(int mode)
{
    if(mode & 1)
        readVariables();

    return vars.feedback;
}

//---------------------------------------------------------------------------
double TJrkUSB::feedbackToPos(int feedback)
{
    double d_deg = max_deg - min_deg;
    double d_fb  = max_fb - min_fb;
    double pos = 0;

    if(d_fb > 0 && d_deg > 0 && feedback >= 0 && feedback <= 4095)
        pos = min_deg + (d_deg / d_fb) * ((double) (feedback - min_fb));

    return pos;
}

//---------------------------------------------------------------------------
unsigned short TJrkUSB::posToTarget(double deg)
{
    double t, d_deg, dpt;
    unsigned short u16;

    deg = ((deg < min_deg) ? min_deg:((deg > max_deg) ? max_deg:deg));

    if(deg >= max_deg)
        u16 = 4095;
    else if(deg <= min_deg)
        u16 = 0;
    else {
        d_deg = max_deg - min_deg;
        if(d_deg <= 0)
            return 0;

        dpt = d_deg / 4095.0; // degrees per target tick

        t = (deg - min_deg) / dpt;
        u16 = rint(t);
    }

    return (u16 & 0x0fff);
}
