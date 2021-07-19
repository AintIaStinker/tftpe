#ifndef TFTP_H
#define TFTP_H

#include <QObject>
#include <QByteArray>
#include <QUdpSocket>
#include <QFile>
#include <QString>
#include "fileobj.h"
#include <QFileInfo>
#include <QTimer>
#include <QHostInfo>

class tftp : public QObject
{
    Q_OBJECT
public:
    explicit tftp(QObject *parent = nullptr);

    ~tftp();

    QHostAddress getIpAddress() const;
    void setIpAddress(const QString &value);

    quint16 getIpPort() const;
    void setIpPort(const quint16 &value);

    QString getFileToSend() const;
    void setFileToSend(const QString &value);

    int getBlockSize() const;
    void setBlockSize(int value);

    QString getFilePath() const;
    void setFilePath(const QString &value);

    float getSizeOfData() const;
    void setSizeOfData(qint32 value);

    int getTSize() const;
    void setTSize(int newTSize);

private:
    enum    OpCode{
        RRQ = 1,
        WRQ = 2,
        DATA = 3,
        ACK = 4,
        ERROR = 5,
        OACK = 6
    };

    QUdpSocket *socket;
    QHostAddress ipAddress;
    quint16 ipPort;
    QString fileToSend;
    QString filePath;
    int blockSize;
    int tSize;

    unsigned short blockNumber;
    unsigned short pktBlockNumber;
    unsigned short prevBlockNumber;

    bool lastPacket;
    bool error;

    QByteArray packet;
    QByteArray prevData;
    fileObj *file;
    qint32 sizeOfData;

    QTimer socketTimer {this};
    QTimer progressBarTimer {this};

    QByteArray sendRequest(OpCode requestType, QString fileName, int blockSize, int fileSize);
    QByteArray dataPacket(ushort blockNumber, QByteArray data);
    void writeDataPacket();
    void saveDataPacket(QByteArray &pkt);
    void pktReadOACK(QByteArray &pkt);
    void pktReadError(QByteArray &pkt);


    QByteArray pktACK(ushort blockNumber);
signals:
    void cancel();
    void quit();
    void completed();
    void setMaximum(int maximum);       // Connected to progress bar
    void setValue(int value);           // Connected to progress bar
    void reset();                       // Connected to progress bar
    void sendStatus(QString status);    // Status updates to send to UI
    void setEnabled(bool value);        // Enables the upload button after a timeout

public slots:
    void startPut();
    void startGet();
    void stop();

private slots:
    void readyReadPut();
    void readyReadGet();
    void timeExpired();
    void progressBarUpdate();
    QByteArray errorPacket(QString errorMessage);
};

#endif // TFTP_H
