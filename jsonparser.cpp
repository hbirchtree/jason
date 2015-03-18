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

int jsonparser::parseStage1(QJsonObject mainObject,QHash<QString,QVariant> *systemTable,QHash<QString,QVariant> *substitutes, QList<QVariant> *subsystems,QStringList *importedFiles, QHash<QString,QVariant> *activeOptions,QHash<QString,QVariant> *procEnv){
    if(!mainObject.value("systems").isUndefined()){
        QJsonArray sysArray = mainObject.value("systems").toArray();
        for(int i=0;i<sysArray.size();i++){
            QHash<QString,QVariant> systemElement = jsonExamineObject(sysArray.at(i).toObject());
            if(systemHandle(&systemElement)!=0)
                return 1;
            systemTable->insert(systemElement.value("identifier").toString(),systemElement);
        }
    }
    if(!mainObject.value("subsystems").isUndefined()){
        QJsonArray subsArray = mainObject.value("subsystems").toArray();
        for(int i=0;i<subsArray.size();i++){
            QHash<QString,QVariant> subsystemElement = jsonExamineObject(subsArray.at(i).toObject());
            if(subsystemHandle(&subsystemElement)!=0)
                return 1;
            subsystems->append(subsystemElement);
        }
    }
    if(!mainObject.value("imports").isUndefined()){
        QJsonArray *impArray = new QJsonArray(mainObject.value("imports").toArray());
        for(int i=0;i<impArray->size();i++){
            QString impFile = impArray->at(i).toObject().value("file").toString();
            importedFiles->append(impFile);
            QHash<QString,QVariant> *resultHash = new QHash<QString,QVariant>();
            jsonParse(jsonOpenFile(impFile),resultHash);
            if(!resultHash->isEmpty()){
                if(resultHash->value("systems").isValid())
                    *systemTable = systemTable->unite(resultHash->value("systems").toHash());
                if(resultHash->value("subsystems").isValid())
                    subsystems->append(resultHash->value("subsystems").toList());
                if(resultHash->value("activeopts").isValid()){ //When merging, we want to avoid having keys with the same names.
                    QVariant *actOpts = new QVariant(resultHash->value("activeopts"));
                    foreach(QString key,actOpts->toHash().keys()){
                        QVariant *instance = new QVariant(actOpts->toHash().value(key));
                        QString insertKey = key;
                        int insertInt = 0;
                        while(activeOptions->contains(insertKey)){
                            insertInt++;
                            insertKey = key+"."+QString::number(insertInt);
                        }
                        activeOptions->insert(insertKey,*instance);
                        delete instance;
                    }
                    delete actOpts;
                }
                if(resultHash->value("variables").isValid())
                    *substitutes = substitutes->unite(resultHash->value("variables").toHash());
                if(resultHash->value("procenv").isValid())
                    *procEnv = procEnv->unite(resultHash->value("procenv").toHash());
            }
            delete resultHash;
        }
    }
    if(!mainObject.value("variables").isUndefined()){
        variablesImport(jsonExamineArray(mainObject.value("variables").toArray()),substitutes,*activeOptions);
    }
    return 0;
}

int jsonparser::stage2ActiveOptionAdd(QHash<QString,QVariant> *activeOptions,QJsonValue instance,QString key){
    if(instance.isNull())
        return 1;
    QString insertKey = key;
    int insertInt = 0;
    while(activeOptions->contains(insertKey)){
        insertInt++;
        insertKey = key+"."+QString::number(insertInt);
    }
    if(instance.isObject()){
        activeOptions->insert(insertKey,jsonExamineObject(instance.toObject()));
    } else if(instance.isArray()){
        QList<QVariant> *mergeArray = new QList<QVariant>(jsonExamineArray(instance.toArray()));
        if(activeOptions->value(key).type()==QVariant::List){
            mergeArray->append(activeOptions->value(key).toList());
            activeOptions->insert(key,*mergeArray);
        }else
            activeOptions->insert(insertKey,*mergeArray);
        delete mergeArray;
    } else
        activeOptions->insert(insertKey,instance.toVariant());
    return 0;
}

void jsonparser::parseStage2(QJsonObject mainObject,QHash<QString,QVariant> *activeOptions){
    /*
     * Stage 2:
     * Where options specified by the user or distributor are applied.
     * Parses through the options of the initial file only, will see if it is a good idea to parse
     * through the imported files as well.
    */

    foreach(QString key,mainObject.keys()){
        if((key!="variables")&&
                (key!="subsystems")&&
                (key!="imports")&&
                (key!="systems")
                )
            stage2ActiveOptionAdd(activeOptions,mainObject.value(key),key);
    }
}

