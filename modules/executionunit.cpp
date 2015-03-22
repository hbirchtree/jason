#include "executionunit.h"

ExecutionUnit::ExecutionUnit(QObject *parent,VariableHandler* varHandler,ActiveOptionsContainer* activeOpts, QHash<QString,QVariant> inputHash,QList<SystemContainer*> systems) : QObject(parent)
{
    envContainer = new EnvironmentContainer(this,varHandler);
    for(QString key : inputHash.keys()){
        QVariant value = inputHash.value(key);
        if(key=="lazy-exit-status")
            m_lazyExit = value.toBool();
        if(key=="workdir")
            m_workDir = value.toString();
        if(key=="detachable-process")
            m_detachable = value.toBool();
        if(key=="start-detached")
            m_startsDetached = value.toBool();
        if(key=="private.process-environment")
            envContainer->environmentImport(StatFuncs::mapToHash(value.toMap()));
        if(key=="desktop.title")
            d_title = value.toString();
        if(key=="desktop.icon")
            d_icon = value.toString();
        if(key=="desktop.file"){
            d_title = value.toMap().value("desktop.displayname").toString();
            d_icon = value.toMap().value("desktop.icon").toString();
        }
        if(key.endsWith(".exec")){
            m_execType = key.split(".").first();
            m_execCommand = value.toString();
            for(SystemContainer* sys : systems)
                if(sys->getConfigPrefix().first()==m_execType)
                    m_execString = varHandler->resolveVariable(QString(m_execCommand).prepend(" ").prepend(sys->getLaunchPrefix()));
        }
    }
    if(!m_execString.isEmpty())
        s_validUnit = true;
}

ExecutionUnit::~ExecutionUnit()
{
    if(envContainer!=0)
        delete envContainer;
}

