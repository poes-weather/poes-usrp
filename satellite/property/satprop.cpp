/*
    POES-USRP, a software for recording and decoding POES high resolution weather satellite images.
    Copyright (C) 2009-2012 Free Software Foundation, Inc.

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

#include <stdlib.h>
#include <memory.h>
#include <QSettings>

#include "plist.h"
#include "satprop.h"

//---------------------------------------------------------------------------
TSatProp::TSatProp(void)
{
    rgblist = new PList;
    ndvilist = new PList;

    _decoderFlags = 0;
}

//---------------------------------------------------------------------------
TSatProp::~TSatProp(void)
{
    zero();
    delete rgblist;
    delete ndvilist;
}

//---------------------------------------------------------------------------
TSatProp& TSatProp::operator = (TSatProp& src)
{
    if(this == &src)
        return *this;

    int i;

    zero();
    for(i=0; i<src.rgblist->Count; i++)
        rgblist->Add(new TRGBConf(*((TRGBConf *) src.rgblist->ItemAt(i))));

    for(i=0; i<src.ndvilist->Count; i++)
        ndvilist->Add(new TNDVI(*((TNDVI *) src.ndvilist->ItemAt(i))));

    _decoderFlags = src.decoderFlags();

    return *this;
}

//---------------------------------------------------------------------------
void TSatProp::zero(void)
{
    clear_rgb();
    clear_ndvi();
}

//---------------------------------------------------------------------------
void TSatProp::add_rgb(TRGBConf *rgb)
{
    if(rgb == NULL)
        return;

    int *rgb_ch = rgb->rgb_ch();

    add_rgb(rgb->name(), rgb_ch[0], rgb_ch[1], rgb_ch[2]);
}

//---------------------------------------------------------------------------
TRGBConf *TSatProp::add_rgb(const QString& name, int r, int g, int b)
{
    if(name.isEmpty())
        return NULL;

    TRGBConf *rc = get_rgb(name);

    if(rc)
        rc->rgb_ch(r, g, b);
    else {
        rc = new TRGBConf(name, r, g, b);
        rgblist->Add(rc);
    }

    return rc;
}

//---------------------------------------------------------------------------
void TSatProp::del_rgb(const QString& name)
{
    TRGBConf *rc = get_rgb(name);

    if(rc) {
        rgblist->Delete(rc);
        delete rc;
    }
}

//---------------------------------------------------------------------------
TRGBConf *TSatProp::get_rgb(QString name, int *index)
{
    if(name.isEmpty())
        return NULL;

    TRGBConf *rc;
    int i;

    for(i=0; i<rgblist->Count; i++) {
        rc = (TRGBConf *) rgblist->ItemAt(i);
        if(rc->name() == name) {
            if(index)
                *index = i;

            return rc;
        }
    }

    return NULL;
}

//---------------------------------------------------------------------------
void TSatProp::clear_rgb(void)
{
    TRGBConf *rgbconf;

    while((rgbconf = (TRGBConf *)rgblist->Last())) {
        rgblist->Delete(rgbconf);
        delete rgbconf;
    }
}

//---------------------------------------------------------------------------
void TSatProp::add_defaults(int mode)
{
    add_rgb_defaults(mode);
}

//---------------------------------------------------------------------------
void TSatProp::add_rgb_defaults(int mode)
{
    if(mode & 1)
        clear_rgb();

    // add some NOAA defaults (applies also to Feng-Yun and Meteor M-N1 also)
    add_rgb("RGB Daytime", 1, 2, 4);
    add_rgb("RGB Nighttime", 3, 4, 5);
    add_rgb("RGB Daytime cyan snow", 4, 2, 1);
}

//---------------------------------------------------------------------------
void TSatProp::check(int max_ch)
{
    TRGBConf *rc;
    TNDVI *vi;
    int i;

    for(i=0; i<rgblist->Count; i++) {
        rc = (TRGBConf *) rgblist->ItemAt(i);
        rc->check(max_ch);
    }

    for(i=0; i<ndvilist->Count; i++) {
        vi = (TNDVI *) ndvilist->ItemAt(i);
        vi->check(max_ch);
    }

}

//---------------------------------------------------------------------------
void TSatProp::readSettings(QSettings *reg)
{
    TRGBConf *rc;
    TNDVI    *vi;
    QString  str;
    int      i;

    reg->beginGroup("Decoder");

    str = reg->value("Flags", "0").toString();
    _decoderFlags = str.toUInt();

    reg->endGroup(); // Decoder

    rc = new TRGBConf;
    i = 0;
    reg->beginGroup("RGB-Conf");

    while(true) {
        str.sprintf("Conf-%d", i++);
        reg->beginGroup(str);

        if(!reg->contains("ID")) {
            reg->endGroup();
            break;
        }

        rc->readSettings(reg);

        if(!get_rgb(rc->name()))
            rgblist->Add(new TRGBConf(*rc));

        reg->endGroup();
    }

    reg->endGroup(); // RGB-Conf
    delete rc;

    vi = new TNDVI;
    i = 0;
    reg->beginGroup("NDVI-Conf");

    while(true) {
        str.sprintf("Conf-%d", i++);
        reg->beginGroup(str);

        if(!reg->contains("ID")) {
            reg->endGroup();
            break;
        }

        vi->readSettings(reg);

        if(!get_ndvi(vi->name()))
            ndvilist->Add(new TNDVI(*vi));

        reg->endGroup();
    }

    reg->endGroup(); // NDVI-Conf
    delete vi;
}

//---------------------------------------------------------------------------
void TSatProp::writeSettings(QSettings *reg)
{
    TRGBConf *rc;
    TNDVI *vi;
    QString str;
    int i;

    reg->beginGroup("Decoder");

    reg->setValue("Flags", decoderFlags());

    reg->endGroup(); // Decoder

    reg->beginGroup("RGB-Conf");

    for(i=0; i<rgblist->Count; i++) {
        rc = (TRGBConf *) rgblist->ItemAt(i);
        str.sprintf("Conf-%d", i);

        reg->beginGroup(str);
        rc->writeSettings(reg);
        reg->endGroup();
    }

    reg->endGroup(); // RGB-Conf

    reg->beginGroup("NDVI-Conf");

    for(i=0; i<ndvilist->Count; i++) {
        vi = (TNDVI *) ndvilist->ItemAt(i);
        str.sprintf("Conf-%d", i);

        reg->beginGroup(str);

        vi->writeSettings(reg);

        reg->endGroup();
    }

    reg->endGroup(); // NDVI-Conf

}

//---------------------------------------------------------------------------
//
//              NDVI
//
//---------------------------------------------------------------------------
void TSatProp::clear_ndvi(void)
{
    TNDVI *vi;

    while((vi = (TNDVI *)ndvilist->Last())) {
        ndvilist->Delete(vi);
        delete vi;
    }
}

//---------------------------------------------------------------------------
TNDVI *TSatProp::get_ndvi(const QString name)
{
    if(name.isEmpty())
        return NULL;

    TNDVI *vi;
    for(int i=0; i<ndvilist->Count; i++) {
        vi = (TNDVI *) ndvilist->ItemAt(i);
        if(vi->name() == name)
            return vi;
    }

    return NULL;
}

//---------------------------------------------------------------------------
void TSatProp::add_ndvi(TNDVI *ndvi)
{
    if(ndvi == NULL)
        return;

    TNDVI *vi = get_ndvi(ndvi->name());
    if(vi)
        *vi = *ndvi;
    else
        ndvilist->Add(new TNDVI(*ndvi));
}

//---------------------------------------------------------------------------
void TSatProp::del_ndvi(const QString& name)
{
    TNDVI *vi = get_ndvi(name);

    if(vi) {
        ndvilist->Delete(vi);
        delete vi;
    }
}

//---------------------------------------------------------------------------
void TSatProp::add_ndvi_defaults(int mode)
{
    if(mode & 1)
        clear_ndvi();
}

//---------------------------------------------------------------------------
//
//      Decoder
//
//---------------------------------------------------------------------------
void TSatProp::derandomize(bool yes)
{
    flagState(&_decoderFlags, DF_DERANDOMIZE, yes);
}

//---------------------------------------------------------------------------
bool TSatProp::derandomize(void)
{
    return flagState(&_decoderFlags, DF_DERANDOMIZE);
}

//---------------------------------------------------------------------------
void TSatProp::rs_decode(bool yes)
{
    flagState(&_decoderFlags, DF_RSDECODE, yes);
}

//---------------------------------------------------------------------------
bool TSatProp::rs_decode(void)
{
    return flagState(&_decoderFlags, DF_RSDECODE);
}

//---------------------------------------------------------------------------
void TSatProp::syncCheck(bool yes)
{
    flagState(&_decoderFlags, DF_SYNCCHECK, yes);
}

//---------------------------------------------------------------------------
bool TSatProp::syncCheck(void)
{
    return flagState(&_decoderFlags, DF_SYNCCHECK);
}

//---------------------------------------------------------------------------
void TSatProp::flagState(unsigned int *flag, unsigned int bitmap, bool on)
{
    *flag &= ~bitmap;
    *flag |= on ? bitmap:0;
}

//---------------------------------------------------------------------------
bool TSatProp::flagState(unsigned int *flag, unsigned int bitmap)
{
    return (*flag & bitmap) ? true:false;
}

//---------------------------------------------------------------------------

