#include "jasoncore.h"

JasonCore::JasonCore(QObject *parent, VariableHandler *varHandler, EnvironmentContainer *envContainer) : QObject(parent)
{
    this->varHandler = varHandler;
    this->envContainer = envContainer;
    runQueue = new RuntimeQueue();
}

JasonCore::~JasonCore()
{

}

RuntimeQueue* JasonCore::resolveDependencies(QVariantMap *totalMap,ActiveOptionsContainer* activeOpts){
    for(auto it=totalMap->begin();it!=totalMap->end();it++){
        varHandler->variablesImport(it.value().toList(),activeOpts);
    }

    activeOptions = activeOpts;

    getExecSystem(totalMap);
    getSubsystems(totalMap);

    for(SubsystemContainer* subsys : m_subsystems)
        subsys->interpretOption(activeOptions->getOption("subsystem."+subsys->getActivator()).first(),runQueue,envContainer,varHandler,m_systems.first()->getInherited() << m_systems.first(),activeOptions);
    m_systems.first()->systemActivate(activeOptions,&this->activeSystems,varHandler,envContainer);
    varHandler->resolveVariables();

    QStringList activePrefixes = m_systems.first()->getConfigPrefix();
    auto optMap = activeOptions->getHash();
    for(auto it=optMap.begin();it!=optMap.end();it++){
        if(it.key().endsWith(".prerun")&&activePrefixes.contains(it.key().split(".").first()))
            for(QVariant el : it.value().toList()){
                ExecutionUnit* unit = new ExecutionUnit(this,varHandler,activeOptions,StatFuncs::mapToHash(el.toMap()),getSystems());
                if(unit->isValid())
                    runQueue->addPrerun(unit);
                else
                    delete unit;
            }
        if(it.key().endsWith(".postrun")&&activePrefixes.contains(it.key().split(".").first()))
            for(QVariant el : it.value().toList()){
                ExecutionUnit* unit = new ExecutionUnit(this,varHandler,activeOptions,StatFuncs::mapToHash(el.toMap()),getSystems());
                if(unit->isValid())
                    runQueue->addPostrun(unit);
                else
                    delete unit;
            }
    }


    ExecutionUnit* mainRun = new ExecutionUnit(this,varHandler,activeOptions,activeOptions->getHash(),getSystems());
    if(mainRun->isValid()){
        mainRun->addFixation(envContainer);
        runQueue->setMainrun(mainRun);
    }else{
        delete mainRun;
        return new RuntimeQueue();
    }
    envContainer->resolveVariables();
    runQueue->resolveVariables(varHandler);
    for(ExecutionUnit* unit : runQueue->getQueue()){
        unit->getEnvironment()->merge(envContainer);
        unit->getEnvironment()->importSystem();
        unit->resolveVariables(varHandler);
    }

    return runQueue;
}

int JasonCore::getExecSystem(QVariantMap *totalMap){
    QString requestedSystem;
    auto optMap = activeOptions->getHash();
    for(auto it = optMap.begin();it!=optMap.end();it++)
        if(it.key()=="launchtype"){
            requestedSystem=it.value().toString();
            break;
        }
    QList<QVariant> systems;
    for(auto it = totalMap->begin();it!=totalMap->end();it++){
        if(it.key()=="systems")
            systems.append(it.value().toList());
    }
    for(auto sys : systems){
        SystemContainer* s = new SystemContainer(this,varHandler,new auto(StatFuncs::mapToHash(sys.toMap())),activeOptions);
        if(s->getIdentifier()==requestedSystem){
            m_systems.append(s);
        }else
            delete s;
    }
    if(m_systems.size()<1)
        return 1;
    m_systems.first()->expandInheritance(&systems,activeOptions);
    return 0;
}

int JasonCore::getSubsystems(QVariantMap *totalMap){
    QHash<QString,QVariant> requestedSubsystems;
    auto optMap = activeOptions->getHash();
    for(auto it = optMap.begin();it!=optMap.end();it++){
        if(it.key().startsWith("subsystem.")){
            requestedSubsystems.insertMulti(it.key(),it.value());
        }
    }
    QList<QVariant> subsystems;
    for(auto it = totalMap->begin();it!=totalMap->end();it++){
        if(it.key()=="subsystems")
            subsystems.append(it.value().toList());
    }
    for(QVariant subsys : subsystems){
        SubsystemContainer* s = new SubsystemContainer(this,new auto(StatFuncs::mapToHash(subsys.toMap())),varHandler,envContainer);
        for(QString key : requestedSubsystems.keys())
            if(key.endsWith(s->getActivator())){
                m_subsystems.append(s);
                break;
            }
    }
    if(subsystems.size()>m_subsystems.size())
        return 1;
    return 0;
}
