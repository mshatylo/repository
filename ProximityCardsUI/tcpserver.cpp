#include "tcpserver.h"
#include <QtNetwork>
#include <stdlib.h>
//#include <QHBoxLayout>

TCPServer::TCPServer(QObject *parent)
    : QObject(parent),
      tcpServer_(nullptr),
      networkSession_(nullptr)
{
}

void TCPServer::init(void) {
    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("ProximityCardsUI Ltd"));
        settings.beginGroup(QLatin1String("ProximityCardsUINetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }
        networkSession_ = new QNetworkSession(config, this);
        if (networkSession_) {
            connect(networkSession_, &QNetworkSession::opened, this, &TCPServer::sessionOpened);
            emit signalSendStatusBarMessage(tr("Opening network session."));
            networkSession_->open();
        }
    } else {
        sessionOpened();
    }

    listMessages_ << tr("You've been leading a dog's life. Stay off the furniture.")
             << tr("You've got to think about tomorrow.")
             << tr("You will be surprised by a loud noise.")
             << tr("You will feel hungry again in another hour.")
             << tr("You might have mail.")
             << tr("You cannot kill time without injuring eternity.")
             << tr("Computers are not intelligent. They only think they are.");
    connect(tcpServer_, &QTcpServer::newConnection, this, &TCPServer::sendMessage);
}

void TCPServer::sessionOpened()
{
    // Save the used configuration
    if (networkSession_) {
        QNetworkConfiguration config = networkSession_->configuration();
        QString id;
        if (config.type() == QNetworkConfiguration::UserChoice)
            id = networkSession_->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
        else
            id = config.identifier();

        QSettings settings(QSettings::UserScope, QLatin1String("ProximityCardsUI Ltd"));
        settings.beginGroup(QLatin1String("ProximityCardsUINetwork"));
        settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
        settings.endGroup();
    }

    tcpServer_ = new QTcpServer(this);
    if (!tcpServer_->listen()) {
        emit signalSendStatusBarMessage(tr("Unable to start TCP Server: %1.")
                                        .arg(tcpServer_->errorString()));
        return;
    }
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    emit signalSendStatusBarMessage(tr("TCP server is running on\n\nIP: %1\nport: %2\n\n")
                                    .arg(ipAddress).arg(tcpServer_->serverPort()));
}

void TCPServer::sendMessage()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << listMessages_.at(qrand() % listMessages_.size());

    QTcpSocket *clientConnection = tcpServer_->nextPendingConnection();
    connect(clientConnection, &QAbstractSocket::disconnected,
            clientConnection, &QObject::deleteLater);

    clientConnection->write(block);
    clientConnection->disconnectFromHost();
}
