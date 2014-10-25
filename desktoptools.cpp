#include "desktoptools.h"

desktoptools::desktoptools(QObject *parent) :
    QObject(parent)
{
}

QHash<QString,QVariant> desktoptools::desktopFileBuild(QHash<QString,QVariant> const &desktopObject){
/*
 * Desktop files execute by ./Jason [JSON-file]
 * A desktop action entry will use the appendage of --action [id] to make a distinction between the
 * different entries that may exist.
*/
    QHash<QString,QVariant> desktopHash;

    //Defaults
    desktopHash.insert("Version","1.0");
    desktopHash.insert("Type","Application");
    desktopHash.insert("Terminal","false");

    if(desktopObject.value("desktop.displayname").isValid())
        desktopHash.insert("Name",desktopObject.value("desktop.displayname").toString());
    if(desktopObject.value("desktop.icon").isValid())
        desktopHash.insert("Icon",desktopObject.value("desktop.icon").toString());
    if(desktopObject.value("desktop.description").isValid())
        desktopHash.insert("Comment",desktopObject.value("desktop.description").toString());
    if(desktopObject.value("desktop.wmclass").isValid())
        desktopHash.insert("StartupWMClass",desktopObject.value("desktop.wmclass").toString());
    if(desktopObject.value("desktop.categories").isValid())
        desktopHash.insert("Categories",desktopObject.value("desktop.categories").toString());
    desktopHash.insert("Actions",QString());
    QHash<QString,QVariant> desktopActions;
    foreach(QString key,desktopObject.keys()){
        if(key.startsWith("desktop.action."))
            if(desktopObject.value(key).toHash().value("desktop.displayname").isValid()){
                desktopHash.insert("Actions",key.split(".")[2]+";"+desktopHash.value("Actions").toString());
                desktopActions.insert(key.split(".")[2],desktopObject.value(key).toHash().value("desktop.displayname").toString());
            }
        if(key.startsWith("rawdesktop."))
            desktopHash.insert(key.split(".")[1],desktopObject.value(key).toString());
    }
    desktopHash.insert("desktopactions",desktopActions);
    return desktopHash;
}

void desktoptools::generateDesktopFile(QHash<QString,QVariant> const &desktopData, QString desktopFile, QString jasonPath, QString inputDoc){
    QFile outputFile(desktopFile);
    if(outputFile.exists())
        return;
    if(!outputFile.open(QIODevice::WriteOnly|QIODevice::Text)){
        return;
    }
    QString execString;
    QStringList outputFileContents;
    QFileInfo jasonExec(jasonPath);
    QFileInfo inputFile(inputDoc);
    execString.append("'"+jasonExec.absoluteFilePath()+"' '"+inputFile.absoluteFilePath()+"'");
    QHash<QString,QVariant> desktopDataCopy = desktopData;
    QHash<QString,QVariant> actionData = desktopDataCopy.value("desktopactions").toHash();
    desktopDataCopy.remove("desktopactions");
    desktopDataCopy.insert("Exec",execString);

    outputFileContents.append("[Desktop Entry]");
    foreach(QString key,desktopDataCopy.keys())
        outputFileContents.append(key+"="+desktopDataCopy.value(key).toString());
    QStringList actionTemplate,aCopy;
    actionTemplate.append("");
    actionTemplate << "[Desktop Action %ACTION%]" << "Name=%TITLE%" << "Exec="+execString+" --action %ACTION%";
    foreach(QString action,actionData.keys()){
        aCopy=actionTemplate;
        if(!action.isEmpty())
            if(!actionData.value(action).toString().isEmpty())
                foreach(QString line,aCopy)
                    outputFileContents.append(line.replace("%ACTION%",action).replace("%TITLE%",actionData.value(action).toString()));
    }
    QTextStream textStreamer;
    textStreamer.setDevice(&outputFile);
    foreach(QString line,outputFileContents)
        textStreamer << line << endl;
    textStreamer << endl;
    outputFile.close();
}
