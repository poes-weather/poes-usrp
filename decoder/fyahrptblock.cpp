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
#include <QImage>
#include <QDateTime>
#include <QDate>
#include <stdlib.h>
#include "fyahrptblock.h"
#include "block.h"

//---------------------------------------------------------------------------
/*

 Source: http://events.eoportal.org/presentations/7148/12759.html

 The modulation is QPSK, encoding: CONV (7, ¾), real-time broadcasting.

 Supported satellites: Feng Yun 3 Series Advance High Resolution
 2nd Generation Polar Orbiting Meteorological Satellite Series
 The spatial resolution at nadir is 1.1 km on a swath of 2800 km (FOV = ±55.4º)

 VIRR (Visible and Infrared Radiometer)
 Band No         Spectral Range (µm)     Primary use of data
  1               0.58 - 0.68 (VIS)       Daytime clouds, ice and snow, vegetation
  2               0.84 - 0.89 (VNIR)      Daytime clouds, vegetation, water
  3               3.55 - 3.93 (MWIR)      Heat source, night cloud
  4               10.3 - 11.3 (TIR)       SST, day/night clouds
  5               11.5 - 12.5 (TIR)       SST, day/night clouds
  6               1.58 - 1.64 (SWIR)      Soil humidity, provision of ice/snow cover distinguishing capability
  7               0.43 - 0.48 (VIS)       Ocean color
  8               0.48 - 0.53 (VIS)       Ocean color
  9               0.53 - 0.58 (VIS)       Ocean color
 10               0.90 - 0.965 (VNIR)     Water vapor

Application     VCID (bin)      Hex         APID (bin)      Hex
MERSI           000011 VC2      0x03
VIRR(day)       000101 VC2      0x05
VIRR(night)     001001 VC3      0x09
MWRI            001010 VC4      0x0A

IRAS            001100 VC5      0x0C        000 0000 0011   0x003
ERM                                         000 0000 0101   0x005
MWTS                                        000 0000 0111   0x007
TOU                                         000 0000 1001   0x009
SBUS                                        000 0000 1011   0x00B
SIM                                         000 0000 1101   0x00D
MWHS                                        000 0001 0000   0x010
SEM                                         000 0000 1111   0x00F
Telemetry


 */
//---------------------------------------------------------------------------

const int FY_AHRPT_CADU_SIZE    = 1024;   // bytes
const int FY_AHRPT_NUM_CHANNELS = 10;
const int FY_AHRPT_SCAN_WIDTH   = 2048;   // 10 bit, one image scan
const int FY_AHRPT_SCAN_SIZE    = 20480;  // 10 bit, width * channels
const int FY_AHRPT_IMAGE_START  = 88;     // CCSDS bytes + 6 bits (20 + 68 bytes + 550 bits)

//---------------------------------------------------------------------------
//#define DEBUG_FRAME

//---------------------------------------------------------------------------
TFYAHRPT::TFYAHRPT(TBlock *_block)
{
  block = _block;
  cadu = block->getCADU();

  scanLine = NULL;
  fp = NULL;
}

//---------------------------------------------------------------------------
TFYAHRPT::~TFYAHRPT(void)
{
    if(scanLine)
        free(scanLine);
}

//---------------------------------------------------------------------------
bool TFYAHRPT::init(void)
{
    if(block == NULL || cadu == NULL)
        return false;

    if(scanLine == NULL)
        scanLine = (quint16 *) malloc(FY_AHRPT_SCAN_SIZE << 1); // 20480 bytes

    fp = block->getHandle();

    if(!cadu->init(fp, FY_AHRPT_CADU_SIZE - CADU_SYNC_SIZE)) // CCSDS size, 1020 bytes
        return false;

    block->setFrames(0);
    block->setFirstFrameSyncPos(-1);
    block->setLittleEndian(true); // USRP default format
    block->setLittleEndian(false); // USRP default format

    return countFrames();
}

//---------------------------------------------------------------------------
// flags&1 = check found data
bool TFYAHRPT::check(int flags)
{
    // check allocation and file pointer status
    if(block == NULL || cadu == NULL || fp == NULL || scanLine == NULL)
        return false;

    // check found stuff
    if(flags&1) {
        if(block->getFrames() <= 0 || block->getFirstFrameSyncPos() < 0)
            return false;
    }

    return true;
}

//---------------------------------------------------------------------------
int TFYAHRPT::countFrames(void)
{
    if(!check())
        return 0; // fatal error

    return check(1) ? block->getFrames():count_AVHRR_HR_frames();
}

