#include "jsonparser.h"

jsonparser::jsonparser(QObject *parent) :
    QObject(parent)
{
}

QJsonDocument jsonparser::jsonOpenFile(QString filename){
    QFile jDocFile;
    QFileInfo cw(filename);
    jDocFile.setFileName(cw.absolutePath()+"/"+cw.fileName());
    if (!jDocFile.exists()) {
        sendProgressTextUpdate(tr("Failed due to the file not existing"));
        return QJsonDocument();
    }
    if (!jDocFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        sendProgressTextUpdate(tr("Failed to open the requested file"));
        return QJsonDocument();
    }
    QJsonParseError initError;
    QJsonDocument jDoc = QJsonDocument::fromJson(jDocFile.readAll(),&initError);
    if (initError.error != 0){
        reportError(2,"ERROR: jDoc: "+initError.errorString()+"\n");
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
    /*Stage 1: Parse information about systems, variables, subsystems
     *  - Systems are used to start programs in a specific way, example system launch
     *  versus WINE. They serve a .exec value that is used to launch their programs,
     * which simply prepends a value to the QProcess command line.
     *  - Variables in the context of Jason should work like shell variables where
     * they are used for text substitution. They will pull in some essential system
     * variables such as LD_LIBRARY_PATH, HOME, PATH, XDG_DATA_DIRS and several others
     * that are useful in launching a program.
     *  - Subsystems' behavior are decided by their type, and may rely on an enabler
     * which modifies its behavior. The type may decide what other elements are used and
     * may modify the environment, launch a program and more.
     *  - Imports cause another .json file to be pulled in and parsed along with the
     * original document. This may be used to share variables and systems between launchers.
     *  - .prerun is used in conjunction with systems to run commands before the actual
     * program is run.
     *  - .postrun is used to run commands after the program has run.
     *  - information is only gathered in this phase unless it is a subsystem of type constant, is a
     * global variable
     */
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
        QJsonArray impArray = mainObject.value("imports").toArray();
        for(int i=0;i<impArray.size();i++){
            QString impFile = impArray.at(i).toObject().value("file").toString();
            importedFiles->append(impFile);
            QHash<QString,QVariant> resultHash;
            jsonParse(jsonOpenFile(impFile),&resultHash);
            if(!resultHash.isEmpty()){
                if(resultHash.value("systems").isValid())
                    systemTable->unite(resultHash.value("systems").toHash());
                if(resultHash.value("subsystems").isValid())
                    subsystems->append(resultHash.value("subsystems").toList());
                if(resultHash.value("activeopts").isValid())
                    activeOptions->unite(resultHash.value("activeopts").toHash());
                if(resultHash.value("variables").isValid())
                    substitutes->unite(resultHash.value("variables").toHash());
                if(resultHash.value("procenv").isValid())
                    procEnv->unite(resultHash.value("procenv").toHash());
            }
        }
    }
    if(!mainObject.value("variables").isUndefined()){
        QList<QVariant> varRay = mainObject.value("variables").toArray().toVariantList();
        variablesImport(varRay,substitutes,*activeOptions);
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
        activeOptions->insert(insertKey,jsonExamineArray(instance.toArray()));
    } else
        activeOptions->insert(insertKey,instance.toVariant());
    return 0;
}

