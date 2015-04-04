#include <QCommandLineOption>
#include <QCommandLineParser>
#include "kaidan.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("Jason");
    QApplication::setApplicationVersion("1.0");

    //Parse the command line
    QCommandLineParser cParse;
    cParse.setApplicationDescription("Jason launcher");
    cParse.addHelpOption();
    cParse.addPositionalArgument("file",QApplication::translate("init","File to open"));
    QCommandLineOption desktopAction = QCommandLineOption("action",QApplication::translate("init","Action within the Jason document to launch"),"action","");
    QCommandLineOption desktopGen = QCommandLineOption("desktop-file-generate",QApplication::translate("init","Create a desktop file"),"desktop-file-generate","");
    desktopAction.setValueName("action");
    desktopGen.setValueName(QApplication::translate("init","output %1 file").arg(".desktop"));
    cParse.addOption(desktopAction);
    cParse.addOption(desktopGen);
    cParse.addOption(QCommandLineOption("dry",QApplication::translate("dry","Run without making changes"),"dry","0"));
    //Actual processing
    cParse.process(a);

    //Required arguments
    QStringList posArgs = cParse.positionalArguments();
    QString filename;
    if (posArgs.length()>=1){
        filename = posArgs[0];
    }else
        return 0;

    //Optional arguments
    QStringList options = cParse.optionNames();
    QString actionToLaunch;
    QString desktopFile;
    bool dryrun = false;
    foreach(QString option, options){
        if(option=="action")
            actionToLaunch = cParse.value(option);
        if(option=="desktop-file-generate")
            desktopFile = cParse.value(option);
        if(option=="dry")
            if(cParse.value(option)=="1")
                dryrun=true;
    }

    Kaidan jasongui;
    jasongui.initializeParse(filename,actionToLaunch,desktopFile,a.arguments()[0],dryrun);

    return 0;
}
