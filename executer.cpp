#include "executer.h"

Executer::Executer(QObject *parent) :
    QThread(parent)
{
}

Executer::~Executer(){
    delete executer;
}

int Executer::exec(QString *shell, QStringList *arguments, QString *workDir, QProcessEnvironment *procEnv, bool *lazyExitStatus,bool startDetached){

    executer = new QProcess(this);
    executer->setProcessEnvironment(*procEnv);
    executer->setProcessChannelMode(QProcess::SeparateChannels);
    if(!workDir->isEmpty())
        executer->setWorkingDirectory(*workDir);
    executer->setProgram(*shell);
    executer->setArguments(*arguments);

    connect(executer, SIGNAL(finished(int,QProcess::ExitStatus)),SLOT(processFinished(int,QProcess::ExitStatus)));

    if(startDetached){
        executer->startDetached(*shell,*arguments,*workDir);
        executer->waitForStarted(-1);
        return 0;
    }else
        executer->start();
    executer->waitForFinished(-1);
    if(!lazyExitStatus){
        return 0;
    }
    return executer->exitCode();
}

void Executer::processFinished(int exitCode, QProcess::ExitStatus exitStatus){
    if((exitCode!=0)||(exitStatus!=0)){
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
    emit processReturnValues(exitCode,exitStatus);
}

void Executer::processStarted(){
    return;
}
