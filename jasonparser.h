#ifndef JASONPARSER_H
#define JASONPARSER_H

#include <QPointer>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFile>
#include <QDebug>
#include <stdio.h>
#include <QString>
#include <QStringList>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QProcess>
#include <QProcessEnvironment>

class JasonParser : public QObject
{
public:
    int jsonParse(QJsonDocument jDoc);

    //General
    void testEnvironment();

    //JSON
    QJsonDocument jsonOpenFile(QString filename);
    void jsonExamineArray(QJsonArray jArray,QString parentKey);
    void jsonExamineValue(QJsonValue jValue,QString parentKey);
    void jsonExamineObject(QJsonObject jObject,QString parentKey);

    //Option-handlers
    void subsystemHandle(QJsonObject subsystemObject);
    void setEnvVar(QString key, QString value);

    void variableHandle(QString key, QString value);
    QStringList *substituteNames;
    QStringList *substituteValues;

private:
    QProcessEnvironment *procEnv();
};

#endif // JASONPARSER_H
