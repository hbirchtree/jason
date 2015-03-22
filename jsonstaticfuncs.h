#ifndef JSONSTATICFUNCS_H
#define JSONSTATICFUNCS_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QJsonDocument>
#include <QHash>
#include <QVariantMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include "modules/variablehandler.h"
#include "jason-tools/systemcontainer.h"

typedef QHash<QString,QVariant> VarHash;

class StatFuncs : public QObject
{
    Q_OBJECT
public:

    explicit StatFuncs(QObject *parent = 0);
    ~StatFuncs();

    static QHash<QString,QVariant> mapToHash(QMap<QString,QVariant> input){
        QHash<QString,QVariant> outHash;
        for(auto it = input.begin();it!=input.end();it++)
            outHash.insertMulti(it.key(),it.value());
        return outHash;
    }

    static int addExecution(const QMap<QString, QVariant> &sourceElement, QHash<QString,QVariant> *targetElement);
    static void processExecutionElement(VariableHandler* varHandler, QHash<QString,QVariant> *execElement, QHash<QString,QString> const &prefixTable);

signals:

public slots:
};

#endif // JSONSTATICFUNCS_H
