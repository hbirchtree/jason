#ifndef ACTIVEOPTIONSCONTAINER_H
#define ACTIVEOPTIONSCONTAINER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QList>

class ActiveOptionsContainer : public QObject
{
    Q_OBJECT
public:
    explicit ActiveOptionsContainer(QObject *parent = 0);
    ~ActiveOptionsContainer();

    void addOption(QString optname,QVariant value){
        activeOptions.insertMulti(optname,value);
    }

    QHash<QString,QVariant> getHash(){
        return QHash<QString,QVariant>(activeOptions);
    }

    QList<QVariant> getOption(QString optname){
        QList<QVariant> result;
        for(QHash<QString,QVariant>::iterator it = activeOptions.begin(); it!=activeOptions.end();it++)
            if(it.key()==optname)
                result.append(it.value());
        return result;
    }
    QList<QVariant> getOptStartingWith(QString startsWith){
        QList<QVariant> result;
        for(QHash<QString,QVariant>::iterator it = activeOptions.begin(); it!=activeOptions.end();it++)
            if(it.key().startsWith(startsWith))
                result.append(it.value());
        return result;
    }

    void importMap(QMap<QString,QVariant> options){ //Will not replace entries
        for(QString key : options.keys())
            activeOptions.insert(key,options.value(key));
    }
    void importHash(QHash<QString,QVariant> options){ //Will not replace entries
        for(QString key : options.keys())
            if(!activeOptions.keys().contains(key))
                activeOptions.insert(key,options.value(key));
    }

    void importReplacingHash(QHash<QString,QVariant> options){
        for(QString key : options.keys())
            activeOptions.insert(key,options.value(key));
    }

signals:

public slots:

private:
    QHash<QString,QVariant> activeOptions;
};

#endif // ACTIVEOPTIONSCONTAINER_H
