#ifndef SERIAL_PORT_MASTER_THREAD_H
#define SERIAL_PORT_MASTER_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class SerialPortMasterThread : public QThread
{
    Q_OBJECT

public:
    explicit SerialPortMasterThread(QObject *parent = nullptr);
    ~SerialPortMasterThread();

    void transaction(const QString &portName,
                     int waitTimeout,
                     const QString &request,
                     bool closeSerial = false,
                     bool closeSerialInTimeOut = false);
    void run() Q_DECL_OVERRIDE;

signals:
    void response(const QString &portName_, const QString &s);
    void error(const QString &portName_, const QString &s);
    void timeout(const QString &portName_, const QString &s);

private:
    QString portName_;
    QString request_;
    int waitTimeout_;
    QMutex mutex_;
    QWaitCondition waitCondition_;
    bool quit_;
    bool closeSerial_;
    bool closeSerialInTimeOut_;
};

#endif // SERIAL_PORT_MASTER_THREAD_H
