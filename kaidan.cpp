#include "kaidan.h"
#include "ui_kaidan.h"
#include "jasonparser.h"

Kaidan::Kaidan(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Kaidan)
{
    ui->setupUi(this);
    ui->progressBar_2->setHidden(true);
}

Kaidan::~Kaidan()
{
    for(QMetaObject::Connection cnct : connections)
        disconnect(cnct);
    delete ui;
}

void Kaidan::initializeParse(QString startDocument, QString actionId, QString desktopFile, QString jasonPath,bool dryrun){

    UIGlue *parser = new JasonParser();
    QThread parserThread;

    parser->moveToThread(&parserThread);
    parser->setStartDoc(startDocument);
    parser->setActionId(actionId);
    parser->setDesktopFile(desktopFile);
    parser->setPath(jasonPath);
    parser->setDryRun(dryrun);

    if(!parser->isReady())
        return;

    connect(parser,SIGNAL(failedProcessing()),&parserThread,SLOT(quit()));
    connect(parser,SIGNAL(finishedProcessing()),&parserThread,SLOT(quit()));
    connect(&parserThread,SIGNAL(started()),parser,SLOT(startParse()));

    connections.append(connect(parser,&UIGlue::finishedProcessing,[=](){
        updateMainProgressBarLimits(0,100);
        updateMainProgressBarValue(100);
        ui->progressBar->setTextVisible(true);
        modifySecondPBarVisibility(false);
        updateProgressText(tr("Everything done."));
    }));

    connections.append(connect(parser,&UIGlue::failedProcessing,[=](){
        updateMainProgressBarLimits(0,100);
        updateMainProgressBarValue(100);
        ui->progressBar->setTextVisible(true);
        modifySecondPBarVisibility(false);
    }));
    //Cosmetics
    connections.append(connect(parser,&UIGlue::broadcastMessage,[=](int status,QString msg){
        QMessageBox *messageBox = new QMessageBox(this);
        messageBox->setText(msg);
        if(status==0)
            messageBox->setWindowTitle(tr("Information"));
        if(status==1)
            messageBox->setWindowTitle(tr("Warning"));
        if(status==2)
            messageBox->setWindowTitle(tr("Error"));
        messageBox->show();
    }));
    connections.append(connect(parser,&UIGlue::emitOutput,[=](QString stdOut,QString stdErr){
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
    }));
    connections.append(connect(parser,&UIGlue::updateIconSize,[=](int size){
        int oldSize = ui->iconLabel->height();
        ui->iconLabel->setMinimumSize(size,size);
        ui->iconLabel->setMaximumSize(size,size);
        setMinimumHeight(height()-oldSize+size);
        setMaximumHeight(height()-oldSize+size);
    }));
    connections.append(connect(parser,&UIGlue::hideMainWindow,[=](bool hide){
        if(hide)
            setWindowState(Qt::WindowMinimized);
        else
            setWindowState(Qt::WindowActive);
    }));
    connections.append(connect(parser,&UIGlue::displayDetachedMessage,[=](QString title){
        QMessageBox *detachedProgramNotify = new QMessageBox(this);
        detachedProgramNotify->setText(tr("%1 is currently running in a detached state. Close this window to notify Jason when it has been closed properly.").arg(title));
        detachedProgramNotify->setWindowTitle(tr("Jason - Detached process"));
        detachedProgramNotify->setButtonText(0,tr("It is closed"));
        connections.append(connect(detachedProgramNotify,&QMessageBox::finished,[=](int ret){
            parser->detachedProgramExit();
        }));
        detachedProgramNotify->show();
    }));
    connect(parser,SIGNAL(changeProgressBarRange(qint64,qint64)),this,SLOT(updateMainProgressBarLimits(qint64,qint64)));
    connect(parser,SIGNAL(changeProgressBarValue(int)),this,SLOT(updateMainProgressBarValue(int)));
    connect(parser,SIGNAL(changeSecondProgressBarRange(qint64,qint64)),this,SLOT(updateSecondProgressBarLimits(qint64,qint64)));
    connect(parser,SIGNAL(changeSecondProgressBarValue(int)),this,SLOT(updateSecondaryProgressBarValue(int)));
    connect(parser,SIGNAL(changeSecondProgressBarVisibility(bool)),this,SLOT(modifySecondPBarVisibility(bool)));
    connect(parser,SIGNAL(updateProgressIcon(QString)),this,SLOT(updateProgressIcon(QString)));
    connect(parser,SIGNAL(updateProgressText(QString)),this,SLOT(updateProgressText(QString)));
    connect(parser,SIGNAL(updateProgressTitle(QString)),this,SLOT(updateWindowTitle(QString)));
    QEventLoop workerLoop;
    connect(&parserThread,SIGNAL(finished()),&workerLoop,SLOT(quit()));
    parserThread.start();
    show();
    workerLoop.exec();
    delete parser;
}

void Kaidan::updateProgressText(QString newText){
    if(!newText.isEmpty())
        ui->infoLabel->setText(newText);
}

void Kaidan::updateProgressIcon(QString newIconPath){
    QFile *imageFile = new QFile(newIconPath);
    if(!imageFile->exists())
        return;
    QImage newIcon(newIconPath);
    qreal iconScale = ui->iconLabel->height();
    ui->iconLabel->setPixmap(QPixmap::fromImage(newIcon.scaled(iconScale,iconScale)));
}

void Kaidan::updateWindowTitle(QString newTitle){
    if(!newTitle.isEmpty())
        setWindowTitle(newTitle);
}

void Kaidan::updateMainProgressBarLimits(qint64 min, qint64 max){
    ui->progressBar->setMinimum(min);
    ui->progressBar->setMaximum(max);
}

void Kaidan::updateMainProgressBarValue(int val){
    ui->progressBar->setValue(val);
}

void Kaidan::updateSecondaryProgressBarValue(int val){
    ui->progressBar_2->setValue(val);
}

void Kaidan::updateSecondProgressBarLimits(qint64 min, qint64 max){
    ui->progressBar_2->setMinimum(min);
    ui->progressBar_2->setMaximum(max);
}

void Kaidan::modifySecondPBarVisibility(bool doShow){
    ui->progressBar_2->setHidden(!doShow); //Inverted statement because setHidden does the opposite
}
