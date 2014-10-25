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

//void JasonParser::generateDesktopFile(QString desktopFile, QString jasonPath, QString inputDoc){
//    //Not a very robust function, but I believe it is sufficient for its purpose.
//    QFile outputDesktopFile;
//    if(!desktopFile.endsWith(".desktop"))
//        broadcastMessage(1,tr("Warning: The filename specified for the output .desktop file does not have the correct extension.\n"));
//    outputDesktopFile.setFileName(desktopFile);
//    if(outputDesktopFile.exists()){
//        emit toggleCloseButton(true);
//        updateProgressText(tr("The file exists. Will not proceed."));
//        emit failedProcessing();
//        return;
//    }
//    if(!outputDesktopFile.open(QIODevice::WriteOnly | QIODevice::Text)){
//        emit toggleCloseButton(true);
//        updateProgressText(tr("Failed to open the output file for writing. Will not proceed."));
//        emit failedProcessing();
//        return;
//    }
//    QString desktopActions;
//    QString outputContents;
//    outputContents.append("[Desktop Entry]\nVersion=1.0\nType=Application\nTerminal=False\nExec='%JASON_EXEC%' '%INPUT_FILE%'\n%DESKTOP_ACTION_LIST%\n");
//    foreach(QString entry,runtimeValues.keys())
//        if(entry=="launchables"){
//            QHash<QString,QVariant> launchables = runtimeValues.value(entry).toHash();
//            foreach(QString key,launchables.keys()){
//                if(key=="default.desktop"){
//                    QHash<QString,QVariant> defaultDesktop = launchables.value(key).toHash();
//                    foreach(QString dKey,defaultDesktop.keys()){
//                        if(dKey=="displayname")
//                            outputContents.append("Name="+defaultDesktop.value(dKey).toString()+"\n");
//                        if((dKey=="description")&&(!defaultDesktop.value(dKey).toString().isEmpty()))
//                            outputContents.append("Comment="+defaultDesktop.value(dKey).toString()+"\n");
//                        if((dKey=="wmclass")&&(!defaultDesktop.value(dKey).toString().isEmpty()))
//                            outputContents.append("StartupWMClass="+defaultDesktop.value(dKey).toString()+"\n");
//                        if((dKey=="icon")&&(!defaultDesktop.value(dKey).toString().isEmpty()))
//                            outputContents.append("Icon="+defaultDesktop.value(dKey).toString()+"\n");
//                        if((dKey=="categories")&&(!defaultDesktop.value(dKey).toString().isEmpty()))
//                            outputContents.append("Categories="+defaultDesktop.value(dKey).toString()+"\n");
//                    }
//                }
//            }
//            foreach(QString key,launchables.keys()){
//                if((key!="default")&&(key!="default.desktop")){
//                    QHash<QString,QVariant> desktopAction = launchables.value(key).toHash();
//                    QString desktopActionEntry;
//                    foreach(QString dKey,desktopAction.keys())
//                        if(dKey=="displayname"){
//                            desktopActions.append(key+";");
//                            desktopActionEntry = "[Desktop Action "+key+"]\nName="+desktopAction.value(dKey).toString()+"\nExec='%JASON_EXEC%' --action "+key+" '%INPUT_FILE%'\n";
//                        }
//                    outputContents.append("\n"+desktopActionEntry);
//                }
//            }
//        }
//    if(!jasonPath.isEmpty()){
//        QFileInfo jasonInfo(jasonPath);
//        outputContents.replace("%JASON_EXEC%",jasonInfo.canonicalFilePath());
//    }
//    if(!jasonPath.isEmpty()){
//        QFileInfo docInfo(inputDoc);
//        outputContents.replace("%INPUT_FILE%",docInfo.canonicalFilePath());
//        outputContents.replace("%WORKINGDIR%",docInfo.canonicalPath());
//    }
//    if(!desktopActions.isEmpty()){
//        outputContents.replace("%DESKTOP_ACTION_LIST%","Actions="+desktopActions);
//    }else
//        outputContents.replace("%DESKTOP_ACTION_LIST%",QString());

//    QTextStream outputDocument(&outputDesktopFile);
//    outputDocument << outputContents;
//    outputDesktopFile.setPermissions(QFile::ExeOwner|outputDesktopFile.permissions());
//    outputDesktopFile.close();

//    updateProgressText(tr("Desktop file was generated successfully."));

//    emit toggleCloseButton(true);
//    emit finishedProcessing();
//}
