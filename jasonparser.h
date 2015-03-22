#ifndef JASONPARSER_H
#define JASONPARSER_H

#include "executer.h"
#include "jsonparser.h"
#include "desktoptools.h"

#include <QMetaObject>
#include <QFileInfo>
#include <stdio.h>
#include <QString>
#include <QStringList>
#include <QHash>

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
    void changeWindowDimensions(int,int);

    //Related to processes
    void detachedRunEnd();
    void emitOutput(QString,QString);

private:
    QList<QMetaObject::Connection> connectedSlots;
    //General
    QHash<QString, QString> startOpts;

    jsonparser *parser;
    QHash<QString,QVariant> *jsonFinalData;
    QHash<QString,QVariant> *runtimeValues;
    QEventLoop *waitLoop;

    void quitProcess();
};

#endif // JASONPARSER_H
