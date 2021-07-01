#ifndef PROGRESS_H
#define PROGRESS_H

#include <QObject>
#include <QProgressBar>

class Progress : public QObject
{
    Q_OBJECT
public:
    explicit Progress(QObject *parent = nullptr);

signals:
    void progress();
    void completed();
    void error();

public slots:

};

#endif // PROGRESS_H
