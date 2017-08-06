#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QDataStream>
#include <QNetworkSession>
#include <QComboBox>

class TCPClient : public QObject
{
    Q_OBJECT
public:
    explicit TCPClient(QObject *parent = nullptr);

    void init(QComboBox &ipAddressesComboBox);
signals:
    void signalErrorMessage(const QString &title, const QString &message);
    void sendEnableAddProximityCardToControllerButton(bool);
    void signalSendStatusBarMessage(const QString &message);

public slots:
    void setHostIpAddress(const QString &ipAddress);
    void setHostPort(const QString &port);
    void sendMessage(const QString &cardData);

private slots:
    void readMessage(void);
    void error(QAbstractSocket::SocketError socketError);
    void sessionOpened(void);

private:
    void fillIpAddressesComboBox(QComboBox &ipAddressesComboBox);
    void initNetworkSession(void);
    QString createDataPacket(const QString &cardData);
    bool verifyServerResponce(const QString &responce);

    QString hostIpAddress_;
    QString hostPort_;
    QTcpSocket *tcpSocket_;
    QNetworkSession *networkSession_;
};

#endif // TCPCLIENT_H
