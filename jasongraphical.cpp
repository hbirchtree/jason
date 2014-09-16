#include "jasongraphical.h"

JasonGraphical::JasonGraphical(QWidget *parent) :
    QWidget(parent)
{
    QThread interfaceThread;
    QProgressDialog loadingThingamajig;
    QProgressBar testBar;
    loadingThingamajig.setBar(&testBar);
    loadingThingamajig.moveToThread(&interfaceThread);
    interfaceThread.start();
}

void JasonGraphical::displayLaunchProgress(QString currentText){
    QMessageBox *messageBox = new QMessageBox;
    messageBox->setText(currentText);
    messageBox->information(this,"Test","Lol");
}
