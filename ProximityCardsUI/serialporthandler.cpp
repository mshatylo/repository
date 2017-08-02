#include "serialporthandler.h"
#include <QThread>
#include <QDebug>
#include "tracing.h"

#define ONE_HUNDRED_FIFTY_MILI_SECOND (150)
#define FIVE_MILI_SECOND                (5)

SerialPortHandler::SerialPortHandler(QObject *parent)
    : QObject(parent),
      serialPortInformation_(QSerialPortInfo::availablePorts()),
      proximityCardsReaderBusActions_(new ProximityCardsReaderBusActions),
      transactionCount_(0),
      queriedNumbers_(0),
      stopProcessCardReader_(false)
{
    connect(&serialPortMasterThread_, &SerialPortMasterThread::response, this, &SerialPortHandler::serialPortResponse);
    connect(&serialPortMasterThread_, &SerialPortMasterThread::error, this, &SerialPortHandler::serialPortProcessError);
    connect(&serialPortMasterThread_, &SerialPortMasterThread::timeout, this, &SerialPortHandler::serialPortProcessTimeout);
    connect(&serialPortMasterThread_, &SerialPortMasterThread::sendStatusBarMessage, this, &SerialPortHandler::slotStusbarMessage);
    connect(this, &SerialPortHandler::signalSendProximityCardMessage, this, &SerialPortHandler::proximityCardInformationProcess);
}

QString SerialPortHandler::getSerialPortInformation(void) {
    serialPortInformation_ = QSerialPortInfo::availablePorts();
    QString serialPortInformationString;
    for (const QSerialPortInfo &info : serialPortInformation_) {
        serialPortInformationString += "\n" + QObject::tr("Port: ") + info.portName() + "\n"
                    + QObject::tr("Location: ") + info.systemLocation() + "\n"
                    + QObject::tr("Description: ") + info.description() + "\n"
                    + QObject::tr("Manufacturer: ") + info.manufacturer() + "\n"
                    + QObject::tr("Serial number: ") + info.serialNumber() + "\n"
                    + QObject::tr("Vendor Identifier: ") + (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Product Identifier: ") + (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Busy: ") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) + "\n";
    }
    return serialPortInformationString;
}

void SerialPortHandler::setSerialSettings(int parity, int baudRate, int dataBits, int stopBits,
                       int responseTime,
                       int numberOfRetries) {
    settings_.parity_ = parity;
    settings_.baudRate_ = baudRate;
    settings_.dataBits_ = dataBits;
    settings_.stopBits_ = stopBits;
    settings_.responseTime_ = responseTime;
    settings_.numberOfRetries_ = numberOfRetries;
}

bool SerialPortHandler::veryfyDataPacket(SerialBusActions *serialBusActions,
                                         DataRecord **dataRecord,
                                         uchar address,
                                         uchar command,
                                         QString dataPacket) {
    serialBusActions_ = serialBusActions;
    *dataRecord = static_cast<DataRecord *>(serialBusActions_->getDataRomRTUFrame(address, command, dataPacket));
    if (*dataRecord)
        return true;
    else
        return false;
}

void SerialPortHandler::slotStusbarMessage(QString message) {
    QMutexLocker ml(&statusBarMessageMutex_);
    emit signalSendStatusBarMessage(message);
}


void SerialPortHandler::proximityCardInformation(void) {
    queriedNumbers_ = 0;
    stopProcessCardReader_ = false;
    serialPortInformation_ = QSerialPortInfo::availablePorts();
    if (serialPortInformation_.isEmpty())
        return;

    const QSerialPortInfo &info = serialPortInformation_.takeFirst();
    slotStusbarMessage(tr("Status: Running, connecting to port %1.")
                         .arg(info.portName()));
    if (proximityCardsReaderBusActions_) {
        currentRequest_ = cbufferToQString(proximityCardsReaderBusActions_->PingQuery_
                                           , sizeof(proximityCardsReaderBusActions_->PingQuery_));
        serialPortMasterThread_.transaction(info.portName(), settings_, currentRequest_, false, true);
    }
    emit signalSendControlsEnabled(false);
}

void SerialPortHandler::proximityCardInformationProcess(const MessageType &messageType) {
    if (MessageType::error_ == messageType || MessageType::timout_ == messageType ) {
        if (serialPortInformation_.isEmpty()) {
            emit signalSendControlsEnabled(true);
            return;
        }

        const QSerialPortInfo &info = serialPortInformation_.takeFirst();

        if (!proximityCardsReaderBusActions_)
            return;
        queriedNumbers_ = 0;
        currentRequest_ = cbufferToQString(proximityCardsReaderBusActions_->PingQuery_
                                           , sizeof(proximityCardsReaderBusActions_->PingQuery_));
        QThread::msleep(FIVE_MILI_SECOND);
        serialPortMasterThread_.transaction(info.portName(), settings_, currentRequest_, false, true);
    } else {
        emit signalSendControlsEnabled(true);
        return;
    }
}

