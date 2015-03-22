#include "environmentcontainer.h"

EnvironmentContainer::EnvironmentContainer(QObject *parent,VariableHandler* varHandler) : QObject(parent)
{
    this->varHandler = varHandler;
    procEnv = new QProcessEnvironment();
}

EnvironmentContainer::~EnvironmentContainer()
{
    delete procEnv;
}

void EnvironmentContainer::setEnvVar(QString key, QString value){
    procEnv->insert(key,value);
}

QString EnvironmentContainer::getRunPrefix(){
    QString outString;
    for(QString entry : runPrefixes)
        outString.append(entry+" ");
    return outString;
}

QString EnvironmentContainer::getRunSuffix(){
    QString outString;
    for(QString entry : runSuffixes)
        outString.append(" "+entry);
    return outString;
}

QProcessEnvironment EnvironmentContainer::getProcEnv(){
    return QProcessEnvironment(*procEnv);
}

void EnvironmentContainer::merge(EnvironmentContainer* otherEnvContainer){
    QProcessEnvironment oprocEnv = otherEnvContainer->getProcEnv();
    for(QString key : oprocEnv.keys())
        if(!procEnv->contains(key))
            setEnvVar(key,oprocEnv.value(key));
    for(QString prefix : otherEnvContainer->getRunPrefixList())
        runPrefixes.append(prefix);
    for(QString suffix : otherEnvContainer->getRunSuffixList())
        runSuffixes.append(suffix);
}

void EnvironmentContainer::merge(EnvironmentContainer* otherEnvContainer,bool overwrite){
    QProcessEnvironment oprocEnv = otherEnvContainer->getProcEnv();
    for(QString key : oprocEnv.keys())
        if(!procEnv->contains(key)||overwrite)
            setEnvVar(key,oprocEnv.value(key));
    for(QString prefix : otherEnvContainer->getRunPrefixList())
        runPrefixes.append(prefix);
    for(QString suffix : otherEnvContainer->getRunSuffixList())
        runSuffixes.append(suffix);
}

void EnvironmentContainer::environmentImport(QHash<QString,QVariant> const &environmentHash){
    QString type = environmentHash.value("type").toString();
    //Treat the different types according to their parameters and realize their properties
    if(type=="run-prefix"){
        if(environmentHash.value("exec.prefix").isValid()) //CHANGE: WE NO LONGER GIVE A BUNG ABOUT WHAT SYSTEM IT IS; BRACE YOURSELF, USER
            runPrefixes.append(environmentHash.value("exec.prefix").toString());
    }else if(type=="run-suffix"){
        if(environmentHash.value("exec.suffix").isValid())
            runSuffixes.append(environmentHash.value("exec.suffix").toString());
    }else if(type=="variable"){
        if(environmentHash.value("name").isValid()&&environmentHash.value("value").isValid())
            setEnvVar(environmentHash.value("name").toString(),varHandler->resolveVariable(environmentHash.value("value").toString()));
    }
    return;
}
