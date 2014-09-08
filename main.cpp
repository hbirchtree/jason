//#include "jason.h"
#include "jasonparser.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("Jason");
    QCoreApplication::setApplicationVersion("0.1");

    JasonParser jParse;

    //Interface? Ain't nobody got time fo' dat.
    //    Jason w;
    //    w.show();

    //Parse the command line
    QCommandLineParser cParse;
    cParse.setApplicationDescription("Jason launcher");
    cParse.addHelpOption();
    cParse.addPositionalArgument("file",QCoreApplication::translate("init","File to open"));
    //Actual processing
    cParse.process(a);

    QStringList posArgs = cParse.positionalArguments();
    QString filename;
    if (posArgs.length()>=1){
        filename = posArgs[0];
    } else
        return 0;

    //Open document
    QJsonDocument jDoc;
    jDoc = jParse.jsonOpenFile(filename);
    jParse.jsonParse(jDoc,1);

    qDebug()<< "bing";
    jParse.testEnvironment();

    return 0;
}

void JasonParser::testEnvironment(){
//    qDebug() << substitutes;
//    qDebug() << subsystems;
//    qDebug() << activeOptions;
//    qDebug() << preparatoryExec;
//    qDebug() << procEnv.toStringList();
}

QJsonDocument JasonParser::jsonOpenFile(QString filename){
    QFile jDocFile;
    jDocFile.setFileName(filename);
    if (!jDocFile.exists()) {
        qDebug() << "jDocFile::File not found";
        return QJsonDocument();
    }

    if (!jDocFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qDebug() << "jDocFile::Failed to open";
        return QJsonDocument();
    }
    QJsonParseError initError;
    QJsonDocument jDoc = QJsonDocument::fromJson(jDocFile.readAll(),&initError);
    if (initError.error != 0)
        qDebug() << "jDoc:" << initError.errorString();
    if (jDoc.isNull() || jDoc.isEmpty()) {
        qDebug() << "jDoc::IsNull or IsEmpty";
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
*/
    QJsonObject mainTree = jDoc.object();
    QHash<QString,QHash<QString,QVariant> > underlyingObjects;
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
         *  - subsystem. is prepended to subsystem enablers as a means of identifying them.
         */
        if(key=="systems")
            underlyingObjects.insert("systems",jsonExamineArray(instanceValue.toArray()));
        if(key=="variables"){
            underlyingObjects.insert("variables",jsonExamineArray(instanceValue.toArray()));
        }
        if(key=="imports")
            underlyingObjects.insert("imports",jsonExamineArray(instanceValue.toArray()));
        if(key=="subsystems")
            underlyingObjects.insert("subsystems",jsonExamineArray(instanceValue.toArray()));
        if(key.endsWith(".prerun"))
            qDebug() << instanceValue.toArray();
//            underlyingObjects.insert("prerun",jsonExamineArray(instanceValue.toArray()));
        if(key.endsWith(".postrun"))
            qDebug() << instanceValue.toArray();
//            underlyingObjects.insert("postrun",jsonExamineArray(instanceValue.toArray()));
    }
    parseUnderlyingObjects(underlyingObjects);
    procEnv.insert(QProcessEnvironment::systemEnvironment());
    if(level==2)
        return 0;
    //Resolve variables, making them usable values
    resolveVariables();
/*
 * Stage 2 where options specified by the user or distributor are applied.
 * Parses through the options of the initial file only, will see if it is a good idea to parse
 * through the imported files as well.
*/
    foreach(QString key,mainTree.keys()){
        qDebug() << "jsonParse stage 2:" << key;
        QJsonValue instanceValue = mainTree.value(key);
        if(key.startsWith("subsystem."))
            activeOptions.insert(key,jsonExamineValue(instanceValue));
        if(key=="desktop.file")
            desktopFileBuild(instanceValue.toObject());
//        if(key=="imports"){
//            QJsonArray importArray = instanceValue.toArray();
//            for(int i = 0;i<importArray.count();i++){
//                jsonParse(jsonOpenFile(importArray.at(i).toObject().value("file").toString()),2);
//            }
//        }
        foreach(QString systemKey,systemTable.keys()){
            if((key.startsWith(systemKey))&&(!instanceValue.toString().isEmpty()))
                activeOptions.insert(key,instanceValue.toString());
        }
    }

    qDebug() << activeOptions;
/*
 * The active options have been listed and need to be parsed. With this it needs to look for
 * active subsystems and apply their options, prepare a list of programs to execute with QProcess
 * and prepare the system for launch.
*/

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
    QHash<QString,QVariant> subEnv;
    QHash<QString,QVariant> selections;
    QString subAction;
    QString subVar;
    //Identify and catch possible values for a subsystem
    foreach(QString key, subsystemElement.keys()){
        if(key=="type")
            subType = subsystemElement.value(key).toString();
        if(key=="enabler")
            subEnabler = subsystemElement.value(key).toString();
        if(key=="selections")
            foreach(QString key, subsystemElement.value(key).toHash().keys())
                qDebug() << key;
        if(key=="environment")
            for(int i=0;i<subsystemElement.value(key).toJsonArray().count();i++){
                QHash<QString,QVariant> envTable = jsonExamineObject(subsystemElement.value(key).toJsonArray().at(i).toObject());
                if((envTable.value("name").isValid()&&(envTable.value("value").isValid())))
                        subEnv.insert(envTable.value("name").toString(),envTable.value("value").toString());
            }
        if(key=="action")
            subAction = subsystemElement.value(key).toString();
        if(key=="variable")
            subVar = subsystemElement.value(key).toString();
    }
//    qDebug() << subEnabler << "=" << subType << subEnv;
    if(subType!="constant"){ //constants are always set, don't add it to the subsystems list
        QHash<QString, QVariant> insertHash;
        insertHash.insert("type",subType);
        if(!subEnabler.isEmpty())
            insertHash.insert("enabler",subEnabler);
        if(!subAction.isEmpty())
            insertHash.insert("action",subAction);
        if(!subEnv.isEmpty())
            insertHash.insert("environment",subEnv);
        int i = subsystems.count();
        subsystems.insert(i,insertHash);
    } else {
        foreach(QString key,subEnv.keys())
            if(!subEnv.value(key).isNull()){
                setEnvVar(key,subEnv.value(key).toString());
            }
    }

    return;
}

