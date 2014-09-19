#include "jasonparser.h"

JasonParser::JasonParser(){

}

JasonParser::~JasonParser(){

}

void JasonParser::testEnvironment(){
//    qDebug() << substitutes;
//    qDebug() << subsystems;
//    qDebug() << systemTable;
//    qDebug() << activeOptions;
//    qDebug() << procEnv.toStringList();
//    qDebug() << runtimeValues.value("launchables");
    foreach(QString var,procEnv.toStringList()){
        if(var.contains("%"))
            qDebug() << "unresolved variable?" << var.split("=")[1];
        if(var.contains("//"))
            qDebug() << "unresolved variable?" << var.split("=")[1];
    }
    foreach(QString var,substitutes.keys()){
        if(substitutes.value(var).contains("%"))
            qDebug() << "unresolved variable?" << var << substitutes.value(var);
    }
}

void JasonParser::startParse(){
    updateProgressText(tr("Starting to parse JSON document"));
    QString startDocument,actionId,desktopFile;
    startDocument=startOpts.value("start-document");
    actionId=startOpts.value("action-id");
    desktopFile=startOpts.value("desktop-file");
    if(jsonParse(jsonOpenFile(startDocument),1)!=0){
        broadcastMessage(2,tr("Apples is stuck in a tree! We need to call the fire department!\n"));
        return;
    }

    if(desktopFile.isEmpty()){
        if(runProcesses(actionId)!=0)
            broadcastMessage(2,tr("Shit.\n"));
    }else{
        qDebug() << "would generate a .desktop file here.";
    }
}

void JasonParser::setStartOpts(QString startDocument, QString actionId, QString desktopFile){
    if(!startDocument.isEmpty())
        startOpts.insert("start-document",startDocument);
    if(!actionId.isEmpty())
        startOpts.insert("action-id",actionId);
    if(!desktopFile.isEmpty())
        startOpts.insert("desktop-file",desktopFile);
    return;
}

QJsonDocument JasonParser::jsonOpenFile(QString filename){
    QFile jDocFile;
    jDocFile.setFileName(filename);
    if (!jDocFile.exists()) {
        broadcastMessage(0,"ERROR: jDocFile::File not found\n");
        return QJsonDocument();
    }

    if (!jDocFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        broadcastMessage(2,"ERROR: jDocFile::Failed to open\n");
        return QJsonDocument();
    }
    QJsonParseError initError;
    QJsonDocument jDoc = QJsonDocument::fromJson(jDocFile.readAll(),&initError);
    if (initError.error != 0)
        broadcastMessage(2,"ERROR: jDoc: "+initError.errorString()+"\n");
    if (jDoc.isNull() || jDoc.isEmpty()) {
        broadcastMessage(2,"ERROR: jDoc::IsNull or IsEmpty\n");
        return QJsonDocument();
    }
    return jDoc;
}

int JasonParser::jsonParse(QJsonDocument jDoc,int level){ //level is used to identify the parent instance of jsonOpenFile
/*
     *  - The JSON file is parsed in two stages; the JSON file is parsed randomly and as such
     * you cannot expect data to appear at the right time. For this, parsing in two stages
     * allows substituted values, systems, subsystems and etc. to listed in the first run
     * and applied in the second run. The int level is used for recursive parsing where
     * you do not want the process to proceed before everything is done.
     *  - The execution and/or creation of a desktop file is done post-parsing when all variables
     * are known and resolved.
     *
     *
     * Parameter int:
     *   - 1 means it runs the main thread.
     *   - 2 is looking for stage 1 objects
     *   - 3 is looking for active options
*/
    QJsonObject mainTree = jDoc.object();
    QHash<QString,QHash<QString,QVariant> > underlyingObjects;
    if(level!=3){
        foreach(QString key, mainTree.keys()){
            //        qDebug() << "jsonParse stage 1:" << key;
            QJsonValue instanceValue = mainTree.value(key);
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
            if(key=="systems")
                underlyingObjects.insert("systems",jsonExamineArray(instanceValue.toArray()));
            if(key=="variables")
                underlyingObjects.insert("variables",jsonExamineArray(instanceValue.toArray()));
            if(key=="imports")
                underlyingObjects.insert("imports",jsonExamineArray(instanceValue.toArray()));
            if(key=="subsystems")
                underlyingObjects.insert("subsystems",jsonExamineArray(instanceValue.toArray()));
        }
        parseUnderlyingObjects(underlyingObjects);
        procEnv.insert(QProcessEnvironment::systemEnvironment());
    }
    if(level==2)
        return 0;
/*
 * Stage 2:
 * Where options specified by the user or distributor are applied.
 * Parses through the options of the initial file only, will see if it is a good idea to parse
 * through the imported files as well.
*/
    foreach(QString key,mainTree.keys()){
//        qDebug() << "jsonParse stage 2:" << key;
        QJsonValue instanceValue = mainTree.value(key);
        if(key.startsWith("subsystem."))
            activeOptions.insert(key,jsonExamineValue(instanceValue));
        if(key=="desktop.file")
            desktopFileBuild(instanceValue.toObject());
        foreach(QString systemKey,systemTable.keys()){
            QString configPrefix = systemTable.value(systemKey).toHash().value("config-prefix").toString();
            if((key.startsWith(configPrefix))&&(!instanceValue.toString().isEmpty()))
                activeOptions.insert(key,instanceValue.toString());
        }
        if(key.startsWith("global.")){
            if(instanceValue.isString())
                activeOptions.insert(key,instanceValue.toString());
            if(instanceValue.isBool())
                activeOptions.insert(key,instanceValue.toBool());
            if(instanceValue.isDouble())
                activeOptions.insert(key,instanceValue.toDouble());
        }
        if(key=="launchtype")
            activeOptions.insert(key,instanceValue.toString());
        if(key=="shell.properties")
            activeOptions.insert(key,instanceValue.toObject());
    }
    if(level==3)
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
    if(runtimeValues.isEmpty()){ //If the table is empty, initialize it with empty QStrings and QStringLists
        runtimeValues.insert("run-prefix",QList<QVariant>());
        runtimeValues.insert("run-suffix",QList<QVariant>());
        runtimeValues.insert("sys-prerun",QList<QVariant>());
        runtimeValues.insert("sys-postrun",QList<QVariant>());
        runtimeValues.insert("launchables",QHash<QString,QVariant>());
    }
    QStringList activeSystems;
    QString currSystemConfPrefix;
    foreach(QString import,importedFiles)
        jsonParse(jsonOpenFile(import),3);
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
                if(key=="config-prefix")
                    currSystemConfPrefix=currentSystem.value(key).toString();
            }
            if(systemActivate(currentSystem,activeSystems)!=0)
                return 1;
        }
    }
    resolveVariables();
    foreach(QString option,activeOptions.keys())
        if(option.startsWith("subsystem.")){
            QString subsystemName = option;
            subsystemName.remove("subsystem.");
            foreach(int sub,subsystems.keys())
                if(subsystems.value(sub).value("enabler")==subsystemName)
                    subsystemActivate(subsystems.value(sub),activeOptions.value(option).toString(),activeSystems);
        }

    resolveVariables();

    foreach(QString key, mainTree.keys()){
        QJsonValue instanceValue = mainTree.value(key);
        if(key.startsWith(currSystemConfPrefix)){            if(key.endsWith(".prerun"))
                insertPrerunPostrun(jsonExamineArray(instanceValue.toArray()),0);
            if(key.endsWith(".postrun"))
                insertPrerunPostrun(jsonExamineArray(instanceValue.toArray()),1);
        }
    }
    return 0;
}

