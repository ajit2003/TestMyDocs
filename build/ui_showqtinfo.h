/********************************************************************************
** Form generated from reading UI file 'showqtinfo.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHOWQTINFO_H
#define UI_SHOWQTINFO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ShowQtInfo
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *ShowQtInfo)
    {
        if (ShowQtInfo->objectName().isEmpty())
            ShowQtInfo->setObjectName(QString::fromUtf8("ShowQtInfo"));
        ShowQtInfo->resize(800, 600);
        centralwidget = new QWidget(ShowQtInfo);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        ShowQtInfo->setCentralWidget(centralwidget);
        menubar = new QMenuBar(ShowQtInfo);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        ShowQtInfo->setMenuBar(menubar);
        statusbar = new QStatusBar(ShowQtInfo);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        ShowQtInfo->setStatusBar(statusbar);

        retranslateUi(ShowQtInfo);

        QMetaObject::connectSlotsByName(ShowQtInfo);
    } // setupUi

    void retranslateUi(QMainWindow *ShowQtInfo)
    {
        ShowQtInfo->setWindowTitle(QCoreApplication::translate("ShowQtInfo", "ShowQtInfo", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ShowQtInfo: public Ui_ShowQtInfo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHOWQTINFO_H
