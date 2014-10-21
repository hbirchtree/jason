#ifndef DESKTOPTOOLS_H
#define DESKTOPTOOLS_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QDebug>

class desktoptools : public QObject
{
    Q_OBJECT
public:
    explicit desktoptools(QObject *parent = 0);
    QHash<QString,QVariant> desktopFileBuild(const QHash<QString, QVariant> &desktopObject, const QHash<QString, QVariant> &systemTable);

signals:

public slots:

};

#endif // DESKTOPTOOLS_H