QHash<QString,QVariant> JasonParser::jsonExamineArray(QJsonArray jArray){
    if (jArray.isEmpty())
        return QHash<QString,QVariant>();
    QHash<QString,QVariant> returnTable;
    //Iterate over the input array
    for(int i = 0;i<jArray.count();i++){
        QJsonValue instance = jArray.at(i);
        if (instance.isArray()){
            //Method used when an array is found
            returnTable.insert(instance.toString(),jsonExamineArray(instance.toArray()));
        } if (instance.isObject()) {
            //Method used when an object is found
            QHash<QString,QVariant> objectTable = jsonExamineObject(instance.toObject());
            int i2 = returnTable.count();
            returnTable.insert(QString::number(i2),objectTable);
        } else {
            // When all else fails, return the possible other type of value from the QJsonValue
            returnTable.insert(instance.toString(),jsonExamineValue(instance));
        }
    }
    return returnTable;
}


QVariant JasonParser::jsonExamineValue(QJsonValue jValue){
    if (jValue.isNull())
        return QVariant();
    QVariant returnValue;
    //Returns only what the QJsonValue contains, no branching off, parent will take care of the rest.
    if(jValue.isString()){
        returnValue = jValue.toString();
    }if(jValue.isDouble()){
        returnValue = jValue.toDouble();
    }if(jValue.isBool()){
        returnValue = jValue.toBool();
    }if(jValue.isObject()){
        returnValue = jValue.toObject();
    }
    return returnValue;
}


QHash<QString,QVariant> JasonParser::jsonExamineObject(QJsonObject jObject){
    if (jObject.isEmpty())
        return QHash<QString,QVariant>();
    QHash<QString,QVariant> returnTable; //Create a list for the returned objects
    //Objects may contain all different kinds of values, take care of this
    foreach(QString key, jObject.keys()){
        if(jObject.value(key).isBool())
            returnTable.insert(key,jObject.value(key).toBool());
        if(jObject.value(key).isString())
            returnTable.insert(key,jObject.value(key).toString());
        if(jObject.value(key).isDouble())
            returnTable.insert(key,jObject.value(key).toDouble());
        if(jObject.value(key).isArray())
            returnTable.insert(key,jObject.value(key).toArray());
        if(jObject.value(key).isObject())
            returnTable.insert(key,jObject.value(key).toObject());
    }

    return returnTable;
}


void JasonParser::setEnvVar(QString key, QString value) {
    //Insert the variable into the environment
    procEnv.insert(key,value);
    return;
}