int jsonparser::jsonParse(QJsonDocument jDoc, QHash<QString, QVariant> *targetHash){ //level is used to identify the parent instance of jsonOpenFile
    QHash<QString,QVariant> *systemTable = new QHash<QString,QVariant>();
    QHash<QString,QVariant> *activeOptions = new QHash<QString,QVariant>();
    QHash<QString,QVariant> *variables = new QHash<QString,QVariant>();
    QHash<QString,QVariant> *procEnv = new QHash<QString,QVariant>();
    QList<QVariant> *subsystems = new QList<QVariant>();
    QStringList importedFiles;

    QJsonObject *mainTree = new QJsonObject(jDoc.object());
    if((mainTree->isEmpty())||(jDoc.isEmpty())){
//        sendProgressTextUpdate(tr("No objects found. Will not proceed."));
        return 1;
    }

    sendProgressTextUpdate(tr("Gathering active options"));
    parseStage2(*mainTree,activeOptions);
    foreach(QString import,importedFiles){
        parseStage2(jsonOpenFile(import).object(),activeOptions);
    } //Yes, very odd, but we want to populate activeOptions before running stage 1.

    sendProgressTextUpdate(tr("Gathering fundamental values"));
    if(parseStage1(*mainTree,systemTable,variables,subsystems,&importedFiles,activeOptions,procEnv)!=0){
        sendProgressTextUpdate(tr("Failed to parse fundamental values. Will not proceed."));
        return 1;
    }
    delete mainTree;

    if(activeOptions->value("shell.properties").toHash().value("import-env-variables").isValid()){
        sendProgressTextUpdate(tr("Importing environment variables"));
        QProcessEnvironment envSource = QProcessEnvironment::systemEnvironment();
        foreach(QString var,activeOptions->value("shell.properties").toHash().value("import-env-variables").toString().split(",")){
            if(envSource.keys().contains(var))
                variableHandle(variables,var,envSource.value(var));
        }
    }

    targetHash->insert("systems",*systemTable);
    targetHash->insert("variables",*variables);
    targetHash->insert("activeopts",*activeOptions);
    targetHash->insert("subsystems",*subsystems);
    targetHash->insert("procenv",*procEnv);

    delete systemTable;
    delete variables;
    delete activeOptions;
    delete subsystems;
    delete procEnv;

    return 0;
}

QList<QVariant> jsonparser::jsonExamineArray(QJsonArray jArray){
    QList<QVariant> returnTable;
    //Iterate over the input array
    for(int i = 0;i<jArray.count();i++){
        QJsonValue instance = jArray.at(i);
        if (instance.isArray()){
            //Method used when an array is found
            returnTable.append(jsonExamineArray(instance.toArray()));
        } if (instance.isObject()) {
            //Method used when an object is found
            returnTable.append(jsonExamineObject(instance.toObject()));
        } else {
            // When all else fails, return the possible other type of value from the QJsonValue
            returnTable.append(instance.toVariant());
        }
    }
    return returnTable;
}

QHash<QString,QVariant> jsonparser::jsonExamineObject(QJsonObject jObject){
    QHash<QString,QVariant> returnTable; //Create a list for the returned objects
    //Objects may contain all different kinds of values, take care of this
    foreach(QString key, jObject.keys()){
        if(jObject.value(key).isObject()){
            returnTable.insert(key,jsonExamineObject(jObject.value(key).toObject()));
        } else if(jObject.value(key).isArray()){
            returnTable.insert(key,jsonExamineArray(jObject.value(key).toArray()));
        } else
            returnTable.insert(key,jObject.value(key).toVariant());
    }
    return returnTable;
}

int jsonparser::subsystemHandle(QHash<QString,QVariant> *subsystemElement){
    QHash<QString, QVariant> *insertHash = new QHash<QString, QVariant>();
    //We just dump everything into the hash, resolving arrays and objects
    foreach(QString key, subsystemElement->keys()){
        if(subsystemElement->value(key).type()==QMetaType::QJsonObject){
            insertHash->insert(key,jsonExamineObject(subsystemElement->value(key).toJsonObject()));
        } else if(subsystemElement->value(key).type()==QMetaType::QJsonArray){
            insertHash->insert(key,jsonExamineArray(subsystemElement->value(key).toJsonArray()));
        } else
            insertHash->insert(key,subsystemElement->value(key));
    }
    subsystemElement = insertHash;
    delete insertHash;
    return 0;
}