void SerialPortHandler::serialPortResponse(const QString &portName, const QString &responce)
{
    QMutexLocker mutexLocker(&mutex_);
    if (stopProcessCardReader_) {
        emit signalSendProximityCardMessage(MessageType::response_);
        slotStusbarMessage(tr("User stop!"));
        return;
    }
    if (!proximityCardsReaderBusActions_)
        return;
    DataRecord * dataRecord = nullptr;
    if (veryfyDataPacket(proximityCardsReaderBusActions_.get(),
                         &dataRecord,
                         currentRequest_.toLatin1().at(proximityCardsReaderBusActions_->AddressByteNumber_),
                         currentRequest_.toLatin1().at(proximityCardsReaderBusActions_->CommandByteNumber_),
                         responce)) {
        if (dataRecord) {
            uchar command = dataRecord->command_;
            QString data = dataRecord->getData();
            delete dataRecord;
            if (command == proximityCardsReaderBusActions_->PingCommand_ && data.isEmpty()) {
                currentRequest_ = cbufferToQString(proximityCardsReaderBusActions_->ResetKeyQuery_
                                                   , sizeof(proximityCardsReaderBusActions_->ResetKeyQuery_));
                QThread::msleep(FIVE_MILI_SECOND);
                serialPortMasterThread_.transaction(portName, settings_, currentRequest_, false, true);
            } else if (command == proximityCardsReaderBusActions_->ResetKeyCommand_  && data.isEmpty()) {
                currentRequest_ = cbufferToQString(proximityCardsReaderBusActions_->KeyQuery_
                                                   , sizeof(proximityCardsReaderBusActions_->KeyQuery_));
                QThread::msleep(FIVE_MILI_SECOND);
                serialPortMasterThread_.transaction(portName, settings_, currentRequest_);
                queriedNumbers_ ++;
            } else if (command == proximityCardsReaderBusActions_->KeyCommand_) {
                if (data.isEmpty() && queriedNumbers_ < settings_.numberOfRetries_ - 1) {
                    currentRequest_ = cbufferToQString(proximityCardsReaderBusActions_->KeyQuery_
                                                       , sizeof(proximityCardsReaderBusActions_->KeyQuery_));
                    QThread::msleep(FIVE_MILI_SECOND);
                    serialPortMasterThread_.transaction(portName, settings_, currentRequest_);
                    queriedNumbers_ ++;
                }else if (data.isEmpty() && queriedNumbers_ == settings_.numberOfRetries_ - 1) {
                    currentRequest_ = cbufferToQString(proximityCardsReaderBusActions_->KeyQuery_
                                                       , sizeof(proximityCardsReaderBusActions_->KeyQuery_));
                    QThread::msleep(FIVE_MILI_SECOND);
                    serialPortMasterThread_.transaction(portName, settings_, currentRequest_, true);
                    queriedNumbers_ ++;
                }else if (data.isEmpty() && queriedNumbers_ == settings_.numberOfRetries_ ) {
                    queriedNumbers_ = 0;
                } else {
                    lastReadCardData_ = data.toLatin1();
                    QByteArray lastReadCardDataInversion;
                    for  (auto it = lastReadCardData_.crbegin(); it != lastReadCardData_.crend(); it ++ )
                        lastReadCardDataInversion.append(*it);
                    emit signalSendProximityCardInformation(QString(lastReadCardDataInversion.toHex(' ')));
                    emit signalSendProximityCardMessage(MessageType::response_);
                    slotStusbarMessage(tr("Traffic, transaction #%1:"
                                             "\n\r-request: %2"
                                             "\n\r-response: command (%3), data (%4)")
                                       .arg(++transactionCount_)
                                       .arg(currentRequest_)
                                       .arg(!lastReadCardDataInversion.isEmpty() ? QString(lastReadCardDataInversion.toHex(' ')) : "error get data"));
                }
            } else {
                serialPortProcessError(portName, QString(tr("Algorithm sequence error!")));
            }
        }
    } else {
        serialPortProcessError(portName, QString(tr("Modbus RTU CRC16 is not correct!")));
    }
}

void SerialPortHandler::serialPortProcessError(const QString &portName, const QString &s)
{
    QMutexLocker mutexLocker(&mutex_);
    if (stopProcessCardReader_) {
        emit signalSendProximityCardMessage(MessageType::response_);
        slotStusbarMessage(tr("User stop!"));
        return;
    }
    emit signalSendProximityCardMessage(MessageType::error_);
    slotStusbarMessage(tr("Status: Not running, %1, port: %2. No traffic.").arg(s).arg(portName));
}

void SerialPortHandler::serialPortProcessTimeout(const QString &portName, const QString &s)
{
    QMutexLocker mutexLocker(&mutex_);
    if (stopProcessCardReader_) {
        emit signalSendProximityCardMessage(MessageType::response_);
        slotStusbarMessage(tr("User stop!"));
        return;
    }
    if (!proximityCardsReaderBusActions_)
        return;

    uchar command = currentRequest_.at(proximityCardsReaderBusActions_->CommandByteNumber_).cell();
    if (command == proximityCardsReaderBusActions_->KeyCommand_
        && queriedNumbers_ < settings_.numberOfRetries_) {

        currentRequest_ = cbufferToQString(proximityCardsReaderBusActions_->KeyQuery_
                                           , sizeof(proximityCardsReaderBusActions_->KeyQuery_));
        QThread::msleep(ONE_HUNDRED_FIFTY_MILI_SECOND);
        serialPortMasterThread_.transaction(portName, settings_, currentRequest_);
        queriedNumbers_ ++;
#ifdef TRACE
         slotStusbarMessage(tr("SerialPortProcessTimeout slot, queriedNumbers: %1, settings_.numberOfRetries_: %2, port: %3. No traffic.")
                            .arg(queriedNumbers_).arg(settings_.numberOfRetries_).arg(portName));
#endif
    } else {
        emit signalSendProximityCardMessage(MessageType::timout_);
        slotStusbarMessage(tr("Status: Running, %1 port: %2. No traffic.").arg(s).arg(portName));
    }
}

void SerialPortHandler::stopProximityCard(void) {
    QMutexLocker mutexLocker(&mutex_);
    stopProcessCardReader_ = true;
}


