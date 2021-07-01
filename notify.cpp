#include "notify.h"
#include <QDebug>

Notify::Notify(QObject *parent) : QObject(parent)
{

}

Notify::Notify(QString errors)
{
    qDebug() << errors;
}
