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

    enum class MessageType {
        response_,
        timout_,
        error_
    };

    explicit SerialPortHandler(QObject *parent = nullptr);

    QString getSerialPortInformation(void);
    void proximityCardInformation(void);
    void setSerialSettings(int parity, int baudRate, int dataBits, int stopBits,
                           int responseTime = ONE_HUNDRID_MILI_SECONDS,
                           int numberOfRetries = MAX_QUERY_TO_PROXIMITY_CARD_READER);
    QString getLastCardData(void) const;

signals:
    void signalSendControlsEnabled(bool enable);
    void signalSendAddProximityCardToControllerButtonEnabled(bool enable);
    void signalSendStatusBarMessage(QString message);
    void signalSendProximityCardInformation(const QString &cardData);
    void signalSendProximityCardMessage(const MessageType &messageType);
    void sendDataByNetwork(const QString &data);

public slots:
    void slotStusbarMessage(QString message);
    void proximityCardInformationProcess(const MessageType &messageType);
    void serialPortResponse(const QString &portName, const QString &responce);
    void serialPortProcessError(const QString &portName, const QString &s);
    void serialPortProcessTimeout(const QString &portName, const QString &s);
    void stopProximityCardGetting(void);
    void findCardReader(void);
    void readCardData(void);
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
    std::unique_ptr<ProximityCardsReaderBusActions> proximityCardsReaderBusActions_;
    SerialBusActions *serialBusActions_;
    Settings settings_;
    SerialPortMasterThread serialPortMasterThread_;
    QMutex mutex_;
    QMutex statusBarMessageMutex_;
    int transactionCount_;
    QString currentRequest_;
    uint queriedNumbers_;
    QByteArray lastReadCardData_;
    QString foundCardReaderPortName_;
    bool stopProcessCardReader_;
    bool stoppedProcessCardReader_;
    bool isNeededToFindCardReader_;
    bool isCardReaderFound_;
    bool isNeededToReadCardData_;
    bool isCardDataRead_;
    bool isNeededToSendDataByNetwork_;
};

#endif // SERIALPORTHANDLER_H
