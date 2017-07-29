#ifndef SERIALBUSACTIONS_H
#define SERIALBUSACTIONS_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QQueue>
class SerialBusActions : public QObject
{
    Q_OBJECT
public:
    explicit SerialBusActions(QObject *parent = nullptr);
    virtual ~SerialBusActions(){}

    virtual void * getDataRomRTUFrame(uchar address, uchar command, QString receiveDataPacketBuffer) = 0;
    virtual QByteArray makeRTUFrame(int address, int command, const QByteArray & dataPacket) = 0;
protected:

    virtual ushort calculateCRC16(uchar *dataPacket, ushort dataPacketLength) = 0;
    virtual bool equalCRC(ushort calculateCRC, uchar hiByte, uchar lowByte) = 0;


signals:

public slots:
};

#endif // SERIALBUSACTIONS_H
