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
*/

//---------------------------------------------------------------------------

#ifndef CADU_H
#define CADU_H
//---------------------------------------------------------------------------
#include <QtGlobal>

#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

#define CADU_RS_DECODE      1
#define CADU_DERANDOMIZE    2

#define CADU_PACKET_SIZE    1020
#define CADU_SYNC_SIZE 4
static const unsigned char CADU_SYNC[CADU_SYNC_SIZE] = {
  0x1A, 0xCF, 0xFC, 0x1D,
};

//---------------------------------------------------------------------------
class TCADU
{
public:
    TCADU(void);
    ~TCADU(void);

    bool init(FILE *fp_, size_t payload_size_, FILE *oufp_= NULL);
    void reset(void);

    // VCDU Primary Header Information
    quint8     scid(void);
    quint8     vcid(void);
    bool       isencrypted(void);
    quint8     key(void);

    // CCSDS (M-PDU) Header Information
    quint8     *get_mpdu(void);
    bool       first_hdr_ptr(quint16 *pos);
    quint8     *get_mpdu_packet(quint16 hdr_ptr);
    quint16    apid(quint8 *mpdu);
    quint8     sequenceflag(quint8 *mpdu);
    quint16    sequence_count(quint8 *mpdu);


    bool reed_solomon(void) { return flags & CADU_RS_DECODE ? true:false; }
    void reed_solomon(bool enable);
    bool rsdecode(void);

    bool derandomize(void) { return flags & CADU_DERANDOMIZE ? true:false; }
    void derandomize(bool enable);

    bool           findsync(const unsigned char *sync = CADU_SYNC, int sync_size = CADU_SYNC_SIZE);
    unsigned char *getpayload(void);
    unsigned char *getpayload_buffer(void) { return payload_buf; }

    long getpacketaddress(void) { return packet_address; }
    long getpackets(void) { return packets; }

    void writepacket(bool include_sync);
    void writeVCDU(void);

    FILE *outfp;

protected:
    void setflag(int flag, bool on);
    bool init_derandomizer(void);
    bool init_reed_solomon(void);
    void randomize(void);

private:
    FILE *fp;

    size_t         payload_size;
    unsigned char *payload_buf;
    unsigned char *derand_buf;

    size_t        rs_size;
    unsigned char *rs_buf;

    long packets, packet_address;

    int flags;
};

#endif // CADU_H
