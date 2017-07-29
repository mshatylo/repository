#ifndef SERIALPORTHANDLER_H
#define SERIALPORTHANDLER_H

#include <QObject>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QList>
#include <QSerialPort>
#include <QStatusBar>
#include "serialbusactions.h"
#include "serialportmasterthread.h"
#include <memory>
#include <QMutex>
#include <QMutexLocker>
#include "proximitycardsreaderbusactions.h"


class SerialPortHandler : public QObject
{
    Q_OBJECT
public:
    struct Settings {
        int parity_;
        int baudRate_;
        int dataBits_;
        int stopBits_;
        int responseTime_;
        int numberOfRetries_;

        Settings()
            : parity_(QSerialPort::EvenParity),
              baudRate_(QSerialPort::Baud19200),
              dataBits_(QSerialPort::Data8),
              stopBits_(QSerialPort::OneStop),
              responseTime_(100),
              numberOfRetries_(3)
        {}
    };
    enum class MessageType {
        response_,
        timout_,
        error_
    };

    explicit SerialPortHandler(QObject *parent = nullptr);

    QString getSerialPortInformation(void);
    void proximityCardInformation(void);

signals:
    void signalSendControlsEnabled(bool enable);
    void signalSendStatusBarMessage(QString message);
    void signalSendProximityCardInformation(const QString &cardData);
    void signalSendProximityCardMessage(const MessageType &messageType);

public slots:
    void slotStusbarMessage(QString message);
    void serialPortResponse(const QString &portName, const QString &responce);
    void serialPortProcessError(const QString &portName, const QString &s);
    void serialPortProcessTimeout(const QString &portName, const QString &s);
    void proximityCardInformationProcess(const MessageType &messageType);
private:

    bool veryfyDataPacket(SerialBusActions *serialBusActions,
                          DataRecord **dataRecord,
                          uchar address,
                          uchar command,
                          QString dataPacket);

    inline QString cbufferToQString(const uchar * cbuffer, ulong size) {
        const char *buffer = reinterpret_cast<const char *>(cbuffer);
        QString s;
        QString result;
        for (uint i = 0; i < size; i++) {
            s = QString("%1").arg(buffer[i]);
            result.append(s);
        }
        return result;
    }

private:
    QList<QSerialPortInfo> serialPortInformation_;
    std::unique_ptr<ProximityCardsReaderBusActions> proximityCardsReaderBusActions;
    SerialBusActions *serialBusActions_;
    Settings settings_;
    SerialPortMasterThread serialPortMasterThread_;
    QMutex mutex_;
    int transactionCount_/*{0}*/;
    QString currentRequest_;
    uint queriedNumbers_/*{0}*/;
    const uint MaxQueryToProximityCardReader_/*{5}*/;
};

#endif // SERIALPORTHANDLER_H
