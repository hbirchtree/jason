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
        if(object.isValid()&&object.type()==QVariant::Hash)
            desktopfilegenerator.desktopFileBuild(object.toHash(),jsonFinalData.value("systems").toHash());
    } else {
        parser.jasonActivateSystems(jsonFinalData);
    }

    emit finishedProcessing();
//    if(jsonParse(jsonOpenFile(startDocument))!=0){
//        updateProgressText(tr("Error occured"));
//        broadcastMessage(2,tr("Apples is stuck in a tree! We need to call the fire department!\n"));
//        emit toggleCloseButton(true);
//        emit failedProcessing();
//        return;
//    }

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

//void JasonParser::subsystemActivate(QHash<QString, QVariant> subsystemElement, QVariant option,QStringList activeSystems){
//    QString type;
//    QStringList launchPrefixes;
//    QStringList activeSystemsConfig;
//    foreach(QString system, activeSystems){
//        QString configPrefix = systemTable.value(system).toHash().value("config-prefix").toString();
//        activeSystemsConfig.append(configPrefix);
//        launchPrefixes.append(configPrefix+"="+systemTable.value(system).toHash().value("launch-prefix").toString());
//    }
//    foreach(QString key,subsystemElement.keys())
//        if(key=="type")
//            type = subsystemElement.value(key).toString();
//    if(type.isEmpty())
//        return;
//    if(option.isNull())
//        return;
//    if(type=="bool")
//        if(!option.toBool())//Do not run if boolean value is false
//            return;

//    /* Handling for each type of subsystem
//     *  - select - the option (in singular) chooses which object in a list is activated. These may
//     * contain variables (which are substituted in an executable statement), environment
//     * variables.
//     *  - bool - activates the environment depending on what the boolean value is. Nothing is done
//     * if the boolean value is false.
//     *  - substitution - a variable is substituted either globally or inside an executable
//     * statement.
//     *  - option - a list of options in an array that can be picked depending on the option
//     * provided. The option is a comma-separated list.
//     *
//    */
//    QHash<QString,QVariant> runtimeReturnHash;
//    foreach(QString key,subsystemElement.keys())
//        if(key=="appearance"){
//            QHash<QString,QVariant> appearHash = jsonExamineObject(subsystemElement.value(key).toJsonObject());
//            runtimeReturnHash.insert("desktop.title",appearHash.value("desktop.title").toString());
//            runtimeReturnHash.insert("desktop.icon",appearHash.value("desktop.icon").toString());
//        }

//    if(type=="select"){
//        /*
//         * Y'all need subtypes, goshdarnit!
//        */
//        QString launchPrefix,action,trigger;
//        trigger="sys-prerun"; //Default placement in case nothing is selected
//        QJsonObject selection;
//        QString subtype = subsystemElement.value("subtype").toString();
//        /*
//         * subtypes:
//         *  - key-value-sets:
//         *          sets containing variables in which the name is used as key and value as variable.
//         *  - I'll add more as I see fit.
//         */
//        foreach(QString key,subsystemElement.keys()){
//            if(subtype=="key-value-set"){
//                if(key.endsWith(".exec")){
//                    action = resolveVariable(subsystemElement.value(key).toString());
//                    foreach(QString prefix,launchPrefixes)
//                        if(prefix.split("=")[0]==key.split(".")[0]) //Compare the config prefixes
//                            launchPrefix=prefix.split("=")[1];
//                }
//                if(key=="sets")
//                    selection = subsystemElement.value(key).toHash().value(option.toString()).toJsonObject(); //Out of all the sets, grab the one matching the selected one.
//                if(key=="trigger")
//                    trigger = subsystemElement.value(key).toString();
//            }
//        }
//        if(!action.isEmpty())
//            foreach(QString key,selection.keys())
//                if(key=="keysets"){
//                    QJsonArray selectedArray = selection.value(key).toArray();
//                    for(int i=0;i<selectedArray.count();i++){
//                        QString actionCopy = action; //We copy the action so that the original is not altered by replacements
//                        QString keyName,keyValue,valueName,valueValue; //Silly indeed.
//                        QJsonObject keyset = selectedArray.at(i).toObject();
//                        foreach(QString thingy,keyset.keys()){
//                            if(thingy.startsWith("key.")){
//                                keyName=thingy.split(".")[1];
//                                keyValue=keyset.value(thingy).toString();
//                            }
//                            if(thingy.startsWith("value.")){
//                                valueName=thingy.split(".")[1];
//                                valueValue=keyset.value(thingy).toString();
//                            }
//                        }

