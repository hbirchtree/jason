#include "jasongraphical.h"
#include "jasonparser.h"

JasonGraphical::JasonGraphical(QWidget *parent) :
    QWidget(parent)
{

}

void JasonGraphical::startParse(QString startDocument, QString actionId, QString desktopFile, QString jasonPath){
    QProgressDialog *progressWindow = new QProgressDialog(this);
    QPushButton *closeProgressWindowBtn = new QPushButton(this);
    closeProgressWindowBtn->setText(tr("Close"));
    progressWindow->setWindowTitle(tr("Jason Launcher"));
    closeProgressWindowBtn->setDisabled(true);
    progressWindow->setCancelButton(closeProgressWindowBtn);
    progressWindow->setRange(0,0);
    progressWindow->setFixedHeight(80);
    progressWindow->setFixedWidth(450);

    //Open document
    JasonParser jParse;
    QThread *workerThread = new QThread;
    jParse.moveToThread(workerThread);
    jParse.setStartOpts(startDocument,actionId,desktopFile,jasonPath);
    connect(workerThread,SIGNAL(started()),&jParse,SLOT(startParse()),Qt::QueuedConnection);
    connect(&jParse,SIGNAL(finishedProcessing()),workerThread,SLOT(quit()),Qt::QueuedConnection);
    connect(&jParse,SIGNAL(failedProcessing()),workerThread,SLOT(quit()),Qt::QueuedConnection);

    //Progress window
    connect(&jParse,SIGNAL(updateProgressText(QString)),progressWindow,SLOT(setLabelText(QString)));
    connect(&jParse,SIGNAL(updateProgressText(QString)),this,SLOT(printMessage(QString))); //Print messages directed for the GUI to stdout, so that we may read them even if the GUI swishes by.
    connect(&jParse,SIGNAL(updateProgressTitle(QString)),progressWindow,SLOT(setWindowTitle(QString)));
    connect(&jParse,SIGNAL(finishedProcessing()),progressWindow,SLOT(close()));
    connect(&jParse,SIGNAL(toggleCloseButton(bool)),closeProgressWindowBtn,SLOT(setEnabled(bool)));
    connect(&jParse,SIGNAL(toggleProgressVisible(bool)),progressWindow,SLOT(setVisible(bool)));
    connect(&jParse,SIGNAL(changeProgressBarRange(int,int)),progressWindow,SLOT(setRange(int,int)));
    connect(&jParse,SIGNAL(changeProgressBarValue(int)),progressWindow,SLOT(setValue(int)));

    //Displaying messages and etc
    connect(&jParse,SIGNAL(broadcastMessage(int,QString)),SLOT(showMessage(int,QString)));
    connect(&jParse,SIGNAL(displayDetachedMessage(QString)),SLOT(detachedMessage(QString)));
    connect(this,SIGNAL(detachedHasClosed()),&jParse,SLOT(detachedMainProcessClosed()));
    connect(&jParse,SIGNAL(emitOutput(QString,QString)),this,SLOT(showOutput(QString,QString)));

    QEventLoop waitingLoop;
    connect(&jParse,SIGNAL(finishedProcessing()),&waitingLoop,SLOT(quit()));
    connect(closeProgressWindowBtn,SIGNAL(clicked()),&waitingLoop,SLOT(quit()));
    progressWindow->show();
    workerThread->start();
    /* If we don't run an eventloop here, it will close before postrun because there are no more events to process and nothing blocking the thread.
     * This is likely the fix to that problem.
    */
    waitingLoop.exec();
}

void JasonGraphical::showMessage(int status, QString message){
    QMessageBox *messageBox = new QMessageBox(this);
    messageBox->setText(message);
    if(status==0)
        messageBox->setWindowTitle(tr("Jason information"));
    if(status==1)
        messageBox->setWindowTitle(tr("Jason warning"));
    if(status==2)
        messageBox->setWindowTitle(tr("Jason error"));
    messageBox->show();
}

void JasonGraphical::detachedMessage(QString title){
    QMessageBox *detachedProgramNotify = new QMessageBox(this);
    QString windowTitle = tr("Jason - Detached process");
    QString text = tr("%1 is currently detached. Close this window to notify Jason when it has been closed properly.").arg(title);
    detachedProgramNotify->setText(text);
    detachedProgramNotify->setWindowTitle(windowTitle);
    detachedProgramNotify->setButtonText(0,tr("It is closed"));
    connect(detachedProgramNotify,SIGNAL(finished(int)),this,SLOT(detachedProgramNotifyEmit(int)));
    detachedProgramNotify->show();
}

void JasonGraphical::detachedProgramNotifyEmit(int retInt){
    emit detachedHasClosed();
}

void JasonGraphical::showOutput(QString stdOut, QString stdErr){
    QDialog *outputWindow = new QDialog(this);
    QGridLayout *outputLayout = new QGridLayout(this);
    QTextEdit *errEdit = new QTextEdit(this);
    QTextEdit *outEdit = new QTextEdit(this);
    errEdit->setReadOnly(true);
    outEdit->setReadOnly(true);
    errEdit->setText(stdErr);
    outEdit->setText(stdOut);
    outputLayout->addWidget(outEdit,1,1);
    outputLayout->addWidget(errEdit,1,2);
    outputWindow->setLayout(outputLayout);
    outputWindow->show();
}

void JasonGraphical::printMessage(QString message){
    //Simple implementation to print to stdout without messing with qDebug all over
    printf("%s\n",qPrintable(message));
}
