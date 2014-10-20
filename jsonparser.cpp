#include "jsonparser.h"

jsonparser::jsonparser(QObject *parent) :
    QObject(parent)
{
}

QJsonDocument jsonparser::jsonOpenFile(QString filename){
    QFile jDocFile;
    QFileInfo cw(filename);

    jDocFile.setFileName(startOpts.value("working-directory")+"/"+cw.fileName());
    if (!jDocFile.exists()) {
        updateProgressText(tr("Failed due to the file not existing"));
        return QJsonDocument();
    }

    if (!jDocFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        updateProgressText(tr("Failed to open the requested file"));
        return QJsonDocument();
    }
    QJsonParseError initError;
    QJsonDocument jDoc = QJsonDocument::fromJson(jDocFile.readAll(),&initError);
    if (initError.error != 0){
        broadcastMessage(2,"ERROR: jDoc: "+initError.errorString()+"\n");
        updateProgressText(tr("An error occured. Please take a look at the error message to see what went wrong."));
        return QJsonDocument();
    }
    if (jDoc.isNull() || jDoc.isEmpty()) {
        updateProgressText(tr("Failed to import file"));
        return QJsonDocument();
    }
    return jDoc;
}

QHash<QString,QVariant> jsonparser::parseStage1(QJsonObject mainObject,QHash<QString,QVariant> *systemTable,QHash<QString,QVariant> *substitutes){
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
    foreach(QString key, mainObject.keys()){
        QJsonValue instanceValue = mainObject.value(key);
        if(key=="systems")
            underlyingObjects.insert("systems",jsonExamineArray(instanceValue.toArray()));
        if(key=="variables")
            underlyingObjects.insert("variables",jsonExamineArray(instanceValue.toArray()));
        if(key=="imports")
            underlyingObjects.insert("imports",jsonExamineArray(instanceValue.toArray()));
        if(key=="subsystems")
            underlyingObjects.insert("subsystems",jsonExamineArray(instanceValue.toArray()));
    }
    if(parseUnderlyingObjects(underlyingObjects)!=0)
        return 1;
    procEnv.insert(QProcessEnvironment::systemEnvironment());
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

int jsonparser::parseStage2(QJsonObject mainObject,QHash<QString,QVariant> systemTable,QHash<QString,QVariant> *activeOptions){
    /*
     * Stage 2:
     * Where options specified by the user or distributor are applied.
     * Parses through the options of the initial file only, will see if it is a good idea to parse
     * through the imported files as well.
    */
    foreach(QString key,mainObject.keys()){
        QJsonValue instanceValue = mainObject.value(key);
        if(key.startsWith("subsystem."))
            activeOptions->insert(key,instanceValue.toVariant());
        if(key=="desktop.file")
            desktopFileBuild(instanceValue.toObject());
        foreach(QString systemKey,systemTable.keys()){
            QString configPrefix = systemTable.value(systemKey).toHash().value("config-prefix").toString();
            if((key.startsWith(configPrefix+"."))&&(!instanceValue.isUndefined())){
                stage2ActiveOptionAdd(&activeOptions,instanceValue,key);
            }
        }
        if(key.startsWith("global.")){
            stage2ActiveOptionAdd(&activeOptions,instanceValue,key);
        }
        if(key=="launchtype")
            activeOptions->insert(key,instanceValue.toString());
        if(key=="shell.properties")
            activeOptions->insert(key,jsonExamineObject(instanceValue.toObject()));
    }
    return 0;
}

int jsonparser::jsonParse(QJsonDocument jDoc){ //level is used to identify the parent instance of jsonOpenFile
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
    QHash<QString,QVariant> activeOptions;

    QJsonObject mainTree = jDoc.object();
    if((mainTree.isEmpty())||(jDoc.isEmpty())){
//        updateProgressText(tr("No objects found. Will not proceed."));
        return 1;
    }
    updateProgressText(tr("Gathering fundamental values"));
    if(parseStage1(mainTree)!=0){
        updateProgressText(tr("Failed to parse fundamental values. Will not proceed."));
        return 1;
    }

    updateProgressText(tr("Gathering active options"));
    parseStage2(mainTree);
    foreach(QString import,importedFiles){
        parseStage2(jsonOpenFile(import).object());
    }

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
    updateProgressText(tr("Activating main system"));
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
            if(systemActivate(currentSystem,activeSystems)!=0){
                updateProgressText(tr("Failed to activate system. Will not proceed."));
                return 1;
            }
        }
    }
    updateProgressText(tr("Resolving variables"));
    resolveVariables();
    updateProgressText(tr("Activating subsystems"));
    foreach(QString option,activeOptions.keys())
        if(option.startsWith("subsystem.")){
            QString subsystemName = option;
            subsystemName.remove("subsystem.");
            foreach(int sub,subsystems.keys())
                if(subsystems.value(sub).value("enabler")==subsystemName)
                    subsystemActivate(subsystems.value(sub),activeOptions.value(option).toString(),activeSystems);
        }

    updateProgressText(tr("Resolving variables"));
    resolveVariables();

    updateProgressText(tr("Processing preruns and postruns"));
    foreach(QString key, activeOptions.keys()){
        QVariant instanceValue = activeOptions.value(key);
        foreach(QString system,activeSystems)
            if(key.startsWith(systemTable.value(system).toHash().value("config-prefix").toString())){ //I puked all over my keyboard while writing this.
                if(key.contains(".prerun")){
                    insertPrerunPostrun(jsonExamineArray(instanceValue.toJsonArray()),0);
                }
                if(key.contains(".postrun")){
                    insertPrerunPostrun(jsonExamineArray(instanceValue.toJsonArray()),1);
                }
            }
    }
    return 0;
}

