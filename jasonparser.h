#ifndef JASONPARSER_H
#define JASONPARSER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFile>
#include <QDebug>
#include <stdio.h>
#include <QString>

#include <QProcess>
#include <QProcessEnvironment>

class JasonParser : public QObject
{
public:
    int jsonParse(QJsonDocument jDoc);

    //General
    void setEnvVar(QString key, QString value);

    //JSON
    void jsonExamineArray(QJsonArray jArray,QString parentKey);
    void jsonExamineValue(QJsonValue jValue,QString parentKey);
    void jsonExamineObject(QJsonObject jObject,QString parentKey);

    //Option-handlers
    void subsystemHandle(QJsonObject subsystemObject);
    void variableHandle(QJsonObject variableObject);

private:
    QProcessEnvironment *procEnv();
};

#endif // JASONPARSER_H
