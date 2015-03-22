#include "jsonstaticfuncs.h"

StatFuncs::StatFuncs(QObject *parent) : QObject(parent)
{

}

StatFuncs::~StatFuncs()
{

}

void StatFuncs::processExecutionElement(VariableHandler* varHandler,QHash<QString,QVariant> *execElement,QHash<QString,QString> const &prefixTable){
    if((execElement->value("exec.command").isValid())&&(execElement->value("exec.type").isValid())){
        QString command = prefixTable.value(execElement->value("exec.type").toString())+" "+execElement->value("exec.command").toString();
        command = varHandler->resolveVariable(command);
        execElement->insert("exec",command);
        execElement->remove("exec.command");
        execElement->remove("exec.type");
    }
    if(execElement->value("workdir").isValid())
        execElement->insert("workdir",varHandler->resolveVariable(execElement->value("workdir").toString()));
    if(execElement->value("desktop.icon").isValid())
        execElement->insert("desktop.icon",varHandler->resolveVariable(execElement->value("desktop.icon").toString()));
    if(execElement->value("desktop.title").isValid())
        execElement->insert("desktop.title",varHandler->resolveVariable(execElement->value("desktop.title").toString()));
}

int StatFuncs::addExecution(QMap<QString,QVariant> const &sourceElement,QHash<QString,QVariant> *targetElement){
    bool validExec = false;
    foreach(QString key,sourceElement.keys()){
        if(key.endsWith(".exec")){
            validExec=true;
            targetElement->insert("exec.command",sourceElement.value(key).toString());
            targetElement->insert("exec.type",key.split(".")[0]);
        }
        if(key=="lazy-exit-status")
            targetElement->insert("lazyexit",sourceElement.value(key).toBool());
        if(key=="workdir")
            targetElement->insert(key,sourceElement.value(key).toString());
        if(key=="detachable-process")
            targetElement->insert("detach",sourceElement.value(key).toBool());
        if(key=="start-detached")
            targetElement->insert("start-detach",sourceElement.value(key).toBool());
        if(key=="private.process-environment")
            targetElement->insert("procenv",sourceElement.value(key).toHash()); //Yeah, we're just throwing this in and hoping it works. I have yet to test this. The pipeline may just as well issue some malicious command instead of doing what it's supposed to do with it.
        if(key=="desktop.title")
            targetElement->insert(key,sourceElement.value(key).toString());
        if(key=="desktop.icon")
            targetElement->insert(key,sourceElement.value(key).toString());
    }
    if(validExec)
        return 0;
    return 1;
}
