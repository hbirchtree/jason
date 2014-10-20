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

class jsonparser : public QObject
{
    Q_OBJECT
public:
    explicit jsonparser(QObject *parent = 0);

private:
    QJsonDocument jsonOpenFile(QString filename);
    int parseStage1(QJsonObject mainObject,QHash<QString,QVariant> *systemTable,QHash<QString,QVariant> *substitutes);
    int parseStage2(QJsonObject mainObject,QHash<QString,QVariant> systemTable,QHash<QString,QVariant> *activeOptions);
    int stage2ActiveOptionAdd(QHash<QString,QVariant> *activeOptions,QJsonValue instance,QString key);

    QHash<QString,QVariant> jsonExamineObject(QJsonObject jObject);
    QHash<QString,QVariant> jsonExamineArray(QJsonArray jArray);

    QHash<QString,QVariant> subsystemHandle(QHash<QString,QVariant> subsystemElement);
    void variableHandle(QHash<QString,QVariant> *variables, QString key, QString value);
    void variablesImport(QHash<QString,QVariant> *variables);

signals:

public slots:

};

#endif // JSONPARSER_H