void JasonParser::subsystemHandle(QHash<QString,QVariant> subsystemElement){
    QString subType;
    QString subEnabler;
    QVector<QVariant> subEnv; //Environment container for environment held by subsystem
    QHash<QString,QVariant> selections; //Selections for the select type
    QHash<QString,QVariant> options; //Options for the option type
    QStringList subAction; //The executable component
    QString subVar; //The variable that this subsystem replaces
    QString trigger; //When this subsystem will run if it has an executable component
    QJsonObject appearance;
    //Identify and catch possible values for a subsystem
    foreach(QString key, subsystemElement.keys()){
        if(key=="type")
            subType = subsystemElement.value(key).toString();
        if(key=="enabler")
            subEnabler = subsystemElement.value(key).toString();
        if(key=="selections"){
            QJsonArray selectArray = subsystemElement.value(key).toJsonArray();
            for(int i = 0;i<selectArray.count();i++){
                QJsonObject selectObject = selectArray.at(i).toObject();
                QString listname = selectObject.value("listname").toString();
                selectObject.remove(listname);
                selections.insert(listname,selectObject);
            }
        }
        if(key=="environment")
            for(int i=0;i<subsystemElement.value(key).toJsonArray().count();i++){
                QHash<QString,QVariant> envTable = jsonExamineObject(subsystemElement.value(key).toJsonArray().at(i).toObject());
                subEnv.insert(subEnv.count(),envTable);
            }
        if(key=="options"){
            QJsonArray optionArray = subsystemElement.value(key).toJsonArray();
            for(int i = 0;i<optionArray.count();i++){
                QHash<QString,QVariant> optionObject = jsonExamineObject(optionArray.at(i).toObject());
                QString optName = optionObject.value("id").toString();
                optionObject.remove(optName);
                options.insert(optName,optionObject);
            }
        }

        if(key=="trigger")
            trigger=subsystemElement.value(key).toString();
        if(key=="appearance")
            appearance=subsystemElement.value(key).toJsonObject();

        if(key.endsWith(".exec")){ //Matches the config prefix
            subAction.insert(0,subsystemElement.value(key).toString());
            subAction.insert(1,key);
        }
        if(key=="variable")
            subVar = subsystemElement.value(key).toString();
    }
    if(subType!="constant"){ //constants are always set, don't add it to the subsystems list
        QHash<QString, QVariant> insertHash;
        insertHash.insert("type",subType);
        if(!subEnabler.isEmpty())
            insertHash.insert("enabler",subEnabler);
        if(!subAction.isEmpty())
            insertHash.insert(subAction[1],subAction[0]);
        if(!subVar.isEmpty())
            insertHash.insert("variable",subVar);
        if(!trigger.isEmpty())
            insertHash.insert("trigger",trigger);
        if(!subEnv.isEmpty())
            insertHash.insert("environment",subEnv.toList());
        if(!options.isEmpty())
            insertHash.insert("options",options);
        if(!appearance.isEmpty())
            insertHash.insert("appearance",appearance);
        if(!selections.isEmpty())
            insertHash.insert("selections",selections);
        int i = subsystems.count();
        subsystems.insert(i,insertHash);
    } else
        for(int i=0;i<subEnv.count();i++)
            if(!subEnv.at(i).isNull()){
                QString key = subEnv.at(i).toHash().value("name").toString();
                setEnvVar(key,subEnv.at(i).toHash().value("value").toString());
            }
    return;
}


void JasonParser::variableHandle(QString key, QString value){
    //Insert variable in form "NAME","VAR"
    substitutes.insert(key,value);
}


void JasonParser::variablesImport(QHash<QString,QVariant> variables){
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
            qDebug() << "unsupported variable type" << varType;
    }
}


void JasonParser::parseUnderlyingObjects(QHash<QString, QHash<QString, QVariant> > underlyingObjects){
    QHash<QString, QVariant> importTable;
    QHash<QString, QVariant> variablesTable;
    QHash<QString, QVariant> subsystemTable;
    QHash<QString, QVariant> prerunTable;
    QHash<QString, QVariant> postrunTable;
    QHash<QString, QVariant> systemTTable;
    //Grab tables from inside the input table
    foreach(QString key, underlyingObjects.keys()){
        if(key=="imports")
            importTable = underlyingObjects.value(key);
        if(key=="variables")
            variablesTable = underlyingObjects.value(key);
        if(key=="subsystems")
            subsystemTable = underlyingObjects.value(key);
        if(key.endsWith(".prerun"))
            prerunTable = underlyingObjects.value(key);
        if(key.endsWith(".postrun"))
            postrunTable = underlyingObjects.value(key);
        if(key=="systems")
            systemTTable = underlyingObjects.value(key);
    }
    //Handle the tables in their different ways, nothing will be returned
    foreach(QString key,importTable.keys())
        foreach(QString kKey, importTable.value(key).toHash().keys()){
            jsonParse(jsonOpenFile(importTable.value(key).toHash().value(kKey).toString()),2);
            importedFiles.append(importTable.value(key).toHash().value(kKey).toString());
        }

    foreach(QString key,variablesTable.keys()){
        foreach(QString kKey, variablesTable.value(key).toHash().keys())
            if(kKey=="name")
                variableHandle(variablesTable.value(key).toHash().value("name").toString(),variablesTable.value(key).toHash().value("value").toString());
    }
    foreach(QString key,subsystemTable.keys())
        subsystemHandle(subsystemTable.value(key).toHash());
    foreach(QString key,systemTTable.keys())
        systemHandle(systemTTable.value(key).toHash());

    insertPrerunPostrun(prerunTable,0);
    insertPrerunPostrun(postrunTable,1);
}


void JasonParser::resolveVariables(){
    QStringList systemVariables;
    //System variables that are used in substitution for internal variables. Messy indeed, but it kind of works in a simple way.
    systemVariables<<"HOME"<<"PATH"<<"XDG_DATA_DIRS";
    foreach(QString variable, systemVariables){
        QProcessEnvironment variableValue = QProcessEnvironment::systemEnvironment();
        substitutes.insert(variable,variableValue.value(variable));
    }


    int indicator = 0; //Indicator for whether the operation is done or not. May cause an infinite loop when a variable cannot be resolved.
    while(indicator!=1){
        foreach(QString key, substitutes.keys()){
            QString insert = substitutes.value(key);
            substitutes.remove(key);
            substitutes.insert(key,resolveVariable(insert));
        }
        int indicatorLocal = 0;
        foreach(QString key,substitutes.keys())
            if(!substitutes.value(key).contains("%")){
                indicatorLocal++;
            }/*else
                qDebug() << "problem with" << key;*/
        if(indicatorLocal==(substitutes.count()))
            indicator = 1;
    }
    return;
}


