#ifndef SERIAL_PORT_MASTER_THREAD_H
#define SERIAL_PORT_MASTER_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#define ONE_HUNDRID_MILI_SECONDS            (100)
#define MAX_QUERY_TO_PROXIMITY_CARD_READER  (5)

struct Settings {
    Settings();
    Settings & operator=(const Settings &other) = default;
    bool operator==(const Settings &other);

    int parity_;
    int baudRate_;
    int dataBits_;
    int stopBits_;
    int responseTime_;
    uint numberOfRetries_;
};

class SerialPortMasterThread : public QThread
{
    Q_OBJECT

public:
    explicit SerialPortMasterThread(QObject *parent = nullptr);
    ~SerialPortMasterThread();

    void transaction(const QString &portName,
                     Settings settings,
                     const QString &request,
                     bool closeSerial = false,
                     bool closeSerialInTimeOut = false);
    void run() Q_DECL_OVERRIDE;

signals:
    void response(const QString &portName_, const QString &s);
    void error(const QString &portName_, const QString &s);
    void timeout(const QString &portName_, const QString &s);
    void sendStatusBarMessage(QString message);

private:
    QString portName_;
    QString request_;
    Settings settings_;
    QMutex mutex_;
    QWaitCondition waitCondition_;
    bool quit_;
    bool closeSerial_;
    bool closeSerialInTimeOut_;
};

#endif // SERIAL_PORT_MASTER_THREAD_H