//---------------------------------------------------------------------------
long TFYAHRPT::count_AVHRR_HR_frames(void)
{
    quint8  vcid;
    long    frames = 0;

#ifdef DEBUG_AHRPT
    quint16 hdr_ptr;

    //cadu->outfp = fopen("/home/patrik/tmp/fy3a-derand.cadu", "wb");
#endif

    fseek(fp, 0, SEEK_SET);

    while(cadu->findsync()) {
        if(cadu->getpayload() == NULL)
            break;

        vcid = cadu->vcid();

#ifdef DEBUG_AHRPT
        cadu->writepacket(true);

        qDebug(" ");
        qDebug("address: 0x%08x", (unsigned int) cadu->getpacketaddress());
        qDebug("SCID: %d [0x%02x]", cadu->scid(), cadu->scid());
        qDebug("VCID: %d [0x%02x]", vcid, vcid);
        if(cadu->isencrypted())
            qDebug("encryption key: %d [0x%02x]", cadu->key(), cadu->key());
        else
            qDebug("Not encrypted");

        cadu->first_hdr_ptr(&hdr_ptr);
        qDebug("1st header pointer: %d [0x%04x]", hdr_ptr, hdr_ptr);
#endif

        if(!(vcid == 0x05 || vcid == 0x09)) // TODO: check if it is encrypted...
            continue;

        if(frames == 0)
            block->setFirstFrameSyncPos(cadu->getpacketaddress());

        frames++;
    }

    block->setFrames(frames);

#ifdef DEBUG_AHRPT
    if(cadu->outfp)
        fclose(cadu->outfp);
    cadu->outfp = NULL;
#endif

    return frames;
}


//---------------------------------------------------------------------------
int TFYAHRPT::getWidth(void)
{
   return FY_AHRPT_SCAN_WIDTH;
}

//---------------------------------------------------------------------------
int TFYAHRPT::getNumChannels(void)
{
    return FY_AHRPT_NUM_CHANNELS;
}

