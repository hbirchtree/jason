#include "jasonparser.h"
#include "executer.h"
#include "jsonparser.h"
#include "desktoptools.h"

JasonParser::JasonParser(){

}

JasonParser::~JasonParser(){

}

void JasonParser::testEnvironment(){

}

void JasonParser::startParse(){
    updateProgressText(tr("Starting to parse JSON document"));
    QString startDocument,actionId,desktopFile,jasonPath;
    startDocument=startOpts.value("start-document");
    actionId=startOpts.value("action-id");
    desktopFile=startOpts.value("desktop-file");
    jasonPath=startOpts.value("jason-path");

    QHash<QString,QVariant> jsonFinalData;
    //Populated with QHash "systems", QHash "activeopts", QList "subsystems", QHash "variables", QHash "procenv"

    jsonparser parser;

    connect(&parser,SIGNAL(sendProgressTextUpdate(QString)),this,SLOT(forwardProgressTextUpdate(QString)));

    if(parser.jsonParse(parser.jsonOpenFile(startDocument),&jsonFinalData)!=0){
        emit toggleCloseButton(true);
        emit failedProcessing();
        jsonFinalData.clear();
        return;
    }

//    qDebug() << jsonFinalData.value("systems").toHash() << "\n\n" << jsonFinalData.value("subsystems").toList() << "\n\n" << jsonFinalData.value("activeopts").toHash();

    if(!desktopFile.isEmpty()){
        updateProgressText(tr("We are generating a .desktop file now. Please wait for possible on-screen prompts."));
        desktoptools desktopfilegenerator;
        QVariant object = jsonFinalData.value("activeopts").toHash().value("desktop.file");
        if(object.isValid()&&object.type()==QVariant::Hash){
            desktopfilegenerator.desktopFileBuild(object.toHash(),jsonFinalData.value("systems").toHash());
        }
    } else {
        QHash<QString,QVariant> runtimeValues;
        if(parser.jasonActivateSystems(jsonFinalData,&runtimeValues)!=0){
            emit toggleCloseButton(true);
            emit failedProcessing();
            return;
        }
        if(runtimeValues.value("jason-opts").toHash().value("jason.window-dimensions").isValid()){
            QString source = runtimeValues.value("jason-opts").toHash().value("jason.window-dimensions").toString();
            if(source.split("x").size()==2)
                changeWindowDimensions(source.split("x")[0].toInt(),source.split("x")[1].toInt());
        }
        if(executeQueue(runtimeValues,actionId)!=0){
            emit toggleCloseButton(true);
            emit failedProcessing();
            return;
        }
    }

    emit finishedProcessing();

//    if(desktopFile.isEmpty()){
//        if(runProcesses(actionId)!=0){
//            updateProgressText(tr("Error occured while trying to launch"));
//            broadcastMessage(2,tr("Shit.\n"));
//            emit toggleCloseButton(true);
//            emit failedProcessing();
//            return;
//        }
//    }else{
//        updateProgressText(tr("We are generating a .desktop file now. Please wait for possible on-screen prompts."));
//        generateDesktopFile(desktopFile,jasonPath,startDocument);
//    }
//    QEventLoop waitForEnd;
//    connect(this,SIGNAL(finishedProcessing()),&waitForEnd,SLOT(quit()));
//    waitForEnd.exec();
//    return;
}

void JasonParser::forwardProgressTextUpdate(QString message){
    updateProgressText(message);
}

