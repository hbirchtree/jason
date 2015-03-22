#ifndef VARIABLEHANDLER_H
#define VARIABLEHANDLER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QVariant>
#include <QRegExp>
#include <QDebug>
#include "jason-tools/activeoptionscontainer.h"

class VariableHandler : public QObject
{
    Q_OBJECT
public:
    explicit VariableHandler(QObject *parent = 0);
    ~VariableHandler();

    QString resolveVariable(QString variable);
    void variableHandle(QString key, QString value);

    void variablesImport(QList<QVariant> inputVariables, ActiveOptionsContainer *activeOptions);
    void resolveVariables();

    void merge(VariableHandler *handler);
    void merge(QHash<QString, QVariant> *variableHash);

    QList<QString> getVariableList();
    QString getVariable(QString key);
signals:
    void sendProgressTextUpdate(QString);
    void reportError(int,QString);
public slots:

private:
    QHash<QString, QVariant> *variables;
    int solveForVariable(QString varname);

    int i_recursionlevel = 0;
};

#endif // VARIABLEHANDLER_H
