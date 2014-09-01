#ifndef JASON_H
#define JASON_H

#include <QMainWindow>
#include <QJsonDocument>
#include <QFile>

class Jason : public QMainWindow
{
    Q_OBJECT

public:
    Jason(QWidget *parent = 0);
    ~Jason();
};

#endif // JASON_H
