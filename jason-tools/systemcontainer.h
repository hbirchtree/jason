#ifndef SYSTEMCONTAINER_H
#define SYSTEMCONTAINER_H

#include <QObject>
#include "jsonstaticfuncs.h"
#include "jason-tools/environmentcontainer.h"

class SystemContainer : public QObject
{
    Q_OBJECT
public:
    explicit SystemContainer(QObject *parent = 0, VariableHandler* varHandler = 0, QHash<QString, QVariant> *systemElement = 0, ActiveOptionsContainer *activeOptions = 0);
    ~SystemContainer();

    bool isValid();

    int systemInherit(const QHash<QString, QVariant> &systemElement, QHash<QString, QVariant> *activeOptions, QStringList *activeSystems, QHash<QString, QVariant> *variables, QHash<QString, QVariant> *procEnv, QHash<QString, QVariant> *runtimeValues);
    int systemActivate(ActiveOptionsContainer *activeOptions, QStringList *activeSystems, VariableHandler *varHandler, EnvironmentContainer *envContainer);

    QStringList getInherits(){
        return inherits;
    }
    QStringList getConfigPrefix(){
        QStringList result = QStringList() << configPrefix;
        for(SystemContainer* sys : inherited)
            result.append(sys->getConfigPrefix());
        return result;
    }
    QString getIdentifier(){
        return identifier;
    }
    QString getLaunchPrefix(){
        return launchPrefix;
    }

    EnvironmentContainer* getSysEnv(){
        return sysEnv;
    }
    VariableHandler* getSysVar(){
        return sysVars;
    }
    QList<SystemContainer*> getInherited(){
        return inherited;
    }

    void expandInheritance(QList<QVariant> *systemsList, ActiveOptionsContainer *activeOptions);
signals:
    void sendProgressTextUpdate(QString);
public slots:

private:
    QList<SystemContainer*> inherited;
    EnvironmentContainer* sysEnv;
    VariableHandler* sysVars;
    QString configPrefix;
    QStringList inherits;
    QString identifier;
    QString launchPrefix;
    bool validSystem = false;
};

#endif // SYSTEMCONTAINER_H