//                        QString execLine = actionCopy.replace(keyName,keyValue).replace(valueName,valueValue);
//                        runtimeReturnHash.insert("launch-prefix",launchPrefix);
//                        runtimeReturnHash.insert("command",execLine);
//                        addToRuntime(trigger,runtimeReturnHash);
//                    }
//                }
//    }
//    if(type=="option"){
//        QStringList chosenOpts = option.toString().split(","); //Contains the options that are chosen
//        QHash<QString,QVariant> possibleOpts = subsystemElement.value("options").toHash(); //Contains all possible options that are defined
//        foreach(QString opt,chosenOpts)
//            foreach(QString pOpt,possibleOpts.keys())
//                if(pOpt==opt){
//                    QHash<QString,QVariant> currOpt = possibleOpts.value(pOpt).toHash();
//                    foreach(QString key,currOpt.keys()){
//                        if(key=="environment")
//                            foreach(QString cKey,jsonExamineArray(currOpt.value(key).toJsonArray()).keys())
//                                environmentActivate(jsonExamineArray(currOpt.value(key).toJsonArray()).value(cKey).toHash(),activeSystems);
//                        if(key=="variables")
//                            variablesImport(jsonExamineArray(currOpt.value(key).toJsonArray()));
//                    }
//                }
//    }
//    if(type=="bool") //This is the prettiest type of them all. Sadly.
//            foreach(QString key,subsystemElement.keys()){
//                if(key=="environment")
//                    environmentActivate(subsystemElement.value(key).toList().at(0).toHash(),activeSystems);
//                if(key=="variables")
//                    variablesImport(subsystemElement.value(key).toList().at(0).toHash());
//            }
//    if(type=="substitution"){ //Needs cleanup
//        QString launchPrefix;
//        QString trigger;
//        foreach(QString key,subsystemElement.keys()){
//            if(key.endsWith(".exec"))
//                if(subsystemElement.keys().contains("variable")){
//                    //Because there is a .exec value we work on it and its command line.
//                    foreach(QString prefix,launchPrefixes)
//                        if(prefix.contains(key.split(".")[0]))
//                            launchPrefix=prefix.split("=")[1]+" ";
//                    QString command;
//                    command = resolveVariable(subsystemElement.value(key).toString().replace("%"+subsystemElement.value("variable").toString()+"%",activeOptions.value("subsystem."+subsystemElement.value("enabler").toString()).toString()));
//                    if(trigger.isEmpty()){
//                        if(subsystemElement.value("trigger").isValid()){
//                            trigger=subsystemElement.value("trigger").toString();
//                        }else
//                            trigger="sys-prerun";
//                    }
//                    runtimeReturnHash.insert("launch-prefix",launchPrefix);
//                    if(!command.isEmpty()){
//                        runtimeReturnHash.insert("command",command);
//                        addToRuntime(trigger,runtimeReturnHash);
//                    }
//                }
//            if(key=="environment"){
//                //Insert the string of the enabler into the substitute list, making this a global substitution. Is to be used for the environment below.
//                substitutes.insert(subsystemElement.value("variable").toString(),activeOptions.value("subsystem."+subsystemElement.value("enabler").toString()).toString());
//                environmentActivate(subsystemElement.value(key).toList()[0].toHash(),activeSystems);
//            }
//            if(key=="variables"){
//                variablesImport(subsystemElement.value(key).toList()[0].toHash());
//            }
//        }
//    }
//    return;
//}


//int JasonParser::systemActivate(QHash<QString,QVariant> *systemElement,QStringList *activeSystems){
    /*activeSystems contain the identifiers for the systems currently active, including the
     * system itself as well as any it may inherit. Inheritance allows the system to access
     * elements that the other system uses, such as .postrun and .prerun, but will not inherit
     * .workdir or .exec as these are directly related to the system.
     * Properties will be explicitly inherited in order to  avoid possible collisions.
    */
