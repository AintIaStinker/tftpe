#include "tftp.h"
#include <QApplication>



tftp::tftp(QObject  *parent) : QObject(parent)
{
    blockNumber = 0;
    lastPacket = false;
    sizeOfData = {};
    error = false;

    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::Any);

    connect(&socketTimer, &QTimer::timeout, this, &tftp::timeExpired);
    connect(&progressBarTimer, &QTimer::timeout, this, &tftp::progressBarUpdate);

}

tftp::~tftp()
{
    delete socket;
    delete file;
}

void tftp::startPut()
{
    file = new fileObj();
    if(!file->openFile(filePath, QIODevice::ReadOnly)){
        emit sendStatus("Error: Unable to open file <" + file->fileName(filePath) + ">");
        return;
    }

    connect(socket, &QIODevice::readyRead, this, &tftp::readyReadPut);

    socket->writeDatagram(sendRequest(WRQ, getFileToSend(), getBlockSize(), file->fileSize()), getIpAddress(), getIpPort());
    emit setMaximum(100);
    emit sendStatus("Sending file <" + getFileToSend() + ">...");
    socketTimer.start(3000);
    progressBarTimer.start(500);
}

void tftp::startGet()
{
    file = new fileObj();
    if(!file->openFile(filePath, QIODevice::WriteOnly)){
        emit sendStatus("Error: Unable to open file <" + file->fileName(filePath) + ">");
        return;
    }

    connect(socket, &QIODevice::readyRead, this, &tftp::readyReadGet);

    socket->writeDatagram(sendRequest(RRQ, getFileToSend(), getBlockSize(), file->fileSize()), getIpAddress(), getIpPort());
    emit setMaximum(100);
    emit sendStatus("Beginning transfer of file <" + getFileToSend() + ">...");
    socketTimer.start(3000);
    progressBarTimer.start(500);
}


QHostAddress tftp::getIpAddress() const
{
    return ipAddress;
}

void tftp::setIpAddress(const QString &value)
{
    QHostAddress ip = (QHostAddress)value;

    if(ip.isNull())
    {
        QHostInfo info = QHostInfo::fromName(value);
        for(const auto &v : info.addresses()){
            if(v.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv4Protocol){
                ipAddress = v;
                return;
            }
        }
    }
    ipAddress = (QHostAddress)value;
}

quint16 tftp::getIpPort() const
{
    return ipPort;
}

void tftp::setIpPort(const quint16 &value)
{
    ipPort = value;
}

QString tftp::getFileToSend() const
{
    return fileToSend;
}

void tftp::setFileToSend(const QString &value)
{
    fileToSend = value;
}

int tftp::getBlockSize() const
{
    return blockSize;
}

void tftp::setBlockSize(int value)
{
    blockSize = value;
}

QString tftp::getFilePath() const
{
    return filePath;
}

void tftp::setFilePath(const QString &value)
{
    filePath = value;
}

float tftp::getSizeOfData() const
{
    return sizeOfData;
}

void tftp::setSizeOfData(qint32 value)
{
    sizeOfData = value;
}

int tftp::getTSize() const
{
    return tSize;
}

void tftp::setTSize(int newTSize)
{
    tSize = newTSize;
}

void tftp::stop()
{
    socketTimer.stop();
    progressBarTimer.stop();
    file->closeFile();
    socket->close();
    progressBarUpdate();
    if(!error)
    {
        emit completed();
    }
}

// Build the request packet
QByteArray tftp::sendRequest(OpCode requestType, QString fileName, int blockSize, int fileSize)
{
    QByteArray request;
    request.append('\0').append(requestType);
    request.append(fileName.toStdString());
    request.append('\0');
    request.append("octet");
    request.append('\0');
    request.append("blksize");
    request.append('\0');
    request.append(QByteArray::number(blockSize));
    request.append('\0');
    request.append("tsize");
    request.append('\0');
    request.append(QByteArray::number(fileSize));
    request.append('\0');
    return request;
}

// Build the data packet
QByteArray tftp::dataPacket(ushort blockNumber, QByteArray data)
{
    QByteArray dataPkt;
    dataPkt.append('\0');
    dataPkt.append(DATA);
    dataPkt.append(blockNumber >> 8);
    dataPkt.append(blockNumber & 0xFF);
    dataPkt.append(data);
    return dataPkt;
}

