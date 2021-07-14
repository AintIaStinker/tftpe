#include "fileobj.h"
#include <QFileInfo>

fileObj::fileObj(QObject *parent) : QObject(parent)
{

}

fileObj::~fileObj()
{
    file.close();
}

// Open the file and return success or failure
bool fileObj::openFile(QString fileName, QIODevice::OpenModeFlag openMode)
{
    file.setFileName(fileName);

    if(file.open(openMode))
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

void fileObj::appendBytes(const QByteArray &bytes)
{
   file.write(bytes);
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
