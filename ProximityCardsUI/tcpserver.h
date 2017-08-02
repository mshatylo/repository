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
    void signalSendStatusBarMessage(QString message);
public slots:
private slots:
    void sessionOpened();
    void sendMessage();
private:
    QString status_;
    QTcpServer *tcpServer_;
    QStringList listMessages_;
    QNetworkSession *networkSession_;
};

#endif // TCPSERVER_H
