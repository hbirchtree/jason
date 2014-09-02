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

    jParse.testEnvironment();

    return 0;
}

void JasonParser::testEnvironment(){
//    substituteNames = new QStringList;
//    substituteValues = new QStringList;
//    qDebug() << substituteValues;
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
    foreach(QString key, mainTree.keys()){
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
    for(int i = 0;i<jArray.count();i++){
        QJsonValue instance = jArray.at(i);
        if (instance.isArray()){
            jsonExamineArray(instance.toArray(),parentKey);
        } if (instance.isObject()) {
            jsonExamineObject(instance.toObject(),parentKey);
        } else {
            jsonExamineValue(instance,parentKey);
        }
    }
    return;
}

void JasonParser::jsonExamineValue(QJsonValue jValue,QString parentKey){
    if (jValue.isNull())
        return;
//    qDebug() << "Examining value from"<<parentKey;

    if (parentKey == "launchtype")
        qDebug() << "Program is of type" << jValue.toString();
    if (parentKey == "startdir")
        qDebug() << "Will start in directory" <<jValue.toString();
    if (parentKey == "glxosd")
        qDebug() << "State of GLXOSD:" << jValue.toBool();
    return;
}


void JasonParser::jsonExamineObject(QJsonObject jObject, QString parentKey){
    if (jObject.isEmpty())
        return;
//    qDebug() << "Examining object from"<<parentKey;
    foreach(QString key, jObject.keys()){
        if (parentKey == "wine-prerun"){
            if (key == "title")
                qDebug() << key << jObject.value(key).toString();
            if (key == "priority")
                qDebug() << key << jObject.value(key).toDouble();
            if (key == "wine.exec")
                qDebug() << key << jObject.value(key).toString();
            if (key=="sys.exec")
                qDebug() << key << jObject.value(key).toString();
        }
        if (parentKey == "wine-postrun"){
            if (key == "title")
                qDebug() << key << jObject.value(key).toString();
            if (key == "priority")
                qDebug() << key << jObject.value(key).toDouble();
            if (key == "wine.exec")
                qDebug() << key << jObject.value(key).toString();
            if (key=="sys.exec")
                qDebug() << key << jObject.value(key).toString();
        }
        if (parentKey == "variables"){
            variableHandle(jObject.value("name").toString(),jObject.value("value").toString());
        }
        if (parentKey == "subsystems"){
            subsystemHandle(jObject);
        }
        if (parentKey=="imports")
            if(key=="file")
                if(!jObject.value(key).toString().isEmpty()){
                    jsonParse(jsonOpenFile(jObject.value(key).toString()));
                    qDebug() << "Importing file:" << jObject.value(key).toString();
                }
        if (parentKey=="selections"){
            if(key=="listname")
                qDebug() << jObject.value(key).toString();
        }
        if(key.startsWith("subsystem.",Qt::CaseSensitive))
            qDebug()<< "Subsystem switch:" << key << jObject.value(key).toString();
    }

    return;
}

void JasonParser::setEnvVar(QString key, QString value) {
    QProcessEnvironment *procEnv = new QProcessEnvironment;
    procEnv->insert(key,value);
    return;
}

void JasonParser::subsystemHandle(QJsonObject subsystemObject){
//    qDebug() << "Examining subsystem";
    QString subType;
    QString enabler;
    foreach(QString key, subsystemObject.keys()){
        if(key=="type")
            subType = subsystemObject.value(key).toString();
        if(key=="enabler")
            enabler = subsystemObject.value(key).toString();
        if(key=="selections")
            jsonExamineArray(subsystemObject.value(key).toArray(),key);
        if(key=="environment")
            jsonExamineArray(subsystemObject.value(key).toArray(),key);
    }
    qDebug()<<"Found subsystem:"<<subType<<enabler;


    return;
}

void JasonParser::variableHandle(QString key, QString value){

    if(!substituteNames->length()==substituteValues->length()){
        qDebug() << "Oops! Mismatch between indexes of variable lists! Something is wrong!";
        return;
    }
    qDebug() << "Inserting variable";
    int i = substituteNames->count();
    substituteNames->insert(i,key);
    substituteValues->insert(i,value);
    qDebug() << *substituteNames << *substituteValues;
}
