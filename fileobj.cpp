#include "fileobj.h"
#include "notify.h"
#include <QFileInfo>

fileObj::fileObj(QObject *parent) : QObject(parent)
{

}

// Set the filename to open


fileObj::~fileObj()
{
    file.close();
}

// Open the file and return success or failure
bool fileObj::openFile(QString fileName)
{
    file.setFileName(fileName);

    if(file.open(QIODevice::ReadOnly))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void fileObj::closeFile()
{

    file.close();
}

// Read bytes in blocks
QByteArray fileObj::readBytes(int blockSize)
{
    QByteArray readTheBytes = file.read(blockSize);
    return readTheBytes;
}


qint64 fileObj::fileSize()
{
    return file.size();
}

QString fileObj::fileName(QString filePath)
{
    QFileInfo file(filePath);
    return file.fileName();
}
