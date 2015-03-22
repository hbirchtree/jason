#ifndef ENVIRONMENTCONTAINER_H
#define ENVIRONMENTCONTAINER_H

#include <QObject>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#include "modules/variablehandler.h"

class EnvironmentContainer : public QObject
{
    Q_OBJECT
public:
    explicit EnvironmentContainer(QObject *parent = 0, VariableHandler *varHandler = 0);
    ~EnvironmentContainer();

    void setEnvVar(QString key, QString value);
    void environmentImport(const QHash<QString, QVariant> &environmentHash);

    QString getRunSuffix();
    QString getRunPrefix();
    QStringList getRunPrefixList(){
        return runPrefixes;
    }
    QStringList getRunSuffixList(){
        return runSuffixes;
    }

    QProcessEnvironment getProcEnv();
    void merge(EnvironmentContainer *otherEnvContainer);
    void merge(EnvironmentContainer *otherEnvContainer,bool overwrite);

    void resolveVariables(){
        for(QString key : procEnv->keys())
            procEnv->insert(key,varHandler->resolveVariable(procEnv->value(key)));
    }

    void importSystem(){
        QProcessEnvironment sysenv = QProcessEnvironment::systemEnvironment();
        for(QString key : sysenv.keys())
            if(!procEnv->contains(key))
                procEnv->insert(key,sysenv.value(key));
    }

signals:

public slots:

private:
    VariableHandler* varHandler;
    QProcessEnvironment *procEnv;
    QStringList runPrefixes;
    QStringList runSuffixes;
};

#endif // ENVIRONMENTCONTAINER_H