QString JasonParser::resolveVariable(QString variable){
    //Takes the variable's contents as input and replaces %%-enclosed pieces of text with their variables. Will always return a variable, whether it has all variables resolved or not.
    if(variable.isEmpty())
        return QString();
    foreach(QString sub, substitutes.keys()){
        QString replace = sub;
        replace.prepend("%");replace.append("%");
        variable = variable.replace(replace,substitutes.value(sub));
    }
    if(!variable.contains("%"))
        return variable;
    return variable;
}


void JasonParser::desktopFileBuild(QJsonObject desktopObject){
/*
 * Desktop files execute by ./Jason [JSON-file]
 * A desktop action entry will use the appendage of --action [id] to make a distinction between the
 * different entries that may exist.
*/
    QString dName,dDesc,dWMClass,dIcon;
    QHash<QString,QVariant> containerHash = runtimeValues.value("launchables").toHash();
    foreach(QString key, desktopObject.keys()){
        if(key=="desktop.displayname")
            dName = desktopObject.value(key).toString();
        if(key=="desktop.description")
            dDesc = desktopObject.value(key).toString();
        if(key=="desktop.wmclass")
            dWMClass = desktopObject.value(key).toString();
        if(key=="desktop.icon")
            dIcon = desktopObject.value(key).toString();
        if(key.startsWith("desktop.action.")){
            QJsonObject action = desktopObject.value(key).toObject();
            QString aName,aExec,aID,aWD,aCPrefix,aLPrefix;
            foreach(QString actKey, action.keys()){
                if(actKey.endsWith(".exec")){
                    if(!actKey.split(".")[0].isEmpty())
                        aCPrefix = actKey.split(".")[0];
                    aExec = action.value(actKey).toString();
                }
                if(actKey.endsWith(".workdir"))
                    aWD = action.value(actKey).toString();
                if(actKey=="action-id")
                    aID = action.value(actKey).toString();
                if(actKey=="desktop.displayname")
                    aName = action.value(actKey).toString();
            }
            if(!aCPrefix.isEmpty()){
                foreach(QString system,systemTable.keys())
                    if(systemTable.value(system).toHash().value("config-prefix").toString()==aCPrefix)
                        if(!systemTable.value(system).toHash().value("launch-prefix").toString().isEmpty())
                            aLPrefix = systemTable.value(system).toHash().value("launch-prefix").toString();
            }
            QHash<QString,QVariant> actionHash;
            actionHash.insert("command",aExec);
            if(!aWD.isEmpty())
                actionHash.insert("workingdir",aWD);
            if(!aLPrefix.isEmpty())
                actionHash.insert("launch-prefix",aLPrefix);
            actionHash.insert("displayname",aName);
            containerHash.insert(aID,actionHash);
        }
    }
    QHash<QString,QVariant> desktopHash;
    desktopHash.insert("displayname",dName);
    desktopHash.insert("description",dDesc);
    desktopHash.insert("wmclass",dWMClass);
    desktopHash.insert("icon",dIcon);
    runtimeValues.remove("launchables");
    containerHash.insert("default.desktop",desktopHash);
    runtimeValues.insert("launchables",containerHash);
    return;
}


void JasonParser::systemHandle(QHash<QString, QVariant> systemElement){
    QHash<QString,QVariant> systemHash;
    foreach(QString key, systemElement.keys()){
        if(key=="config-prefix")
            systemHash.insert(key,systemElement.value(key).toString());
        if(key=="launch-prefix") //prefix for command line, ex. "[prefix, ex. wine] test.exe"
            systemHash.insert(key,systemElement.value(key).toString());
        if(key=="launch-suffix") //suffix for command line, ex. "wine test.exe [suffix]"
            systemHash.insert(key,systemElement.value(key).toString());
        if(key=="identifier") //the different keys found in "launchtype"
            systemHash.insert(key,systemElement.value(key).toString());
        if(key=="variables") //internal variables, with different types such as config-input
            systemHash.insert(key,systemElement.value(key).toJsonArray());
        if(key=="environment") //environment settings
            systemHash.insert(key,systemElement.value(key).toJsonArray());
        if(key=="inherit")
            systemHash.insert(key,systemElement.value(key).toString());
    }
    systemTable.insert(systemHash.value("identifier").toString(),systemHash);
}


