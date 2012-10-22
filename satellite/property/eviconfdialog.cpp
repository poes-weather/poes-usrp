#include "eviconfdialog.h"
#include "ui_eviconfdialog.h"

EVIConfDialog::EVIConfDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EVIConfDialog)
{
    ui->setupUi(this);
}

EVIConfDialog::~EVIConfDialog()
{
    delete ui;
}
