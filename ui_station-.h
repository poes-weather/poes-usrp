/********************************************************************************
** Form generated from reading ui file 'station.ui'
**
** Created: Mon Nov 23 22:44:35 2009
**      by: Qt User Interface Compiler version 4.5.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_STATION_H
#define UI_STATION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StationDlg
{
public:
    QWidget *widget;
    QFormLayout *formLayout;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *lonEdit;
    QLabel *label_2;
    QLineEdit *latEdit;
    QLabel *label_4;
    QLineEdit *altEdit;
    QLabel *label_3;
    QLineEdit *nameEdit;
    QPushButton *addButton;
    QDialogButtonBox *buttonBox;
    QTreeWidget *treeWidget;

    void setupUi(QDialog *StationDlg)
    {
        if (StationDlg->objectName().isEmpty())
            StationDlg->setObjectName(QString::fromUtf8("StationDlg"));
        StationDlg->setWindowModality(Qt::WindowModal);
        StationDlg->resize(358, 221);
        widget = new QWidget(StationDlg);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(10, 10, 341, 201));
        formLayout = new QFormLayout(widget);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        lonEdit = new QLineEdit(widget);
        lonEdit->setObjectName(QString::fromUtf8("lonEdit"));

        gridLayout->addWidget(lonEdit, 0, 1, 1, 1);

        label_2 = new QLabel(widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        latEdit = new QLineEdit(widget);
        latEdit->setObjectName(QString::fromUtf8("latEdit"));

        gridLayout->addWidget(latEdit, 1, 1, 1, 1);

        label_4 = new QLabel(widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 2, 0, 1, 1);

        altEdit = new QLineEdit(widget);
        altEdit->setObjectName(QString::fromUtf8("altEdit"));

        gridLayout->addWidget(altEdit, 2, 1, 1, 1);

        label_3 = new QLabel(widget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        nameEdit = new QLineEdit(widget);
        nameEdit->setObjectName(QString::fromUtf8("nameEdit"));

        gridLayout->addWidget(nameEdit, 3, 1, 1, 1);

        addButton = new QPushButton(widget);
        addButton->setObjectName(QString::fromUtf8("addButton"));

        gridLayout->addWidget(addButton, 3, 2, 1, 1);


        formLayout->setLayout(0, QFormLayout::LabelRole, gridLayout);

        buttonBox = new QDialogButtonBox(widget);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        formLayout->setWidget(0, QFormLayout::FieldRole, buttonBox);

        treeWidget = new QTreeWidget(widget);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem(treeWidget);
        new QTreeWidgetItem(__qtreewidgetitem);
        new QTreeWidgetItem(__qtreewidgetitem);
        new QTreeWidgetItem(__qtreewidgetitem);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
        treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeWidget->setItemsExpandable(true);
        treeWidget->header()->setVisible(false);

        formLayout->setWidget(1, QFormLayout::LabelRole, treeWidget);


        retranslateUi(StationDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), StationDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), StationDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(StationDlg);
    } // setupUi

    void retranslateUi(QDialog *StationDlg)
    {
        StationDlg->setWindowTitle(QApplication::translate("StationDlg", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("StationDlg", "Longitude:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("StationDlg", "Latitude:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("StationDlg", "Altitude", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("StationDlg", "Name:", 0, QApplication::UnicodeUTF8));
        addButton->setText(QApplication::translate("StationDlg", "Add to list", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem = treeWidget->headerItem();
        ___qtreewidgetitem->setText(0, QApplication::translate("StationDlg", "Stations", 0, QApplication::UnicodeUTF8));

        const bool __sortingEnabled = treeWidget->isSortingEnabled();
        treeWidget->setSortingEnabled(false);
        QTreeWidgetItem *___qtreewidgetitem1 = treeWidget->topLevelItem(0);
        ___qtreewidgetitem1->setText(0, QApplication::translate("StationDlg", "poes-weather", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem2 = ___qtreewidgetitem1->child(0);
        ___qtreewidgetitem2->setText(0, QApplication::translate("StationDlg", "Longitude: 21.2", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem3 = ___qtreewidgetitem1->child(1);
        ___qtreewidgetitem3->setText(0, QApplication::translate("StationDlg", "Latitude: 63.5", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem4 = ___qtreewidgetitem1->child(2);
        ___qtreewidgetitem4->setText(0, QApplication::translate("StationDlg", "Altitude: 10.5", 0, QApplication::UnicodeUTF8));
        treeWidget->setSortingEnabled(__sortingEnabled);

        Q_UNUSED(StationDlg);
    } // retranslateUi

};

namespace Ui {
    class StationDlg: public Ui_StationDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STATION_H