void JasonParser::subsystemActivate(QHash<QString, QVariant> subsystemElement, QVariant option,QStringList activeSystems){
    QString type;
    QStringList launchPrefixes;
    QStringList activeSystemsConfig;
    foreach(QString system, activeSystems){
        QString configPrefix = systemTable.value(system).toHash().value("config-prefix").toString();
        activeSystemsConfig.append(configPrefix);
        launchPrefixes.append(configPrefix+"="+systemTable.value(system).toHash().value("launch-prefix").toString());
    }
    foreach(QString key,subsystemElement.keys())
        if(key=="type")
            type = subsystemElement.value(key).toString();
    if(type.isEmpty())
        return;
    if(option.isNull())
        return;
    if(type=="bool")
        if(!option.toBool())//Do not run if boolean value is false
            return;

    /* Handling for each type of subsystem
     *  - select - the option (in singular) chooses which object in a list is activated. These may
     * contain variables (which are substituted in an executable statement), environment
     * variables.
     *  - bool - activates the environment depending on what the boolean value is. Nothing is done
     * if the boolean value is false.
     *  - substitution - a variable is substituted either globally or inside an executable
     * statement.
     *  - option - a list of options in an array that can be picked depending on the option
     * provided. The option is a comma-separated list.
     *
    */
    QHash<QString,QVariant> runtimeReturnHash;
    foreach(QString key,subsystemElement.keys())
        if(key=="appearance"){
            QHash<QString,QVariant> appearHash = jsonExamineObject(subsystemElement.value(key).toJsonObject());
            runtimeReturnHash.insert("desktop.title",appearHash.value("desktop.title").toString());
            runtimeReturnHash.insert("desktop.icon",appearHash.value("desktop.icon").toString());
        }

    if(type=="select"){
        QString action;
        QJsonObject selection;
        QString launchPrefix;
        foreach(QString key,subsystemElement.keys()){
            if(key.endsWith(".exec")){
                action = resolveVariable(subsystemElement.value(key).toString());
                foreach(QString prefix,launchPrefixes)
                    if(prefix.contains(key.split(".")[0]))
                        launchPrefix=prefix.split("=")[1];
            }
            if(key=="selections")
                selection = subsystemElement.value(key).toHash().value(option.toString()).toJsonObject();
        }
        if(!action.isEmpty())
            foreach(QString key,selection.keys())
                if(key=="variables"){
                    QString trigger;
                    QJsonArray selectedArray = selection.value(key).toArray();
                    for(int i=0;i<selectedArray.count();i++){
                        QString actionCopy = action;
                        QString variable = selectedArray.at(i).toObject().value("name").toString();
                        QString value = selectedArray.at(i).toObject().value("value").toString();
                        QString execLine = actionCopy.replace("%VARIABLE%",variable).replace("%VALUE%",value);
                        if(trigger.isEmpty()){
                            if(subsystemElement.value("trigger").isValid()){
                                trigger=subsystemElement.value("trigger").toString();
                            }else
                                trigger="sys-prerun";
                        }
                        runtimeReturnHash.insert("launch-prefix",launchPrefix);
                        runtimeReturnHash.insert("command",execLine);
                        addToRuntime(trigger,runtimeReturnHash);
                    }
                }
    }
    if(type=="option"){
        QStringList chosenOpts = option.toString().split(",");
        QHash<QString,QVariant> possibleOpts = subsystemElement.value("options").toHash();
        foreach(QString opt,chosenOpts)
            foreach(QString pOpt,possibleOpts.keys())
                if(pOpt==opt){
                    QHash<QString,QVariant> currOpt = possibleOpts.value(pOpt).toHash();
                    foreach(QString key,currOpt.keys()){
                        if(key=="environment")
                            foreach(QString cKey,jsonExamineArray(currOpt.value(key).toJsonArray()).keys())
                                environmentActivate(jsonExamineArray(currOpt.value(key).toJsonArray()).value(cKey).toHash(),activeSystems);
                    }
                }
    }
    if(type=="bool")
            foreach(QString key,subsystemElement.keys())
                if(key=="environment")
                    environmentActivate(subsystemElement.value(key).toList().at(0).toHash(),activeSystems);
    if(type=="substitution"){
        QString launchPrefix;
        QString trigger;
        foreach(QString key,subsystemElement.keys()){
            if(key.endsWith(".exec"))
                if(subsystemElement.keys().contains("variable")){
                    foreach(QString prefix,launchPrefixes)
                        if(prefix.contains(key.split(".")[0]))
                            launchPrefix=prefix.split("=")[1]+" ";
                    QString command;
                    command = resolveVariable(subsystemElement.value(key).toString().replace("%"+subsystemElement.value("variable").toString()+"%",activeOptions.value("subsystem."+subsystemElement.value("enabler").toString()).toString()));
                    if(trigger.isEmpty()){
                        if(subsystemElement.value("trigger").isValid()){
                            trigger=subsystemElement.value("trigger").toString();
                        }else
                            trigger="sys-prerun";
                    }
                    runtimeReturnHash.insert("launch-prefix",launchPrefix);
                    if(!command.isEmpty()){
                        runtimeReturnHash.insert("command",command);
                        addToRuntime(trigger,runtimeReturnHash);
                    }
                }
            if(key=="environment"){
                substitutes.insert(subsystemElement.value("variable").toString(),activeOptions.value("subsystem."+subsystemElement.value("enabler").toString()).toString());
                environmentActivate(subsystemElement.value(key).toList()[0].toHash(),activeSystems);
            }
        }
    }
    return;
}


void JasonParser::environmentActivate(QHash<QString,QVariant> environmentHash,QStringList activeSystems){
    QString type;
    QStringList activeSystemsConfig;
    foreach(QString system, activeSystems)
        activeSystemsConfig.append(systemTable.value(system).toHash().value("config-prefix").toString());
    //Get type in order to determine how to treat this environment entry
    foreach(QString key, environmentHash.keys()){
        if(key=="type")
            type = environmentHash.value(key).toString();
    }
    //Treat the different types according to their parameters and realize their properties
    if(type=="run-prefix"){
        foreach(QString key,environmentHash.keys())
            if(key.endsWith(".exec.prefix")) //Execution prefix, given that the system is active
                foreach(QString system,activeSystemsConfig)
                    if(key.split(".")[0]==system){
                        QString execLine = resolveVariable(environmentHash.value(key).toString());
                        QHash<QString,QVariant> runtimeReturnHash;
                        runtimeReturnHash.insert("command",resolveVariable(execLine));
                        addToRuntime("run-prefix",runtimeReturnHash);
                    }
    }else if(type=="run-suffix"){
        foreach(QString key,environmentHash.keys())
            if(key.endsWith(".exec.suffix")) //Execution prefix, given that the system is active
                foreach(QString system,activeSystemsConfig)
                    if(key.split(".")[0]==system){
                        QHash<QString,QVariant> runtimeReturnHash;
                        runtimeReturnHash.insert("command",resolveVariable(environmentHash.value(key).toString()));
                        addToRuntime("run-suffix",runtimeReturnHash);
                    }
    }else if(type=="variable"){
//        qDebug() << environmentHash;
        foreach(QString key,environmentHash.keys()){
            if(key=="name")
                if(environmentHash.keys().contains("value"))
                    setEnvVar(environmentHash.value(key).toString(),resolveVariable(environmentHash.value("value").toString()));
        }
    }else{
        qDebug() << "unsupported environment type" << type;
        return;
    }
    return;
}