void jsonparser::parseStage2(QJsonObject mainObject,QHash<QString,QVariant> const &systemTable,QHash<QString,QVariant> *activeOptions){
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
/*
     *  - The JSON file is parsed in two stages; the JSON file is parsed randomly and as such
     * you cannot expect data to appear at the right time. For this, parsing in two stages
     * allows substituted values, systems, subsystems and etc. to listed in the first run
     * and applied in the second run. The int level is used for recursive parsing where
     * you do not want the process to proceed before everything is done.
     *  - The execution and/or creation of a desktop file is done post-parsing when all variables
     * are known and resolved.
     *
*/
    QHash<QString,QVariant> systemTable;
    QHash<QString,QVariant> substitutes;
    QList<QVariant> subsystems;
    QHash<QString,QVariant> activeOptions;
    QHash<QString,QVariant> procEnv;
    QStringList importedFiles;

    QJsonObject mainTree = jDoc.object();
    if((mainTree.isEmpty())||(jDoc.isEmpty())){
        sendProgressTextUpdate(tr("No objects found. Will not proceed."));
        return 1;
    }

    sendProgressTextUpdate(tr("Gathering active options"));
    parseStage2(mainTree,systemTable,&activeOptions);
    foreach(QString import,importedFiles){
        parseStage2(jsonOpenFile(import).object(),systemTable,&activeOptions);
    } //Yes, very odd, but we want to populate activeOptions before running stage 1.

    sendProgressTextUpdate(tr("Gathering fundamental values"));
    if(parseStage1(mainTree,&systemTable,&substitutes,&subsystems,&importedFiles,&activeOptions,&procEnv)!=0){
        sendProgressTextUpdate(tr("Failed to parse fundamental values. Will not proceed."));
        return 1;
    }

    targetHash->insert("systems",systemTable);
    targetHash->insert("variables",substitutes);
    targetHash->insert("activeopts",activeOptions);
    targetHash->insert("subsystems",subsystems);
    targetHash->insert("procenv",procEnv);
    return 0;
/*
 * Stage 3:
 * The active options have been listed and need to be parsed. With this it needs to look for
 * active subsystems and apply their options, prepare a list of programs to execute with QProcess
 * and prepare the system for launch.
 * Systems have default switches such as .exec and .workdir, but can also define variables according to a
 * switch, ex. wine.version referring to the internal variable %WINEVERSION%.
 * Subsystems will be triggered according to their type and what option is entered.
 *
*/
    sendProgressTextUpdate(tr("Activating main system"));
    QHash<QString,QVariant> runtimeValues;
    runtimeValues.insert("run-prefix",QList<QVariant>());
    runtimeValues.insert("run-suffix",QList<QVariant>());
    runtimeValues.insert("sys-prerun",QList<QVariant>());
    runtimeValues.insert("sys-postrun",QList<QVariant>());
    runtimeValues.insert("launchables",QHash<QString,QVariant>());

    QStringList activeSystems;
    foreach(QString option,activeOptions.keys()){
        if(option=="launchtype"){
            if(!systemTable.value(activeOptions.value("launchtype").toString()).isValid())
                return 1;
            QHash<QString,QVariant> currentSystem = systemTable.value(activeOptions.value("launchtype").toString()).toHash();
            foreach(QString key,currentSystem.keys()){
                if(key=="identifier")
                    activeSystems.append(currentSystem.value(key).toString());
                if(key=="inherit")
                    activeSystems.append(currentSystem.value(key).toString().split(","));
            }
//            if(systemActivate(currentSystem,activeSystems)!=0){
//                sendProgressTextUpdate(tr("Failed to activate system. Will not proceed."));
//                return 1;
//            }
        }
    }
    sendProgressTextUpdate(tr("Resolving variables"));
    resolveVariables(&substitutes);
    sendProgressTextUpdate(tr("Activating subsystems"));
    foreach(QString option,activeOptions.keys())
        if(option.startsWith("subsystem.")){
            QString subsystemName = option;
            subsystemName.remove("subsystem.");
            for(int i=0;i<subsystems.size();i++) //!!!
                qDebug() << subsystems.at(i).toHash();
//                if(subsystems.at(i).toHash().value("enabler")==subsystemName)
//                    subsystemActivate(subsystems.value(sub),activeOptions.value(option).toString(),activeSystems);
        }

    sendProgressTextUpdate(tr("Resolving variables"));
    resolveVariables(&substitutes);

    sendProgressTextUpdate(tr("Processing preruns and postruns"));
//    foreach(QString key, activeOptions.keys()){
//        QVariant instanceValue = activeOptions.value(key);
//        foreach(QString system,activeSystems)
//            if(key.startsWith(systemTable.value(system).toHash().value("config-prefix").toString())){ //I puked all over my keyboard while writing this.
//                if(key.contains(".prerun")){
//                    insertPrerunPostrun(jsonExamineArray(instanceValue.toJsonArray()),0);
//                }
//                if(key.contains(".postrun")){
//                    insertPrerunPostrun(jsonExamineArray(instanceValue.toJsonArray()),1);
//                }
//            }
//    }
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
            returnTable.insert(key,jObject.value(key));
    }
    return returnTable;
}

