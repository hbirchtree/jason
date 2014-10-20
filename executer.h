#ifndef EXECUTER_H
#define EXECUTER_H

#include <QThread>
#include <QProcess>

class Executer : public QThread
{
    Q_OBJECT
public:
    explicit Executer(QObject *parent = 0);
    int initializeProcess(QString shell, QStringList arguments, QString workDir, QProcessEnvironment procEnv, bool lazyExitStatus);
    int exec();

private:
    QProcess *executer;
    QList<QVariant> initValues;

signals:
    void processFailed(QProcess::ProcessError);
    void emitOutput(QString,QString);
    void processReturnValues(int exitCode, QProcess::ExitStatus exitStatus);

public slots:
    void processFinished(int exitCode,QProcess::ExitStatus exitStatus);
    void processStateChanged(QProcess::ProcessState);

};

#endif // EXECUTER_H
