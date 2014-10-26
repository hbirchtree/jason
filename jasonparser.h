#ifndef JASONPARSER_H
#define JASONPARSER_H

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

private slots:
//    void doPrerun();
//    void doPostrun();
    void receiveLogOutput(QString stdOut,QString stdErr);
    void forwardProgressTextUpdate(QString message);
    void forwardProgressValueUpdate(int value);
    void forwardErrorMessage(int status,QString message);

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
    void changeWindowDimensions(int,int);

    //Related to processes
    void detachedRunEnd();
    void emitOutput(QString,QString);

private:
    //General
    QHash<QString, QString> startOpts;

    //Fucking finally
    int executeProcess(QString shell, QStringList arguments, QString workDir, QProcessEnvironment procEnv, bool lazyExitStatus, bool detached, QString title);
    int executeInstance(const QHash<QString, QVariant> &shellData, QHash<QString,QVariant> const &execInstance, const QProcessEnvironment &procEnv);
    int executeQueue(QHash<QString,QVariant> const &runtimeValues, QString actionId);

};

#endif // JASONPARSER_H
