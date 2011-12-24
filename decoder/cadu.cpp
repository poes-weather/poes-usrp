/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite imagery.
    Copyright (C) 2010,2011 Free Software Foundation, Inc.

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

    Parts copied from metopizer software
*/

//---------------------------------------------------------------------------
#include <QtGlobal>

#ifdef HAVE_LIBFEC
extern "C" {
#  include <fec.h>
}
#endif

#include <memory.h>
#include "cadu.h"

//#define DEBUG_RS

//---------------------------------------------------------------------------
TCADU::TCADU(void)
{
    fp = NULL;
    outfp = NULL;

    flags = 0;
    payload_size = 0;
    rs_size = 0;
    packets = 0;

    payload_buf = NULL;
    derand_buf = NULL;
    rs_buf = NULL;
}

//---------------------------------------------------------------------------
bool TCADU::init(FILE *fp_, size_t payload_size_, FILE *outfp_)
{
    outfp = outfp_;
    fp = fp_;

    if(fp == NULL)
        return false;

    payload_size = payload_size_;

    payload_buf = (unsigned char *) malloc(payload_size); // CVCDU, 4 byte sync is NOT included
    if(payload_buf == NULL)
        return false;

    if(!init_derandomizer())
        return false;

#ifdef HAVE_LIBFEC

    rs_size = 255; // ccsds 255,233
    if(!init_reed_solomon())
        return false;

#else

    reed_solomon(false);

#endif

    return true;
}

//---------------------------------------------------------------------------
TCADU::~TCADU(void)
{
    reset();
}

//---------------------------------------------------------------------------
void TCADU::reset(void)
{
    if(payload_buf)
        free(payload_buf);
    payload_buf = NULL;

    if(derand_buf)
        free(derand_buf);
    derand_buf = NULL;

    if(rs_buf)
        free(rs_buf);
    rs_buf = NULL;

    fp = NULL;
    outfp = NULL;

    flags = 0;
    packets = 0;
    payload_size = 0;
    rs_size = 0;
}

//---------------------------------------------------------------------------
void TCADU::lrit_cadu(bool enable)
{
    setflag(CADU_LRIT, enable);
}

//---------------------------------------------------------------------------
bool TCADU::isLRIT(void)
{
    return (flags & CADU_LRIT) ? true:false;
}

//---------------------------------------------------------------------------
void TCADU::setflag(int flag, bool on)
{
    flags &= ~flag;
    flags |= on ? flag:0;
}

//---------------------------------------------------------------------------
void TCADU::reed_solomon(bool enable)
{
    setflag(CADU_RS_DECODE, enable);
}

//---------------------------------------------------------------------------
void TCADU::derandomize(bool enable)
{
    setflag(CADU_DERANDOMIZE, enable);
}

//---------------------------------------------------------------------------
bool TCADU::init_reed_solomon(void)
{
#ifdef HAVE_LIBFEC

    if(!reed_solomon() || rs_buf != NULL) // not enabled or already inited
        return true;

    rs_buf = (unsigned char *) malloc(rs_size);
    if(rs_buf == NULL) {
        reed_solomon(false);
        qDebug("Failed to allocate reed solomon buffer %s:%d", __FILE__,__LINE__);

        return false;
    }

#endif

    return true;
}

//---------------------------------------------------------------------------
bool TCADU::init_derandomizer(void)
{
    if(!derandomize() || derand_buf != NULL) // not enabled or already inited
        return true;

    derand_buf = (unsigned char *) calloc(payload_size, sizeof(unsigned char));
    if(derand_buf == NULL) {
        derandomize(false);
        qDebug("Failed to allocate derandomizer buffer %s:%d", __FILE__,__LINE__);

        return false;
    }

    size_t i, r, nbits;
    int regs[8], reg_7;

    // init shift registers to 1
    for(i=0; i<8; i++)
        regs[i] = 1;

    nbits = payload_size << 3; // payload_size * 8 bits

    for(i=0; i<nbits; i++) {
        // copy register 0 into next bit of decode array
        if(regs[0] == 1)
            derand_buf[i >> 3] |= (1 << ((7 - (i % 8))));

        // calculate new top bit
        reg_7 = ((regs[0] + regs[3] + regs[5] + regs[7]) % 2);

        // shift registers
        for(r=0; r<7; r++)
            regs[r] = regs[r + 1];

        // set new top bit
        regs[7] = reg_7;
    }

    return true;
}

//---------------------------------------------------------------------------
void TCADU::randomize(void)
{
    if(derandomize()) {
        for(size_t i=0; i<payload_size; i++)
            payload_buf[i] ^= derand_buf[i];
    }
}