int jsonparser::systemHandle(QHash<QString, QVariant> *systemElement){
    QHash<QString,QVariant> *systemHash = new QHash<QString,QVariant>();
    //We verify that it contains the keys we need
    if(!systemElement->value("config-prefix").isValid())
        return 1;
    if(!systemElement->value("identifier").isValid())
        return 1;

    foreach(QString key,systemElement->keys()){
        if(systemElement->value(key).type()==QMetaType::QJsonArray){
            systemHash->insert(key,jsonExamineArray(systemElement->value(key).toJsonArray()));
        } else if(systemElement->value(key).type()==QMetaType::QJsonObject){
            systemHash->insert(key,jsonExamineObject(systemElement->value(key).toJsonObject()));
        } else
            systemHash->insert(key,systemElement->value(key));
    }
    systemElement = systemHash;
    delete systemHash;
    return 0;
}

void jsonparser::variableHandle(QHash<QString, QVariant> *variables, QString key, QString value){
    //Insert variable in form "NAME","VAR"
    variables->insert(key,value);
}

void jsonparser::variablesImport(QList<QVariant> inputVariables, QHash<QString,QVariant> *variables, QHash<QString,QVariant> const &activeOptions){
    for(int i=0;i<inputVariables.size();i++){
        QHash<QString,QVariant> *varHash = new QHash<QString,QVariant>(inputVariables.at(i).toHash());
        QString varType;
        varType = varHash->value("type","undefined").toString();
        if(varType=="config-input"){
            //If no option is found, insert the default value
            QString defaultValue = varHash->value("default","undefined").toString();
            QString option = varHash->value("input").toString();
            if((!option.isEmpty())&&(varHash->value("name").isValid())){
                QString variable = activeOptions.value(option,defaultValue).toString();
                variables->insert(varHash->value("name").toString(),variable);
            }
        }else if(varHash->value("name").isValid()&&varHash->value("value").isValid()){
            variableHandle(variables,varHash->value("name").toString(),resolveVariable(*variables,varHash->value("value").toString()));
        }else
            reportError(1,"unsupported variable type:"+varType);
        delete varHash;
    }
}

void jsonparser::resolveVariables(QHash<QString, QVariant> *variables){
    int indicator = 0; //Indicator for whether the operation is done or not. May cause an infinite loop when a variable cannot be resolved.
    while(indicator!=1){
        foreach(QString key, variables->keys()){
            QString insert = variables->value(key).toString();
            variables->remove(key);
            variables->insert(key,resolveVariable(*variables,insert));
        }
        int indicatorLocal = 0;
        foreach(QString key,variables->keys())
            if(!variables->value(key).toString().contains("%")){
                indicatorLocal++;
            }else
                sendProgressTextUpdate(tr("Variable %1 is being slightly problematic.").arg(key));
        if(indicatorLocal==variables->count())
            indicator = 1;
    }
    return;
}

QString jsonparser::resolveVariable(QHash<QString, QVariant> const &variables, QString variable){
    foreach(QString sub, variables.keys()){
        QString replace = sub;
        replace.prepend("%");replace.append("%");
        variable = variable.replace(replace,variables.value(sub).toString());
    }
    return variable;
}

void jsonparser::setEnvVar(QHash<QString, QVariant> *procEnv, QString key, QString value){
    //Insert the variable into the environment
    procEnv->insert(key,value);
    return;
}

