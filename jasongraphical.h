#ifndef JASONGRAPHICAL_H
#define JASONGRAPHICAL_H

#include <QWidget>
#include <QMessageBox>
#include <QProgressDialog>
#include <QProgressBar>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMenu>

#include <QThread>

class JasonGraphical : public QWidget
{
    Q_OBJECT
public:
    explicit JasonGraphical(QWidget *parent = 0);

    void startParse(QString startDocument, QString actionId, QString desktopFile);

private:
    QProgressDialog *progressWindow;
    QProgressBar *infiniteBar;
    QMessageBox *messageBox;
    QLabel *statusText;
    QThread *workerThread;
signals:
    void updateLaunchProgress(QString);
    void closeWindow();
    void updateWIndowTitle(QString);
    void postMessage(int,QString);

public slots:
    void showMessage(int status,QString message);
    void showProgressWindow();
//    void waitForQuit(QString waitTitle);
};

#endif // JASONGRAPHICAL_H
