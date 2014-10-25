#include "jasongui.h"
#include "ui_jasongui.h"
#include "jasonparser.h"

JasonGui::JasonGui(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JasonGui)
{
    ui->setupUi(this);
    ui->closeButton->setEnabled(false);
}

JasonGui::~JasonGui()
{
    delete ui;
}

void JasonGui::startParse(QString startDocument, QString actionId, QString desktopFile, QString jasonPath){
    show();

    //Open document
    JasonParser jParse;
    QThread *workerThread = new QThread;
    jParse.moveToThread(workerThread);
    jParse.setStartOpts(startDocument,actionId,desktopFile,jasonPath);
    connect(workerThread,SIGNAL(started()),&jParse,SLOT(startParse()),Qt::QueuedConnection);
    connect(&jParse,SIGNAL(finishedProcessing()),workerThread,SLOT(quit()),Qt::QueuedConnection);
    connect(&jParse,SIGNAL(failedProcessing()),workerThread,SLOT(quit()),Qt::QueuedConnection);

    //Progress window
    connect(&jParse,SIGNAL(updateProgressText(QString)),ui->statusLabel,SLOT(setText(QString)));
    connect(&jParse,SIGNAL(updateProgressText(QString)),this,SLOT(printMessage(QString))); //Print messages directed for the GUI to stdout, so that we may read them even if the GUI swishes by.
    connect(&jParse,SIGNAL(updateProgressTitle(QString)),this,SLOT(setWindowTitle(QString)));
    connect(&jParse,SIGNAL(finishedProcessing()),this,SLOT(close()));
    connect(&jParse,SIGNAL(toggleCloseButton(bool)),ui->closeButton,SLOT(setEnabled(bool)));
    connect(&jParse,SIGNAL(toggleProgressVisible(bool)),this,SLOT(hideJasonGui(bool)));
    connect(&jParse,SIGNAL(changeProgressBarRange(int,int)),ui->progressBar,SLOT(setRange(int,int)));
    connect(&jParse,SIGNAL(changeProgressBarValue(int)),ui->progressBar,SLOT(setValue(int)));

    //Displaying messages and etc
    connect(&jParse,SIGNAL(broadcastMessage(int,QString)),SLOT(showMessage(int,QString)));
    connect(&jParse,SIGNAL(displayDetachedMessage(QString)),SLOT(detachedMessage(QString)));
    connect(this,SIGNAL(detachedHasClosed()),&jParse,SLOT(detachedMainProcessClosed()));
    connect(&jParse,SIGNAL(emitOutput(QString,QString)),this,SLOT(showOutput(QString,QString)));

    QEventLoop waitingLoop;
    connect(&jParse,SIGNAL(finishedProcessing()),&waitingLoop,SLOT(quit()));
    connect(&jParse,SIGNAL(failedProcessing()),&waitingLoop,SLOT(quit()));
    workerThread->start();
    /* If we don't run an eventloop here, it will close before postrun because there are no more events to process and nothing blocking the thread.
     * This is likely the fix to that problem.
    */
    waitingLoop.exec();
}

void JasonGui::showMessage(int status, QString message){
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

void JasonGui::resizeWindow(int width, int height){
    setFixedWidth(width);
    setFixedHeight(height);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}

void JasonGui::hideJasonGui(bool doShow){
    if(doShow)
        showNormal();
    if(!doShow)
        showMinimized();
}

void JasonGui::detachedMessage(QString title){
    QMessageBox *detachedProgramNotify = new QMessageBox(this);
    QString windowTitle = tr("Jason - Detached process");
    QString text = tr("%1 is currently running in a detached state. Close this window to notify Jason when it has been closed properly.").arg(title);
    detachedProgramNotify->setText(text);
    detachedProgramNotify->setWindowTitle(windowTitle);
    detachedProgramNotify->setButtonText(0,tr("It is closed"));
    connect(detachedProgramNotify,SIGNAL(finished(int)),this,SLOT(detachedProgramNotifyEmit(int)));
    detachedProgramNotify->show();
}

void JasonGui::detachedProgramNotifyEmit(int retInt){
    emit detachedHasClosed();
}

void JasonGui::showOutput(QString stdOut, QString stdErr){
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

void JasonGui::printMessage(QString message){
    //Simple implementation to print to stdout without messing with qDebug all over
    printf("%s\n",qPrintable(message));
}
