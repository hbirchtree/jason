#ifndef JASONGUI_H
#define JASONGUI_H

#include <QDialog>
#include <QMessageBox>
#include <QGridLayout>
#include <QTextEdit>

namespace Ui {
class JasonGui;
}

class JasonGui : public QDialog
{
    Q_OBJECT

public:
    explicit JasonGui(QWidget *parent = 0);
    ~JasonGui();
    void startParse(QString startDocument, QString actionId, QString desktopFile, QString jasonPath);

private:
    Ui::JasonGui *ui;
    QMessageBox *messageBox;
    QMessageBox *detachedProgramNotify;
    QThread *workerThread;
    QDialog *outputWindow;
    QGridLayout *outputLayout;
    QTextEdit *errEdit, *outEdit;

signals:
    void updateLaunchProgress(QString);
    void closeWindow();
    void updateWIndowTitle(QString);
    void postMessage(int,QString);
    void detachedHasClosed();

private slots:
    void showMessage(int status,QString message);
    void detachedMessage(QString title);
    void showOutput(QString stdOut,QString stdErr);
    void printMessage(QString message);
    void detachedProgramNotifyEmit(int retInt);
    void resizeWindow(int width,int height);
    void hideJasonGui(bool doShow);
};

#endif // JASONGUI_H
