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

    void displayLaunchProgress(QString currentText);
    void waitForQuit(QString waitTitle);

private:
    QMessageBox *messageBox;
signals:


public slots:
};

#endif // JASONGRAPHICAL_H
