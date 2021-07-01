#ifndef FILEOBJ_H
#define FILEOBJ_H

#include <QObject>
#include <QFile>

class fileObj : public QObject
{
    Q_OBJECT
public:
    explicit fileObj(QObject *parent = nullptr);
    ~fileObj();

    QByteArray readBytes(int blockSize);
    qint64 fileSize();
    QString fileName(QString filePath);
    bool openFile(QString fileName);
    void closeFile();

private:
    QFile file;

};

#endif // FILEOBJ_H