int JasonParser::systemActivate(QHash<QString,QVariant> systemElement,QStringList activeSystems){
    /*activeSystems contain the identifiers for the systems currently active, including the
     * system itself as well as any it may inherit. Inheritance allows the system to access
     * elements that the other system uses, such as .postrun and .prerun, but will not inherit
     * .workdir or .exec as these are directly related to the system.
     * Properties will be explicitly inherited in order to  avoid possible collisions.
    */
    QString configPrefix = systemElement.value("config-prefix").toString();
    substitutes.insert("CONFIG_PREFIX",configPrefix);
    QString sysIdentifier = systemElement.value("identifier").toString();
    activeSystems.removeOne(sysIdentifier);
    QStringList activeSystemsConfig;
    foreach(QString system, activeSystems)
        activeSystemsConfig.append(systemTable.value(system).toHash().value("config-prefix").toString());
    //Find variables and insert them
    foreach(QString key,systemElement.keys()){
        if(key=="variables"){
            QHash<QString,QVariant> vHash = jsonExamineArray(systemElement.value(key).toJsonArray());
            variablesImport(vHash);
        }
    }
    //Resolve recently imported variables
    resolveVariables();
    //Take care of environment variables
    foreach(QString key, systemElement.keys())
        if(key=="environment"){
            QJsonArray systemEnvironmentArray = systemElement.value(key).toJsonArray();
            for(int i=0;i<systemEnvironmentArray.count();i++){
                QJsonObject instance = systemEnvironmentArray.at(i).toObject();
                environmentActivate(jsonExamineObject(instance),activeSystems);
            }
        }
    //Inheritance, inherit variables and environment
    foreach(QString inherit,activeSystems){
        QHash<QString,QVariant> inheritHash = systemTable.value(inherit).toHash();
        foreach(QString key, inheritHash.keys()){
            if(key=="environment"){
                QJsonArray systemEnvironmentArray = systemElement.value(key).toJsonArray();
                for(int i=0;i<systemEnvironmentArray.count();i++){
                    QJsonObject instance = systemEnvironmentArray.at(i).toObject();
                    environmentActivate(jsonExamineObject(instance),activeSystems);
                }
            }
            foreach(QString key,inheritHash.keys()){
                if(key=="variables"){
                    QHash<QString,QVariant> vHash = jsonExamineArray(inheritHash.value(key).toJsonArray());
                    variablesImport(vHash);
                }
            }
        }
    }
    QString mainExec,mainWDir;
    bool detachStart;
    QString launchPrefix = systemElement.value("launch-prefix").toString(); //The launchprefix is determined by the launchtype of the main execution statement. If this does not match the main system, puke an error and refuse to continue, because it's not good practice to ditch the system that is chosen.
    foreach(QString key,activeOptions.keys())
        if(key.startsWith(configPrefix)){
            if(key.endsWith(".exec")){
                mainExec = activeOptions.value(key).toString();
            }
            if(key.endsWith(".workdir"))
                mainWDir = activeOptions.value(key).toString();
            if(key=="global.detachable-process")
                detachStart = activeOptions.value(key).toBool();
        }
    if(!mainExec.isEmpty()){
        if(mainExec.isEmpty()){
            broadcastMessage(1,tr("WARN: There is no main execution value. Just a heads up.\n"));
        }
        QHash<QString,QVariant> runHash;
        runHash.insert("command",mainExec);
        runHash.insert("workingdir",mainWDir);
        runHash.insert("launch-prefix",launchPrefix);
        runHash.insert("detachable",detachStart);
        QHash<QString,QVariant> containerHash = runtimeValues.value("launchables").toHash();
        runtimeValues.remove("launchables");
        containerHash.insert("default",runHash);
        runtimeValues.insert("launchables",containerHash);
    }else{
        broadcastMessage(2,tr("ERROR: No primary command line found. Is there a mismatch between the launcher type and .exec value?\n"));
        return 1;
    }
    return 0;
}


void JasonParser::addToRuntime(QString role, QVariant input){
    /*
     * Roles:
     *  -Defines where the command is placed in the process of launching the program.
     *
     * sys-prerun - Runs before the program starts, useful for changing options and etc
     *      -QStringList, may be extended to contain information about appearance
     * sys-postrun - Runs after the program has exited
     *      -QStringList, may be extended to contain information about appearance
     * run-prefix - A piece of text, often representing a command line, prepended to the command line. Multiples of
     *      these will be appended to eachother. The launch-prefix property of the current system is appended
     *      -QStringList, may be extended to contain information about appearance
     *      to this.
     * run-suffix - A piece of text appended to the end of the command line.
     *      -QStringList, may be extended to contain information about appearance
     *
     * These conditions apply to all systems regardlessly.
    */
    if((role=="run-prefix")||(role=="run-suffix")||(role=="sys-prerun")||(role=="sys-postrun")){
        QList<QVariant> roleHashes = runtimeValues.value(role).toList();
        runtimeValues.remove(role);
        roleHashes.append(input.toHash());
        runtimeValues.insert(role,roleHashes);
    }else
        return;
}

