#ifndef EXECUTER_H
#define EXECUTER_H

#include <QThread>
#include <QProcess>
#include <QProcessEnvironment>
#include <QList>
#include <QVariant>

class Executer : public QThread
{
    Q_OBJECT
public:
    explicit Executer(QObject *parent = 0);
    int initializeProcess(QString shell, QStringList arguments, QString workDir, QProcessEnvironment procEnvImport, bool lazyExitStatus);
    int exec();

private:
    QProcess *executer;
    QProcessEnvironment procEnv;
    QList<QVariant> initValues;

signals:
    void processFailed(QProcess::ProcessError);
    void emitOutput(QString,QString);
    void processReturnValues(int exitCode, QProcess::ExitStatus exitStatus);

public slots:
    void processFinished(int exitCode,QProcess::ExitStatus exitStatus);
    void processStarted();

};

#endif // EXECUTER_H