//    QString configPrefix = systemElement.value("config-prefix").toString();
//    substitutes.insert("CONFIG_PREFIX",configPrefix);
//    QString sysIdentifier = systemElement.value("identifier").toString();
//    activeSystems.removeOne(sysIdentifier);
//    QStringList activeSystemsConfig;
//    foreach(QString system, activeSystems)
//        activeSystemsConfig.append(systemTable.value(system).toHash().value("config-prefix").toString());
//    //Find variables and insert them
//    foreach(QString key,systemElement.keys()){
//        if(key=="variables"){
//            QHash<QString,QVariant> vHash = jsonExamineArray(systemElement.value(key).toJsonArray());
//            variablesImport(vHash);
//        }
//    }
//    //Resolve recently imported variables
//    resolveVariables();
//    //Take care of environment variables
//    foreach(QString key, systemElement.keys())
//        if(key=="environment"){
//            QJsonArray systemEnvironmentArray = systemElement.value(key).toJsonArray();
//            for(int i=0;i<systemEnvironmentArray.count();i++){
//                QJsonObject instance = systemEnvironmentArray.at(i).toObject();
//                environmentActivate(jsonExamineObject(instance),activeSystems);
//            }
//        }
//    //Inheritance, inherit variables and environment
//    foreach(QString inherit,activeSystems){
//        QHash<QString,QVariant> inheritHash = systemTable.value(inherit).toHash();
//        foreach(QString key, inheritHash.keys()){
//            if(key=="environment"){
//                QJsonArray systemEnvironmentArray = systemElement.value(key).toJsonArray();
//                for(int i=0;i<systemEnvironmentArray.count();i++){
//                    QJsonObject instance = systemEnvironmentArray.at(i).toObject();
//                    environmentActivate(jsonExamineObject(instance),activeSystems);
//                }
//            }
//            foreach(QString key,inheritHash.keys()){
//                if(key=="variables"){
//                    QHash<QString,QVariant> vHash = jsonExamineArray(inheritHash.value(key).toJsonArray());
//                    variablesImport(vHash);
//                }
//            }
//        }
//    }
//    QString mainExec,mainWDir;
//    QString launchPrefix = systemElement.value("launch-prefix").toString(); //The launchprefix is determined by the launchtype of the main execution statement. If this does not match the main system, puke an error and refuse to continue, because it's not good practice to ditch the system that is chosen.
//    foreach(QString key,activeOptions.keys())
//        if(key.startsWith(configPrefix)){
//            if(key.endsWith(".exec")){
//                mainExec = activeOptions.value(key).toString();
//            }
//            if(key.endsWith(".workdir"))
//                mainWDir = activeOptions.value(key).toString();
//        }
//    if(!mainExec.isEmpty()){
//        if(mainExec.isEmpty()){
//            broadcastMessage(1,tr("WARN: There is no main execution value. Just a heads up.\n"));
//        }
//        QHash<QString,QVariant> runHash;
//        runHash.insert("command",mainExec);
//        runHash.insert("workingdir",mainWDir);
//        runHash.insert("launch-prefix",launchPrefix);
//        QHash<QString,QVariant> containerHash = runtimeValues.value("launchables").toHash();
//        runtimeValues.remove("launchables");
//        containerHash.insert("default",runHash);
//        runtimeValues.insert("launchables",containerHash);
//    }else{
//        broadcastMessage(2,tr("ERROR: No primary command line found. Is there a mismatch between the launcher type and .exec value?\n"));
//        return 1;
//    }
//    return 0;
//}


