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
    QProgressDialog *progressWindow = new QProgressDialog(this);
    QPushButton *closeProgressWindowBtn = new QPushButton(this);
    closeProgressWindowBtn->setText(tr("Close"));
    closeProgressWindowBtn->setDisabled(true);
    progressWindow->setCancelButton(closeProgressWindowBtn);
//    progressWindow->setCancelButtonText(tr("Close"));
    progressWindow->setMinimum(0);
    progressWindow->setMaximum(0);
    progressWindow->setFixedHeight(70);
    progressWindow->setFixedWidth(300);

    //Open document
    JasonParser jParse;
    QThread *workerThread = new QThread;
    jParse.moveToThread(workerThread);
    jParse.setStartOpts(startDocument,actionId,desktopFile);
    connect(workerThread,SIGNAL(started()),&jParse,SLOT(startParse()),Qt::QueuedConnection);
    connect(&jParse,SIGNAL(finishedProcessing()),workerThread,SLOT(quit()),Qt::QueuedConnection);
    connect(&jParse,SIGNAL(updateProgressText(QString)),progressWindow,SLOT(setLabelText(QString)));
    connect(&jParse,SIGNAL(updateProgressTitle(QString)),progressWindow,SLOT(setWindowTitle(QString)));
    connect(&jParse,SIGNAL(toggleProgressVisible(bool)),progressWindow,SLOT(setVisible(bool)));
    connect(&jParse,SIGNAL(finishedProcessing()),progressWindow,SLOT(close()));
//    connect(&jParse,SIGNAL(updateProgressText(QString)),SLOT(setProgressText(QString)));
//    connect(&jParse,SIGNAL(updateProgressTitle(QString)),SLOT(setProgressTitle(QString)));
    connect(&jParse,SIGNAL(broadcastMessage(int,QString)),SLOT(showMessage(int,QString)));
    connect(&jParse,SIGNAL(displayDetachedMessage(QString)),SLOT(detachedMessage(QString)));
    connect(this,SIGNAL(detachedHasClosed()),&jParse,SLOT(detachedMainProcessClosed()));
    workerThread->start();
//    showProgressWindow();

    progressWindow->exec();
}

void JasonGraphical::showMessage(int status, QString message){
    QMessageBox *messageBox = new QMessageBox;
//    return;
    if(status==0)
        messageBox->information(this,tr("Jason information"),message);
    if(status==1)
        messageBox->warning(this,tr("Jason warning"),message);
    if(status==2)
        messageBox->warning(this,tr("Jason error"),message);
}

void JasonGraphical::detachedMessage(QString title){
    QMessageBox detachedProgramNotify;
    QString windowTitle = tr("Jason - Detached process");
    QString text = title+tr(" is currently detached. Close this window to notify Jason when it has been closed properly.");
    detachedProgramNotify.information(this,windowTitle,text,tr("It is closed"));
    emit detachedHasClosed();
}