QHash<QString,QVariant> jsonparser::jsonExamineArray(QJsonArray jArray){
    QList<QVariant> returnTable;
    //Iterate over the input array
    for(int i = 0;i<jArray.count();i++){
        QJsonValue instance = jArray.at(i);
        if (instance.isArray()){
            //Method used when an array is found
            returnTable.append(jsonExamineArray(instance.toArray()));
        } if (instance.isObject()) {
            //Method used when an object is found
            QHash<QString,QVariant> objectTable = jsonExamineObject(instance.toObject());
            int i2 = returnTable.count();
            returnTable.append(objectTable);
        } else {
            // When all else fails, return the possible other type of value from the QJsonValue
            returnTable.insert(instance.toString(),instance.toVariant());
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

QHash<QString,QVariant> jsonparser::subsystemHandle(QHash<QString,QVariant> subsystemElement){
    QHash<QString, QVariant> insertHash;
    //We just dump everything into the hash, resolving arrays and objects
    foreach(QString key, subsystemElement.keys()){
        if(subsystemElement.value(key).isObject()){
            insertHash.insert(key,jsonExamineObject(subsystemElement.value(key).toObject()));
        } else if(subsystemElement.value(key).isArray()){
            insertHash.insert(key,jsonExamineArray(subsystemElement.value(key).toArray()));
        } else
            insertHash.insert(key,subsystemElement.value(key));
    }
    return insertHash;
}

void jsonparser::variableHandle(QHash<QString,QVariant> *variables, QString key, QString value){
    //Insert variable in form "NAME","VAR"
    variables.insert(key,value);
}


void jsonparser::variablesImport(QHash<QString,QVariant> *variables){
    foreach(QString var,variables.keys()){
        QHash<QString,QVariant> varHash = variables.value(var).toHash();
        QString varType;
        foreach(QString option,varHash.keys())
            if(option=="type")
                varType = varHash.value(option).toString();
        if(varType=="config-input"){
            QString defaultValue = varHash.value("default").toString(); //insert default value if no option is found in activeOptions
            foreach(QString option,varHash.keys())
                if((option=="input")&&(varHash.contains("name"))){
                    QString variable = activeOptions.value(resolveVariable("%CONFIG_PREFIX%."+varHash.value(option).toString())).toString();
                    if(variable.isEmpty())
                        variable = defaultValue;
                    substitutes.insert(varHash.value("name").toString(),variable);
                }
        }else if(varHash.value("name").isValid()&&varHash.value("value").isValid()){
            setEnvVar(varHash.value("name").toString(),resolveVariable(varHash.value("value").toString()));
        }else
            broadcastMessage(1,"unsupported variable type"+varType);
    }
}
