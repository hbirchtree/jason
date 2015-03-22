#include "jasongui.h"
#include "ui_jasongui.h"

JasonGui::JasonGui(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JasonGui)
{
    ui->setupUi(this);
    ui->closeButton->setEnabled(false);
    setWindowFlags(Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint);
}

JasonGui::~JasonGui()
{
    delete ui;
    delete jParse;
}

void JasonGui::startParse(QString startDocument, QString actionId, QString desktopFile, QString jasonPath){
    show();

    //Open document
    jParse = new JasonParser();
    workerThread = new QThread();
    jParse->moveToThread(workerThread);
    jParse->setStartOpts(startDocument,actionId,desktopFile,jasonPath);
    connect(workerThread,SIGNAL(started()),jParse,SLOT(startParse()),Qt::QueuedConnection);
    connect(jParse,SIGNAL(finishedProcessing()),workerThread,SLOT(quit()),Qt::QueuedConnection);
    connect(jParse,SIGNAL(failedProcessing()),workerThread,SLOT(quit()),Qt::QueuedConnection);

    //Progress window
    connect(jParse,SIGNAL(updateProgressText(QString)),ui->statusLabel,SLOT(setText(QString)));
    connect(jParse,SIGNAL(updateProgressText(QString)),this,SLOT(printMessage(QString))); //Print messages directed for the GUI to stdout, so that we may read them even if the GUI swishes by.
    connect(jParse,SIGNAL(updateProgressTitle(QString)),this,SLOT(setWindowTitle(QString)));
    connect(jParse,SIGNAL(finishedProcessing()),this,SLOT(close()));
    connect(jParse,SIGNAL(toggleCloseButton(bool)),ui->closeButton,SLOT(setEnabled(bool)));
    connect(jParse,SIGNAL(toggleProgressVisible(bool)),this,SLOT(hideJasonGui(bool)));
    connect(jParse,SIGNAL(changeProgressBarRange(int,int)),ui->progressBar,SLOT(setRange(int,int)));
    connect(jParse,SIGNAL(changeProgressBarValue(int)),ui->progressBar,SLOT(setValue(int)));

    //Displaying messages and etc
    connect(jParse,SIGNAL(broadcastMessage(int,QString)),SLOT(showMessage(int,QString)));
    connect(jParse,SIGNAL(displayDetachedMessage(QString)),SLOT(detachedMessage(QString)));
    connect(this,SIGNAL(detachedHasClosed()),jParse,SLOT(detachedMainProcessClosed()));
    connect(jParse,SIGNAL(emitOutput(QString,QString)),this,SLOT(showOutput(QString,QString)));

    connect(ui->closeButton,SIGNAL(clicked()),this,SLOT(close()));

    connect(jParse,SIGNAL(finishedProcessing()),&waitingLoop,SLOT(quit()));
    connect(this,SIGNAL(destroyed()),&waitingLoop,SLOT(quit()));
    connect(ui->closeButton,SIGNAL(clicked()),&waitingLoop,SLOT(quit()));
//    connect(jParse,SIGNAL(failedProcessing()),&waitingLoop,SLOT(quit()));
    workerThread->start();
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
    setMinimumHeight(height);
    setMaximumHeight(height);
    setMinimumWidth(width);
    setMaximumWidth(width);
}

void JasonGui::hideJasonGui(bool doShow){
    if(!doShow)
        setWindowState(Qt::WindowMinimized);
    if(doShow)
        setWindowState(Qt::WindowActive);
}

void JasonGui::detachedMessage(QString title){
    QMessageBox *detachedProgramNotify = new QMessageBox(this);
    detachedProgramNotify->setText(tr("%1 is currently running in a detached state. Close this window to notify Jason when it has been closed properly.").arg(title));
    detachedProgramNotify->setWindowTitle(tr("Jason - Detached process"));
    detachedProgramNotify->setButtonText(0,tr("It is closed"));
    connect(detachedProgramNotify,SIGNAL(finished(int)),this,SLOT(detachedProgramNotifyEmit(int)));
    detachedProgramNotify->show();
}

void JasonGui::detachedProgramNotifyEmit(int retInt){
    emit detachedHasClosed();
}

void JasonGui::showOutput(QString stdOut, QString stdErr){
    QDialog *outputWindow = new QDialog(this);
    QGridLayout *outputLayout = new QGridLayout(outputWindow);
    QTextEdit *errEdit = new QTextEdit(outputWindow);
    QTextEdit *outEdit = new QTextEdit(outputWindow);
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
