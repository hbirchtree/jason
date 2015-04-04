#ifndef KAIDAN_H
#define KAIDAN_H

#include <QDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QGridLayout>
#include <QPixmap>
#include <QImageReader>
#include <QFile>
#include <QFileDialog>

namespace Ui {
class Kaidan;
}

class Kaidan : public QDialog
{
    Q_OBJECT

public:
    explicit Kaidan(QWidget *parent = 0);
    ~Kaidan();

    void initializeParse(QString startDocument, QString actionId, QString desktopFile, QString jasonPath, bool dryrun);

private:
    Ui::Kaidan *ui;

    QList<QMetaObject::Connection> connections;

    //updateProgressIcon
    QFile *imageFile;
    QPixmap *newLabelIcon;


private slots:
    void updateProgressText(QString newText);
    void updateProgressIcon(QString newIconPath);
    void updateWindowTitle(QString newTitle);

    void updateMainProgressBarLimits(qint64 min, qint64 max);
    void updateMainProgressBarValue(int val);

    void updateSecondProgressBarLimits(qint64 min, qint64 max);
    void updateSecondaryProgressBarValue(int val);

    void modifySecondPBarVisibility(bool doShow);
};

#endif // KAIDAN_H