int jsonparser::subsystemHandle(QHash<QString,QVariant> *subsystemElement){
    QHash<QString, QVariant> insertHash;
    //We just dump everything into the hash, resolving arrays and objects
    foreach(QString key, subsystemElement->keys()){
        if(subsystemElement->value(key).type()==QMetaType::QJsonObject){
            insertHash.insert(key,jsonExamineObject(subsystemElement->value(key).toJsonObject()));
        } else if(subsystemElement->value(key).type()==QMetaType::QJsonArray){
            insertHash.insert(key,jsonExamineArray(subsystemElement->value(key).toJsonArray()));
        } else
            insertHash.insert(key,subsystemElement->value(key));
    }
    *subsystemElement = insertHash;
    return 0;
}

int jsonparser::systemHandle(QHash<QString, QVariant> *systemElement){
    QHash<QString,QVariant> systemHash;
    //We verify that it contains the keys we need
    if(!systemElement->value("config-prefix").isValid())
        return 1;
    if(!systemElement->value("identifier").isValid())
        return 1;

    foreach(QString key,systemElement->keys()){
        if(systemElement->value(key).type()==QMetaType::QJsonArray){
            systemHash.insert(key,jsonExamineArray(systemElement->value(key).toJsonArray()));
        } else if(systemElement->value(key).type()==QMetaType::QJsonObject){
            systemHash.insert(key,jsonExamineObject(systemElement->value(key).toJsonObject()));
        } else
            systemHash.insert(key,systemElement->value(key));
    }
    *systemElement = systemHash;
    return 0;
}

void jsonparser::variableHandle(QHash<QString, QVariant> *variables, QString key, QString value){
    //Insert variable in form "NAME","VAR"
    variables->insert(key,value);
}

void jsonparser::variablesImport(QList<QVariant> inputVariables, QHash<QString,QVariant> *substitutes, QHash<QString,QVariant> const &activeOptions){
    for(int i=0;i<inputVariables.size();i++){
        QHash<QString,QVariant> varHash = inputVariables.at(i).toHash();
        QString varType;
        varType = varHash.value("type","undefined").toString();
        if(varType=="config-input"){
            //If no option is found, insert the default value
            QString defaultValue = varHash.value("default","undefined").toString();
            QString option = varHash.value("input").toString();
            if((!option.isEmpty())&&(varHash.value("name").isValid())){
                QString variable = activeOptions.value(varHash.value(option).toString(),defaultValue).toString();
                substitutes->insert(varHash.value("name").toString(),variable);
            }
        }else if(varHash.value("name").isValid()&&varHash.value("value").isValid()){
            variableHandle(substitutes,varHash.value("name").toString(),resolveVariable(substitutes,varHash.value("value").toString()));
        }else
            reportError(1,"unsupported variable type:"+varType);
    }
}

void jsonparser::resolveVariables(QHash<QString, QVariant> *substitutes){
    int indicator = 0; //Indicator for whether the operation is done or not. May cause an infinite loop when a variable cannot be resolved.
    while(indicator!=1){
        foreach(QString key, substitutes->keys()){
            QString insert = substitutes->value(key).toString();
            substitutes->remove(key);
            substitutes->insert(key,resolveVariable(substitutes,insert));
        }
        int indicatorLocal = 0;
        foreach(QString key,substitutes->keys())
            if(!substitutes->value(key).toString().contains("%")){
                indicatorLocal++;
            }else
                sendProgressTextUpdate(tr("Variable %1 is being slightly problematic.").arg(key));
        if(indicatorLocal==substitutes->count())
            indicator = 1;
    }
    return;
}

QString jsonparser::resolveVariable(QHash<QString, QVariant> *substitutes, QString variable){
    foreach(QString sub, substitutes->keys()){
        QString replace = sub;
        replace.prepend("%");replace.append("%");
        variable = variable.replace(replace,substitutes->value(sub).toString());
    }
    return variable;
}

void jsonparser::setEnvVar(QHash<QString, QVariant> *procEnv, QString key, QString value){
    //Insert the variable into the environment
    procEnv->insert(key,value);
    return;
}
