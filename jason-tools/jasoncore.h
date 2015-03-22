#ifndef JASONCORE_H
#define JASONCORE_H

#include <QObject>
#include "jsonstaticfuncs.h"
#include "jason-tools/environmentcontainer.h"
#include "jason-tools/subsystemcontainer.h"
#include "jason-tools/systemcontainer.h"
#include "jason-tools/runtimequeue.h"

class JasonCore : public QObject
{
    Q_OBJECT
public:
    explicit JasonCore(QObject *parent = 0, VariableHandler *varHandler = 0,EnvironmentContainer* envContainer = 0);
    ~JasonCore();

    int jasonActivateSystems(const QHash<QString, QVariant> &jsonData, QHash<QString, QVariant> *runtimeValues);

    QStringList getCoreElements(){ //These are not included in active options
        return QStringList() << "systems" << "subsystems" << "variables" << "imports";
    }

    RuntimeQueue *resolveDependencies(QVariantMap* totalMap,ActiveOptionsContainer* activeOpts);

    ActiveOptionsContainer* getActiveOptions() const {
        return activeOptions;
    }
    QList<SystemContainer*> getSystems() const {
        return QList<SystemContainer*>() << m_systems.first() << m_systems.first()->getInherited();
    }

signals:
    void reportError(int,QString);
    void sendProgressTextUpdate(QString);
public slots:

private:
    QStringList activeSystems;
    QList<SubsystemContainer*> m_subsystems;
    QList<SystemContainer*> m_systems;
    VariableHandler *varHandler;
    EnvironmentContainer* envContainer;
    RuntimeQueue* runQueue;
    ActiveOptionsContainer* activeOptions;

    void activateVariablesAndEnvironments(const QHash<QString, QVariant> &inputHash, ActiveOptionsContainer *activeOptions);

    int getSubsystems(QVariantMap* totalMap);
    int getExecSystem(QVariantMap* totalMap);
    SystemContainer *getSystem(QString identifier);
};

#endif // JASONCORE_H
