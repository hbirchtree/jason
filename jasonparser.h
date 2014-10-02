#ifndef JASONPARSER_H
#define JASONPARSER_H

#include "jasongraphical.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <stdio.h>
#include <QString>
#include <QStringList>

#include <QProcess>
#include <QProcessEnvironment>
#include <QEventLoop>

class JasonParser : public QObject
{
    Q_OBJECT
public:
    JasonParser();
    ~JasonParser();

    //General
    void testEnvironment();
    void setStartOpts(QString startDocument, QString actionId, QString desktopFile, QString jasonPath);

    int exitResult;

public slots:
    void processStarted();
    void startParse();
    void detachedMainProcessClosed();

private slots:
    void processOutputError(QProcess::ProcessError processError);
    void processFinished(int exitCode,QProcess::ExitStatus exitStatus);
    void doPrerun();
    void doPostrun();

signals:

    //Related to the general look and workings
    void finishedProcessing();
    void failedProcessing();
    //Directly about the GUI
    void toggleCloseButton(bool);
    void updateProgressText(QString);
    void updateProgressTitle(QString);
    void broadcastMessage(int,QString);
    void toggleProgressVisible(bool);
    void displayDetachedMessage(QString);
    void changeProgressWIcon(QString);
    void changeProgressBarRange(int,int); //0,0 will make it indefinite, something else will make it normal.
    void changeProgressBarValue(int);

    //Related to processes
    void mainProcessStart();
    void mainProcessEnd();
    void processFailed(QProcess::ProcessError);
    void processReturnOutput();
    void emitOutput(QString,QString);

private:
    //General
    QHash<QString, QString> startOpts;
    int jsonParse(QJsonDocument jDoc);
    //Sections of parsing process
    int parseStage1(QJsonObject mainObject);
    int parseStage2(QJsonObject mainObject);
    void stage2ActiveOptionAdd(QJsonValue instanceValue,QString key);

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
    int parseUnderlyingObjects(QHash<QString, QHash<QString,QVariant> > underlyingObjects);
    void desktopFileBuild(QJsonObject desktopObject);
    void parseImportArray(QJsonArray imports);
    int systemHandle(QHash<QString,QVariant> systemElement);

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
    void executeProcess(QString argument,QString program,QString workDir, QString title, QString runprefix, QString runsuffix);
    void generateDesktopFile(QString desktopFile,QString jasonPath, QString inputDoc);
    QProcess *executer;

    //Hashes/arrays/vectors
    QHash<QString, QString> substitutes;
    QHash<int, QHash<QString,QVariant> > subsystems;
    QHash<QString,QVariant> activeOptions;
    QHash<QString,QVariant> systemTable;
    QHash<QString,QVariant> runtimeValues;
    QStringList importedFiles;

};

#endif // JASONPARSER_H