int jsonparser::jasonActivateSystems(const QHash<QString, QVariant> &jsonData, QHash<QString, QVariant> *runtimeValues){
    QHash<QString,QVariant> *systemTable = new QHash<QString,QVariant>(jsonData.value("systems").toHash());
    QHash<QString,QVariant> *activeOptions = new QHash<QString,QVariant>(jsonData.value("activeopts").toHash());
    QHash<QString,QVariant> *variables = new QHash<QString,QVariant>(jsonData.value("variables").toHash());
    QHash<QString,QVariant> *procEnv = new QHash<QString,QVariant>(jsonData.value("procenv").toHash());
    QList<QVariant> *subsystems = new QList<QVariant>(jsonData.value("subsystems").toList());

    sendProgressTextUpdate(tr("Activating main system"));
    QHash<QString,QVariant> *prepRunValues = new QHash<QString,QVariant>();
    //run-prefix: QString
    //run-suffix: QString
    //sys-prerun: QList
    //sys-postrun: QList

    QStringList activeSystems; //Will be used later to determine which preruns and postruns will be activated.
    //Handling system
    if((!activeOptions->value("launchtype").isValid())&&
            (!activeOptions->value("launchtype").type()==QMetaType::QString)){
        sendProgressTextUpdate(tr("Unable to determine the system in use. Cannot proceed."));
        return 1;
    }
    QHash<QString,QVariant> *systemObject = new QHash<QString,QVariant>(systemTable->value(activeOptions->value("launchtype").toString()).toHash());
    if(systemObject->isEmpty()){
        sendProgressTextUpdate(tr("Invalid system object. Cannot proceed."));
        return 1;
    }
    if(systemActivate(*systemObject,activeOptions,&activeSystems,variables,procEnv,prepRunValues)!=0){
        sendProgressTextUpdate(tr("Failed to set up system. Please check that the configuration is correct. Cannot proceed."));
        return 1;
    }
    //Handling inherited systems
    foreach(QString sys,activeSystems){ //Does not need extensive error-checking
        systemInherit(systemTable->value(sys).toHash(),activeOptions,&activeSystems,variables,procEnv,prepRunValues);
    }
    activeSystems.append(systemObject->value("identifier").toString());
    delete systemObject;

    sendProgressTextUpdate(tr("Activating subsystems"));
    //Handling subsystems
    for(int i=0;i<subsystems->size();i++)
        if(!subsystems->at(i).toHash().isEmpty()){
            QHash<QString,QVariant> *subsystemElement = new QHash<QString,QVariant>(subsystems->at(i).toHash());
            if(subsystemElement->value("enabler").isValid())
                if(activeOptions->value("subsystem."+subsystemElement->value("enabler").toString()).isValid())
                    subsystemActivate(subsystemElement,procEnv,variables,prepRunValues,activeOptions);
            delete subsystemElement;
        }


    sendProgressTextUpdate(tr("Resolving variables"));
    resolveVariables(variables); //We might have new variables in the system
    sendProgressTextUpdate(tr("Processing preruns and postruns"));
    //Handling preruns and postruns, the standard way!
    QList<QVariant> *prerunList = new QList<QVariant>();
    QList<QVariant> *postrunList = new QList<QVariant>();
    foreach(QString sys,activeSystems){
        QVariant *system = new QVariant(systemTable->value(sys));
        QString prefix = system->toHash().value("config-prefix").toString();
        delete system;
        foreach(QString opt,activeOptions->keys())
            if((opt.startsWith(prefix+".prerun"))||(opt.startsWith(prefix+".postrun")))
                if(activeOptions->value(opt).type()==QVariant::List){
                    QList<QVariant> *runs = new QList<QVariant>(activeOptions->value(opt).toList());
                    for(int i=0;i<runs->size();i++){
                        QHash<QString,QVariant> *currentRun = new QHash<QString,QVariant>(runs->at(i).toHash());
                        QHash<QString,QVariant> *outputExec = new QHash<QString,QVariant>();
                        int priority = currentRun->value("priority").toInt();
                        addExecution(*currentRun,outputExec);
                        if(opt.startsWith(prefix+".prerun"))
                            switch(priority){
                            case 0: prerunList->append(*outputExec); break;
                            case 1: prerunList->append(*outputExec); break;
                            case -1: prerunList->prepend(*outputExec); break;
                            }
                        if(opt.startsWith(prefix+".postrun"))
                            switch(priority){
                            case 0: postrunList->prepend(*outputExec); break;
                            case 1: postrunList->append(*outputExec); break;
                            case -1: postrunList->prepend(*outputExec); break;
                            }
                        delete currentRun;
                        delete outputExec;
                    }
                    delete runs;
                }
    }

    sendProgressTextUpdate(tr("Setting up execution queue"));
    //At this point everything is verified and ready to go. We will lastly do variable resolution on the command lines and other variables.
    QHash<QString,QString> prefixTable;
    foreach(QString sys,activeSystems){ //We prepare the launch prefixes
        prefixTable.insert(systemTable->value(sys).toHash().value("config-prefix").toString(),systemTable->value(sys).toHash().value("launch-prefix").toString());
    }
    //Containers for the execution elements
    QList<QVariant> *prerunQueue = new QList<QVariant>();
    QHash<QString,QVariant> *mainExec = new QHash<QString,QVariant>();
    QList<QVariant> *postrunQueue = new QList<QVariant>();
    QHash<QString,QVariant> *actions = new QHash<QString,QVariant>();
    QHash<QString,QVariant> shellData;

    //We'll reuse "currentRun"
    for(QVariant element : *prerunList){
        QHash<QString,QVariant> *currentRun = new QHash<QString,QVariant>(element.toHash());
        processExecutionElement(currentRun,*variables,prefixTable);
        prerunQueue->append(*currentRun);
        delete currentRun;
    }
    QList<QVariant> prerunQueueDupe = prepRunValues->value("sys-prerun").toList();
    for(QVariant element : prerunQueueDupe){
        QHash<QString,QVariant> *currentRun = new QHash<QString,QVariant>(element.toHash());
        processExecutionElement(currentRun,*variables,prefixTable);
        prerunQueue->append(*currentRun);
        delete currentRun;
    }
    for(QVariant element : *postrunList){
        QHash<QString,QVariant> *currentRun = new QHash<QString,QVariant>(element.toHash());
        processExecutionElement(currentRun,*variables,prefixTable);
        postrunQueue->append(*currentRun);
        delete currentRun;
    }
    QList<QVariant> postrunQueueDupe = prepRunValues->value("sys-postrun").toList();
    for(QVariant element : postrunQueueDupe){
        QHash<QString,QVariant> *currentRun = new QHash<QString,QVariant>(element.toHash());
        processExecutionElement(currentRun,*variables,prefixTable);
        postrunQueue->append(*currentRun);
        delete currentRun;
    }
    delete prerunList;
    delete postrunList;
    addExecution(*activeOptions,mainExec); //We'll grab all options from activeOptions, this should be standard.

    if(activeOptions->value("desktop.file").isValid())
        if(activeOptions->value("desktop.file").toHash().value("desktop.displayname").isValid())
            mainExec->insert("desktop.title",activeOptions->value("desktop.file").toHash().value("desktop.displayname").toString());
    if(activeOptions->value("desktop.file").isValid())
        if(activeOptions->value("desktop.file").toHash().value("desktop.icon").isValid())
            mainExec->insert("desktop.icon",activeOptions->value("desktop.file").toHash().value("desktop.icon").toString());
    processExecutionElement(mainExec,*variables,prefixTable);

    //Adding the run prefix and suffix is a great idea. (Fixed 14-11-10)
    mainExec->insert("exec",prepRunValues->value("run-prefix").toString()+" "+mainExec->value("exec").toString()+" "+prepRunValues->value("run-suffix").toString());

    foreach(QString desktopkey,activeOptions->value("desktop.file").toHash().keys())
        if(desktopkey.startsWith("desktop.action.")){
            QHash<QString,QVariant> *currentRun = new QHash<QString,QVariant>(activeOptions->value("desktop.file").toHash().value(desktopkey).toHash());
            QHash<QString,QVariant> *outputExec = new QHash<QString,QVariant>();
            addExecution(*currentRun,outputExec);
            if(outputExec->value("desktop.displayname").isValid())
                outputExec->insert("desktop.title",outputExec->value("desktop.displayname").toString());
            processExecutionElement(outputExec,*variables,prefixTable);
            actions->insert(desktopkey.split(".")[2],*outputExec);
            delete currentRun;
            delete outputExec;
        }

    shellData.insert("shell",activeOptions->value("shell.properties").toHash().value("shell").toString());
    shellData.insert("shell.argument",activeOptions->value("shell.properties").toHash().value("shell.argument").toString());

    //AT LONG FUCKING LAST
    runtimeValues->insert("prerun",*prerunQueue);
    runtimeValues->insert("actions",*actions);
    runtimeValues->insert("main",*mainExec);
    runtimeValues->insert("postrun",*postrunQueue);
    runtimeValues->insert("procenv",*procEnv);
    runtimeValues->insert("shelldata",shellData);
    runtimeValues->insert("jason-opts",activeOptions->value("global.jason-opts").toHash());

    delete systemTable;
    delete variables;
    delete activeOptions;
    delete subsystems;
    delete procEnv;
    delete prerunQueue;
    delete postrunQueue;
    delete mainExec;
    delete actions;
    delete prepRunValues;

    return 0;
}