//void JasonParser::addToRuntime(QString role, QVariant input){
//    /*
//     * Roles:
//     *  -Defines where the command is placed in the process of launching the program.
//     *
//     * sys-prerun - Runs before the program starts, useful for changing options and etc
//     *      -QStringList, may be extended to contain information about appearance
//     * sys-postrun - Runs after the program has exited
//     *      -QStringList, may be extended to contain information about appearance
//     * run-prefix - A piece of text, often representing a command line, prepended to the command line. Multiples of
//     *      these will be appended to eachother. The launch-prefix property of the current system is appended
//     *      -QStringList, may be extended to contain information about appearance
//     *      to this.
//     * run-suffix - A piece of text appended to the end of the command line.
//     *      -QStringList, may be extended to contain information about appearance
//     *
//     * These conditions apply to all systems regardlessly.
//    */
//    if((role=="run-prefix")||(role=="run-suffix")||(role=="sys-prerun")||(role=="sys-postrun")){
//        QList<QVariant> roleHashes = runtimeValues.value(role).toList();
//        runtimeValues.remove(role);
//        roleHashes.append(input.toHash());
//        runtimeValues.insert(role,roleHashes);
//    }else
//        return;
//}

//void JasonParser::insertPrerunPostrun(QHash<QString, QVariant> runtable, int mode){
//    if(runtable.isEmpty())
//        return;
//    foreach(QString i,runtable.keys()){
//        int priority;
//        QString role;
//        if(mode==0){
//            role="sys-prerun";
//            priority=1;
//        }
//        if(mode==1){
//            role="sys-postrun";
//            priority=0;
//        }
//        QString command,desktopTitle,workingDir,launchPrefix,desktopIcon;
//        QHash<QString,QVariant> returnHash;
//        foreach(QString key,runtable.value(i).toHash().keys()){
//            if(key.endsWith(".exec")){
//                foreach(QString system,systemTable.keys()){
//                    QString prefix = key.split(".")[0];
//                    if(prefix==systemTable.value(system).toHash().value("config-prefix").toString()){
//                        launchPrefix=systemTable.value(system).toHash().value("launch-prefix").toString();
//                    }
//                }
//                command=runtable.value(i).toHash().value(key).toString();
//            }
//            if(key=="priority")
//                priority = runtable.value(i).toHash().value(key).toInt();
//            if(key=="display.title")
//                desktopTitle = runtable.value(i).toHash().value(key).toString();
//            if(key=="display.icon")
//                desktopIcon = runtable.value(i).toHash().value(key).toString();
//            if(key.endsWith(".workdir"))
//                workingDir = runtable.value(i).toHash().value(key).toString();
//        }
//        if(!command.isEmpty())
//            returnHash.insert("command",command);
//        returnHash.insert("priority",priority);
//        if(!desktopTitle.isEmpty())
//            returnHash.insert("desktop.title",desktopTitle);
//        if(!desktopIcon.isEmpty())
//            returnHash.insert("desktop.icon",desktopIcon);
//        if(!launchPrefix.isEmpty())
//            returnHash.insert("launch-prefix",launchPrefix);
//        if(!workingDir.isEmpty())
//            returnHash.insert("workingdir",workingDir);
//        addToRuntime(role,returnHash);
//    }
//    return;
//}

//int JasonParser::runProcesses(QString launchId){
//    /*
//     *
//     * launchId:
//     *  - Either is empty (thus launching the default program) or filled with an ID. All preruns and postruns are
//     * run as normal before and after, but prefixes and suffixes are not added.
//     *
//     * Runmodes:
//     *  - 0: Launch action or default option
//    */
//    if(launchId.isEmpty())
//        launchId = "default";

//    QList<QVariant> runPrefix;
//    QList<QVariant> runSuffix;
//    QHash<QString,QVariant> launchables;
//    foreach(QString element, runtimeValues.keys()){
//        if((element=="run-prefix")&&(!runtimeValues.value(element).toList().isEmpty()))
//            runPrefix = runtimeValues.value(element).toList();
//        if((element=="launchables")&&(!runtimeValues.value(element).toHash().isEmpty()))
//            launchables = runtimeValues.value(element).toHash();
//        if((element=="run-suffix")&&(!runtimeValues.value(element).toList().isEmpty()))
//            runSuffix = runtimeValues.value(element).toList();
//    }

//    //We compile the run prefixes and suffixes to strings that we may use much more easily
//    QString runprefixStr,runsuffixStr;
//    for(int i = 0;i<runPrefix.count();i++){
//        runprefixStr.append(" "+runPrefix.at(i).toHash().value("command").toString());
//    }
//    for(int i = 0;i<runSuffix.count();i++){
//        runsuffixStr.append(" "+runSuffix.at(i).toHash().value("command").toString());
//    }

