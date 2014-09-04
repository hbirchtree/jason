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
    QHash<QString,QVariant> jsonExamineArray(QJsonArray jArray);
    QVariant jsonExamineValue(QJsonValue jValue);
    QHash<QString,QVariant> jsonExamineObject(QJsonObject jObject);

    //Handlers
    void subsystemHandle(QHash<QString,QVariant> subsystemElement);
    void setEnvVar(QString key, QString value); QProcessEnvironment *procEnv();
    void variableHandle(QString key, QString value);
    void parseUnderlyingObjects(QHash<QString, QHash<QString,QVariant> > underlyingObjects);

    //Associative arrays
    QHash<QString, QString> substitutes;
    QHash<int, QHash<QString,QVariant> > subsystems;
    QHash<QString,QVariant> activeOptions;
    QHash<QString,QVariant> systemTable;
    QHash<int,QHash<QString,QString> > preparatoryExec;

private:
};

#endif // JASONPARSER_H