//---------------------------------------------------------------------------
// frame_nr is zero based (0, 1, 2, ... frames - 1)
bool TFYAHRPT::readFrameScanLine(int frame_nr)
{
    quint8  *vcdu, *ccsds, last_byte, vcid;
    quint16 hdr_ptr, pixel;
    long    bits_to_read, tot_bits_red;
    int     cadu_flags, shift;
    int     scan_index, read_size, scan_pos;
    int     i, index;
    bool    error;


    // missed pixels will be shown as black line
    memset(scanLine, 0, FY_AHRPT_SCAN_SIZE << 1);

    // file pointer MUST be set at firstFrameSyncPos using fseek
    // when started, frame_nr = 0

    if(frame_nr == 0)
        vcdu = cadu->getpayload(); // read the whole CADU
    else {
        if(frame_nr >= block->getFrames())
            return false;

        vcdu = cadu->getpayload_buffer(); // CADU is already in buffer
    }

    if(vcdu == NULL)
        return false;

    cadu_flags = 1; //  &1 = first loop
                    //  &2 = file pointer points at frame_nr + CH1
                    //  &4 = next scanline (frame) found
                    // &16 = add byte from previous packet
                    // &32 = image starts in next packet
    scan_index = 0;
    scan_pos = 0;
    tot_bits_red = 0;
    bits_to_read = 0;
    index = 1;
    shift = 0;

    while(true) {
        error = false;

        if(cadu_flags & 1)
            cadu_flags &= ~1;
        else {
            // find next AVHRR-HR packet (part)
            error = true;
            while(cadu->findsync()) {
                if(cadu->getpayload() == NULL)
                    break;

                vcid = cadu->vcid(); // TODO: check if it is encrypted
                if(vcid == 0x05 || vcid == 0x09) {
                    error = false;
                    break;
                }
            }
        }

        if(error) // nothing of interest found or EOF?
            break;

        ccsds = cadu->get_mpdu();
        // ccsds = vcdu + 0x08;
        hdr_ptr = 0;
        if(cadu->first_hdr_ptr(&hdr_ptr)) {
            hdr_ptr += 2; // M-PDU header is 2 bytes

            if(cadu_flags & 2) {
                cadu_flags |= 4;
                scan_pos = 2;
                bits_to_read = 0;
                // almost done with this scanline, this packet is the last
            }
            else {
                cadu_flags |= 2;
                scan_pos = hdr_ptr + FY_AHRPT_IMAGE_START; // points at CH 1
                shift = 0;
                bits_to_read = -6;

#ifdef DEBUG_FRAME
                if(frame_nr == debug_frame) {
                    tmplong = ftell(fp) - 1024;

                    qDebug("Sequence count: %d @ 0x%08x hdr_ptr @ 0x%08x image start: %d",
                           get_sequence_count(ccsds + hdr_ptr),
                           (unsigned int) tmplong,
                           (unsigned int) tmplong + hdr_ptr,
                           scan_pos);
                }
#endif

                if(scan_pos > 884) {
                    // image starts in next packet
                    scan_pos = hdr_ptr + FY_AHRPT_IMAGE_START + 2 - 884;
                    cadu_flags |= 32;

                    continue;
                }
            }

        }
        else {
            if(!(cadu_flags & 32)) {
                scan_pos = 2; // points at M-PDU start (continues...)
                bits_to_read = 0;
            }

            cadu_flags &= ~32;
        }

        if(scan_index >= FY_AHRPT_SCAN_SIZE)
            return true;

        read_size = 884 - scan_pos;

        if(read_size < 1) {
            read_size = 99;
            continue; // ??? sumthing is VERY wrong...
        }

       // cadu->rsdecode();

        bits_to_read += (read_size << 3);
        tot_bits_red += bits_to_read;

        ccsds += scan_pos; // buffer points at CH 1

#ifdef DEBUG_FRAME
        if(frame_nr == debug_frame) {
            qDebug("start bytes 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
                   ccsds[0],ccsds[1],ccsds[2],ccsds[3],
                   ccsds[4],ccsds[5],ccsds[6],ccsds[7]);
        }

#endif

        if(cadu_flags & 16) {
            pixel = (((last_byte << 8) | ccsds[0]) >> shift) & 0x03ff;
            scanLine[scan_index] = pixel;

#ifdef DEBUG_FRAME
            if(frame_nr == debug_frame) {
                qDebug("add prev byte sample %d, index: %d: shift: %d", (scan_index / 5) - 1, scan_index, shift);
                qDebug("add prev pixel: %d shift: %d", pixel, shift);
            }
#endif

            if(shift == 0) {
                shift = 6;
                index = 2;
            }
            else {
                shift -= 2;
                index = 1;
            }

            scan_index++;
        }
        else
            index = 1;


        for(i=index; i<read_size && scan_index<FY_AHRPT_SCAN_SIZE; scan_index++) {
            pixel = (((ccsds[i-1] << 8) | ccsds[i]) >> shift) & 0x03ff;
            scanLine[scan_index] = pixel;

#ifdef DEBUG_FRAME
            if(frame_nr == debug_frame)
                if(i == index)
                    qDebug("first pixel: %d", pixel);
#endif

            if(shift == 0) {
                shift = 6;
                i += 2;
            }
            else {
                shift -= 2;
                i++;
            }
        }

        cadu_flags &= ~16;
        if((tot_bits_red % 10) != 0)
            cadu_flags |= 16;

        last_byte = ccsds[read_size-1];

#ifdef DEBUG_FRAME
        if(frame_nr == debug_frame) {
            qDebug("scan index: %d", (scan_index / 5) - 1);
            qDebug("buffer size: %d bytes, bits to read: %d, tot: %d", read_size, (int)bits_to_read, (int)tot_bits_red);
            qDebug("last sub pixels: %04d %04d %04d %04d %04d",
               scanLine[scan_index-5], scanLine[scan_index-4],
               scanLine[scan_index-3], scanLine[scan_index-2], scanLine[scan_index-1]);

            qDebug("last of image 0x%02x 0x%02x 0x%02x 0x%02x",
                   ccsds[read_size-4], ccsds[read_size-3], ccsds[read_size-2], ccsds[read_size-1]);

            qDebug("last byte 0x%02x shift: %d\n", last_byte, shift);
        }
#endif
        if(cadu_flags & 4)
            break; // done with this scanline

    }

#ifdef DEBUG_FRAME

    if(frame_nr == debug_frame) {
        qDebug("frame last sample %d: %04d %04d %04d %04d %04d", (scan_index / 5) - 1,
           scanLine[scan_index-5], scanLine[scan_index-4],
           scanLine[scan_index-3], scanLine[scan_index-2], scanLine[scan_index-1]);

    }

#endif

    return true;
}

//---------------------------------------------------------------------------
// returns a 16 bit pixel from a frame channel
// sample and channel are zero based
quint16 TFYAHRPT::getPixel_16(int channel, int sample)
{
 quint16 pixel, pos;

  if(!check())
     return 0;

  if(!block->isNorthBound())
     pos = channel + sample * FY_AHRPT_NUM_CHANNELS;
  else
     pos = channel + (FY_AHRPT_SCAN_WIDTH - sample - 1) * FY_AHRPT_NUM_CHANNELS;

  pixel = scanLine[pos] & 0x03ff;

  return pixel;
}

//---------------------------------------------------------------------------
// returns a 8 bit pixel from a frame channel
// sample and channel are zero based
quint8 TFYAHRPT::getPixel_8(int channel, int sample)
{
  return SCALE16TO8(getPixel_16(channel, sample));
}