//    bool doHideUi = false;
//    foreach(QString opt,activeOptions.keys())
//        if(opt=="global.jason-opts"){
//            QJsonObject jasonOpts = activeOptions.value(opt).toJsonObject();
//            foreach(QString key,jasonOpts.keys())
//                if(key.startsWith("jason.")){
//                        if((key.endsWith(".hide-ui-on-run"))&&(jasonOpts.value(key).isBool()))
//                            doHideUi = jasonOpts.value(key).toBool();
//                }
//        }
//    doPrerun();

//    //This is where the magic happens.
//    // We need to see if the 'global.detachable-process'-key is true
//    bool isDetach = false;
//    if(launchId=="default")
//        foreach(QString key, activeOptions.keys())
//            if(key=="global.detachable-process")
//                isDetach=activeOptions.value(key).toBool();

//    QString desktopTitle;
//    foreach(QString launchable,launchables.keys())
//        if(launchable==launchId){
//            QString argument,program,workDir,name;
//            QHash<QString,QVariant> launchObject = launchables.value(launchable).toHash();
//            foreach(QString key,launchObject.keys()){
//                if(key=="command")
//                    argument=resolveVariable(launchObject.value(key).toString());
//                if(key=="launch-prefix")
//                    program=resolveVariable(launchObject.value(key).toString());
//                if(key=="workingdir")
//                    workDir=resolveVariable(launchObject.value(key).toString());
//                if(key=="desktop.title")
//                    name=resolveVariable(launchObject.value(key).toString());
//            }
//            if(launchId=="default"){
//                foreach(QString key,launchables.value("default.desktop").toHash().keys())
//                    if(key=="displayname")
//                        name=resolveVariable(launchables.value("default.desktop").toHash().value(key).toString());
//            }
//            desktopTitle=name;
//            updateProgressTitle(name);
//            if((doHideUi))
//                toggleProgressVisible(false);
//            connect(this,SIGNAL(mainProcessEnd()),SLOT(doPostrun()));
//            if(!argument.isEmpty())
//                executeProcess(argument,program,workDir,name,runprefixStr,runsuffixStr);
//            if((doHideUi))
//                toggleProgressVisible(true);
//        }
//    //Aaaand it's gone.

//    if((isDetach)){
//        emit displayDetachedMessage(desktopTitle);
//    }else
//        emit mainProcessEnd();
//    return 0;
//}

//void JasonParser::doPostrun(){
//    QList<QVariant> sysPostrun;
//    sysPostrun = runtimeValues.value("sys-postrun").toList();
//    if(sysPostrun.isEmpty())
//        return;
//    //Two passes, one where priority options are forced to run first, then the others may run.
//    for(int i=0;i<sysPostrun.count();i++){
//        QHash<QString,QVariant> object = sysPostrun.at(i).toHash();
//        if(object.value("priority").toString()!="1"){
//            QString prefix,workDir,name,icon;
//            QStringList arguments;
//            foreach(QString key,object.keys()){
//                if(key=="command")
//                    arguments.append(resolveVariable(object.value(key).toString()));
//                if(key=="launch-prefix")
//                    prefix=resolveVariable(object.value(key).toString());
//                if(key=="workingdir")
//                    workDir=resolveVariable(object.value(key).toString());
//                if(key=="desktop.title")
//                    name=resolveVariable(object.value(key).toString());
//                if(key=="desktop.icon")
//                    icon=resolveVariable(object.value(key).toString());
//            }
//            arguments.prepend(prefix);
//            updateProgressText(name);
//            updateProgressIcon(icon);
//            executeProcess(argument,program,workDir,name,QString(),QString());
//        }
//    }
//    for(int i=0;i<sysPostrun.count();i++){
//        QHash<QString,QVariant> object = sysPostrun.at(i).toHash();
//        if(object.value("priority").toString()=="1"){
//            QString prefix,workDir,name,icon;
//            QStringList arguments;
//            foreach(QString key,object.keys()){
//                if(key=="command")
//                    arguments.append(resolveVariable(object.value(key).toString()));
//                if(key=="launch-prefix")
//                    prefix=resolveVariable(object.value(key).toString());
//                if(key=="workingdir")
//                    workDir=resolveVariable(object.value(key).toString());
//                if(key=="desktop.title")
//                    name=resolveVariable(object.value(key).toString());
//                if(key=="desktop.icon")
//                    icon=resolveVariable(object.value(key).toString());
//            }
//            arguments.prepend(prefix);
//            updateProgressText(name);
//            updateProgressIcon(icon);
//            executeProcess(argument,program,workDir,name,QString(),QString());
//        }
//    }
//    emit toggleCloseButton(true);
//    if(exitResult==0){
//        emit finishedProcessing(); //When postrun is over, the program is one and so it is appropriate to call it here.
//    }else{
//        updateProgressText(tr("Non-zero exit status was encountered."));
//        emit failedProcessing();
//    }
//}

