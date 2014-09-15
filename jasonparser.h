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
    Q_OBJECT

public:
    int jsonParse(QJsonDocument jDoc,int level);

    //General
    void testEnvironment();

    //JSON
    QJsonDocument jsonOpenFile(QString filename);
    QHash<QString,QVariant> jsonExamineArray(QJsonArray jArray);
    QVariant jsonExamineValue(QJsonValue jValue);
    QHash<QString,QVariant> jsonExamineObject(QJsonObject jObject);

    //Handlers
    void subsystemHandle(QHash<QString,QVariant> subsystemElement);
    void setEnvVar(QString key, QString value);
    QProcessEnvironment procEnv;
    void variableHandle(QString key, QString value);
    void resolveVariables();
    QString resolveVariable(QString variable);
    void parseUnderlyingObjects(QHash<QString, QHash<QString,QVariant> > underlyingObjects);
    void desktopFileBuild(QJsonObject desktopObject);
    void parseImportArray(QJsonArray imports);
    void systemHandle(QHash<QString,QVariant> systemElement);

    //Activate options
    int systemActivate(QHash<QString,QVariant> systemElement,QStringList activeSystems);
    void subsystemActivate(QHash<QString,QVariant> subsystemElement,QVariant option,QStringList activeSystems);
    QHash<QString,QVariant> createExecutionQueue(QString launchType);
    void environmentActivate(QHash<QString,QVariant> environmentHash,QStringList activeSystems);
    void variablesImport(QHash<QString,QVariant> variables);
    void addToRuntime(QString role,QVariant input);
    void insertPrerunPostrun(QHash<QString,QVariant> runtable,int mode); //int mode is used to differentiate between post- and prerun tables, where prerun is 0 and postrun is 1. It's ugly.

    //Fucking finally
    int runProcesses(QString launchId);
    void executeProcess(QString argument,QString program,QString workDir);
    void generateDesktopFile(QString desktopFile);

    //Hashes/arrays/vectors
    QHash<QString, QString> substitutes;
    QHash<int, QHash<QString,QVariant> > subsystems;
    QHash<QString,QVariant> activeOptions;
    QHash<QString,QVariant> systemTable;
    QHash<QString,QVariant> runtimeValues;
    QStringList importedFiles;

private:

public slots:
    void processOutputError(QProcess::ProcessError processError);
    void processOutputProcess(int exitCode,QProcess::ExitStatus exitStatus);
    void processStarted();
};

#endif // JASONPARSER_H
