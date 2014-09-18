#ifndef JASONGRAPHICAL_H
#define JASONGRAPHICAL_H

#include <QWidget>
#include <QMessageBox>
#include <QProgressDialog>
#include <QProgressBar>
#include <QPushButton>
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
    QMessageBox *messageBox;
    QPushButton *closeProgressWindowBtn;
    QThread *workerThread;
signals:
    void updateLaunchProgress(QString);
    void closeWindow();
    void updateWIndowTitle(QString);
    void postMessage(int,QString);
    void detachedHasClosed();

public slots:
    void showMessage(int status,QString message);
    void detachedMessage(QString title);
};

#endif // JASONGRAPHICAL_H
