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
    qDebug() << subsystems;
    qDebug() << activeOptions;
    qDebug() << preparatoryExec;
    QProcessEnvironment *procEnv = new QProcessEnvironment;
    qDebug() << procEnv->toStringList();
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
        qDebug() << "jsonParse:" << key;
        QJsonValue instanceValue = mainTree.value(key);
        //Automatically handled
        if(key=="variables")
            underlyingObjects.insert("variables",jsonExamineArray(instanceValue.toArray()));
        if(key=="imports")
            underlyingObjects.insert("imports",jsonExamineArray(instanceValue.toArray()));
        if(key=="subsystems")
            underlyingObjects.insert("subsystems",jsonExamineArray(instanceValue.toArray()));
        if(key=="wine.prerun")
            underlyingObjects.insert("wine.prerun",jsonExamineArray(instanceValue.toArray()));
        if(key=="wine.postrun")
            underlyingObjects.insert("wine.postrun",jsonExamineArray(instanceValue.toArray()));
        if((key.startsWith("subsystem.")))
            activeOptions.insert(key,jsonExamineValue(instanceValue));
        if((key.endsWith(".workdir")))
            activeOptions.insert(key,jsonExamineValue(instanceValue));
    }
    qDebug() << "beep";
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
            returnTable.insert(instance.toString(),jsonExamineArray(instance.toArray()));
        } if (instance.isObject()) {
            qDebug() << "object";
            returnTable.insert(instance.toString(),jsonExamineObject(instance.toObject()));
        } else {
            qDebug() << "value";
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
//        if (parentKey == "wine.prerun"){
//            if (key == "display.title")
//                qDebug() << key << jObject.value(key).toString();
//            if (key == "priority")
//                qDebug() << key << jObject.value(key).toDouble();
//            if (key == "wine.exec")
//                qDebug() << key << jObject.value(key).toString();
//            if (key=="sys.exec")
//                qDebug() << key << jObject.value(key).toString();
//        }
//        if (parentKey == "wine.postrun"){
//            if (key=="display.title")
//                qDebug() << key << jObject.value(key).toString();
//            if (key=="priority")
//                qDebug() << key << jObject.value(key).toDouble();
//            if (key=="wine.exec")
//                qDebug() << key << jObject.value(key).toString();
//            if (key=="sys.exec")
//                qDebug() << key << jObject.value(key).toString();
//        }
//        if (parentKey == "variables"){
//            variableHandle(jObject.value("name").toString(),jObject.value("value").toString());
//        }
//        if (parentKey == "subsystems"){
//            subsystemHandle(jObject);
//        }
//        if (parentKey=="imports")
//            if(key=="file")
//                if(!jObject.value(key).toString().isEmpty()){
//                    jsonParse(jsonOpenFile(jObject.value(key).toString()));
//                    qDebug() << "Importing file:" << jObject.value(key).toString();
//                }
//        if (parentKey=="selections"){
//            if(key=="listname")
//                qDebug() << jObject.value(key).toString();
//        }
//        if(key.startsWith("subsystem.",Qt::CaseSensitive))
//            qDebug()<< "Subsystem switch:" << key << jObject.value(key).toString();
    }

    return returnTable;
}

void JasonParser::setEnvVar(QString key, QString value) {
    QProcessEnvironment *procEnv = new QProcessEnvironment;
    procEnv->insert(key,value);
    return;
}

void JasonParser::subsystemHandle(QJsonObject subsystemObject){
//    qDebug() << "Examining subsystem";
    QString subType;
    QString subEnabler;
    QHash<QString,QVariant> subEnv;
    QString subAction;
    QString subVar;
    //Identify and catch possible values for a subsystem
    foreach(QString key, subsystemObject.keys()){
        if(key=="type")
            subType = subsystemObject.value(key).toString();
        if(key=="enabler")
            subEnabler = subsystemObject.value(key).toString();
        if(key=="selections")
            jsonExamineArray(subsystemObject.value(key).toArray());
        if(key=="environment")
            subEnv = jsonExamineArray(subsystemObject.value(key).toArray());
        if(key=="action")
            subAction = subsystemObject.value(key).toString();
        if(key=="variable")
            subVar = subsystemObject.value(key).toString();
    }
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

    return;
}

void JasonParser::variableHandle(QString key, QString value){
    substitutes.insert(key,value);
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
        if(key=="wine.prerun")
            prerunTable = underlyingObjects.value(key);
        if(key=="wine.postrun")
            postrunTable = underlyingObjects.value(key);
    }
    qDebug() << importTable;
}