void jsonparser::processExecutionElement(QHash<QString,QVariant> *execElement,QHash<QString,QVariant> const &variables,QHash<QString,QString> const &prefixTable){
    if((execElement->value("exec.command").isValid())&&(execElement->value("exec.type").isValid())){
        QString command = prefixTable.value(execElement->value("exec.type").toString())+" "+execElement->value("exec.command").toString();
        command = resolveVariable(variables,command); //We perform substitution
        execElement->insert("exec",command);
        execElement->remove("exec.command");
        execElement->remove("exec.type");
    }
    if(execElement->value("workdir").isValid())
        execElement->insert("workdir",resolveVariable(variables,execElement->value("workdir").toString()));
    if(execElement->value("desktop.icon").isValid())
        execElement->insert("desktop.icon",resolveVariable(variables,execElement->value("desktop.icon").toString()));
    if(execElement->value("desktop.title").isValid())
        execElement->insert("desktop.title",resolveVariable(variables,execElement->value("desktop.title").toString()));
}

int jsonparser::systemActivate(QHash<QString,QVariant> const &systemElement,QHash<QString,QVariant> *activeOptions,QStringList *activeSystems,QHash<QString,QVariant> *variables,QHash<QString,QVariant> *procEnv,QHash<QString,QVariant> *runtimeValues){
    QString configPrefix;
    if(systemElement.value("variables").isValid())
        if(systemElement.value("variables").type()==QVariant::List)
            variablesImport(systemElement.value("variables").toList(),variables,*activeOptions);
    resolveVariables(variables); //We need to resolve the variables before they are applied to the environment and etc.
    foreach(QString key,systemElement.keys()){
        QVariant object = systemElement.value(key);
        if(key=="config-prefix")
            if(object.type()==QMetaType::QString)
                configPrefix=object.toString();
        if(key=="inherits")
            if(object.type()==QMetaType::QString)
                activeSystems->append(object.toString().split(","));
        if(key=="environment")
            if(object.type()==QVariant::List){
                QList<QVariant> envList = object.toList();
                for(int i=0;i<envList.size();i++){
                    if(envList.at(i).isValid())
                        if(!envList.at(i).toHash().isEmpty())
                            environmentActivate(envList.at(i).toHash(),procEnv,runtimeValues,*variables);
                }
            }
    }
    bool execFound = false;
    foreach(QString key,activeOptions->keys()){
        if(key.endsWith(".exec"))
            if(key.startsWith(configPrefix+"."))
                execFound = true;
    }
    if(execFound!=true){
        sendProgressTextUpdate(tr("Failed to find an execution value that matches the system. Cannot proceed."));
        return 1;
    }
    if(configPrefix.isEmpty()){
        sendProgressTextUpdate(tr("Failed to find a configuration prefix for the system. Cannot proceed."));
        return 1;
    }

    return 0;
}

