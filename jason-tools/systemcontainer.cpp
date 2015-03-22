#include "systemcontainer.h"

SystemContainer::SystemContainer(QObject *parent,VariableHandler* varHandler,QHash<QString, QVariant> *systemElement,ActiveOptionsContainer* activeOptions) : QObject(parent)
{
    sysVars = new VariableHandler();
    sysVars->merge(varHandler);
    sysEnv = new EnvironmentContainer(this,sysVars);
    //We verify that it contains the keys we need
    if(!systemElement->value("config-prefix").isValid())
        return;
    if(!systemElement->value("identifier").isValid())
        return;

    if(systemElement->value("variables").isValid())
        sysVars->variablesImport(systemElement->value("variables").toList(),activeOptions);
    varHandler->resolveVariables();
    foreach(QString key,systemElement->keys()){
        QVariant object = systemElement->value(key);
        if(key=="identifier")
            identifier=object.toString();
        if(key=="config-prefix")
            configPrefix=object.toString();
        if(key=="inherits")
            inherits.append(object.toString().split(","));
        if(key=="launch-prefix")
            launchPrefix=sysVars->resolveVariable(object.toString());
        if(key=="environment"){
            for(auto envMap : object.toList()){
                QHash<QString,QVariant> envHash = StatFuncs::mapToHash(envMap.toMap());
                sysEnv->environmentImport(envHash);
            }
        }
    }
    validSystem = true;
}

SystemContainer::~SystemContainer()
{
}

void SystemContainer::expandInheritance(QList<QVariant> *systemsList,ActiveOptionsContainer* activeOptions){
    for(QVariant systemSpec : *systemsList)
        for(QString sys : inherits){
            SystemContainer* s = new SystemContainer(this,sysVars,new auto(StatFuncs::mapToHash(systemSpec.toMap())),activeOptions);
            if(s->getIdentifier()==sys)
                inherited.append(s);
        }
    for(SystemContainer* sys : inherited){
        sysVars->merge(sys->getSysVar());
        sysEnv->merge(sys->getSysEnv());
    }
}

int SystemContainer::systemActivate(ActiveOptionsContainer* activeOptions,QStringList *activeSystems,VariableHandler *varHandler,EnvironmentContainer *envContainer){
    activeSystems = new QStringList(getInherits());
    *activeSystems << getIdentifier();
    varHandler->merge(this->sysVars);
    envContainer->merge(this->sysEnv,true);
    return 0;
}
