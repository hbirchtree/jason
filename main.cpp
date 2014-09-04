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
    jParse.jsonParse(jDoc);

//    jParse.substitutes[0][0] = "test";e
    qDebug()<< "bing";
    jParse.testEnvironment();

    return 0;
}

void JasonParser::testEnvironment(){
    qDebug() << substitutes;
//    qDebug() << subsystems;
//    qDebug() << activeOptions;
//    qDebug() << preparatoryExec;
//    QProcessEnvironment *procEnv = new QProcessEnvironment;
//    qDebug() << procEnv->toStringList();
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

int JasonParser::jsonParse(QJsonDocument jDoc){
    QJsonObject mainTree = jDoc.object();
    QHash<QString,QHash<QString,QVariant> > underlyingObjects;
    foreach(QString key, mainTree.keys()){
        qDebug() << "jsonParse stage 1:" << key;
        QJsonValue instanceValue = mainTree.value(key);
        //Automatically handled
        if(key=="systems")
            underlyingObjects.insert("systems",jsonExamineObject(instanceValue.toObject()));
        if(key=="variables")
            underlyingObjects.insert("variables",jsonExamineArray(instanceValue.toArray()));
        if(key=="imports")
            underlyingObjects.insert("imports",jsonExamineArray(instanceValue.toArray()));
        if(key=="subsystems")
            underlyingObjects.insert("subsystems",jsonExamineArray(instanceValue.toArray()));
        if(key.endsWith(".prerun"))
            underlyingObjects.insert("prerun",jsonExamineObject(instanceValue.toObject()));
        if(key.endsWith(".postrun"))
            underlyingObjects.insert("postrun",jsonExamineObject(instanceValue.toObject()));
        if(key.startsWith("subsystem."))
            activeOptions.insert(key,jsonExamineValue(instanceValue));
    }
    parseUnderlyingObjects(underlyingObjects);
    return 0;
}

QHash<QString,QVariant> JasonParser::jsonExamineArray(QJsonArray jArray){
    if (jArray.isEmpty())
        return QHash<QString,QVariant>();
    QHash<QString,QVariant> returnTable;
    for(int i = 0;i<jArray.count();i++){
        QJsonValue instance = jArray.at(i);
        if (instance.isArray()){
            qDebug() << "array";
            qDebug() << instance.toArray();
            qDebug() << jsonExamineArray(instance.toArray());
            returnTable.insert(instance.toString(),jsonExamineArray(instance.toArray()));
        } if (instance.isObject()) {
            qDebug() << "object";
            QHash<QString,QVariant> objectTable = jsonExamineObject(instance.toObject());
            foreach(QString key,objectTable.keys())
                returnTable.insert(key,jsonExamineObject(instance.toObject()).value(key).toString());
        } else {
            qDebug() << "value";
            qDebug() << instance;
            qDebug() << jsonExamineValue(instance);
            returnTable.insert(instance.toString(),jsonExamineValue(instance));
        }
    }
    return returnTable;
}

QVariant JasonParser::jsonExamineValue(QJsonValue jValue){
    if (jValue.isNull())
        return QVariant();
    QVariant returnValue;

//    if (parentKey == "launchtype")
//        activeOptions.insert("launchtype",jValue.toString());
//    if (parentKey == "wine.workdir")
//        activeOptions.insert("wine.workdir",jValue.toString());
//    if (parentKey.startsWith("subsystem.")){
//        if(jValue.isBool())
//            activeOptions.insert(parentKey,jValue.toBool());
//        if(jValue.isString())
//            activeOptions.insert(parentKey,jValue.toString());
//        if(jValue.isDouble())
//            activeOptions.insert(parentKey,jValue.toDouble());
//    }
    if(jValue.isString()){
        returnValue = jValue.toString();
    }if(jValue.isDouble()){
        returnValue = jValue.toDouble();
    }if(jValue.isBool()){
        returnValue = jValue.toBool();
    }
    return returnValue;
}


QHash<QString,QVariant> JasonParser::jsonExamineObject(QJsonObject jObject){
    if (jObject.isEmpty())
        return QHash<QString,QVariant>();
    QHash<QString,QVariant> returnTable;
    foreach(QString key, jObject.keys()){
        if(jObject.value(key).isBool())
            returnTable.insert(key,jObject.value(key).toBool());
        if(jObject.value(key).isString())
            returnTable.insert(key,jObject.value(key).toString());
        if(jObject.value(key).isDouble())
            returnTable.insert(key,jObject.value(key).toDouble());
    }

    return returnTable;
}

void JasonParser::setEnvVar(QString key, QString value) {
    QProcessEnvironment *procEnv = new QProcessEnvironment;
    procEnv->insert(key,value);
    return;
}

void JasonParser::subsystemHandle(QHash<QString,QVariant> subsystemElement){
//    qDebug() << "Examining subsystem";
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
            jsonExamineArray(subsystemElement.value(key));
        if(key=="environment")
            subEnv = jsonExamineArray(subsystemElement.value(key).toArray());
        if(key=="action")
            subAction = subsystemElement.value(key).toString();
        if(key=="variable")
            subVar = subsystemElement.value(key).toString();
    }
    qDebug() << subEnabler << subEnv;
    if(subType!="constant"){
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
        qDebug() << "Constant found";
        foreach(QString key,subEnv.keys())
            if(!subEnv.value(key).isNull())
                setEnvVar(key,subEnv.value(key).toString());
    }

    return;
}

void JasonParser::variableHandle(QString key, QString value){
    substitutes.insert(key,value);
    qDebug() << key << "=" << value;
}

void JasonParser::parseUnderlyingObjects(QHash<QString, QHash<QString, QVariant> > underlyingObjects){
    QHash<QString, QVariant> importTable;
    QHash<QString, QVariant> variablesTable;
    QHash<QString, QVariant> subsystemTable;
    QHash<QString, QVariant> prerunTable;
    QHash<QString, QVariant> postrunTable;
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
            systemTable = underlyingObjects.value(key);
    }
    qDebug() << subsystemTable;
    foreach(QString key,importTable.keys())
        if(key=="file")
            jsonParse(jsonOpenFile(importTable.value(key).toString()));
    foreach(QString key,variablesTable.keys())
        if((key=="name")&&(variablesTable["value"].isValid()))
            variableHandle(variablesTable.value(key).toString(),variablesTable.value("value").toString());
    foreach(QString key,subsystemTable.keys())
        if(key=="type")


}
