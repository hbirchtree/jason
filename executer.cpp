#include "executer.h"
#include <QDebug>

Executer::Executer(QObject *parent) :
    QThread(parent)
{
}

int Executer::initializeProcess(QString shell, QStringList arguments, QString workDir, QProcessEnvironment procEnvImport, bool lazyExitStatus){
    initValues.append(QVariant(shell));
    initValues.append(QVariant(arguments));
    initValues.append(QVariant(workDir));
    initValues.append(QVariant(lazyExitStatus));
    procEnv=procEnvImport;
}

int Executer::exec(){
    QString shell,workDir;
    QStringList arguments;
    bool lazyExitStatus;

    shell=initValues.at(0).toString();
    arguments=initValues.at(1).toStringList();
    workDir=initValues.at(2).toString();
    lazyExitStatus=initValues.at(3).toBool();

    QProcess *executer = new QProcess(this);
    executer->setProcessEnvironment(procEnv);
    executer->setProcessChannelMode(QProcess::SeparateChannels);
    if(!workDir.isEmpty())
        executer->setWorkingDirectory(workDir);
    executer->setProgram(shell);
    executer->setArguments(arguments);

    connect(executer, SIGNAL(finished(int,QProcess::ExitStatus)),SLOT(processFinished(int,QProcess::ExitStatus)));
    connect(executer,SIGNAL(started()),SLOT(processStarted()));

    executer->start();
    executer->waitForFinished(-1);
    if(!lazyExitStatus){
        if((executer->exitCode()!=0)||(executer->exitStatus()!=0)){
            QString stdOut,stdErr,argumentString;
            stdOut = executer->readAllStandardOutput();
            stdErr = executer->readAllStandardError();
            foreach(QString arg,executer->arguments())
                argumentString.append(arg+" ");
            stdOut.prepend(tr("Executed: %1 %2\n").arg(executer->program()).arg(argumentString));
            stdOut.prepend(tr("Process returned with status: %1.\n").arg(QString::number(executer->exitCode())));
            stdOut.prepend(tr("QProcess returned with status: %1.\n").arg(QString::number(executer->exitStatus())));
            emit emitOutput(stdOut,stdErr);
        }
        return 0;
    }
    return executer->exitCode();
}

void Executer::processFinished(int exitCode, QProcess::ExitStatus exitStatus){
    emit processReturnValues(exitCode,exitStatus);
}

void Executer::processStarted(){
    return;
}
