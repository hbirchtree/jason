#include "jasongraphical.h"
#include "jasonparser.h"

JasonGraphical::JasonGraphical(QWidget *parent) :
    QWidget(parent)
{
    QProgressDialog loadingThingamajig;
    QProgressBar testBar;
    loadingThingamajig.setBar(&testBar);
}

void JasonGraphical::startParse(QString startDocument, QString actionId, QString desktopFile){
    //Open document
    JasonParser jParse;
    QThread *workerThread = new QThread;
    jParse.moveToThread(workerThread);
    jParse.setStartOpts(startDocument,actionId,desktopFile);
    connect(workerThread,SIGNAL(started()),&jParse,SLOT(startParse()),Qt::QueuedConnection);
    connect(&jParse,SIGNAL(finishedProcessing()),workerThread,SLOT(quit()),Qt::QueuedConnection);
    workerThread->start();
//    jParse.startParse();
    qDebug() << "test";
}

void JasonGraphical::updateLaunchProgress(QString currentText){
    QMessageBox *messageBox = new QMessageBox;
    messageBox->information(this,"Test",currentText);
}