//---------------------------------------------------------------------------
// fills an 24 bpp image line of the selected channel
// frame_nr is zero based
bool TFYAHRPT::frameToImage(int frame_nr, QImage *image)
{
 uchar *imagescan, r, g, b;
 int x, y, *ch_rgb;

  if(!check(1) || image == NULL)
     return false;

  if(!readFrameScanLine(frame_nr))
     return false;

  if(block->isNorthBound())
     y = image->height() - frame_nr - 1;
  else
     y = frame_nr;

  imagescan = (uchar *) image->scanLine(y);
  if(imagescan == NULL)
     return false;

  switch(block->getImageType()) {
  case RGB_ImageType:
      ch_rgb = block->rgbconf->rgb_ch();
      break;

  default:
      break;
  }

  for(x=0; x<FY_AHRPT_SCAN_WIDTH; x++) {
      switch(block->getImageType()) {
      case Channel_ImageType:
          r = getPixel_8(block->getImageChannel(), x);
          g = r;
          b = r;
          break;

      case RGB_ImageType:
          r = getPixel_8(ch_rgb[0] - 1, x);
          g = getPixel_8(ch_rgb[1] - 1, x);
          b = getPixel_8(ch_rgb[2] - 1, x);
          break;

        default:
           return false;
     }

     *imagescan++ = r;
     *imagescan++ = g;
     *imagescan++ = b;
  }

 return true;
}

//---------------------------------------------------------------------------
bool TFYAHRPT::toImage(QImage *image)
{
 int frames, y;

  if(!check(1))
     return false;

  fseek(fp, block->getFirstFrameSyncPos() + CADU_SYNC_SIZE, SEEK_SET);
  frames = block->getFrames();

  for(y=0; y<frames; y++) {
     if(!frameToImage(y, image))
        break;
  }

 return true;
}

#if 0
//---------------------------------------------------------------------------
const char *TFYAHRPT::spacecraftname(quint8 scid)
{
    if(scid == 0x0b) // 1011
        return "METOP 1";
    else if(scid == 0x0c) // 1100
        return "METOP 2";
    else if(scid == 0x0d) // 1101
        return "METOP 3";
    else if(scid == 0x0e) // 1110
        return "METOP simulator";
    else
        return "Unknown satellite";
}

//---------------------------------------------------------------------------
const char *TFYAHRPT::vcidTypeStr(quint8 vcid)
{
    if(vcid == 9)
        return "AVHRR/3";
    else if(vcid == 3)
        return "AMSU-A1/A2, HIRS/4, SEM";
    else if(vcid == 27)
        return "A-DCS, Advanced Data Collection System";
    else if(vcid == 10)
        return "IASI, Infrared Atmospheric Sounding Interferometer";
    else if(vcid == 12)
        return "MHS, Microwave Humidity Sounder";
    else if(vcid == 15)
        return "ASCAT, Advanced Scatterometer";
    else if(vcid == 24)
        return "GOME-2, Global Ozone Monitoring Experiment-2";
    else if(vcid == 29)
        return "GRAS Occultation data";
    else if(vcid == 34)
        return "Satellite housekeeping, Admin msg, GRAS positioning and timing data";
    else if(vcid == 63)
        return "Fill data";
    else
        return "Unkonwn VCID";
}

//---------------------------------------------------------------------------
const char *TFYAHRPT::apidTypeStr(quint16 apid)
{
    if(apid == 103)
        return "AVHRR/3 Channel 3A (day)";
    else if(apid == 104)
        return "AVHRR/3 Channel 3B (night)";
    else if(apid == 39)
        return "AMSU-A1, Advanced Microwave Sounding Units";
    else if(apid == 40)
        return "AMSU-A2, Advanced Microwave Sounding Units";
    else if(apid == 38)
        return "HIRS/4, High-resolution Infrared Radiation Sounder";
    else if(apid == 37)
        return "SEM, Space Environment Monitor";
    else if(apid == 35)
        return "A-DCS, Advanced Data Collection System";
    else if(apid == 130 || apid == 135 || apid == 140 || apid == 145 || apid == 150 || apid == 160 || apid == 180)
        return "IASI, Infrared Atmospheric Sounding Interferometer";
    else if(apid == 34)
        return "MHS, Microwave Humidity Sounder";
    else if(apid >= 192 && apid <= 255)
        return "ASCAT, Advanced Scatterometer";
    else if(apid >= 384 && apid <= 447)
        return "GOME-2, Global Ozone Monitoring Experiment-2";
    else if(apid >= 448 && apid <= 511)
        return "GRAS, Global Navigation Satellite System Receiver for Atmospheric Sounding";
    else if(apid == 1)
        return "Satellite housekeeping packet";
    else if(apid == 6)
        return "Admin messages";
    else if(apid == 63)
        return "FILL VCDU";
    else
        return "Unknown Application Process Identifier";

}

#endif
