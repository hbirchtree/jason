#ifndef SUBSYSTEMCONTAINER_H
#define SUBSYSTEMCONTAINER_H

#include <QObject>
#include "jsonstaticfuncs.h"
#include "jason-tools/environmentcontainer.h"
#include "jason-tools/runtimequeue.h"
#include "jason-tools/systemcontainer.h"
#include "jason-tools/activeoptionscontainer.h"

class SubsystemContainer : public QObject
{
    Q_OBJECT
    enum SubsysType {
        ST_UNDEFINED,ST_BOOL,ST_SELECT,ST_OPTION,ST_SUBSTITUTION
    };
    enum SubsysTriggerType {
        STRIG_UNDEFINED,STRIG_PRERUN,STRIG_POSTRUN
    };

public:
    explicit SubsystemContainer(QObject *parent = 0, QHash<QString, QVariant> *subsystemElement = 0, VariableHandler *varHandler = 0, EnvironmentContainer *envContainer = 0, ActiveOptionsContainer* activeOptions = 0,QList<SystemContainer*> systems = QList<SystemContainer*>());
    ~SubsystemContainer();

    SubsysType getType(){
        return type;
    }
    SubsysTriggerType getTrigger(){
        return trigger;
    }
    QString getActivator(){
        return activator;
    }
    bool isValid(){
        return validSubsystem;
    }

    void interpretOption(QVariant option, RuntimeQueue *runQueue, EnvironmentContainer *envContainer, VariableHandler *varHandler, QList<SystemContainer *> systems, ActiveOptionsContainer *activeOpts);
signals:

public slots:

private:
    VariableHandler* varHandler;
    EnvironmentContainer* envContainer;
    RuntimeQueue* runQueue;
    bool validSubsystem = false;

    QString activator;
    SubsysType type = ST_UNDEFINED;
    SubsysTriggerType trigger = STRIG_PRERUN;

    QHash<QString,QVariant> m_sets;
    QString m_selectExec;
    QString m_selectExecType;
    QString m_substituteVar;

    QString d_title;
    QString d_icon;

    void applyLocalConfiguration(QHash<QString, QVariant> *subsystemElement, ActiveOptionsContainer *activeOpts, QList<SystemContainer *> systems);
    void captureListSet(QHash<QString,QVariant>* subsystemElement, QString key);
    void captureExecUnit(QHash<QString, QVariant> *subsystemElement, ActiveOptionsContainer *activeOpts, QList<SystemContainer *> systems);
    void captureAllVariables(QHash<QString, QVariant> *subsystemElement, ActiveOptionsContainer *activeOpts);
};

#endif // SUBSYSTEMCONTAINER_H