int jsonparser::systemInherit(QHash<QString,QVariant> const &systemElement,QHash<QString,QVariant> *activeOptions,QStringList *activeSystems,QHash<QString,QVariant> *variables,QHash<QString,QVariant> *procEnv,QHash<QString,QVariant> *runtimeValues){
    QString configPrefix;
    if(systemElement.value("variables").isValid())
        if(systemElement.value("variables").type()==QVariant::List)
            variablesImport(systemElement.value("variables").toList(),variables,*activeOptions);
    resolveVariables(variables); //We need to resolve the variables before they are applied to the environment and etc.
    foreach(QString key,systemElement.keys()){
        QVariant object = systemElement.value(key);
        if(key=="config-prefix")
            if(object.type()==QMetaType::QString)
                configPrefix=object.toString();
        if(key=="inherits")
            if(object.type()==QMetaType::QString)
                activeSystems->append(object.toString().split(",")); //We want recursive inheritance
        if(key=="environment")
            if(object.type()==QVariant::List){
                QList<QVariant> envList = object.toList();
                for(int i=0;i<envList.size();i++){
                    if(envList.at(i).isValid())
                        if(!envList.at(i).toHash().isEmpty())
                            environmentActivate(envList.at(i).toHash(),procEnv,runtimeValues,*variables);
                }
            }
    }
    return 0;
}

void jsonparser::environmentActivate(QHash<QString,QVariant> const &environmentHash,QHash<QString,QVariant> *procEnv,QHash<QString,QVariant> *runtimeValues,QHash<QString,QVariant> const &variables){
    QString type;
    //Get type in order to determine how to treat this environment entry
    type = environmentHash.value("type").toString();
    //Treat the different types according to their parameters and realize their properties
    if(type=="run-prefix"){
        if(environmentHash.value("exec.prefix").isValid()) //CHANGE: WE NO LONGER GIVE A BUNG ABOUT WHAT SYSTEM IT IS; LET THE USER TAKE CARE OF THIS
            runtimeValues->insert("run-prefix",resolveVariable(variables,runtimeValues->value("run-prefix").toString().append(environmentHash.value("exec.prefix").toString()+" ")));
    }else if(type=="run-suffix"){
        if(environmentHash.value("exec.suffix").isValid())
            runtimeValues->insert("run-suffix",resolveVariable(variables,runtimeValues->value("run-suffix").toString().append(" "+environmentHash.value("exec.suffix").toString())));
    }else if(type=="variable"){
        if(environmentHash.value("name").isValid()&&environmentHash.value("value").isValid())
            setEnvVar(procEnv,environmentHash.value("name").toString(),resolveVariable(variables,environmentHash.value("value").toString()));
    }else{
        reportError(1,tr("unsupported environment type").arg(type));
        return;
    }
    return;
}