//void JasonParser::doPrerun(){
//    QList<QVariant> sysPrerun;
//    sysPrerun = runtimeValues.value("sys-prerun").toList();
//    if(sysPrerun.isEmpty())
//        return;
//    //Two passes, one where priority options are forced to run first, then the others may run.
//    for(int i=0;i<sysPrerun.count();i++){
//        QHash<QString,QVariant> object = sysPrerun.at(i).toHash();
//        if(object.value("priority").toString()=="0"){
//            QString prefix,workDir,name,icon;
//            QStringList arguments;
//            foreach(QString key,object.keys()){
//                if(key=="command")
//                    arguments.append(resolveVariable(object.value(key).toString()));
//                if(key=="launch-prefix")
//                    prefix=resolveVariable(object.value(key).toString());
//                if(key=="workingdir")
//                    workDir=resolveVariable(object.value(key).toString());
//                if(key=="desktop.title")
//                    name=resolveVariable(object.value(key).toString());
//                if(key=="desktop.icon")
//                    icon=resolveVariable(object.value(key).toString());
//            }
//            arguments.prepend(prefix);
//            updateProgressText(name);
//            updateProgressIcon(icon);
//            executeProcess(argument,program,workDir,name,QString(),QString());
//        }
//    }
//    for(int i=0;i<sysPrerun.count();i++){
//        QHash<QString,QVariant> object = sysPrerun.at(i).toHash();
//        if(object.value("priority").toString()!="0"){
//            QString prefix,workDir,name,icon;
//            QStringList arguments;
//            foreach(QString key,object.keys()){
//                if(key=="command")
//                    arguments.append(resolveVariable(object.value(key).toString()));
//                if(key=="launch-prefix")
//                    prefix=resolveVariable(object.value(key).toString());
//                if(key=="workingdir")
//                    workDir=resolveVariable(object.value(key).toString());
//                if(key=="desktop.title")
//                    name=resolveVariable(object.value(key).toString());
//                if(key=="desktop.icon")
//                    icon=resolveVariable(object.value(key).toString());
//            }
//            arguments.prepend(prefix);
//            updateProgressText(name);
//            updateProgressIcon(icon);
//            executeProcess(arguments,program,workDir,);
//        }
//    }
//}

int JasonParser::executeProcess(QStringList arguments, QString workDir, QProcessEnvironment procEnv, bool lazyExitStatus, bool detached){
    Executer partyTime;

    QString shell;// = activeOptions.value("shell.properties").toJsonObject().value("shell").toString("sh");
    QString shellArg;// = activeOptions.value("shell.properties").toJsonObject().value("shell").toString("-c");
    arguments.prepend(shellArg);
    qDebug() << shell << arguments << workDir;


}

void JasonParser::detachedMainProcessClosed(){
    emit detachedRunEnd();
}

void JasonParser::receiveLogOutput(QString stdOut, QString stdErr){
    emit emitOutput(stdOut,stdErr);
}