void JasonParser::insertPrerunPostrun(QHash<QString, QVariant> runtable, int mode){
    if(runtable.isEmpty())
        return;
    foreach(QString i,runtable.keys()){
        int priority;
        QString role;
        if(mode==0){
            role="sys-prerun";
            priority=1;
        }
        if(mode==1){
            role="sys-postrun";
            priority=0;
        }
        QString command;
        QString desktopTitle;
        QString workingDir;
        QString launchPrefix;
        QHash<QString,QVariant> returnHash;
        foreach(QString key,runtable.value(i).toHash().keys()){
            if(key.endsWith(".exec")){
                foreach(QString system,systemTable.keys()){
                    QString prefix = key.split(".")[0];
                    if(prefix==systemTable.value(system).toHash().value("config-prefix").toString()){
                        launchPrefix=systemTable.value(system).toHash().value("launch-prefix").toString();
                    }
                }
                command=runtable.value(i).toHash().value(key).toString();
            }
            if(key=="priority")
                priority = runtable.value(i).toHash().value(key).toInt();
            if(key=="display.title")
                desktopTitle = runtable.value(i).toHash().value(key).toString();
            if(key.endsWith(".workdir"))
                workingDir = runtable.value(i).toHash().value(key).toString();
        }
        if(!command.isEmpty())
            returnHash.insert("command",command);
        returnHash.insert("priority",priority);
        if(!desktopTitle.isEmpty())
            returnHash.insert("desktop.title",desktopTitle);
        if(!launchPrefix.isEmpty())
            returnHash.insert("launch-prefix",launchPrefix);
        if(!workingDir.isEmpty())
            returnHash.insert("workingdir",workingDir);
        addToRuntime(role,returnHash);
    }
    return;
}

int JasonParser::runProcesses(QString launchId){
    /*
     *
     * launchId:
     *  - Either is empty (thus launching the default program) or filled with an ID. All preruns and postruns are
     * run as normal before and after, but prefixes and suffixes are not added.
     *
     * Runmodes:
     *  - 0: Launch action or default option
    */
    if(launchId.isEmpty())
        launchId = "default";

    QList<QVariant> runPrefix;
    QList<QVariant> runSuffix;
    QHash<QString,QVariant> launchables;
    foreach(QString element, runtimeValues.keys()){
        if((element=="run-prefix")&&(!runtimeValues.value(element).toList().isEmpty()))
            runPrefix = runtimeValues.value(element).toList();
        if((element=="launchables")&&(!runtimeValues.value(element).toHash().isEmpty()))
            launchables = runtimeValues.value(element).toHash();
        if((element=="run-suffix")&&(!runtimeValues.value(element).toList().isEmpty()))
            runSuffix = runtimeValues.value(element).toList();
    }

    //We compile the run prefixes and suffixes to strings that we may use much more easily
    QString runprefixStr,runsuffixStr;
    for(int i = 0;i<runPrefix.count();i++){
        runprefixStr.append(" "+runPrefix.at(i).toHash().value("command").toString());
    }
    for(int i = 0;i<runSuffix.count();i++){
        runsuffixStr.append(" "+runSuffix.at(i).toHash().value("command").toString());
    }

    doPrerun();

    //This is where the magic happens.
    // We need to see if the 'global.detachable-process'-key is true
    bool isDetach;
    if(launchId=="default")
        foreach(QString key, activeOptions.keys())
            if(key=="global.detachable-process")
                isDetach=activeOptions.value(key).toBool();

    QString desktopTitle;
    foreach(QString launchable,launchables.keys())
        if(launchable==launchId){
            QString argument,program,workDir,name;
            QHash<QString,QVariant> launchObject = launchables.value(launchable).toHash();
            foreach(QString key,launchObject.keys()){
                if(key=="command")
                    argument=resolveVariable(launchObject.value(key).toString());
                if(key=="launch-prefix")
                    program=resolveVariable(launchObject.value(key).toString());
                if(key=="workingdir")
                    workDir=resolveVariable(launchObject.value(key).toString());
                if(key=="desktop.title")
                    name=resolveVariable(launchObject.value(key).toString());
            }
            if(launchId=="default"){
                foreach(QString key,launchables.value("default.desktop").toHash().keys())
                    if(key=="displayname")
                        name=resolveVariable(launchables.value("default.desktop").toHash().value(key).toString());
            }
            desktopTitle=name;
//            updateProgressText("Currently launching "+name);
            updateProgressTitle(name);
            if(!argument.isEmpty())
                executeProcess(argument,program,workDir,name,runprefixStr,runsuffixStr);
        }
    //Aaaand it's gone.

    connect(this,SIGNAL(mainProcessEnd()),SLOT(doPostrun()));

    if(isDetach){
        emit displayDetachedMessage(desktopTitle);
    }else
        emit mainProcessEnd();

    return 0;
}

void JasonParser::doPostrun(){
    QList<QVariant> sysPostrun;
    sysPostrun = runtimeValues.value("sys-postrun").toList();
    if(sysPostrun.isEmpty())
        return;
    //Two passes, one where priority options are forced to run first, then the others may run.
    for(int i=0;i<sysPostrun.count();i++){
        QHash<QString,QVariant> object = sysPostrun.at(i).toHash();
        if(object.value("priority").toString()!="1"){
            QString program,argument,workDir,name;
            foreach(QString key,object.keys()){
                if(key=="command")
                    argument=resolveVariable(object.value(key).toString());
                if(key=="launch-prefix")
                    program=resolveVariable(object.value(key).toString());
                if(key=="workingdir")
                    workDir=resolveVariable(object.value(key).toString());
                if(key=="desktop.title")
                    name=resolveVariable(object.value(key).toString());
            }
            executeProcess(argument,program,workDir,name,QString(),QString());
        }
    }
    for(int i=0;i<sysPostrun.count();i++){
        QHash<QString,QVariant> object = sysPostrun.at(i).toHash();
        if(object.value("priority").toString()=="1"){
            QString program,argument,workDir,name;
            foreach(QString key,object.keys()){
                if(key=="command")
                    argument=resolveVariable(object.value(key).toString());
                if(key=="launch-prefix")
                    program=resolveVariable(object.value(key).toString());
                if(key=="workingdir")
                    workDir=resolveVariable(object.value(key).toString());
                if(key=="desktop.title")
                    name=resolveVariable(object.value(key).toString());
            }
            executeProcess(argument,program,workDir,name,QString(),QString());
        }
    }
    emit finishedProcessing(); //When postrun is over, the program is one and so it is appropriate to call it here.
}

