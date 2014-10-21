#include "desktoptools.h"

desktoptools::desktoptools(QObject *parent) :
    QObject(parent)
{
}

QHash<QString,QVariant> desktoptools::desktopFileBuild(QHash<QString,QVariant> const &desktopObject,QHash<QString,QVariant> const &systemTable){
/*
 * Desktop files execute by ./Jason [JSON-file]
 * A desktop action entry will use the appendage of --action [id] to make a distinction between the
 * different entries that may exist.
*/
    QString dName,dDesc,dWMClass,dIcon,dCat;
    qDebug() << desktopObject;
    foreach(QString key, desktopObject.keys()){
        qDebug() << key;
    }
    QHash<QString,QVariant> desktopHash;
    desktopHash.insert("displayname",dName);
    desktopHash.insert("description",dDesc);
    desktopHash.insert("wmclass",dWMClass);
    desktopHash.insert("categories",dCat);
    desktopHash.insert("icon",dIcon);
    return desktopHash;
}