int jsonparser::subsystemActivate(QHash<QString,QVariant> *subsystemElement, QHash<QString,QVariant> *procEnv, QHash<QString,QVariant> *variables, QHash<QString,QVariant> *runtimeValues, QHash<QString,QVariant> *activeOptions){
    QVariant typeVariant = subsystemElement->value("type");
    QString type;
    QVariant option;
    if(typeVariant.isValid()){
        type=typeVariant.toString();
    }else
        return 1;
    option=activeOptions->value("subsystem."+subsystemElement->value("enabler").toString());

    //We first handle the part that is common to all of the types
    QHash<QString,QVariant> appearanceStuff;
    if(subsystemElement->value("appearance").isValid()){
        //And that is all. It should already be formatted correctly.
        appearanceStuff = subsystemElement->value("appearance").toHash();
    }

    if(type=="bool")
        if(option.toBool()){
            //We won't do more than this. Anything similar would be doable through other means.
            if(!appearanceStuff.value("desktop.title").toString().isEmpty())
                sendProgressTextUpdate(resolveVariable(*variables,appearanceStuff.value("desktop.title").toString()));
            activateVariablesAndEnvironments(*subsystemElement,variables,*activeOptions,procEnv,runtimeValues);

            QHash<QString,QVariant> targetElement = appearanceStuff;
            if(addExecution(*subsystemElement,&targetElement)==0){
                //Not efficient. We need to improve this. Soon.
                QString trigger = subsystemElement->value("trigger").toString();
                if(trigger.isEmpty())
                    trigger = "sys-prerun";
                if((trigger!="sys-prerun")&&(trigger!="sys-postrun")){
                    reportError(1,tr("Invalid trigger was specified for boolean subsystem. Please correct this."));
                    return 1;
                }
                QList<QVariant> *runlist = new QList<QVariant>(runtimeValues->value(trigger).toList());
                runlist->append(targetElement);
                runtimeValues->insert(trigger,*runlist);
                delete runlist;
            }
        }
    if(type=="option"){
        if(!appearanceStuff.value("desktop.title").toString().isEmpty())
            sendProgressTextUpdate(resolveVariable(*variables,appearanceStuff.value("desktop.title").toString()));
        QList<QVariant> optsList = subsystemElement->value("options").toList();
        QHash<QString,QVariant> optsHash; QHash<QString,QVariant> optHash; QString currentOpt;
        for(int i=0;i<optsList.size();i++)
            if(optsList.at(i).toHash().value("id").isValid()){ //We'll use this to decide whether it's a good or bad object.
                optHash.clear(); currentOpt.clear(); optHash = optsList.at(i).toHash();
                currentOpt = optHash.value("id").toString(); optHash.remove("id");
                optsHash.insert(currentOpt,optHash);
            } optHash.clear();
        foreach(QString opt,option.toString().split(",")){
            if(optsHash.value(opt).isValid()){
                optHash = optsHash.value(opt).toHash();
                activateVariablesAndEnvironments(optHash,variables,*activeOptions,procEnv,runtimeValues);
            }else
                reportError(1,tr("Invalid object for option subsystem detected. Please correct this."));
        }
    }
    if(type=="substitution"){
        if(!appearanceStuff.value("desktop.title").toString().isEmpty())
            sendProgressTextUpdate(resolveVariable(*variables,appearanceStuff.value("desktop.title").toString()));
        //This class is essentially going to import a variable from the option. We will simply put the variable in the global variable system.
        //First, we insert the substituted variable.
        if(!subsystemElement->value("variable").isValid()){
            reportError(1,tr("Substitution subsystem does not have a variable. Please correct this."));
            return 1;
        }
        variableHandle(variables,subsystemElement->value("variable").toString(),option.toString()); //We won't bother with keeping it local or anything.

        //We then check for stuff that might include this substitution
        activateVariablesAndEnvironments(*subsystemElement,variables,*activeOptions,procEnv,runtimeValues);

        //Exec
        QHash<QString,QVariant> targetElement = appearanceStuff;
        if(addExecution(*subsystemElement,&targetElement)==0){
            //Not efficient. We need to improve this. Soon.
            QList<QVariant> runlist;
            QString trigger = subsystemElement->value("trigger").toString();
            if(trigger.isEmpty())
                trigger = "sys-prerun";
            if((trigger!="sys-prerun")&&(trigger!="sys-postrun")){
                reportError(1,tr("Invalid trigger was specified for substitution subsystem. Please correct this."));
                return 1;
            }
            runlist = runtimeValues->value(trigger).toList();
            runlist.append(targetElement);
            runtimeValues->insert(trigger,runlist);
        }
    }
    if(type=="select"){
        if(!subsystemElement->value("subtype").isValid())
            return 1;
        if(subsystemElement->value("subtype").toString()=="key-value-set"){
            //Works by adding each of the keysets present in the selected set as a command in the prerun or postrun queue
            //We make a template for inserting into the runtimeValues
            QHash<QString,QVariant> templateRunHash = appearanceStuff;
            QString trigger,commandTemplate,execType;
            foreach(QString key,subsystemElement->keys())
                if(key.endsWith(".exec")){
                    commandTemplate=subsystemElement->value(key).toString();
                    execType=key.split(".")[0];
                }
            if(commandTemplate.isEmpty()){ //If there is no command, we go no further.
                reportError(1,tr("Keyset subsystem has no command. Please correct this."));
                return 1;
            }
            templateRunHash.insert("exec.type",execType);
            trigger=subsystemElement->value("trigger").toString();
            if(trigger.isEmpty()) //Default to prerun
                trigger="sys-prerun";

            //We start by converting the set-array to a QHash for an easier time finding the set we want.
            QList<QVariant> optsList = subsystemElement->value("sets").toList();
            QHash<QString,QVariant> optsHash; QHash<QString,QVariant> optHash; QString currentOpt;
            for(int i=0;i<optsList.size();i++)
                if(optsList.at(i).toHash().value("id").isValid()){ //We'll use this to decide whether it's a good or bad object.
                    optHash.clear(); currentOpt.clear(); optHash = optsList.at(i).toHash();
                    currentOpt = optHash.value("id").toString(); optHash.remove("id");
                    optsHash.insert(currentOpt,optHash);
                } optHash.clear();
            if(optsHash.value(option.toString()).isValid()){
                QList<QVariant> runlist = runtimeValues->value(trigger).toList();
                QString value;
                QHash<QString,QVariant> currentRH;
                QString tmpCmd;
                foreach(QString key,optsHash.value(option.toString()).toHash().keys()){
                    tmpCmd = commandTemplate; //replace(QString) modifies the QString, therefore we need to copy it.
                    currentRH = templateRunHash;
                    value = optsHash.value(option.toString()).toHash().value(key).toString();
                    currentRH.insert("exec.command",tmpCmd.replace("%JASON_KEY%",key).replace("%JASON_VALUE%",value));
                    runlist.append(currentRH);
                }
                runtimeValues->insert(trigger,runlist);
            } else{
                reportError(1,tr("Invalid option for keyset subsystem. Please correct this."));
                return 1;
            }
        }
    }
    return 0;
}

