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

    //Activate options
    void subsystemActivate(QHash<QString,QVariant> subsystemElement,QVariant option,QStringList activeSystems);
    void environmentActivate(QHash<QString,QVariant> environmentHash,QStringList activeSystems);
    void addToRuntime(QString role,QVariant input);
    void insertPrerunPostrun(QHash<QString,QVariant> runtable,int mode); //int mode is used to differentiate between post- and prerun tables, where prerun is 0 and postrun is 1. It's ugly.

    //Fucking finally
    int runProcesses(QString launchId);
    void generateDesktopFile(QString desktopFile,QString jasonPath, QString inputDoc);
    int executeProcess(QStringList arguments, QString workDir, QProcessEnvironment procEnv, bool lazyExitStatus, bool detached);

};

#endif // JASONPARSER_H
