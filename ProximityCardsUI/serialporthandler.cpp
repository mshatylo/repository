#include "serialporthandler.h"
#include <QThread>
#include <QDebug>

#define ONE_MILI_SECOND (5)
#define ONE_HUNDRID_MILI_SECONDS (100)

SerialPortHandler::SerialPortHandler(QObject *parent)
    : QObject(parent),
      serialPortInformation_(QSerialPortInfo::availablePorts()),
      proximityCardsReaderBusActions(new ProximityCardsReaderBusActions),
      transactionCount_(0),
      queriedNumbers_(0),
      MaxQueryToProximityCardReader_(5)
{
    connect(&serialPortMasterThread_, &SerialPortMasterThread::response, this, &SerialPortHandler::serialPortResponse);
    connect(&serialPortMasterThread_, &SerialPortMasterThread::error, this, &SerialPortHandler::serialPortProcessError);
    connect(&serialPortMasterThread_, &SerialPortMasterThread::timeout, this, &SerialPortHandler::serialPortProcessTimeout);
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
    QMutexLocker mutexLocker(&mutex_);
    emit signalSendStatusBarMessage(message);
}


void SerialPortHandler::proximityCardInformation(void) {
    queriedNumbers_ = 0;
    serialPortInformation_ = QSerialPortInfo::availablePorts();
    if (serialPortInformation_.isEmpty())
        return;

    const QSerialPortInfo &info = serialPortInformation_.takeFirst();
    slotStusbarMessage(tr("Status: Running, connecting to port %1.")
                         .arg(info.portName()));
    if (proximityCardsReaderBusActions) {
        currentRequest_ = cbufferToQString(proximityCardsReaderBusActions->PingQuery_
                                           , sizeof(proximityCardsReaderBusActions->PingQuery_));
        serialPortMasterThread_.transaction(info.portName(), ONE_HUNDRID_MILI_SECONDS, currentRequest_, false, true);
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
        if (!proximityCardsReaderBusActions)
            return;
        currentRequest_ = cbufferToQString(proximityCardsReaderBusActions->PingQuery_
                                           , sizeof(proximityCardsReaderBusActions->PingQuery_));
        QThread::msleep(ONE_MILI_SECOND);
        serialPortMasterThread_.transaction(info.portName(), ONE_HUNDRID_MILI_SECONDS, currentRequest_, false, true);
    } else {
        emit signalSendControlsEnabled(true);
        return;
    }
}

void SerialPortHandler::serialPortResponse(const QString &portName, const QString &responce)
{
    if (!proximityCardsReaderBusActions)
        return;
    DataRecord * dataRecord = nullptr;
    if (veryfyDataPacket(proximityCardsReaderBusActions.get(),
                         &dataRecord,
                         currentRequest_.toLatin1().at(proximityCardsReaderBusActions->AddressByteNumber_),
                         currentRequest_.toLatin1().at(proximityCardsReaderBusActions->CommandByteNumber_),
                         responce)) {
        if (dataRecord) {
            QString command = dataRecord->command_;
            QString data = dataRecord->getData();
            delete dataRecord;
            if (dataRecord->command_ == proximityCardsReaderBusActions->PingCommand_ && data.isEmpty()) {
                currentRequest_ = cbufferToQString(proximityCardsReaderBusActions->ResetKeyQuery_
                                                   , sizeof(proximityCardsReaderBusActions->ResetKeyQuery_));
                QThread::msleep(ONE_MILI_SECOND);
                serialPortMasterThread_.transaction(portName, ONE_HUNDRID_MILI_SECONDS, currentRequest_, false, true);
            } else if (dataRecord->command_ == proximityCardsReaderBusActions->ResetKeyCommand_  && data.isEmpty()) {
                currentRequest_ = cbufferToQString(proximityCardsReaderBusActions->KeyQuery_
                                                   , sizeof(proximityCardsReaderBusActions->KeyQuery_));
                QThread::msleep(ONE_MILI_SECOND);
                serialPortMasterThread_.transaction(portName, ONE_HUNDRID_MILI_SECONDS, currentRequest_);
                queriedNumbers_ ++;
            } else if (dataRecord->command_ == proximityCardsReaderBusActions->KeyCommand_) {
                if (dataRecord->data_.isEmpty() && queriedNumbers_ < MaxQueryToProximityCardReader_ ) {
                    currentRequest_ = cbufferToQString(proximityCardsReaderBusActions->KeyQuery_
                                                       , sizeof(proximityCardsReaderBusActions->KeyQuery_));
                    QThread::msleep(ONE_MILI_SECOND);
                    serialPortMasterThread_.transaction(portName, ONE_HUNDRID_MILI_SECONDS, currentRequest_);
                    queriedNumbers_ ++;
                }else if (dataRecord->data_.isEmpty() && queriedNumbers_ == MaxQueryToProximityCardReader_ - 1) {
                    currentRequest_ = cbufferToQString(proximityCardsReaderBusActions->KeyQuery_
                                                       , sizeof(proximityCardsReaderBusActions->KeyQuery_));
                    QThread::msleep(ONE_MILI_SECOND);
                    serialPortMasterThread_.transaction(portName, ONE_HUNDRID_MILI_SECONDS, currentRequest_, true);
                    queriedNumbers_ ++;
                }else if (dataRecord->data_.isEmpty() && queriedNumbers_ == MaxQueryToProximityCardReader_ ) {
                    serialPortProcessError(portName, QString(tr("Didn't reseive key's data")));
                } else {
                    emit signalSendProximityCardInformation(data);
                    emit signalSendProximityCardMessage(MessageType::response_);
                    slotStusbarMessage(tr("Traffic, transaction #%1:"
                                             "\n\r-request: %2"
                                             "\n\r-response: command (%3), data (%4)")
                                       .arg(++transactionCount_)
                                       .arg(currentRequest_)
                                       .arg(command)
                                       .arg(!data.isEmpty() ? data: "error get data"));
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
    emit signalSendProximityCardMessage(MessageType::error_);
    slotStusbarMessage(tr("Status: Not running, %1, port: %2. No traffic.").arg(s).arg(portName));
}

void SerialPortHandler::serialPortProcessTimeout(const QString &portName, const QString &s)
{
    if (!proximityCardsReaderBusActions)
        return;

    uchar command = currentRequest_.at(proximityCardsReaderBusActions->CommandByteNumber_).cell();
    if (command == proximityCardsReaderBusActions->KeyCommand_
        && queriedNumbers_ < MaxQueryToProximityCardReader_ ) {
            QThread::msleep(ONE_MILI_SECOND);
            currentRequest_ = cbufferToQString(proximityCardsReaderBusActions->KeyQuery_
                                               , sizeof(proximityCardsReaderBusActions->KeyQuery_));
        serialPortMasterThread_.transaction(portName, ONE_HUNDRID_MILI_SECONDS, currentRequest_);
        queriedNumbers_ ++;
    } else {
        emit signalSendProximityCardMessage(MessageType::timout_);
        slotStusbarMessage(tr("Status: Running, %1 port: %2. No traffic.").arg(s).arg(portName));
    }
}