void jsonparser::activateVariablesAndEnvironments(QHash<QString,QVariant> const &inputHash,QHash<QString,QVariant> *variables,QHash<QString,QVariant> const &activeOptions,QHash<QString,QVariant> *procEnv,QHash<QString,QVariant> *runtimeValues){
    if(inputHash.value("variables").isValid())
        if(inputHash.value("variables").type()==QVariant::List)
            variablesImport(inputHash.value("variables").toList(),variables,activeOptions);
    if(inputHash.value("environment").isValid())
        if(inputHash.value("environment").type()==QVariant::List){
            QList<QVariant> envList = inputHash.value("environment").toList();
            for(int i=0;i<envList.size();i++){
                if(envList.at(i).isValid())
                    if(!envList.at(i).toHash().isEmpty())
                        environmentActivate(envList.at(i).toHash(),procEnv,runtimeValues,*variables);
            }
        }
}

int jsonparser::addExecution(QHash<QString,QVariant> const &sourceElement,QHash<QString,QVariant> *targetElement){
    bool validExec = false;
    foreach(QString key,sourceElement.keys()){
        if(key.endsWith(".exec")){
            validExec=true;
            targetElement->insert("exec.command",sourceElement.value(key).toString());
            targetElement->insert("exec.type",key.split(".")[0]);
        }
        if(key=="lazy-exit-status")
            targetElement->insert("lazyexit",sourceElement.value(key).toBool());
        if(key=="workdir")
            targetElement->insert(key,sourceElement.value(key).toString());
        if(key=="detachable-process")
            targetElement->insert("detach",sourceElement.value(key).toBool());
        if(key=="start-detached")
            targetElement->insert("start-detach",sourceElement.value(key).toBool());
        if(key=="private.process-environment")
            targetElement->insert("procenv",sourceElement.value(key).toHash()); //Yeah, we're just throwing this in and hoping it works. I have yet to test this. The pipeline may just as well issue some malicious command instead of doing what it's supposed to do with it.
        if(key=="desktop.title")
            targetElement->insert(key,sourceElement.value(key).toString());
        if(key=="desktop.icon")
            targetElement->insert(key,sourceElement.value(key).toString());
    }
    if(validExec)
        return 0;
    return 1;
}
