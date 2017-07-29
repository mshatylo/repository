#ifndef PROXIMITY_CARDS_READER_BUS_ACTIONS_H
#define PROXIMITY_CARDS_READER_BUS_ACTIONS_H
#include "serialbusactions.h"

struct DataRecord {
    ushort command_;
    QList<uchar> data_;
    inline QString getData(void) const {

        QString res;
        if (!data_.empty()) {
            for (int i = 0; i < data_.size(); i++)
                res.append(data_[i]);
        }
        return res;
    }
};

class ProximityCardsReaderBusActions : public SerialBusActions
{
    Q_OBJECT
public:
    ProximityCardsReaderBusActions(QObject *parent = nullptr);
    ~ProximityCardsReaderBusActions(){}

    QByteArray makeRTUFrame(int address, int command, const QByteArray & dataPacket);
    void * getDataRomRTUFrame(uchar address, uchar command, QString receiveDataPacketBuffer);

    const uchar PingQuery_[14]          = {0x55, 0x64, 0x09, 0x50, 0x52, 0x2d, 0x30, 0x31, 0x20, 0x55, 0x53, 0x42, 0xc9, 0x11};
    const uchar PingAnswer_[5]          = {0x55, 0x64, 0x00, 0x4b, 0x10};
    const uchar ResetKeyQuery_[5]       = {0x55, 0x65, 0x00, 0x4a, 0x80};
    const uchar ResetKeyAnswer_[5]      = {0x55, 0x65, 0x00, 0x4a, 0x80};
    const uchar KeyQuery_[5]            = {0x55, 0x66, 0x00, 0x4a, 0x70};
    const uchar KeyNegativeAnswer_[5]   = {0x55, 0x66, 0x00, 0x4a, 0x70};
    const uchar KeyPositiveAnswer_[13]  = {0x55, 0x66, 0x08, 0x01, 0x32, 0x41, 0xa0, 0x00, 0x39, 0x00, 0xac, 0x1f, 0x3f};

    const int   AddressByteNumber_{0};
    const int   CommandByteNumber_{1};
    const int   LenghtByteNumber_ {2};

    const uchar PingCommand_    {0x64};
    const uchar ResetKeyCommand_{0x65};
    const uchar KeyCommand_     {0x66};

protected:

    ushort calculateCRC16(uchar *dataPacket, ushort dataPacketLength);
    bool equalCRC(ushort calculateCRC, uchar hiByte, uchar lowByte);

};

#endif // PROXIMITY_CARDS_READER_BUS_ACTIONS_H
