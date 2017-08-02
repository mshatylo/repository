#include "serialportmasterthread.h"
#include <QtSerialPort/QSerialPort>
#include <QTime>
#include <QDebug>
#include "tracing.h"

#define TEN_MILISECONDS (10)


Settings::Settings()
    : parity_(QSerialPort::NoParity),
      baudRate_(QSerialPort::Baud19200),
      dataBits_(QSerialPort::Data8),
      stopBits_(QSerialPort::OneStop),
      responseTime_(ONE_HUNDRID_MILI_SECONDS),
      numberOfRetries_(MAX_QUERY_TO_PROXIMITY_CARD_READER)
{}

bool Settings::operator==(const Settings &other) {
    return  this->parity_ == other.parity_
            && this->baudRate_ == other.baudRate_
            && this->dataBits_ == other.dataBits_
            && this->stopBits_ == other.stopBits_
            && this->responseTime_ == other.responseTime_
            && this->numberOfRetries_ == other.numberOfRetries_
            ? true : false;
}

SerialPortMasterThread::SerialPortMasterThread(QObject *parent)
    : QThread(parent),
      quit_(false),
      closeSerial_(false),
      closeSerialInTimeOut_(false)
{
}

SerialPortMasterThread::~SerialPortMasterThread()
{
    mutex_.lock();
    quit_ = true;
    waitCondition_.wakeOne();
    mutex_.unlock();
    wait();
}

void SerialPortMasterThread::transaction(const QString &portName,
                                         Settings settings,
                                         const QString &request,
                                         bool closeSerial,
                                         bool closeSerialInTimeOut)
{
    QMutexLocker locker(&mutex_);
    portName_ = portName;
    settings_ = settings;
    request_ = request;
    closeSerial_ = closeSerial;
    closeSerialInTimeOut_ = closeSerialInTimeOut;
    if (!isRunning())
        start();
    else
        waitCondition_.wakeOne();
}

void SerialPortMasterThread::run()
{
    bool currentPortNameOrSettingsChanged = false;
    bool serialIsClosed = true;


    mutex_.lock();
    QString currentPortName;
    Settings settings;
    if (currentPortName != portName_ || !(settings == settings_)) {
        currentPortName = portName_;
        settings = settings_;
        currentPortNameOrSettingsChanged = true;
    }
    int currentWaitTimeout = settings_.responseTime_;
    QString currentRequest = request_;
    mutex_.unlock();

    QSerialPort serial;

    while (!quit_) {
        if (currentPortNameOrSettingsChanged) {
            if (!serialIsClosed) {
                serial.close();
                serialIsClosed = true;
            }
            serial.setPortName(currentPortName);
            serial.setParity(static_cast<QSerialPort::Parity>(settings.parity_));
            serial.setBaudRate(static_cast<QSerialPort::BaudRate>(settings.baudRate_));
            serial.setDataBits(static_cast<QSerialPort::DataBits>(settings.dataBits_));
            serial.setStopBits(static_cast<QSerialPort::StopBits>(settings.stopBits_));
            currentWaitTimeout = settings.responseTime_;
        }
        if (serialIsClosed) {
            if (!serial.open(QIODevice::ReadWrite)) {
                emit error(serial.portName(), tr("Can't open %1, error code %2")
                           .arg(portName_).arg(serial.error()));
                return;
            } else {
                serialIsClosed = false;
            }
        }

        // write request
        QByteArray requestData = currentRequest.toLatin1();
        serial.write(requestData);
        if (serial.waitForBytesWritten(settings.responseTime_)) {
            // read response
            if (serial.waitForReadyRead(currentWaitTimeout)) {
                QByteArray responseData = serial.readAll();
                while (serial.waitForReadyRead(TEN_MILISECONDS)) {
                    responseData += serial.readAll();
                }
                QString responseDataString;
                for (int i = 0; i < responseData.size(); i++ )
                    responseDataString += (responseData.at(i));
                qDebug() << responseDataString;
                emit response(serial.portName(), responseDataString);
            } else {
                emit timeout(serial.portName(), tr("Wait read response timeout %1")
                             .arg(QTime::currentTime().toString()));
                mutex_.lock();
                if (closeSerialInTimeOut_) {
                    serial.close();
                    closeSerialInTimeOut_ = false;
                    serialIsClosed = true;
                }
                mutex_.unlock();
            }
        } else {
            emit timeout(serial.portName(), tr("Wait write request timeout %1")
                         .arg(QTime::currentTime().toString()));
            mutex_.lock();
            if (closeSerialInTimeOut_) {
                serial.close();
                closeSerialInTimeOut_ = false;
                serialIsClosed = true;
            }
            mutex_.unlock();
        }

        mutex_.lock();
        if (closeSerial_) {
            serial.close();
            closeSerial_ = false;
            serialIsClosed = true;
        }
#ifdef TRACE
        emit sendStatusBarMessage("waitCondition_.wait(&mutex_)");
#endif
        waitCondition_.wait(&mutex_);
#ifdef TRACE
        emit sendStatusBarMessage("waitCondition_.wait(&mutex_) is done! Thread executes.");
#endif
        if (currentPortName != portName_ || !(settings == settings_)) {
            currentPortName = portName_;
            settings = settings_;
            currentPortNameOrSettingsChanged = true;
        } else {
            currentPortNameOrSettingsChanged = false;
        }
        currentWaitTimeout = settings_.responseTime_;
        currentRequest = request_;
        mutex_.unlock();
    }
}