//---------------------------------------------------------------------------
bool TCADU::rsdecode(void)
{
    if(!reed_solomon())
        return true;

#ifdef HAVE_LIBFEC

    int i, j, rc, errors = 0;

    for(i=0; i<4; i++) {
        for(j=0; j<255; j++)
            rs_buf[j] = payload_buf[i + j*4];

        rc = decode_rs_ccsds(rs_buf, NULL, 0, 0);
        if(rc == -1) {
            qDebug("Reed Solomon failed @ address 0x%08X %s:%d", (unsigned int)packet_address, __FILE__, __LINE__);
            return false;
        }

        errors += rc;

        for(j=0; j<255; j++)
            payload_buf[i + j*4] = rs_buf[j];
    }

#ifdef DEBUG_RS
    if(errors == 0) {
        qDebug("Reed Solomon succeded @ address 0x%08X %s:%d", (unsigned int)packet_address, __FILE__, __LINE__);
    }
    else {
        qDebug("Reed Solomon corrected %d errors @ address 0x%08X %s:%d", errors, (unsigned int)packet_address, __FILE__, __LINE__);
    }
#endif

#endif // #ifdef HAVE_LIBFEC

    return true;
}

//---------------------------------------------------------------------------
bool TCADU::findsync(const unsigned char *sync, int sync_size)
{
    unsigned char ch;
    int i = 0;

    while(fread(&ch, 1, 1, fp) == 1) {
        if(ch == sync[i])
            i++;
        else
            i = 0;

        if(i == sync_size) {
            packet_address = ftell(fp) - sync_size;
            packets++;

            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------
unsigned char *TCADU::getpayload(void)
{
    if(fread(payload_buf, payload_size, 1, fp) != 1)
        return NULL;

    randomize();
    rsdecode();

    return payload_buf;
}

//---------------------------------------------------------------------------
void TCADU::writepacket(bool include_sync)
{
    if(outfp) {
        if(include_sync)
            fwrite(CADU_SYNC, 1, CADU_SYNC_SIZE, outfp);

        fwrite(payload_buf, 1, payload_size, outfp);
    }
}

//---------------------------------------------------------------------------
void TCADU::writeVCDU(void)
{
    if(outfp) {
        fwrite(payload_buf, 1, 892, outfp);
    }
}

//---------------------------------------------------------------------------
//
//      VCDU Primary Header Information
//
//---------------------------------------------------------------------------
quint8 TCADU::scid(void)
{
    return ( ((payload_buf[0] & 0x3f) << 2) | (payload_buf[1] >> 6) ); // 8 bit
}

//---------------------------------------------------------------------------
quint8 TCADU::vcid(void)
{
    return (payload_buf[1] & 0x3f); // 6 bit
}

//---------------------------------------------------------------------------
bool TCADU::isencrypted(void)
{
    return (payload_buf[6] & 0xff) ? true:false; // 8 bit
}

//---------------------------------------------------------------------------
quint8 TCADU::key(void)
{
    return payload_buf[7]; // 8 bit
}

//---------------------------------------------------------------------------
//
//      M-PDU (CCSDS) Information, Packet size = 882 (0x0372) bytes
//
//---------------------------------------------------------------------------
quint8 *TCADU::get_mpdu(void)
{
    quint8 *mpdu;

    if(isLRIT())
        mpdu = payload_buf + 0x06; // VCDU primary header is only 6 bytes
    else
        mpdu = payload_buf + 0x08; // AHRPT

    return mpdu;
}

//---------------------------------------------------------------------------
quint8 *TCADU::get_mpdu_packet(quint16 hdr_ptr)
{
    // AHRPT: M-PDU header is 2 bytes + VCDU header size, 0x08 + 0x02 = 0x0a
    // LRIT:  M-PDU header is 2 bytes + VCDU header size, 0x06 + 0x02 = 0x08
    quint16 pos;

    if(isLRIT())
        pos = 0x08 + hdr_ptr;
    else
        pos = 0x0a + hdr_ptr;

    return pos >= payload_size ? NULL:(payload_buf + pos);
}

//---------------------------------------------------------------------------
bool TCADU::first_hdr_ptr(quint16 *pos)
{
    quint8 *ccsds;

    if(isLRIT())
        ccsds = payload_buf + 0x06;
    else
        ccsds = payload_buf + 0x08;


    *pos = ((ccsds[0] << 8) | ccsds[1]) & 0x07ff; // 11 bit

    return *pos >= 0x0372 ? false:true;
}

//---------------------------------------------------------------------------
quint16 TCADU::apid(quint8 *mpdu)
{
    return ((((mpdu[0] & 0x07) << 8) | mpdu[1]) & 0x07ff); // 11 bit
}

//---------------------------------------------------------------------------
quint8 TCADU::sequenceflag(quint8 *mpdu)
{
    return ((mpdu[2] >> 6) & 0x03); // 2 bit
}

//---------------------------------------------------------------------------
quint16 TCADU::sequence_count(quint8 *mpdu)
{
    return (((mpdu[2] << 8) | mpdu[3]) & 0x3FFF); // 14 bit
}
