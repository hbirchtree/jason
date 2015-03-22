#ifndef RUNTIMEQUEUE_H
#define RUNTIMEQUEUE_H

#include <QObject>
#include <QList>
#include <QProcessEnvironment>
#include <QString>
#include <QHash>
#include <QVariant>
#include "modules/executionunit.h"

class RuntimeQueue : public QObject
{
    Q_OBJECT

public:
    typedef QHash<QString,QVariant> HVMap;

    explicit RuntimeQueue(QObject *parent = 0);
    ~RuntimeQueue();

    void addPostrun(ExecutionUnit* unit){
        if(unit!=0)
            postrun.append(unit);
    }
    void addPrerun(ExecutionUnit* unit){
        if(unit!=0)
            prerun.append(unit);
    }
    void setMainrun(ExecutionUnit* mainrun){
        this->mainrun = mainrun;
    }

    QList<ExecutionUnit*> getPreruns(){
        return prerun;
    }
    QList<ExecutionUnit*> getPostruns(){
        return postrun;
    }
    ExecutionUnit* getMainrun(){
        return mainrun;
    }

    QList<ExecutionUnit*> getQueue(){
        QList<ExecutionUnit*> queue;
        queue.append(prerun);
        if(mainrun!=0)
            queue.append(mainrun);
        queue.append(postrun);
        return queue;
    }
    void merge(RuntimeQueue* otherQueue){
        for(ExecutionUnit* unit : otherQueue->getPreruns())
            this->addPrerun(unit);
        for(ExecutionUnit* unit : otherQueue->getPostruns())
            this->addPostrun(unit);
    }

    void resolveVariables(VariableHandler* varHandler){
        for(ExecutionUnit* unit : prerun)
            unit->resolveVariables(varHandler);
        mainrun->resolveVariables(varHandler);
        for(ExecutionUnit* unit : postrun)
            unit->resolveVariables(varHandler);
    }

signals:

public slots:

private:
    QList<ExecutionUnit*> prerun;
    QList<ExecutionUnit*> postrun;
    ExecutionUnit* mainrun = 0;
};

#endif // RUNTIMEQUEUE_H
