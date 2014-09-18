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
    showProgressWindow();
}

void JasonGraphical::showProgressWindow(){
    QProgressDialog *progressWindow = new QProgressDialog(this);
    QProgressBar *infiniteBar = new QProgressBar(this);
    QLabel *statusText = new QLabel(this);
    statusText->setText("Placeholder text");
    infiniteBar->setMaximum(0);
    infiniteBar->setMinimum(0);
    progressWindow->setBar(infiniteBar);
    progressWindow->setLabel(statusText);

    connect(this,SIGNAL(updateLaunchProgress(QString)),statusText,SLOT(setText(QString)));
    connect(this,SIGNAL(closeWindow()),progressWindow,SLOT(close()));

    progressWindow->exec();
}

void JasonGraphical::showMessage(int status, QString message){
    qDebug() << status << message;
}

