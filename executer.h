#ifndef EXECUTER_H
#define EXECUTER_H

#include <QThread>
#include <QProcess>
#include <QProcessEnvironment>
#include <QList>
#include <QVariant>

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
    explicit Executer(QObject *parent = 0);
    ~Executer();
    int exec(QString *shell, QStringList *arguments, QString *workDir, QProcessEnvironment *procEnv, bool *lazyExitStatus, bool startDetached);

private:
    QProcess *executer;

signals:
    void processFailed(QProcess::ProcessError);
    void emitOutput(QString,QString);
    void processReturnValues(int exitCode, QProcess::ExitStatus exitStatus);

public slots:
    void processFinished(int exitCode,QProcess::ExitStatus exitStatus);
    void processStarted();

};

#endif // EXECUTER_H
