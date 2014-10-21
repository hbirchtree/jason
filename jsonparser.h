#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QProcessEnvironment>
#include <QVariant>

#include <QDebug>

class jsonparser : public QObject
{
    Q_OBJECT
public:
    explicit jsonparser(QObject *parent = 0);
    QJsonDocument jsonOpenFile(QString filename);
    int jsonParse(QJsonDocument jDoc,QHash<QString,QVariant> *targetHash);

private:
    int parseStage1(QJsonObject mainObject, QHash<QString,QVariant> *systemTable, QHash<QString, QVariant> *substitutes, QList<QVariant> *subsystems, QStringList *importedFiles, QHash<QString, QVariant> *activeOptions, QHash<QString, QVariant> *procEnv);
    void parseStage2(QJsonObject mainObject, const QHash<QString, QVariant> &systemTable, QHash<QString,QVariant> *activeOptions);
    int stage2ActiveOptionAdd(QHash<QString,QVariant> *activeOptions,QJsonValue instance,QString key);

    QHash<QString,QVariant> jsonExamineObject(QJsonObject jObject);
    QList<QVariant> jsonExamineArray(QJsonArray jArray);

    int subsystemHandle(QHash<QString,QVariant> *subsystemElement);
    int systemHandle(QHash<QString, QVariant> *systemElement);

    //Variables
    void resolveVariables(QHash<QString,QVariant> *substitutes);
    QString resolveVariable(QHash<QString,QVariant> *substitutes, QString variable);
    void variableHandle(QHash<QString,QVariant> *variables, QString key, QString value);
    void variablesImport(QList<QVariant> inputVariables, QHash<QString,QVariant> *substitutes, QHash<QString,QVariant> const &activeOptions);

    //Environment variables
    void setEnvVar(QHash<QString,QVariant> *procEnv, QString key, QString value);

signals:
    void reportError(int errorLevel,QString message);
    void sendMessage(QString message);
    void sendProgressTextUpdate(QString message);
    void sendProgressBarUpdate(int value);

public slots:

};

#endif // JSONPARSER_H
