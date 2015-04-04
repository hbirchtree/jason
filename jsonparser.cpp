#include "jsonparser.h"

#include <QDebug>

jsonparser::jsonparser(QObject *parent) :
    QObject(parent)
{
}

QJsonDocument jsonparser::jsonOpenFile(QString filename){
    QFile jDocFile;
    QFileInfo cw(filename);
    if(startDir.isEmpty()){
        startDir = cw.absolutePath();
        jDocFile.setFileName(startDir+"/"+cw.fileName());
    }else
        jDocFile.setFileName(startDir+"/"+cw.fileName());
    if (!jDocFile.exists()) {
        sendProgressTextUpdate(tr("Failed due to the file %1 not existing").arg(jDocFile.fileName()));
        return QJsonDocument();
    }
    if (!jDocFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        sendProgressTextUpdate(tr("Failed to open the requested file"));
        return QJsonDocument();
    }
    QJsonParseError initError;
    QJsonDocument jDoc = QJsonDocument::fromJson(jDocFile.readAll(),&initError);
    if (initError.error != 0){
        reportError(2,tr("ERROR: jDoc: %s\n").arg(initError.errorString()));
        sendProgressTextUpdate(tr("An error occured. Please take a look at the error message to see what went wrong."));
        return QJsonDocument();
    }
    if (jDoc.isNull() || jDoc.isEmpty()) {
        sendProgressTextUpdate(tr("Failed to import file"));
        return QJsonDocument();
    }
    return jDoc;
}

void jsonparser::getMap(QVariantMap* depmap,QVariantMap* totalMap,ActiveOptionsContainer* activeOpts, JasonCore* jCore){
    QStringList excludes = jCore->getCoreElements();
    for(QVariantMap::const_iterator it = depmap->begin(); it!=depmap->end(); it++){
        if(!excludes.contains(it.key())){
            activeOpts->addOption(it.key(),it.value());
            continue;
        }
        if(it.key()=="imports")
            for(QVariant el : it.value().toList()){
                getMap(new QVariantMap(jsonOpenFile(el.toMap().value("file").toString()).object().toVariantMap()),totalMap,activeOpts,jCore);
                continue;
            }
        totalMap->insertMulti(it.key(),it.value()); //We want to generate a complete map of all values from all files. We'll deal with conflicts later.
    }
}

int jsonparser::jsonParse(QJsonDocument jDoc){
    ActiveOptionsContainer* activeOpts = new ActiveOptionsContainer();
    VariableHandler* varHandler = new VariableHandler();
    EnvironmentContainer* envContainer = new EnvironmentContainer(this,varHandler);
    JasonCore* jCore = new JasonCore(this,varHandler,envContainer);

    QVariantMap* totalMap = new QVariantMap();

    connect(varHandler,&VariableHandler::sendProgressTextUpdate,[=](QString message){this->sendProgressTextUpdate(message);});
    connect(jCore,&JasonCore::reportError,[=](int severity,QString message){this->reportError(severity,message);});

    QJsonObject mainTree = jDoc.object();
    if((mainTree.isEmpty())||(jDoc.isEmpty())){
//        sendProgressTextUpdate(tr("No objects found. Will not proceed."));
        return 1;
    }

    QVariantMap *mainMap = new QVariantMap(mainTree.toVariantMap());

    getMap(mainMap,totalMap,activeOpts,jCore);

    QList<QVariant> shellOpts = activeOpts->getOption("shell.properties");
    desktopFile = StatFuncs::mapToHash(activeOpts->getOption("desktop.file").first().toMap());
    QProcessEnvironment sysEnv = QProcessEnvironment::systemEnvironment();
    for(QVariant opt : shellOpts){
        QMap<QString,QVariant> optMap = opt.toMap();
        for(QString var : optMap.value("import-env-variables").toString().split(",")){
            varHandler->variableHandle(var,sysEnv.value(var));
        }
        for(QString key : optMap.keys()){
            if(key=="shell")
                shellString = optMap.value(key).toString();
            if(key=="shell.argument")
                shellArgString = optMap.value(key).toString();
        }
    }
    QList<QVariant> jOpts = activeOpts->getOption("global.jason-opts");
    for(QVariant optMap : jOpts){
        QMap<QString,QVariant> opts = optMap.toMap();
        for(QString optkey : opts.keys())
            windowOpts.insert(optkey,opts.value(optkey));
    }

    runQueue = jCore->resolveDependencies(totalMap,activeOpts);

    for(QString key : desktopFile.keys()) //Get actions
        if(key.startsWith("desktop.action.")){
            actions.insert(key.split(".").at(2),new ExecutionUnit(this,varHandler,activeOpts,StatFuncs::mapToHash(desktopFile.value(key).toMap()),jCore->getSystems()));
        }

    if(runQueue->getQueue().size()>0)
        b_hasCompleted = true;

    return 0;
}
