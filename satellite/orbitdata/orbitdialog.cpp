/*
    HRPT-Decoder, a software for processing NOAA-POES high resolution weather satellite images.
    Copyright (C) 2009 Free Software Foundation, Inc.

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
#include "orbitdialog.h"
#include "ui_orbitdialog.h"

#include "Satellite.h"
#include "satutil.h"
#include "plist.h"


//---------------------------------------------------------------------------
orbitdialog::orbitdialog(PList *_satList, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::orbitdialog)
{
 TSat *sat;
 int i;

    m_ui->setupUi(this);
    setLayout(m_ui->horizontalLayout);

    satList = _satList;

    for(i=0; i<satList->Count; i++) {
        sat = (TSat *) satList->ItemAt(i);
        m_ui->satListWidget->addItem(sat->name);
    }

    if(m_ui->satListWidget->count())
        m_ui->satListWidget->setItemSelected(m_ui->satListWidget->item(0), true);
}

//---------------------------------------------------------------------------
orbitdialog::~orbitdialog()
{
    delete m_ui;
}

//---------------------------------------------------------------------------
void orbitdialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
void orbitdialog::on_satListWidget_itemSelectionChanged()
{
 QListWidgetItem *item;
 TSat *sat;
 int i;

    for(i=0; i<m_ui->satListWidget->count(); i++) {
        item = m_ui->satListWidget->item(i);
        if(item->isSelected()) {
            sat = getSat(satList, item->text());
            if(sat)
                sat->Data2Grid(m_ui->tableWidget);

            break;
        }
    }
}
