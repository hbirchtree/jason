#ifndef JASONGRAPHICAL_H
#define JASONGRAPHICAL_H

#include <QWidget>
#include <QMessageBox>
//For the progress window:
#include <QProgressDialog>
#include <QPushButton>
//For the output window
#include <QDialog>
#include <QTextEdit>
#include <QGridLayout>

#include <QThread>

class JasonGraphical : public QWidget
{
    Q_OBJECT
public:
    explicit JasonGraphical(QWidget *parent = 0);


    void startParse(QString startDocument, QString actionId, QString desktopFile, QString jasonPath);

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
    void showOutput(QString stdOut,QString stdErr);
    void printMessage(QString message);
};

#endif // JASONGRAPHICAL_H
