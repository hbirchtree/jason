#ifndef UIGLUE
#define UIGLUE

#include <QObject>
#include <QString>
#include <QProcess>
class UIGlue : public QObject
{
    Q_OBJECT
public:
    virtual void setDryRun(bool dryrun) = 0;
    virtual void setStartDoc(QString startDocument) = 0;
    virtual void setActionId(QString actionId){}
    virtual void setDesktopFile(QString desktopFile){}
    virtual void setPath(QString path){}
    virtual bool isReady() = 0;

public slots:
    virtual void startParse() = 0;
    virtual void detachedProgramExit(){}

signals:
    void finishedProcessing();
    void failedProcessing();

    void updateProgressText(QString);
    void updateProgressTitle(QString);
    void broadcastMessage(int,QString);

    void hideMainWindow(bool);

    void changeProgressBarRange(qint64,qint64);
    void changeProgressBarValue(int);
    void changeSecondProgressBarRange(qint64,qint64);
    void changeSecondProgressBarValue(int);
    void changeSecondProgressBarVisibility(bool);

    void updateIconSize(int);
    void updateProgressIcon(QString);

    void displayDetachedMessage(QString);
    void detachedRunEnd(); //Internal use

    void processFailed(QProcess::ProcessError);
    void processReturnOutput();
    void emitOutput(QString,QString);
};

#endif // UIGLUE

