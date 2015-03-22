#include "subsystemcontainer.h"

SubsystemContainer::SubsystemContainer(QObject *parent, QHash<QString,QVariant> *subsystemElement, VariableHandler* varHandler, EnvironmentContainer* envContainer, ActiveOptionsContainer *activeOptions, QList<SystemContainer *> systems) : QObject(parent)
{
    QString typeString = subsystemElement->value("type").toString();
    if(typeString=="bool")
        type=ST_BOOL;
    else if(typeString=="option")
        type=ST_OPTION;
    else if(typeString=="substitution")
        type=ST_SUBSTITUTION;
    else if(typeString=="select")
        type=ST_SELECT;
    else
        return;
    if(subsystemElement->contains("trigger"))
        if(subsystemElement->value("trigger").toString()=="sys-postrun")
            trigger = STRIG_POSTRUN;
    validSubsystem = true;
    activator = subsystemElement->value("enabler").toString();
    this->varHandler = new VariableHandler();
    this->varHandler->merge(varHandler);
    this->envContainer = new EnvironmentContainer(this,this->varHandler);
    this->envContainer->merge(envContainer);
    runQueue = new RuntimeQueue();
    applyLocalConfiguration(subsystemElement,activeOptions,systems);
}

SubsystemContainer::~SubsystemContainer()
{
    delete runQueue;
}

void SubsystemContainer::applyLocalConfiguration(QHash<QString,QVariant> *subsystemElement, ActiveOptionsContainer* activeOpts,QList<SystemContainer*> systems){
    if(subsystemElement->contains("appearance")){
        d_title = subsystemElement->value("appearance").toMap().value("desktop.title").toString();
        d_icon = subsystemElement->value("appearance").toMap().value("desktop.icon").toString();
    }
    switch(type){
    case ST_BOOL:
        captureAllVariables(subsystemElement,activeOpts);
        for(QString key : subsystemElement->keys()){

            if(key.endsWith(".exec"))
                captureExecUnit(subsystemElement,activeOpts,systems);
        }
        break;
    case ST_SELECT:
        for(QString key : subsystemElement->keys()){
            if(key=="sets")
                captureListSet(subsystemElement,key);
            if(key.endsWith(".exec")){
                m_selectExec = subsystemElement->value(key).toString();
                m_selectExecType = key.split(".").first();
            }
        }
        break;
    case ST_OPTION:
        for(QString key : subsystemElement->keys()){
            if(key=="options")
                captureListSet(subsystemElement,key);
        }
        break;
    case ST_SUBSTITUTION:
        captureAllVariables(subsystemElement,activeOpts);
        for(QString key : subsystemElement->keys()){
            if(key=="variable")
                m_substituteVar = subsystemElement->value(key).toString();
            if(key.endsWith(".exec"))
                captureExecUnit(subsystemElement,activeOpts,systems);
        }
        break;
    }
}

void SubsystemContainer::captureAllVariables(QHash<QString,QVariant> *subsystemElement, ActiveOptionsContainer* activeOpts){
    for(QString key : subsystemElement->keys()){
        if(key=="environment")
            for(QVariant el : subsystemElement->value(key).toList())
                envContainer->environmentImport(StatFuncs::mapToHash(el.toMap()));
        if(key=="variables"){
            varHandler->variablesImport(subsystemElement->value(key).toList(),activeOpts);
            varHandler->resolveVariables();
        }
    }
}

void SubsystemContainer::captureExecUnit(QHash<QString,QVariant> *subsystemElement, ActiveOptionsContainer* activeOpts,QList<SystemContainer*> systems){
    ExecutionUnit* unit = new ExecutionUnit(this,varHandler,activeOpts,*subsystemElement,systems);
    if(unit->isValid())
        if(trigger==STRIG_POSTRUN){
            runQueue->addPostrun(unit);
        }else
            runQueue->addPrerun(unit);
    else
        delete unit;
}

void SubsystemContainer::captureListSet(QHash<QString,QVariant>* subsystemElement, QString key){
    for(QVariant el : subsystemElement->value(key).toList()){
        QMap<QString,QVariant> set = el.toMap();
        QString setKey = set.value("id").toString();
        set.remove("id");
        m_sets.insert(setKey,set);
    }
}

void SubsystemContainer::interpretOption(QVariant option,RuntimeQueue* runQueue,EnvironmentContainer* envContainer,VariableHandler* varHandler,QList<SystemContainer*> systems,ActiveOptionsContainer* activeOpts){
    switch(type){
    case ST_BOOL:
        if(option.toBool()){
            runQueue->merge(this->runQueue);
            envContainer->merge(this->envContainer);
            varHandler->merge(this->varHandler);
        }
        break;
    case ST_SELECT:{
        QMap<QString,QVariant> set = m_sets.value(option.toString()).toMap();
        for(QString key : set.keys()){
            ExecutionUnit* unit = new ExecutionUnit(this,m_selectExecType,m_selectExec.
                                                    replace(QString("%JASON_KEY%"),
                                                    key).replace(QString("%JASON_VALUE%"),
                                                    set.value(key).toString()),systems,this->varHandler);
            if(unit->isValid())
                if(this->getTrigger()==STRIG_POSTRUN){
                    this->runQueue->addPostrun(unit);
                }else
                    this->runQueue->addPrerun(unit);
            else
                delete unit;
        }
        runQueue->merge(this->runQueue);
        break;
    }
    case ST_SUBSTITUTION:
        runQueue->merge(this->runQueue);
        envContainer->merge(this->envContainer);
        varHandler->merge(this->varHandler);
        varHandler->variableHandle(m_substituteVar,option.toString());
        break;
    case ST_OPTION:{
        QMap<QString,QVariant> set = m_sets.value(option.toString()).toMap();
        captureAllVariables(new QHash<QString,QVariant>(StatFuncs::mapToHash(set)),activeOpts);
        envContainer->merge(this->envContainer);
        varHandler->merge(this->varHandler);
        break;
    }
    }
}
