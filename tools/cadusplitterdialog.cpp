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
#include <QFileDialog>
#include <QMessageBox>
#include <stdio.h>

#include "cadusplitterdialog.h"
#include "ui_cadusplitterdialog.h"
#include "cadu.h"

//---------------------------------------------------------------------------
CADUSplitterDialog::CADUSplitterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CADUSplitterDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);

    cadu = new TCADU;

    infp = NULL;
    outfp = NULL;
}

//---------------------------------------------------------------------------
CADUSplitterDialog::~CADUSplitterDialog()
{
    delete ui;

    closeFiles();
    delete cadu;
}

//---------------------------------------------------------------------------
void CADUSplitterDialog::on_infileToolButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("CADU/VCDU Input File"), ui->infileEd->text(),
                                                    tr("CADU/VCDU Files (*.cadu);;All files (*.*)"));

    if(!fileName.isEmpty()) {
        ui->infileEd->setText(fileName);
        ui->ahrptoutfileEd->setText(changePrefix(fileName, true));
        ui->lritoutfileEd->setText(changePrefix(fileName, false));
    }
}

//---------------------------------------------------------------------------
void CADUSplitterDialog::on_MetOpoutfileTB_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("AHRPT Output File"), "",
                                                    tr("All files (*.*)"));

    if(!fileName.isEmpty())
        ui->ahrptoutfileEd->setText(fileName);
}

//---------------------------------------------------------------------------
QString CADUSplitterDialog::changePrefix(QString filename, bool ahrpt)
{
    if(filename.isEmpty())
        return "";

    QFileInfo fi(filename);
    QString name, prefix;
    int i = filename.indexOf('.', filename.length() - fi.fileName().length() + 1);

    if(i > 0)
        name = filename.remove(i, filename.length() - i);
    else
        name = filename;

    if(ahrpt)
        prefix = getAHRPTPrefix();
    else
        prefix = ".lrit";

    name += prefix;

    //qDebug("name: %s", name.toStdString().c_str());

    return name;
}

//---------------------------------------------------------------------------
QString CADUSplitterDialog::getAHRPTPrefix(void)
{
    QString prefix;

    switch(ui->ahrptVCIDCB->currentIndex()) {
    case  0: prefix = ".avhrr"; break;
    case  1: prefix = ".amsu"; break;
    case  2: prefix = ".hirs"; break;
    case  3: prefix = ".sem"; break;
    case  4: prefix = ".a-dcs"; break;
    case  5: prefix = ".iasi"; break;
    case  6: prefix = ".mhs"; break;
    case  7: prefix = ".ascat"; break;
    case  8: prefix = ".gome-2"; break;
    case  9: prefix = ".gras"; break;
    case 10: prefix = ".sat"; break;
    case 11: prefix = ".admin"; break;
    default:
        prefix = ".ccsds";
    }

    return prefix;
}

//---------------------------------------------------------------------------
void CADUSplitterDialog::on_ahrptVCIDCB_currentIndexChanged(int /*index*/)
{
    ui->ahrptoutfileEd->setText(changePrefix(ui->ahrptoutfileEd->text(), true));
}

