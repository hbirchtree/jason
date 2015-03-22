#include "variablehandler.h"
#define MAX_RECURSION_LEVEL 50 //At this point we consider it a circular dependency. Screw those things.

VariableHandler::VariableHandler(QObject *parent) : QObject(parent)
{
    variables = new QHash<QString,QVariant>();
}

VariableHandler::~VariableHandler()
{
    delete variables;
}


void VariableHandler::resolveVariables(){
    i_recursionlevel = 0;
    for(auto it=variables->begin();it!=variables->end();it++){
        solveForVariable(it.key());
    }
    return;
}

int VariableHandler::solveForVariable(QString varname){ //Works recursively!
    if(++i_recursionlevel>MAX_RECURSION_LEVEL){
        return -1;
    }
    if(!variables->contains(varname)){
        return 2;
    }
    QRegExp varRegex("%(\\w+)%");
    varRegex.setMinimal(true);
    QString value = variables->value(varname).toString();
    if(value.contains(QString(varname).prepend("%").append("%")))
        return 1; //Means this is an invalid variable
    QStringList vars;
    int pos = 0;
    while((pos = varRegex.indexIn(value,pos)) != -1){
        vars << varRegex.cap(1);
        pos+=varRegex.matchedLength();
    }
    for(QString dep : vars)
        if(solveForVariable(dep)!=0)
            return 1;
    variables->insert(varname,resolveVariable(value));
    return 0;
}

QString VariableHandler::resolveVariable(QString variable){
    foreach(QString sub, variables->keys()){
        QString replace = sub;
        replace.prepend("%");replace.append("%");
        variable = variable.replace(replace,variables->value(sub).toString());
    }
    return variable;
}

QList<QString> VariableHandler::getVariableList(){
    return variables->keys();
}

QString VariableHandler::getVariable(QString key){
    return variables->value(key).toString();
}

void VariableHandler::variableHandle(QString key, QString value){
    variables->insert(key,value);
}

void VariableHandler::merge(VariableHandler* handler){
    for(QString key : handler->getVariableList())
        variables->insert(key,handler->getVariable(key));
}

void VariableHandler::merge(QHash<QString,QVariant>* variableHash){
    for(QString key : variableHash->keys())
        variables->insert(key,variableHash->value(key).toString());
}

void VariableHandler::variablesImport(QList<QVariant> inputVariables, ActiveOptionsContainer* activeOptions){
    for(QVariant variant : inputVariables){
        QMap<QString,QVariant> varHash = variant.toMap();
        QString varType;
        varType = varHash.value("type","undefined").toString();
        if(varType=="config-input"){
            //If no option is found, insert the default value
            QString defaultValue = varHash.value("default").toString();
            QString option = varHash.value("input").toString();
            if((!option.isEmpty())&&(varHash.value("name").isValid())){
                QList<QVariant> opts = activeOptions->getOption(option);
                QString variable = defaultValue;
                if(!opts.isEmpty()){
                     variable= opts.first().toString();
                }
                variables->insert(varHash.value("name").toString(),variable);
            }
        }else if(varHash.value("name").isValid()&&varHash.value("value").isValid()){
            variableHandle(varHash.value("name").toString(),varHash.value("value").toString());
        }else
            reportError(1,"unsupported variable type:"+varType);
    }
}
