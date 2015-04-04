#ifndef JASONPARSER_H
#define JASONPARSER_H

#include "executer.h"
#include "jsonparser.h"
#include "desktoptools.h"

#include <QMetaObject>
#include <QFileInfo>
#include <stdio.h>
#include <QString>
#include <QStringList>
#include <QHash>

#include <QProcessEnvironment>
#include <QEventLoop>

#include "modules/uiglue.h"

class JasonParser : public UIGlue
{
    Q_OBJECT
public:
    JasonParser();
    ~JasonParser();
    void setDryRun(bool dry){
        this->b_dry = dry;
    }

    void setStartDoc(QString startDoc){
        if(startDoc.isEmpty())
           return;
        this->startDoc = startDoc;
        checkValidity();
    }
    void setActionId(QString aid){
        if(aid.isEmpty())
           return;
        this->actionId = aid;
        checkValidity();
    }
    void setDesktopFile(QString df){
        if(df.isEmpty())
           return;
        this->desktopFile = df;
        checkValidity();
    }
    void setPath(QString jp){
        if(jp.isEmpty())
           return;
        this->jasonPath = jp;
        checkValidity();
    }
    bool isReady(){
        return b_validState;
    }


public slots:
    void startParse();
    void detachedProgramExit();

private:
    void checkValidity(){
        if(startDoc.isEmpty()||(!desktopFile.isEmpty()&&jasonPath.isEmpty()))
            return;
        b_validState = true;
    }

    QString startDoc;
    QString actionId;
    QString desktopFile;
    QString jasonPath;
    bool b_validState = false;
    bool b_dry = false;

    int exitResult;

    QList<QMetaObject::Connection> connectedSlots;

    jsonparser *parser;
    QHash<QString,QVariant> *jsonFinalData;
    QHash<QString,QVariant> *runtimeValues;
    QEventLoop *waitLoop;

    void quitProcess();
};

#endif // JASONPARSER_H
