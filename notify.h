#ifndef NOTIFY_H
#define NOTIFY_H

#include <QObject>

class Notify : public QObject
{
    Q_OBJECT
public:
    explicit Notify(QObject *parent = nullptr);
    Notify(QString errors);

signals:



};

#endif // NOTIFY_H
