#include "serialportmasterthread.h"
#include <QtSerialPort/QSerialPort>
#include <QTime>

#define TEN_MILISECONDS (10)
SerialPortMasterThread::SerialPortMasterThread(QObject *parent)
    : QThread(parent),
      waitTimeout_(0),
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
                                         int waitTimeout,
                                         const QString &request,
                                         bool closeSerial,
                                         bool closeSerialInTimeOut)
{
    QMutexLocker locker(&mutex_);
    this->portName_ = portName;
    this->waitTimeout_ = waitTimeout;
    this->request_ = request;
    closeSerial_ = closeSerial;
    closeSerialInTimeOut_ = closeSerialInTimeOut;
    if (!isRunning())
        start();
    else
        waitCondition_.wakeOne();
}

void SerialPortMasterThread::run()
{
    bool currentPortNameChanged = false;
    bool serialIsClosed = true;

    mutex_.lock();
    QString currentPortName;
    if (currentPortName != portName_) {
        currentPortName = portName_;
        currentPortNameChanged = true;
    }
    int currentWaitTimeout = waitTimeout_;
    QString currentRequest = request_;
    mutex_.unlock();

    QSerialPort serial;

    while (!quit_) {
        if (currentPortNameChanged) {
            if (!serialIsClosed) {
                serial.close();
                serialIsClosed = true;
            }
            serial.setPortName(currentPortName);
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
        QByteArray requestData = currentRequest.toLocal8Bit();
        serial.write(requestData);
        if (serial.waitForBytesWritten(waitTimeout_)) {
            // read response
            if (serial.waitForReadyRead(currentWaitTimeout)) {
                QByteArray responseData = serial.readAll();
                while (serial.waitForReadyRead(TEN_MILISECONDS)) {
                    responseData += serial.readAll();
                }
                QString responseDataString(responseData);
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
        mutex_.unlock();

        mutex_.lock();
        waitCondition_.wait(&mutex_);

        if (currentPortName != portName_) {
            currentPortName = portName_;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = waitTimeout_;
        currentRequest = request_;
        mutex_.unlock();
    }
}
