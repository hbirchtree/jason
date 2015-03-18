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

class jsonparser : public QObject
{
    Q_OBJECT
public:
    explicit jsonparser(QObject *parent = 0);
    QJsonDocument jsonOpenFile(QString filename);
    int jsonParse(QJsonDocument jDoc,QHash<QString,QVariant> *targetHash);
    int jasonActivateSystems(QHash<QString,QVariant> const &jsonData, QHash<QString, QVariant> *runtimeValues);

private:
    //Constants
    QString startDir;

    //Stages
    int parseStage1(QJsonObject mainObject, QHash<QString,QVariant> *systemTable, QHash<QString, QVariant> *substitutes, QList<QVariant> *subsystems, QStringList *importedFiles, QHash<QString, QVariant> *activeOptions, QHash<QString, QVariant> *procEnv);
    void parseStage2(QJsonObject mainObject, QHash<QString,QVariant> *activeOptions);
    int stage2ActiveOptionAdd(QHash<QString,QVariant> *activeOptions,QJsonValue instance,QString key);

    //JSON
    QHash<QString,QVariant> jsonExamineObject(QJsonObject jObject);
    QList<QVariant> jsonExamineArray(QJsonArray jArray);

    //Handlers
    int subsystemHandle(QHash<QString,QVariant> *subsystemElement);
    int systemHandle(QHash<QString, QVariant> *systemElement);

    //Variables
    void resolveVariables(QHash<QString,QVariant> *variables);
    QString resolveVariable(const QHash<QString, QVariant> &variables, QString variable);
    void variableHandle(QHash<QString,QVariant> *variables, QString key, QString value);
    void variablesImport(QList<QVariant> inputVariables, QHash<QString,QVariant> *variables, QHash<QString,QVariant> const &activeOptions);

    //Environment variables
    void setEnvVar(QHash<QString,QVariant> *procEnv, QString key, QString value);

    //Activators
    int systemActivate(const QHash<QString, QVariant> &systemElement, QHash<QString, QVariant> *activeOptions, QStringList *activeSystems, QHash<QString, QVariant> *variables, QHash<QString, QVariant> *procEnv, QHash<QString, QVariant> *runtimeValues);
    int systemInherit(const QHash<QString, QVariant> &systemElement, QHash<QString,QVariant> *activeOptions, QStringList *activeSystems, QHash<QString,QVariant> *variables, QHash<QString,QVariant> *procEnv, QHash<QString,QVariant> *runtimeValues);
    void environmentActivate(QHash<QString,QVariant> const &environmentHash, QHash<QString, QVariant> *procEnv, QHash<QString, QVariant> *runtimeValues,QHash<QString,QVariant> const &variables);
    int subsystemActivate(QHash<QString,QVariant> *subsystemElement, QHash<QString,QVariant> *procEnv, QHash<QString,QVariant> *variables, QHash<QString,QVariant> *runtimeValues, QHash<QString,QVariant> *activeOptions);
    //For subsystems
    void activateVariablesAndEnvironments(QHash<QString,QVariant> const &inputHash, QHash<QString,QVariant> *variables, const QHash<QString, QVariant> &activeOptions, QHash<QString,QVariant> *procEnv, QHash<QString,QVariant> *runtimeValues);
    int addExecution(QHash<QString,QVariant> const &sourceElement, QHash<QString,QVariant> *targetElement);
    void processExecutionElement(QHash<QString,QVariant> *execElement, QHash<QString,QVariant> const &variables, const QHash<QString, QString> &prefixTable);

signals:
    void reportError(int errorLevel,QString message);
    void sendMessage(QString message);
    void sendProgressTextUpdate(QString message);
    void sendProgressBarUpdate(int value);

public slots:

};

#endif // JSONPARSER_H
