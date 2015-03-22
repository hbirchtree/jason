#ifndef EXECUTER_H
#define EXECUTER_H

#include <QThread>
#include <QProcess>
#include <QProcessEnvironment>
#include <QList>
#include <QVariant>
#include "modules/executionunit.h"

namespace ExecuterNS {
enum ExecutionValues {
    Exec_Shell,
    Exec_Arguments,
    Exec_WorkDir,
    Exec_LazyExitStatus
};
class Executer;
}

class Executer : public QThread
{
    Q_OBJECT
public:
    explicit Executer(QObject *parent = 0, QString shell = QString(), QString shellArg = QString());
    ~Executer();
    int exec(ExecutionUnit* unit);

private:
    QProcess *executer;
    QString shell;
    QString shellArg;

signals:
    void processFailed(QProcess::ProcessError);
    void emitOutput(QString,QString);
    void processReturnValues(int exitCode, QProcess::ExitStatus exitStatus);

public slots:
    void processFinished(int exitCode,QProcess::ExitStatus exitStatus);
    void processStarted();

};

#endif // EXECUTER_H
