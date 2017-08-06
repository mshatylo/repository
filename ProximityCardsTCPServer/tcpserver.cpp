#include "tcpserver.h"
#include <QtNetwork>
#include <stdlib.h>
//#include <QHBoxLayout>

TCPServer::TCPServer(QObject *parent)
    : QObject(parent),
      tcpServer_(nullptr),
      clientConnection_(nullptr),
      networkSession_(nullptr),
      OperationSuccessful_(0x01),
      OperationUnsuccessful_(0x02)
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
            emit sendStatusMessage(tr("Opening network session."));
            networkSession_->open();
        }
    } else {
        sessionOpened();
    }
    connect(tcpServer_, &QTcpServer::newConnection, this, &TCPServer::receiveMessage);
}

void TCPServer::sessionOpened(void)
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
        emit sendStatusMessage(tr("Unable to start TCP Server: %1.")
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
    emit sendIpAddressAndPort(tr("IP address: %1; port: %2")
                                    .arg(ipAddress).arg(tcpServer_->serverPort()));
    emit sendStatusMessage(tr("TCP server is running on\n\nIP: %1\nport: %2\n\n")
                      .arg(ipAddress).arg(tcpServer_->serverPort()));
}

void TCPServer::receiveMessage(void) {
    clientConnection_ = tcpServer_->nextPendingConnection();
    if (clientConnection_) {
        connect(clientConnection_, &QTcpSocket::readyRead, this, &TCPServer::readMessage);
        connect(clientConnection_, &QAbstractSocket::disconnected, &QObject::deleteLater);
    }
}

void TCPServer::readMessage(void) {
    if (clientConnection_ ) {
        QDataStream in;
        in.setDevice(clientConnection_);
        in.setVersion(QDataStream::Qt_4_0);
        in.startTransaction();
        in >> receivedMessage_;
        if (!in.commitTransaction()) {
            return;
        } else {
            emit sendHistoryMessage(QString("Received data by server %1").arg(QString(receivedMessage_.toLatin1().toHex(' '))));
            emit sendStatusMessage(QString("Received data by server %1").arg(QString(receivedMessage_.toLatin1().toHex(' '))));
            if (verifyServerResponce(receivedMessage_)) {
                QString receivedData = takeData(receivedMessage_);
                emit sendHistoryMessage(QString("Received data by server %1").arg(QString(receivedData.toLatin1().toHex(' '))));
                emit sendStatusMessage(QString("Received data by server %1").arg(QString(receivedData.toLatin1().toHex(' '))));
                sendMessage(createDataPacket(OperationSuccessful_));
            } else {
                sendMessage(createDataPacket(OperationUnsuccessful_));
            }

        }
    }
}

void TCPServer::sendMessage(QString message)
{
    QByteArray block;
    QDataStream outputDataStream(&block, QIODevice::WriteOnly);
    outputDataStream.setVersion(QDataStream::Qt_4_0);
    outputDataStream << message;
    if (clientConnection_) {
        emit sendHistoryMessage(tr("Sent message to client %1").arg(QString(message.toLatin1().toHex(' '))));
        emit sendStatusMessage(tr("Sent message to client %1").arg(QString(message.toLatin1().toHex(' '))));
        clientConnection_->write(block);
        clientConnection_->disconnectFromHost();
    }
}

QString TCPServer::createDataPacket(uchar operationSuccess) {
    const uchar HeaderFirstPart     = 0x55;
    const uchar HeaderSecondPart    = 0x55;
    const uchar Command             = 0x01;
    QString dataPacket_;
    dataPacket_ += HeaderFirstPart;
    dataPacket_ += HeaderSecondPart;
    dataPacket_ += Command;
    dataPacket_ += operationSuccess;
    return dataPacket_;
}

bool TCPServer::verifyServerResponce(const QString &responce) {
    const uchar HeaderFirstPart             = 0x55;
    const uchar HeaderSecondPart            = 0x55;
    const uchar Command                     = 0x01;

    const int HeaderFirstPartIndex          = 0;
    const int HeaderSecondPartIndex         = 1;
    const int CommandIndex                  = 2;

    if (responce.isEmpty())
        return false;

    for (int i = 0; i < responce.size(); i++) {
        switch(i){
            case HeaderFirstPartIndex : {
                if (responce.at(i).cell() != HeaderFirstPart)
                    return false;
                break;
            } case HeaderSecondPartIndex : {
                if (responce.at(i).cell() != HeaderSecondPart)
                    return false;
                break;
            } case CommandIndex : {
                if (responce.at(i).cell() != Command)
                    return false;
                break;
            } default: {
                break;
            }
        }
    }
    return true;
}

QString TCPServer::takeData(const QString &responce) {
    const int MaxHeaderIndex = 2;
    QString result;
    for (int i = 0; i < responce.size(); i++) {
        if (i > MaxHeaderIndex)
            result += responce.at(i);
    }
    return result;
}
