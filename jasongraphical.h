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

    void waitForQuit(QString waitTitle);
    void startParse(QString startDocument, QString actionId, QString desktopFile);
    void showProgressWindow();
    void showMessage(int status,QString message);

private:
    QProgressDialog *progressWindow;
    QProgressBar *infiniteBar;
    QMessageBox *messageBox;
    QLabel *statusText;
    QThread *workerThread;
signals:


public slots:
    void updateLaunchProgress(QString currentText);
};

#endif // JASONGRAPHICAL_H