QByteArray tftp::pktACK(ushort blockNumber)
{
    QByteArray ack;
    ack.append('\0');
    ack.append(ACK);
    ack.append(blockNumber >> 8);
    ack.append(blockNumber & 0xFF);
    return ack;
}

QByteArray tftp::errorPacket(QString errorMessage)
{
    QByteArray error;
    error.append('\0');
    error.append(ERROR);
    error.append('\0');
    error.append('\0');
    error.append(errorMessage.toLocal8Bit());
    error.append('\0');
    return error;
}


void tftp::writeDataPacket()
{
    ++blockNumber;
    QByteArray data = file->readBytes(getBlockSize());
    socket->writeDatagram(dataPacket(blockNumber, data), data.length() + 4, getIpAddress(), getIpPort());
    prevData = data;
    setSizeOfData(sizeOfData += data.length());
    if(data.length() < blockSize) lastPacket = true;
}

void tftp::saveDataPacket(QByteArray &pkt)
{
    pkt.remove(0, 4);
    file->writeBytes(pkt);
    setSizeOfData(sizeOfData += pkt.length());
    if(pkt.length() < blockSize) lastPacket = true;
}

void tftp::pktReadOACK(QByteArray &pkt)
{
    pkt.remove(0,2);
    QList<QByteArray> splitPkt = pkt.split('\0');
    setBlockSize(splitPkt[1].toInt());
    setTSize(splitPkt[3].toInt());
}

void tftp::pktReadError(QByteArray &pkt)
{
    pkt.remove(0,4);
    QString error = pkt.data();
    emit sendStatus("Recieved Error: " + error);
    emit sendStatus("Ending transfer for " + getFileToSend());
}


void tftp::timeExpired()
{
    socketTimer.stop();
    emit sendStatus("No response from host. Please check network path");
    emit setEnabled(true);
    delete this;
}

void tftp::progressBarUpdate()
{
    emit setValue((getSizeOfData() / getTSize() * 100));
}

void tftp::readyReadPut()
{
    if(socket->hasPendingDatagrams())
    {
        socketTimer.start(3000);
        packet.resize(socket->pendingDatagramSize());

        if(packet.size() == 0) return;

        socket->readDatagram(packet.data(), packet.length(), &ipAddress, &ipPort);

        switch (packet.at(1))
        {
        case ACK:
            pktBlockNumber = (((uchar)packet[2] << 8) | (uchar)packet[3]);
            if(lastPacket && blockNumber == pktBlockNumber)
            {
                stop();
                return;
            }
            if(blockNumber == pktBlockNumber)
            {
                writeDataPacket();
            }
            if(pktBlockNumber == prevBlockNumber)
            {
                socket->writeDatagram(dataPacket(blockNumber, prevData), prevData.length() + 4, ipAddress, ipPort);
            }
            break;

        case ERROR:
            error = true;
            pktReadError(packet);
            socket->writeDatagram(errorPacket("Transfer ended."), getIpAddress(), getIpPort());
            stop();
            break;

        case OACK:
            pktReadOACK(packet);
            writeDataPacket();
            break;

        default:
            break;
        }
    }
}

void tftp::readyReadGet()
{
    if(socket->hasPendingDatagrams())
    {
        socketTimer.start(5000);    // Changed timer to 5 seconds
        packet.resize(socket->pendingDatagramSize());

        if(packet.size() == 0) return;  // for null ICMP messages that trigger readyRead()

        socket->readDatagram(packet.data(), packet.length(), &ipAddress, &ipPort);

        switch (packet.at(1))
        {
        case DATA:
            pktBlockNumber = (((uchar)packet[2] << 8) | (uchar)packet[3]);
            saveDataPacket(packet);
            socket->writeDatagram(pktACK(pktBlockNumber), getIpAddress(), getIpPort());
            if(lastPacket)
            {
                stop();
            }
            break;

        case ERROR:
            error = true;
            pktReadError(packet);
            socket->writeDatagram(errorPacket("Error - Transmission ended."), getIpAddress(), getIpPort());
            stop();
            break;

        case OACK:
            pktReadOACK(packet);
            // send ACK
            socket->writeDatagram(pktACK(blockNumber), getIpAddress(), getIpPort());
            break;

        default:
            break;
        }
    }
}
