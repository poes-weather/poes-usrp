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

#ifndef CADUSPLITTERDIALOG_H
#define CADUSPLITTERDIALOG_H

#include <QDialog>

namespace Ui {
    class CADUSplitterDialog;
}

class TCADU;
//---------------------------------------------------------------------------
class CADUSplitterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CADUSplitterDialog(QWidget *parent = 0);
    ~CADUSplitterDialog();

private:
    Ui::CADUSplitterDialog *ui;
    TCADU *cadu;
    FILE  *infp, *outfp;

protected:
    QString changePrefix(QString filename, bool ahrpt);
    QString getAHRPTPrefix(void);

    bool    openFiles(bool ahrpt);
    void    closeFiles(void);
    void    lrit_msg_header_decoder_debug(unsigned char *hdr, FILE *debug_fp);
    void    lrit_secondary_header_decoder(unsigned char *hdr);
    void    print_debug(char *buf, FILE *dfp);

private slots:
    void on_genGOESdataBtn_clicked();
    void on_genAHRPTDataBtn_clicked();
    void on_ahrptVCIDCB_currentIndexChanged(int index);
    void on_MetOpoutfileTB_clicked();
    void on_infileToolButton_clicked();
};

#endif // CADUSPLITTERDIALOG_H