//---------------------------------------------------------------------------
bool CADUSplitterDialog::openFiles(bool ahrpt)
{
    closeFiles();

    QString outfile = ahrpt ? ui->ahrptoutfileEd->text():ui->lritoutfileEd->text();

    if(ui->infileEd->text().isEmpty() || outfile.isEmpty() ||
       ui->infileEd->text() == outfile)
        return false;

    infp = fopen(ui->infileEd->text().toStdString().c_str(), "rb");
    if(!infp) {
        QMessageBox::critical(this, "Error: Failed to open file!", ui->infileEd->text());
        return false;
    }

#if 0
    outfp = fopen(outfile.toStdString().c_str(), "wb");
    if(!outfp) {
        closeFiles();

        QMessageBox::critical(this, "Error: Failed to open file!", outfile);
        return false;
    }
#endif

    cadu->reed_solomon(ui->rsdecodeCb->isChecked());
    cadu->derandomize(ui->derandomizeCb->isChecked());
    cadu->lrit_cadu(ahrpt ? false:true);

    if(!cadu->init(infp, CADU_PACKET_SIZE, outfp)) {
        closeFiles();

        QMessageBox::critical(this, "Error: Failed to initialize CCSDS class!", "Out of memory?");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void CADUSplitterDialog::closeFiles(void)
{
    if(infp)
        fclose(infp);

    if(outfp)
        fclose(outfp);

    cadu->reset();

    infp = NULL;
    outfp = NULL;
}

//---------------------------------------------------------------------------
void CADUSplitterDialog::on_genAHRPTDataBtn_clicked()
{
    if(!openFiles(true))
        return;

    closeFiles();
}

//---------------------------------------------------------------------------
void CADUSplitterDialog::on_genGOESdataBtn_clicked()
{
    if(!openFiles(false))
        return;

    quint8  *vcdu, *m_pdu, *cp_pdu, *s_pdu;
    quint8  *img_hdr;
    quint8  vcid, sequenceflag;
    quint16 apid, hdr_ptr, packet_length, hdr_len, sequence_count, all_hdr_len;
    quint16 image_index, u16, bytes_read;
    quint32 u32;
    quint64 data_len, u64;

    long    addr;
    bool found;

    char txt[512];
    FILE *txtfp = NULL;

    txtfp = fopen("/home/patrik/tmp/msg_frames-1.txt", "w");

    image_index = 0;
    found = false;


    while(cadu->findsync()) {
        vcdu = cadu->getpayload(); // VCDU (1020 bytes) without sync marker
        if(!vcdu)
            break;

        vcid = cadu->vcid();
        if(vcid == 63) // skip filler packets
            continue;

#if 0
        cadu->writepacket(true);
        continue;
#endif

        addr = cadu->getpacketaddress();

#if 0
//        qDebug("\naddress: 0x%08x vcid: %d", (unsigned int) addr, (int) vcid);
        if(addr < 0x017c4c00)
            continue;
#endif

        m_pdu = cadu->get_mpdu();
        if(m_pdu == NULL)
            continue;

        /*
            M_PDU packet zone (CP_PDU) does not contain a header if
            first header pointer is 2047 (0x07ff). It contains data from a
            previous packet
        */

        if(!cadu->first_hdr_ptr(&hdr_ptr) || hdr_ptr == 0x07ff) {
            hdr_ptr = 0;
        }
        // qDebug("header pointer: 0x%04x (%d)", hdr_ptr, (int)hdr_ptr);

        cp_pdu = m_pdu + 0x02 + hdr_ptr;
        sequenceflag = cadu->sequenceflag(cp_pdu);
        sequence_count = ((cp_pdu[2] << 8) | cp_pdu[3]) & 0x3fff; // 14 bit
        packet_length = (cp_pdu[4] << 8) | cp_pdu[5];
        apid = cadu->apid(cp_pdu);

#if 0
        if(apid != 0) {
            qDebug("\n->address: 0x%08x vcid: %d", (unsigned int) addr, (int) vcid);
            qDebug("->apid: %d, header pointer: 0x%04x, sequence flag: %d, sequence count: %d, packet length: %d",
                   (int) apid,
                   hdr_ptr,
                   (int) sequenceflag,
                   (int) sequence_count,
                   (int) packet_length);
        }
#endif

        if(apid == 0) {
            // it is an MSGi application process id

            // close previous file
            if(cadu->outfp)
                fclose(cadu->outfp);
            cadu->outfp = NULL;
            bytes_read = 0;

            // read the primary header
            s_pdu = cp_pdu + 0x06 + 0x0a;
            hdr_len = (s_pdu[1] << 8) | s_pdu[2];
            all_hdr_len = (s_pdu[4] << 24) | (s_pdu[5] << 16) | (s_pdu[6] << 8) | s_pdu[7];

            data_len = (quint64) ((s_pdu[8] << 56) | (s_pdu[9] << 48) | (s_pdu[10] << 40) | (s_pdu[11] << 32) |
                       (s_pdu[12] << 24) | (s_pdu[13] << 16) | (s_pdu[14] << 8) | s_pdu[15]);
            data_len >>= 3; // in bytes

            if(hdr_len == 0 || all_hdr_len == 0 || data_len == 0 /*|| data_len > 8190*/)
                continue;

            if(s_pdu[0] == 0x00 && s_pdu[3] == 0x00) {
                if(hdr_len != 0x10) // primary header is 16 bytes
                    continue;

                image_index++;

                sprintf(txt, "============================================================");
                print_debug(txt, txtfp);
                sprintf(txt, "\nvcdu address: 0x%08x vcid: %d", (unsigned int) addr, (int) vcid);
                print_debug(txt, txtfp);

                sprintf(txt, "apid: %d, header pointer: 0x%04x, sequence flag: %d, sequence count: %d, packet length: %d, data length: %d",
                        (int) apid,
                        hdr_ptr,
                        (int) sequenceflag,
                        (int) sequence_count,
                        (int) packet_length,
                        (int) data_len);
                print_debug(txt, txtfp);

                sprintf(txt, "s_pdu header type: %d, file type: %d, header length: %d, size of all headers: %d", s_pdu[0], s_pdu[3], hdr_len, all_hdr_len);
                print_debug(txt, txtfp);

                // image file, set pointer at first secondary header
                img_hdr = s_pdu + hdr_len;
                lrit_secondary_header_decoder(img_hdr);

                if(cadu->outfp) {
                    // write this packet to the lrit file, starting from first secondary header until packet end
                    // m_pdu packet is 884 bytes, primary header is 16 bytes

                    u16 = 884 - hdr_ptr - 16;
                    fwrite(s_pdu, 1, u16, cadu->outfp);

                    // data starts at byte 8 from vcdu packet and is 884 bytes, m_pdu packet zone (cp_pdu)
                    // vcdu hdr = 6 + m_pdu hdr = 2 -> 8 bytes
                    // the last packet includes 2 byte crc
                    // write to file until bytes_read < data_len
                }

                // debug
                img_hdr = s_pdu + hdr_len;
                lrit_msg_header_decoder_debug(img_hdr, txtfp);
            }
        } // end apid == 0
        else if(cadu->outfp) {
            // figure out how much to write
            u64 = data_len - bytes_read;
            if(u64 <= 884)
                u16 = (quint16) (u64 - 2); // skip crc
            else
                u16 = 884;

            fwrite(cp_pdu, 1, u16, cadu->outfp);
            bytes_read += u16;

            sprintf(txt, "\nvcdu address: 0x%08x vcid: %d\npacket size: %d, total bytes written: %d of %d",
                    (unsigned int) addr,
                    (int) vcid,
                    (int) u16,
                    (int) bytes_read,
                    (int) data_len);
            print_debug(txt, txtfp);

            if(bytes_read >= (data_len - 2)) { // crc skipped
                fclose(cadu->outfp);
                cadu->outfp = NULL;
            }


        }


    }


    if(txtfp)
        fclose(txtfp);

    closeFiles();

    qDebug("\nDone LRIT HRIT: images found: %d", image_index);
}

//---------------------------------------------------------------------------
void CADUSplitterDialog::lrit_secondary_header_decoder(unsigned char *hdr)
{
    quint16 hdr_len, address, max_address;
    quint8  type, flags;
    char    txt[1024];
    char    buff[256];

    // the *hdr points now at the first secondary header
    // VCDU            6 primary header
    // M_PDU           2 header
    // primary header 16
    // max address is 884 - 16

    address = 0x18;
    max_address = 0x0364;

    flags = 1; // encrypted or bogus header or annotaion header missing
    while(address < max_address) {
        type = hdr[0];
        hdr_len = (hdr[1] << 8) | hdr[2];

        address += hdr_len;
        if(hdr_len == 0 || address > max_address || type < 1)
            break;

        if(type == 4 && hdr_len == 64) {
            flags &= ~1;

            memcpy(buff, hdr + 3, hdr_len - 3);
            buff[hdr_len - 3] = '\0';

            for(int i=0; i<(signed) strlen(buff); i++) {
                if(buff[i] == '-' || buff[i] == '_')
                    continue;
                if(!isalnum(buff[i]) || !isprint(buff[i])) {
                    buff[i] = '?';
                    flags |= 1; // bogus annotation
                }
            }

            if(!(flags & 1) && buff[hdr_len - 4] == 'E')
                flags |= 1; // encrypted, skip

            break;
        }

        hdr += hdr_len;
    }


    if(flags & 1)
        return;

    // create a new lrit file
    sprintf(txt, "/home/patrik/tmp/lrit/ptast-%s.lrit", buff);
    cadu->outfp = fopen(txt, "wb");

}

//---------------------------------------------------------------------------
void CADUSplitterDialog::lrit_msg_header_decoder_debug(unsigned char *hdr, FILE *debug_fp)
{
    quint16 hdr_len, address, max_address;
    quint8  type;
    quint32 u32_1, u32_2;
    char    txt[1024];
    char    buff[256];

    // the *hdr points now at the first secondary header
    // VCDU            6 primary header
    // M_PDU           2 header
    // primary header 16
    // max address is 884 - 16

    address = 0x18;
    max_address = 0x0364;

    while(address < max_address) {
        type = hdr[0];
        hdr_len = (hdr[1] << 8) | hdr[2];

        address += hdr_len;
        if(hdr_len == 0 || address > max_address)
            return;

        sprintf(txt, "\nHeader type #%d, header length: %d", type, hdr_len);
        print_debug(txt, debug_fp);

        switch(type) {
        case 1:
            sprintf(txt, "Image structure\nbits per pixel: %d\ncolumns: %d rows: %d\ncompression type: %d (0=none, 1=lossless, 2=lossy)",
                    hdr[3],
                    (hdr[4] << 8) | hdr[5], (hdr[6] << 8) | hdr[7],
                    hdr[8]);

            break;
        case 2:
            sprintf(txt, "Image Navigation");

            break;
        case 3:
            sprintf(txt, "Image Data Function");
            break;
        case 4:
            if(hdr_len == 64) {
                memcpy(buff, hdr + 3, hdr_len - 3);
                buff[hdr_len - 3] = '\0';

                for(int i=0; i<(signed) strlen(buff); i++) {
                    if(buff[i] == '-' || buff[i] == '_')
                        continue;
                    if(!isalnum(buff[i]) || !isprint(buff[i]))
                        buff[i] = '?';
                }

                sprintf(txt, "Annotation\n%s%s", buff, buff[hdr_len - 4] == 'E' ? "\nEncrypted":" ");
            }
            else
                strcpy(txt, "Annotation\nErroneus annotation lenght");

            break;
        case 5:
            sprintf(txt, "Time Stamp");
            break;
        case 6:
            sprintf(txt, "Ancillary Text");
            break;
        case 7:
            u32_1 = (hdr[4] << 24) | (hdr[5] << 16) | (hdr[6] << 8) | hdr[7]; // high byte
            u32_2 = (hdr[8] << 24) | (hdr[9] << 16) | (hdr[10] << 8) | hdr[11]; // low byte
            sprintf(txt, "Key Header\nkey no: %d Seed for PN enctyption (64 bits): hi32 0x%08x low32 0x%08x",
                    hdr[3],
                    u32_1, u32_2);
            break;
        case 128:
            sprintf(txt, "Segment Identification\nimage id: %d\nchannel id: %d\nsegment sequence no: %d\nstart sequence no: %d\nend sequence no: %d\ndata format: %d (0=none, 1=JPEG, 2=T.4 coded 3=Wavelet)",
                    (hdr[3] << 8) | hdr[4],
                     hdr[5] & 0xff,
                    (hdr[6] << 8) | hdr[7],
                    (hdr[8] << 8) | hdr[9],
                    (hdr[10] << 8) | hdr[11],
                     hdr[12] & 0xff);
            break;
        case 130:
            sprintf(txt, "Image Segment Line Quality");
            print_debug(txt, debug_fp);
            break;

        default:
            continue;
        }

        print_debug(txt, debug_fp);
        hdr += hdr_len;
    }


}

//---------------------------------------------------------------------------
void CADUSplitterDialog::print_debug(char *buf, FILE *dfp)
{
    if(dfp)
        fprintf(dfp, "%s\n", buf);

    qDebug(buf);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
















#if 0
#if 1
        cadu->writepacket(true);
#else
        vcid = cadu->vcid();
        if(vcid == 63) // filler
            continue;

#if 0
        if(vcid != 53)
            continue;
#endif

        addr = cadu->getpacketaddress();

#if 0
        sprintf(txt, "vcid: %d @ 0x%08x", vcid, (unsigned int) addr);
        if(txtfp) fprintf(txtfp, "%s\n", txt);
       // qDebug("\n%s", txt);
#endif

        found = false;
        if(cadu->first_hdr_ptr(&hdr_ptr)) {
            sprintf(txt, "vcid: %d @ 0x%08x", vcid, (unsigned int) addr);
            if(txtfp) fprintf(txtfp, "\n%s\n", txt);

            mpdu = cadu->get_mpdu_packet(hdr_ptr);
            apid = cadu->apid(mpdu);
            sequenceflag = cadu->sequenceflag(mpdu);

            addr += (hdr_ptr + 0x0a);
            sprintf(txt, "apid: %d @ hdr_ptr: 0x%08x [%d]\nsequence flag: %d",
                    apid,
                    (unsigned int) addr,
                    hdr_ptr,
                    sequenceflag);

            if(txtfp) fprintf(txtfp, "%s\n", txt);
            // qDebug(txt);

            // primary header is 16 bytes and image structure record is 9 bytes
            // 882 - 16 - 9 = 857
            if((sequenceflag & (3 | 1)) && hdr_ptr < 857) {
                addr += 0x0a;
                spdu = mpdu + 0x0a; // point at primary header
                if(spdu[0] == 0) { // is it a primary header?
                    if(spdu[3] == 0) {
                        found = true;
                        sprintf(txt, "Image primary header @ 0x%08x", (unsigned int) addr);

                        if(txtfp) fprintf(txtfp, "%s\n", txt);
                        // qDebug(txt);

                        spdu += 0x10; // point at image structure record
                        addr += 0x10;
                        if(spdu[0] == 1) {
                            bpp     = spdu[3];
                            columns = (spdu[4] << 8) | spdu[5];
                            rows    = (spdu[6] << 8) | spdu[7];
                            compressionType = spdu[8];

                            sprintf(txt, "\
Image structure record @ 0x%08x\n\
bpp: %d\n\
columns: %d\n\
rows: %d\n\
compression type: %d",
(unsigned int) addr,
bpp,
columns,
rows,
compressionType);

                            if(txtfp) fprintf(txtfp, "%s\n", txt);
                            // qDebug(txt);
                        }
                        else {
                            sprintf(txt, "Primary header, file type: %d", spdu[3]);
                            if(txtfp) fprintf(txtfp, "%s\n", txt);
                            // qDebug(txt);
                        }
                    }
                    else {
                        sprintf(txt, "Primary header, file type: %d", spdu[3]);
                        if(txtfp) fprintf(txtfp, "%s\n", txt);
                        // qDebug(txt);

                        continue;
                    }

                }

            }
        }
        else
            continue;

        // cadu->rsdecode();
        if(found)
            cadu->writepacket(true);

        if(1==0 && txtfp && found)
            fprintf(txtfp, "\n");
#endif

#endif