void JasonParser::doPrerun(){
    QList<QVariant> sysPrerun;
    sysPrerun = runtimeValues.value("sys-prerun").toList();
    if(sysPrerun.isEmpty())
        return;
    //Two passes, one where priority options are forced to run first, then the others may run.
    for(int i=0;i<sysPrerun.count();i++){
        QHash<QString,QVariant> object = sysPrerun.at(i).toHash();
        if(object.value("priority").toString()=="0"){
            QString program,argument,workDir,name;
            foreach(QString key,object.keys()){
                if(key=="command")
                    argument=resolveVariable(object.value(key).toString());
                if(key=="launch-prefix")
                    program=resolveVariable(object.value(key).toString());
                if(key=="workingdir")
                    workDir=resolveVariable(object.value(key).toString());
                if(key=="desktop.title")
                    name=resolveVariable(object.value(key).toString());
            }
            executeProcess(argument,program,workDir,name,QString(),QString());
        }
    }
    for(int i=0;i<sysPrerun.count();i++){
        QHash<QString,QVariant> object = sysPrerun.at(i).toHash();
        if(object.value("priority").toString()!="0"){
            QString program,argument,workDir,name;
            foreach(QString key,object.keys()){
                if(key=="command")
                    argument=resolveVariable(object.value(key).toString());
                if(key=="launch-prefix")
                    program=resolveVariable(object.value(key).toString());
                if(key=="workingdir")
                    workDir=resolveVariable(object.value(key).toString());
                if(key=="desktop.title")
                    name=resolveVariable(object.value(key).toString());
            }
            executeProcess(argument,program,workDir,name,QString(),QString());
        }
    }
}

void JasonParser::executeProcess(QString argument, QString program, QString workDir, QString title, QString runprefix, QString runsuffix){
    /*
     * program - Prefixed to argument, specifically it could be 'wine' or another frontend program such as
     * 'mupen64plus'. It is not supposed to run shells.
     * argument - The command line, only in this context it is indeed an argument.
     * workDir - The wished working directory for the operation.
    */

    /*
     * TODO:
     *  - Insert prefix and suffix into the command line by way of prepending and appending.
     *  - Show some GUI magic run by a separate thread as a user-friendly indicator for progress, may also
     *      want to show a dialog button for detaching processes so that postrun doesn't run prematurely.
     *  - Implement programming to pick up aforementioned option
     *  - Add the desktop file generation function and decide how the Exec option should look
     *  - Add even more pretty GUIs
     *
    */

    QString shell = "sh"; //Just defaults, in case nothing is specified.
    QString shellArg = "-c"; //We don't want the shell to pick up more arguments.
    QJsonObject shellOptions = activeOptions.value("shell.properties").toJsonObject();
    foreach(QString key,shellOptions.keys()){
        if(key=="shell")
            shell=shellOptions.value(key).toString();
        if(key=="command.argument")
            shellArg=shellOptions.value(key).toString();
    }
    if(shell.isEmpty())
        return;

    QProcess executer;
    QStringList arguments;
    executer.setProcessEnvironment(procEnv);
    executer.setProcessChannelMode(QProcess::SeparateChannels);
    if(!workDir.isEmpty())
        executer.setWorkingDirectory(workDir);
    executer.setProgram(shell);
    arguments.append(shellArg);
    arguments.append("--");

    QString execString;
    execString = program+" "+argument;
    //We apply the prefixes/suffixes
    if(!runprefix.isEmpty())
        execString.prepend(runprefix+" ");
    if(!runsuffix.isEmpty())
        execString.append(" "+runsuffix);

    arguments.append(execString);
    executer.setArguments(arguments);

    //Connect signals and slots as well as update the title.
    connect(&executer, SIGNAL(finished(int,QProcess::ExitStatus)),SLOT(processFinished(int,QProcess::ExitStatus)));
    connect(&executer, SIGNAL(error(QProcess::ProcessError)),SLOT(processOutputError(QProcess::ProcessError)));
    connect(&executer,SIGNAL(started()),SLOT(processStarted()));
    updateProgressText(tr("Launching")+" "+title);

    executer.start();
    executer.waitForFinished();
}

void JasonParser::processFinished(int exitCode, QProcess::ExitStatus exitStatus){
    /*
     * We won't care about exitStatus for now.
     * broadcastMessage() function shows a QMessageBox, its prototype is:
     * int,QString
     * where int is either 0, success, 1, warning, 2, error, or 3, disabled. (The last one exists because I am lazy.)
     * The QString is just the message it shows.
    */
    if(exitCode==0){
        broadcastMessage(3,tr("Process exited successfully."));
    }else
        broadcastMessage(2,tr("Process exited with status:")+" "+QString::number(exitCode)+".");
}

void JasonParser::processOutputError(QProcess::ProcessError processError){
    qDebug() << processError;
    emit processFailed(processError);
}

void JasonParser::processStarted(){

}

void JasonParser::detachedMainProcessClosed(){
    emit mainProcessEnd();
}