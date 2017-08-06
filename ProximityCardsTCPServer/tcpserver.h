#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QNetworkSession>

class TCPServer : public QObject
{
    Q_OBJECT
public:
    explicit TCPServer(QObject *parent = nullptr);

    void init(void);

signals:
    void sendIpAddressAndPort(QString ipAddressAndPort);
    void sendHistoryMessage(QString message);
    void sendStatusMessage(QString message);
public slots:
private slots:
    void sessionOpened(void);
    void receiveMessage(void);
    void readMessage(void);
    void sendMessage(QString message);

private:
    bool verifyServerResponce(const QString &responce);
    QString createDataPacket(uchar operationSuccess);
    QString takeData(const QString &responce);

    QString status_;
    QTcpServer *tcpServer_;
    QTcpSocket *clientConnection_;
    QNetworkSession *networkSession_;
    QString receivedMessage_;
    const uchar OperationSuccessful_;
    const uchar OperationUnsuccessful_;
};

#endif // TCPSERVER_H
