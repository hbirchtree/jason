#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QProcessEnvironment>
#include "jason-tools/jasoncore.h"
#include "jsonstaticfuncs.h"

class jsonparser : public QObject
{
    Q_OBJECT
public:
    explicit jsonparser(QObject *parent = 0);
    QJsonDocument jsonOpenFile(QString filename);
    int jsonParse(QJsonDocument jDoc);
    int jasonActivateSystems(QHash<QString,QVariant> const &jsonData, QHash<QString, QVariant> *runtimeValues);

    bool hasCompleted(){
        return b_hasCompleted;
    }

    QString getShell(){
        return shellString;
    }
    QString getShellArg(){
        return shellArgString;
    }
    RuntimeQueue* getRunQueue() const {
        return runQueue;
    }
    QHash<QString,QVariant> getWindowOpts(){
        return windowOpts;
    }

private:
    bool b_hasCompleted = false;
    //Constants
    QString startDir;

    void getMap(QVariantMap *depmap, QVariantMap *totalMap, ActiveOptionsContainer *activeOpts, JasonCore *jCore);

    RuntimeQueue* runQueue;
    QString shellString;
    QString shellArgString;

    QHash<QString,QVariant> windowOpts;

signals:
    void reportError(int errorLevel,QString message);
    void sendMessage(QString message);
    void sendProgressTextUpdate(QString message);
    void sendProgressBarUpdate(int value);

public slots:

};

#endif // JSONPARSER_H
