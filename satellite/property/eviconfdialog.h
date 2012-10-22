#ifndef EVICONFDIALOG_H
#define EVICONFDIALOG_H

#include <QDialog>

namespace Ui {
class EVIConfDialog;
}

class EVIConfDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit EVIConfDialog(QWidget *parent = 0);
    ~EVIConfDialog();
    
private:
    Ui::EVIConfDialog *ui;
};

#endif // EVICONFDIALOG_H
