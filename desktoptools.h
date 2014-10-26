#ifndef DESKTOPTOOLS_H
#define DESKTOPTOOLS_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

class desktoptools : public QObject
{
    Q_OBJECT
public:
    explicit desktoptools(QObject *parent = 0);
    QHash<QString,QVariant> desktopFileBuild(const QHash<QString, QVariant> &desktopObject);
    void generateDesktopFile(QHash<QString,QVariant> const &desktopData, QString desktopFile, QString jasonPath, QString inputDoc);

signals:
    void sendProgressTextUpdate(QString);
    void reportError(int,QString);

public slots:

};

#endif // DESKTOPTOOLS_H
