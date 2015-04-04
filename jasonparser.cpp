#include "jasonparser.h"

#include <QDebug>

JasonParser::JasonParser(){

}

JasonParser::~JasonParser(){
    for(QMetaObject::Connection cnct : connectedSlots)
        disconnect(cnct);
    delete parser;
}

void JasonParser::quitProcess(){
//    emit toggleCloseButton(true);
    emit failedProcessing();
}

void JasonParser::startParse(){
    updateProgressText(tr("Starting to parse JSON document"));

    exitResult=0;

    parser = new jsonparser();

    connectedSlots.append(connect(parser,&jsonparser::sendProgressTextUpdate,[=](QString message){this->updateProgressText(message);}));
    connectedSlots.append(connect(parser,&jsonparser::reportError,[=](int severity,QString message){exitResult++;this->broadcastMessage(severity,message);}));
    connectedSlots.append(connect(parser,&jsonparser::sendProgressBarUpdate,[=](int value){this->changeProgressBarValue(value);}));

    if(parser->jsonParse(parser->jsonOpenFile(startDoc))!=0){
        quitProcess();
        return;
    }
    updateProgressTitle(parser->getRunQueue()->getMainrun()->getTitle());

    if(!parser->hasCompleted()&&!desktopFile.isEmpty()){
        updateProgressText(tr("We are generating a .desktop file now. Please wait for possible on-screen prompts."));
        desktoptools desktopfilegenerator;
        connectedSlots.append(connect(&desktopfilegenerator,&desktoptools::sendProgressTextUpdate,[=](QString message){this->updateProgressText(message);}));
        connectedSlots.append(connect(&desktopfilegenerator,&desktoptools::reportError,[=](int severity,QString message){exitResult++;this->broadcastMessage(severity,message);}));

        desktopfilegenerator.generateDesktopFile(desktopfilegenerator.desktopFileBuild(parser->getDesktopFile()),desktopFile,jasonPath,startDoc);
        for(QMetaObject::Connection cnct : connectedSlots)
            disconnect(cnct);
    } else if(!b_dry){
        bool hideUi = false;
        QHash<QString,QVariant> windowOpts = parser->getWindowOpts();
        for(QString opt : windowOpts.keys())
            if(opt=="jason.hide-ui-on-run")
                hideUi = windowOpts.value(opt).toBool();
        updateProgressText(tr("Executing program queue"));
        hideMainWindow(hideUi);
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
    } else
        for(ExecutionUnit* unit : parser->getRunQueue()->getQueue()){
            qDebug() << "Execution:";
            qDebug() << "command:" << unit->getExecString();
            qDebug() << "workdir:" << unit->getWorkDir();
            qDebug() << "title:" << unit->getTitle();
            qDebug() << "env:" << unit->getEnvironment() << "\n";
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

void JasonParser::detachedProgramExit(){
    emit detachedRunEnd();
}