//void JasonParser::generateDesktopFile(QString desktopFile, QString jasonPath, QString inputDoc){
//    //Not a very robust function, but I believe it is sufficient for its purpose.
//    QFile outputDesktopFile;
//    if(!desktopFile.endsWith(".desktop"))
//        broadcastMessage(1,tr("Warning: The filename specified for the output .desktop file does not have the correct extension.\n"));
//    outputDesktopFile.setFileName(desktopFile);
//    if(outputDesktopFile.exists()){
//        emit toggleCloseButton(true);
//        updateProgressText(tr("The file exists. Will not proceed."));
//        emit failedProcessing();
//        return;
//    }
//    if(!outputDesktopFile.open(QIODevice::WriteOnly | QIODevice::Text)){
//        emit toggleCloseButton(true);
//        updateProgressText(tr("Failed to open the output file for writing. Will not proceed."));
//        emit failedProcessing();
//        return;
//    }
//    QString desktopActions;
//    QString outputContents;
//    outputContents.append("[Desktop Entry]\nVersion=1.0\nType=Application\nTerminal=False\nExec='%JASON_EXEC%' '%INPUT_FILE%'\n%DESKTOP_ACTION_LIST%\n");
//    foreach(QString entry,runtimeValues.keys())
//        if(entry=="launchables"){
//            QHash<QString,QVariant> launchables = runtimeValues.value(entry).toHash();
//            foreach(QString key,launchables.keys()){
//                if(key=="default.desktop"){
//                    QHash<QString,QVariant> defaultDesktop = launchables.value(key).toHash();
//                    foreach(QString dKey,defaultDesktop.keys()){
//                        if(dKey=="displayname")
//                            outputContents.append("Name="+defaultDesktop.value(dKey).toString()+"\n");
//                        if((dKey=="description")&&(!defaultDesktop.value(dKey).toString().isEmpty()))
//                            outputContents.append("Comment="+defaultDesktop.value(dKey).toString()+"\n");
//                        if((dKey=="wmclass")&&(!defaultDesktop.value(dKey).toString().isEmpty()))
//                            outputContents.append("StartupWMClass="+defaultDesktop.value(dKey).toString()+"\n");
//                        if((dKey=="icon")&&(!defaultDesktop.value(dKey).toString().isEmpty()))
//                            outputContents.append("Icon="+defaultDesktop.value(dKey).toString()+"\n");
//                        if((dKey=="categories")&&(!defaultDesktop.value(dKey).toString().isEmpty()))
//                            outputContents.append("Categories="+defaultDesktop.value(dKey).toString()+"\n");
//                    }
//                }
//            }
//            foreach(QString key,launchables.keys()){
//                if((key!="default")&&(key!="default.desktop")){
//                    QHash<QString,QVariant> desktopAction = launchables.value(key).toHash();
//                    QString desktopActionEntry;
//                    foreach(QString dKey,desktopAction.keys())
//                        if(dKey=="displayname"){
//                            desktopActions.append(key+";");
//                            desktopActionEntry = "[Desktop Action "+key+"]\nName="+desktopAction.value(dKey).toString()+"\nExec='%JASON_EXEC%' --action "+key+" '%INPUT_FILE%'\n";
//                        }
//                    outputContents.append("\n"+desktopActionEntry);
//                }
//            }
//        }
//    if(!jasonPath.isEmpty()){
//        QFileInfo jasonInfo(jasonPath);
//        outputContents.replace("%JASON_EXEC%",jasonInfo.canonicalFilePath());
//    }
//    if(!jasonPath.isEmpty()){
//        QFileInfo docInfo(inputDoc);
//        outputContents.replace("%INPUT_FILE%",docInfo.canonicalFilePath());
//        outputContents.replace("%WORKINGDIR%",docInfo.canonicalPath());
//    }
//    if(!desktopActions.isEmpty()){
//        outputContents.replace("%DESKTOP_ACTION_LIST%","Actions="+desktopActions);
//    }else
//        outputContents.replace("%DESKTOP_ACTION_LIST%",QString());

//    QTextStream outputDocument(&outputDesktopFile);
//    outputDocument << outputContents;
//    outputDesktopFile.setPermissions(QFile::ExeOwner|outputDesktopFile.permissions());
//    outputDesktopFile.close();

//    updateProgressText(tr("Desktop file was generated successfully."));

//    emit toggleCloseButton(true);
//    emit finishedProcessing();
//}
