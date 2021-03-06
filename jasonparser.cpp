#include "jasonparser.h"

#include <QDebug>

JasonParser::JasonParser(){

}

JasonParser::~JasonParser(){
    for(QMetaObject::Connection cnct : connectedSlots)
        disconnect(cnct);
    delete parser;
}

void JasonParser::testEnvironment(){

}

void JasonParser::quitProcess(){
    emit toggleCloseButton(true);
    emit failedProcessing();
}

void JasonParser::startParse(){
    updateProgressText(tr("Starting to parse JSON document"));
    QString startDocument,actionId,desktopFile,jasonPath;
    startDocument=startOpts.value("start-document");
    actionId=startOpts.value("action-id");
    desktopFile=startOpts.value("desktop-file");
    jasonPath=startOpts.value("jason-path");

    exitResult=0;

    parser = new jsonparser();

    connectedSlots.append(connect(parser,&jsonparser::sendProgressTextUpdate,[=](QString message){this->updateProgressText(message);}));
    connectedSlots.append(connect(parser,&jsonparser::reportError,[=](int severity,QString message){exitResult++;this->broadcastMessage(severity,message);}));
    connectedSlots.append(connect(parser,&jsonparser::sendProgressBarUpdate,[=](int value){this->changeProgressBarValue(value);}));

    if(parser->jsonParse(parser->jsonOpenFile(startDocument))!=0){
        quitProcess();
        return;
    }
    updateProgressTitle(parser->getRunQueue()->getMainrun()->getTitle());

    if(!parser->hasCompleted()&&!desktopFile.isEmpty()){
        updateProgressText(tr("We are generating a .desktop file now. Please wait for possible on-screen prompts."));
        desktoptools desktopfilegenerator;
        connectedSlots.append(connect(&desktopfilegenerator,&desktoptools::sendProgressTextUpdate,[=](QString message){this->updateProgressText(message);}));
        connectedSlots.append(connect(&desktopfilegenerator,&desktoptools::reportError,[=](int severity,QString message){exitResult++;this->broadcastMessage(severity,message);}));
        QVariant object = jsonFinalData->value("activeopts").toHash().value("desktop.file");
        if(object.isValid()&&object.type()==QVariant::Hash){
            desktopfilegenerator.generateDesktopFile(desktopfilegenerator.desktopFileBuild(object.toHash()),desktopFile,jasonPath,startDocument);
        }
        for(QMetaObject::Connection cnct : connectedSlots)
            disconnect(cnct);
    } else {
        bool hideUi = false;
        QHash<QString,QVariant> windowOpts = parser->getWindowOpts();
        for(QString opt : windowOpts.keys())
            if(opt=="jason.hide-ui-on-run")
                hideUi = windowOpts.value(opt).toBool();
        updateProgressText(tr("Executing program queue"));
        toggleProgressVisible(!hideUi);
        QEventLoop waiter;
        for(ExecutionUnit* unit : parser->getRunQueue()->getQueue()){
            Executer e(this,parser->getShell(),parser->getShellArg());
            QMetaObject::Connection logger = connect(&e,&Executer::emitOutput,[=](QString out,QString err){this->emitOutput(out,err);qDebug() << out << err;});
            if(!unit->getTitle().isEmpty())
                updateProgressText(unit->getTitle());
            if(unit->isDetachable()){
                connect(&e,SIGNAL(finished()),&waiter,SLOT(quit()));
            } else
                connect(this,SIGNAL(detachedRunEnd()),&waiter,SLOT(quit()));
            int returnValue = e.exec(unit);
            if(unit->isDetachable()){
                emit displayDetachedMessage(unit->getTitle());
                waitLoop->exec();
            }
            exitResult+=returnValue;
            disconnect(logger);
        }
    }

    if(exitResult!=0){
        quitProcess();
        return;
    }else{
        updateProgressText(tr("All done!"));
        emit finishedProcessing();
    }
    return;
}

void JasonParser::setStartOpts(QString startDocument, QString actionId, QString desktopFile, QString jasonPath){
    if(!startDocument.isEmpty())
        startOpts.insert("start-document",startDocument);
    if(!actionId.isEmpty())
        startOpts.insert("action-id",actionId);
    if(!desktopFile.isEmpty())
        startOpts.insert("desktop-file",desktopFile);
    if(!jasonPath.isEmpty())
        startOpts.insert("jason-path",jasonPath);
    QFileInfo cw(startDocument);
    startOpts.insert("working-directory",cw.absolutePath());
    return;
}

void JasonParser::detachedMainProcessClosed(){
    emit detachedRunEnd();
}
