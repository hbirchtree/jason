//#include "jason.h"
#include "jasonparser.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JasonParser jParse;


    //Interface? Ain't nobody got time fo' dat.
    //    Jason w;
    //    w.show();

    QFile jDocFile;
    if (a.arguments().length() > 1) {
        jDocFile.setFileName(a.arguments()[1]);
    } else {
        qDebug() << "jDocFile::No filename supplied";
        return 1;
    }
    if (!jDocFile.exists()) {
        qDebug() << "jDocFile::File not found";
        return 1;
    }
    if (!jDocFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qDebug() << "jDocFile::Failed to open";
        return 1;
    }
    QJsonParseError initError;
    QJsonDocument jDoc = QJsonDocument::fromJson(jDocFile.readAll(),&initError);
    if (initError.error != 0)
        qDebug() << "jDoc:" << initError.errorString();
    if (jDoc.isNull() || jDoc.isEmpty()) {
        qDebug() << "jDoc::IsNull or IsEmpty";
        return 1;
    }

    jParse.jsonParse(jDoc);
    //qDebug() << procEnv()->keys();

    qDebug() << "main::No errors encountered";
    return 0;
}

int JasonParser::jsonParse(QJsonDocument jDoc){
    QJsonObject mainTree = jDoc.object();
    foreach(QString key, mainTree.keys()) {
//        qDebug() << key;
        QJsonValue instanceValue = mainTree.value(key);
        if (instanceValue.isArray()){
            jsonExamineArray(instanceValue.toArray(),key);
        } if (instanceValue.isObject()) {
            jsonExamineObject(instanceValue.toObject(),key);
        } else {
            jsonExamineValue(instanceValue,key);
        }
    }
    return 0;
}

void JasonParser::jsonExamineArray(QJsonArray jArray,QString parentKey){
    if (jArray.isEmpty())
        return;
    qDebug() << "Examining array from" << parentKey;
    for(int i = 0;i<jArray.count();i++){
        QJsonValue instance = jArray.at(i);
//        returnObject.insert(instance.toString(),instance.toObject().value(instance.toString()));
//        qDebug()<< instance.toObject().toVariantMap();
        if (instance.isArray()){
            qDebug() << "Forwarding array";
            jsonExamineArray(instance.toArray(),parentKey);
        } if (instance.isObject()) {
            jsonExamineObject(instance.toObject(),parentKey);
        } else {
            qDebug() << "Forwarding value";
            jsonExamineValue(instance,parentKey);
        }
    }
    return;
}

void JasonParser::jsonExamineValue(QJsonValue jValue,QString parentKey){
    if (jValue.isNull())
        return;
    qDebug() << "Examining value from"<<parentKey;

    if (parentKey == "launchtype")
        qDebug() << "Program is of type" << jValue.toString();
    if (parentKey == "startdir")
        qDebug() << "Will start in directory" <<jValue.toString();
    if (parentKey == "glxosd")
        qDebug() << "State of GLXOSD:" << jValue.toBool();
    if (parentKey=="nvidia-optimizations")
        foreach(QString option,jValue.toString().split(","))
            qDebug() << "Enabling Nvidia option:" << option;
    return;
}


void JasonParser::jsonExamineObject(QJsonObject jObject, QString parentKey){
    if (jObject.isEmpty())
        return;
    qDebug() << "Examining object from"<<parentKey;
    foreach(QString key, jObject.keys()){
        if (parentKey == "wine-prerun"){
            if (key == "title")
                qDebug() << key << jObject.value(key).toString();
            if (key == "priority")
                qDebug() << key << jObject.value(key).toDouble();
            if (key == "wine-exec")
                qDebug() << key << jObject.value(key).toString();
        }
        if (parentKey == "wine-postrun"){
            if (key == "title")
                qDebug() << key << jObject.value(key).toString();
            if (key == "priority")
                qDebug() << key << jObject.value(key).toDouble();
            if (key == "wine-exec")
                qDebug() << key << jObject.value(key).toString();
        }
        if (parentKey == "variables"){
            if (key == "value")
                qDebug() << key << jObject.value(key).toString();
            if (key == "name")
                qDebug() << key << jObject.value(key).toString();
        }
        if (parentKey == "subsystems"){
            subsystemHandle(jObject);
        }
    }

    return;
}

void JasonParser::setEnvVar(QString key, QString value) {
    QProcessEnvironment *procEnv = new QProcessEnvironment;
    procEnv->insert(key,value);
    return;
}

void JasonParser::subsystemHandle(QJsonObject subsystemObject){
    qDebug() << "Examining subsystem";
    QString subType;
    foreach(QString key, subsystemObject.keys()){
        if (key=="type")
            subType = subsystemObject.value(key).toString();
        if (key=="enabler")
            qDebug() << key << subsystemObject.value(key).toString();
    }
    qDebug()<<subType;

    return;
}