void JasonParser::variableHandle(QString key, QString value){
    //Insert variable in form "NAME","VAR"
    substitutes.insert(key,value);
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
        foreach(QString kKey, importTable.value(key).toHash().keys())
            jsonParse(jsonOpenFile(importTable.value(key).toHash().value(kKey).toString()),2);

    foreach(QString key,variablesTable.keys()){
        foreach(QString kKey, variablesTable.value(key).toHash().keys())
            if(kKey=="name")
                variableHandle(variablesTable.value(key).toHash().value("name").toString(),variablesTable.value(key).toHash().value("value").toString());
    }
    foreach(QString key,subsystemTable.keys())
        subsystemHandle(subsystemTable.value(key).toHash());
    foreach(QString key,systemTTable.keys())
        systemHandle(systemTTable.value(key).toHash());
    qDebug() << postrunTable;
    foreach(QString key,prerunTable.keys()){
        int priority;
        QString title;
        if(prerunTable.value("priority").isValid())
            priority = prerunTable.value("priority").toInt();
        if(prerunTable.value("desktop.title").isValid())
            title = prerunTable.value("desktop.title").toString();
        qDebug() << priority << title;
    }
}

void JasonParser::resolveVariables(){
    QStringList systemVariables;
    //System variables that are used in substitution for internal variables. Messy indeed, but it kind of works in a simple way.
    systemVariables<<"HOME"<<"PATH"<<"XDG_DATA_DIRS";
    foreach(QString variable, systemVariables){
        QProcessEnvironment variableValue = QProcessEnvironment::systemEnvironment();
        substitutes.insert(variable,variableValue.value(variable));
    }
    int indicator = 0;
    while(indicator!=1){
        foreach(QString key, substitutes.keys()){
            QString insert = substitutes.value(key);
            substitutes.remove(key);
//            foreach(QString sub, substitutes.keys()){
//                QString replace;
//                replace.append("%");replace.append(sub);replace.append("%");
//                qDebug() << key << substitutes.value(key) << replace << substitutes.value(sub);
//                substitutes.insert(key,insert.replace(replace,substitutes.value(sub)));
//                qDebug() << substitutes.value(key);
//            }
            substitutes.insert(key,resolveVariable(insert));
        }
        int indicatorLocal = 0;
        foreach(QString key,substitutes.keys()){
            if(!substitutes.value(key).contains("%"))
                indicatorLocal++;
        }
        if(indicatorLocal==(substitutes.count()))
            indicator = 1;
    }
    return;
}

QString JasonParser::resolveVariable(QString variable){
    //Takes the variable's contents as input and replaces %%-enclosed pieces of text with their variables.
    if(variable.isEmpty())
        return QString();
    foreach(QString sub, substitutes.keys()){
        QString replace = sub;
        replace.prepend("%");replace.append("%");
        variable = variable.replace(replace,substitutes.value(sub));
    }
    if(!variable.contains("%"))
        return variable;
    return QString();
}

void JasonParser::desktopFileBuild(QJsonObject desktopObject){
    foreach(QString key, desktopObject.keys()){
        if(key=="desktop.displayname")
            qDebug() << desktopObject.value(key).toString();
        if(key=="desktop.description")
            qDebug() << desktopObject.value(key).toString();
        if(key=="desktop.wmclass")
            qDebug() << desktopObject.value(key).toString();
        if(key=="desktop.icon")
            qDebug() << desktopObject.value(key).toString();
        if(key=="desktop.action"){
            QJsonObject action = desktopObject.value(key).toObject();
            foreach(QString actKey, action.keys())
                qDebug() << actKey << action.value(actKey).toString();
        }
    }
    return;
}

void JasonParser::systemHandle(QHash<QString, QVariant> systemElement){
    //Stage 1
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
    }
    QString configPrefix = systemHash.value("config-prefix").toString();
    foreach(QString key, systemElement.keys()){
        if(key=="variables"){
            QJsonArray vArray = systemElement.value(key).toJsonArray();
            for(int i=0;i<vArray.count();i++){
                qDebug() << "new variable";
                QJsonObject vObject = vArray.at(i).toObject();
                foreach(QString vKey, vObject.keys()){
                    qDebug() << vKey << jsonExamineValue(vObject.value(vKey)).toString();
                }
            }
        }
    }
    systemTable.insert(systemHash.value("config-prefix").toString(),systemHash);
}
