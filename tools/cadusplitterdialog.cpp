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

    outfp = fopen(outfile.toStdString().c_str(), "wb");
    if(!outfp) {
        closeFiles();

        QMessageBox::critical(this, "Error: Failed to open file!", outfile);
        return false;
    }

    cadu->reed_solomon(ui->rsdecodeCb->isChecked());
    cadu->derandomize(ui->derandomizeCb->isChecked());

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

    quint8  *vcdu, *mpdu, *spdu;
    quint8  vcid, sequenceflag;
    quint16 apid, hdr_ptr;
    quint8  bpp, compressionType;
    quint16 columns, rows;

    long    addr;
    bool found;

    char txt[512];
    FILE *txtfp = NULL;

    txtfp = fopen("/home/patrik/tmp/GOES-LRIT_cadu_frames_2.txt", "w");

    while(cadu->findsync()) {
        vcdu = cadu->getpayload();
        if(!vcdu)
            break;

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

        cadu->rsdecode();
        if(found)
            cadu->writepacket(true);

        if(1==0 && txtfp && found)
            fprintf(txtfp, "\n");
    }


    if(txtfp)
        fclose(txtfp);

    closeFiles();
}
