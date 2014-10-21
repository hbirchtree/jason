#ifndef JASONPARSER_H
#define JASONPARSER_H

#include "jasongraphical.h"

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
    void startParse();
    void detachedMainProcessClosed();

private slots:
//    void doPrerun();
//    void doPostrun();
    void receiveLogOutput(QString stdOut,QString stdErr);
    void forwardProgressTextUpdate(QString message);
    void forwardProgressValueUpdate(int value);

signals:
    //Related to the general look and workings
    void finishedProcessing();
    void failedProcessing();
    //Directly about the GUI
    void toggleCloseButton(bool);
    void updateProgressText(QString);
    void updateProgressTitle(QString);
    void updateProgressIcon(QString);
    void broadcastMessage(int,QString);
    void toggleProgressVisible(bool);
    void displayDetachedMessage(QString);
    void changeProgressWIcon(QString);
    void changeProgressBarRange(int,int); //0,0 will make it indefinite, something else will make it normal.
    void changeProgressBarValue(int);

    //Related to processes
    void detachedRunEnd();
    void emitOutput(QString,QString);

private:
    //General
    QHash<QString, QString> startOpts;
//    int jsonParse(QJsonDocument jDoc);
    //Sections of parsing process
//    int parseStage1(QJsonObject mainObject);
//    int parseStage2(QJsonObject mainObject);
////    void stage2ActiveOptionAdd(QJsonValue instanceValue,QString key);

    //JSON
//    QJsonDocument jsonOpenFile(QString filename);
//    QHash<QString,QVariant> jsonExamineArray(QJsonArray jArray);
//    QVariant jsonExamineValue(QJsonValue jValue);
//    QHash<QString,QVariant> jsonExamineObject(QJsonObject jObject);

    //Handlers
//    void subsystemHandle(QHash<QString,QVariant> subsystemElement);
//    void setEnvVar(QString key, QString value);
//    QProcessEnvironment procEnv;
//    void variableHandle(QString key, QString value);
//    void resolveVariables();
//    QString resolveVariable(QString variable);
    int parseUnderlyingObjects(QHash<QString, QHash<QString,QVariant> > underlyingObjects);
    void desktopFileBuild(QJsonObject desktopObject);
//    int systemHandle(QHash<QString,QVariant> systemElement);

    //Activate options
    int systemActivate(QHash<QString,QVariant> systemElement,QStringList activeSystems);
    void subsystemActivate(QHash<QString,QVariant> subsystemElement,QVariant option,QStringList activeSystems);
    void environmentActivate(QHash<QString,QVariant> environmentHash,QStringList activeSystems);
//    void variablesImport(QHash<QString,QVariant> variables);
    void addToRuntime(QString role,QVariant input);
    void insertPrerunPostrun(QHash<QString,QVariant> runtable,int mode); //int mode is used to differentiate between post- and prerun tables, where prerun is 0 and postrun is 1. It's ugly.

    //Fucking finally
    int runProcesses(QString launchId);
    void generateDesktopFile(QString desktopFile,QString jasonPath, QString inputDoc);
    int executeProcess(QStringList arguments, QString workDir, QProcessEnvironment procEnv, bool lazyExitStatus, bool detached);
    QProcess *executer;

    //Hashes/arrays/vectors
//    QHash<QString, QString> substitutes;
//    QHash<int, QHash<QString,QVariant> > subsystems;
//    QHash<QString,QVariant> activeOptions;
//    QHash<QString,QVariant> systemTable;
//    QHash<QString,QVariant> runtimeValues;
//    QStringList importedFiles;

};

#endif // JASONPARSER_H