void JasonParser::forwardProgressValueUpdate(int value){
    changeProgressBarValue(value);
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

int JasonParser::executeProcess(QString shell, QStringList arguments, QString workDir, QProcessEnvironment procEnv, bool lazyExitStatus, bool detached,QString title){
    Executer partyTime;
    QEventLoop waitLoop;

    partyTime.initializeProcess(shell,arguments,workDir,procEnv,lazyExitStatus);

    connect(&partyTime,SIGNAL(emitOutput(QString,QString)),SLOT(receiveLogOutput(QString,QString)));
    if(!detached){
        connect(&partyTime,SIGNAL(finished()),&waitLoop,SLOT(quit()));
    } else
        connect(this,SIGNAL(detachedRunEnd()),&waitLoop,SLOT(quit()));
    int returnValue = partyTime.exec();
    if(detached){
        emit displayDetachedMessage(title);
        waitLoop.exec();
    }
    return returnValue;
}

void JasonParser::detachedMainProcessClosed(){
    emit detachedRunEnd();
}

void JasonParser::receiveLogOutput(QString stdOut, QString stdErr){
    emit emitOutput(stdOut,stdErr);
}

int JasonParser::executeInstance(QHash<QString,QVariant> const &shellData,QHash<QString, QVariant> const &execInstance,QProcessEnvironment const &procEnv){
    if(execInstance.value("desktop.title").isValid())
        updateProgressText(execInstance.value("desktop.title").toString());
    if(execInstance.value("desktop.icon").isValid())
        updateProgressIcon(execInstance.value("desktop.icon").toString());
    QProcessEnvironment localProcEnv = procEnv;
    QString shell = shellData.value("shell").toString(); //Setting defaults
    QString shellArg = shellData.value("shell.argument").toString();
    if(shell.isEmpty()){
        shell="sh";
        shellArg="-c";
    }
    if(execInstance.value("procenv").isValid()){
        foreach(QString key,execInstance.value("procenv").toHash().keys())
            localProcEnv.insert(key,execInstance.value("procenv").toHash().value(key).toString());
    }
    QStringList arguments;
    arguments.append(shellArg);
    arguments.append(execInstance.value("exec").toString());
    QString workDir = execInstance.value("workdir").toString();
    bool lazyExit = execInstance.value("lazyexit").toBool();
    bool detached = execInstance.value("detach").toBool();
    return executeProcess(shell,arguments,workDir,localProcEnv,lazyExit,detached,execInstance.value("desktop.title").toString());
}

int JasonParser::executeQueue(QHash<QString,QVariant> const &runtimeValues,QString actionId){
    updateProgressText(tr("Executing queue"));
    bool hideUi;
    if(runtimeValues.value("jason-opts").toHash().value("jason.hide-ui-on-run").isValid())
        hideUi = runtimeValues.value("jason-opts").toHash().value("jason.hide-ui-on-run").toBool();
    QHash<QString,QVariant> shellData = runtimeValues.value("shelldata").toHash();
    QProcessEnvironment procEnv = QProcessEnvironment::systemEnvironment();
    if(runtimeValues.value("procenv").isValid()){
        QHash<QString,QVariant> procEnvHash = runtimeValues.value("procenv").toHash();
        foreach(QString key,procEnvHash.keys()){
            procEnv.insert(key,procEnvHash.value(key).toString());
        }
    }
    //Check if we have an action; we won't do prerun and postrun for it.
    if(!actionId.isEmpty())
        if(runtimeValues.value("actions").toHash().value(actionId).isValid()){
            if(hideUi)
                toggleProgressVisible(false);
            return executeInstance(shellData,runtimeValues.value("actions").toHash().value(actionId).toHash(),procEnv);
        } else {
            updateProgressText(tr("Unknown action ID specified."));
            return 1;
        }

    QList<QVariant> preruns = runtimeValues.value("prerun").toList();
    QHash<QString,QVariant> main = runtimeValues.value("main").toHash();
    QList<QVariant> postruns = runtimeValues.value("postrun").toList();

    updateProgressText(tr("Executing prerun stage"));
    for(int i=0;i<preruns.size();i++){
        executeInstance(shellData,preruns.at(i).toHash(),procEnv);
    }
    if(hideUi)
        toggleProgressVisible(false);
    executeInstance(shellData,main,procEnv);
    if(hideUi)
        toggleProgressVisible(true);
    updateProgressText(tr("Executing postrun stage"));
    for(int i=0;i<postruns.size();i++){
        executeInstance(shellData,postruns.at(i).toHash(),procEnv);
    }

    return 0;
}
