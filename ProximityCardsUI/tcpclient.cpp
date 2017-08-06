#include "tcpclient.h"
#include <QtNetwork>

TCPClient::TCPClient(QObject *parent)
    : QObject(parent),
      tcpSocket_(new QTcpSocket(this)),
      networkSession_(nullptr)
{

}

void TCPClient::init(QComboBox &ipAddressesComboBox) {
    fillIpAddressesComboBox(ipAddressesComboBox);
    if (tcpSocket_) {
        connect(tcpSocket_, &QIODevice::readyRead, this, &TCPClient::readMessage);
        connect(tcpSocket_, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
                this, &TCPClient::error);
    }
}

void TCPClient::fillIpAddressesComboBox(QComboBox &ipAddressesComboBox) {
    ipAddressesComboBox.setEditable(true);
    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        ipAddressesComboBox.addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            ipAddressesComboBox.addItem(name + QChar('.') + domain);
    }
    if (name != QLatin1String("localhost"))
        ipAddressesComboBox.addItem(QString("localhost"));
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback())
            ipAddressesComboBox.addItem(ipAddressesList.at(i).toString());
    }
    // add localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).isLoopback())
            ipAddressesComboBox.addItem(ipAddressesList.at(i).toString());
    }
}

void TCPClient::initNetworkSession(void) {
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
        connect(networkSession_, &QNetworkSession::opened, this, &TCPClient::sessionOpened);

        emit sendEnableAddProximityCardToControllerButton(true);
        emit signalSendStatusBarMessage(tr("Opening network session."));
        networkSession_->open();
    }
}

void TCPClient::setHostIpAddress(const QString &ipAddress) {
    hostIpAddress_ = ipAddress;
}

void TCPClient::setHostPort(const QString &port) {
    hostPort_ = port;
}

void TCPClient::error(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError: {
            break;
        } case QAbstractSocket::HostNotFoundError: {
            emit signalErrorMessage(tr("Proximity Cards UI"),
                                    tr("The host was not found. Please check the "
                                    "host name and port settings."));
            break;
        } case QAbstractSocket::ConnectionRefusedError: {
            emit signalErrorMessage(tr("Proximity Cards UI"),
                                    tr("The connection was refused by the peer. "
                                    "Make sure the ProximityCardsUI is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
            break;
        } default: {
            emit signalErrorMessage(tr("Proximity Cards UI"),
                                    tr("The following error occurred: %1.")
                                    .arg(tcpSocket_->errorString()));
        }
    }
     emit sendEnableAddProximityCardToControllerButton(true);
}

void TCPClient::sessionOpened() {
    // Save the used configuration
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
    emit signalSendStatusBarMessage(tr("Require that you run the Proximity Cards UI Server as well."));

}

void TCPClient::readMessage()
{
    QDataStream inputDataStream;
    inputDataStream.setDevice(tcpSocket_);
    inputDataStream.setVersion(QDataStream::Qt_4_0);
    inputDataStream.startTransaction();
    QString responce;
    inputDataStream >> responce;
    if (!inputDataStream.commitTransaction()) {
        emit signalSendStatusBarMessage(tr("Require that you run the Proximity Cards UI Server as well."));
        emit sendEnableAddProximityCardToControllerButton(true);
        return;
    }
    if (verifyServerResponce(responce))
        emit signalSendStatusBarMessage("Data was added successfully by server!");
    else
        emit signalSendStatusBarMessage("Data was not added successfully by server!");
    emit sendEnableAddProximityCardToControllerButton(true);
}

void TCPClient::sendMessage(const QString &cardData) {
    if (hostIpAddress_.isEmpty() || hostPort_.isEmpty()) {
        emit signalSendStatusBarMessage("Please add host IP address or host port!");
        return;
    }

    emit sendEnableAddProximityCardToControllerButton(false);
    QString dataPacket = createDataPacket(cardData);
    tcpSocket_->abort();
    if (!hostIpAddress_.isEmpty() && !hostPort_.isEmpty()
        && !dataPacket.isEmpty()) {
        QByteArray request;
        QDataStream outputDataStream(&request, QIODevice::WriteOnly);
        outputDataStream.setVersion(QDataStream::Qt_4_0);
        outputDataStream << dataPacket;
        tcpSocket_->connectToHost(hostIpAddress_, hostPort_.toInt());
        if (tcpSocket_) {
            tcpSocket_->write(request);
        }
    } else {
        emit sendEnableAddProximityCardToControllerButton(true);
    }
}

QString TCPClient::createDataPacket(const QString &cardData) {
    const uchar HeaderFirstPart     = 0x55;
    const uchar HeaderSecondPart    = 0x55;
    const uchar Command             = 0x01;
    QString dataPacket_;
    dataPacket_ += HeaderFirstPart;
    dataPacket_ += HeaderSecondPart;
    dataPacket_ += Command;
    dataPacket_ += cardData;
    return dataPacket_;
}

bool TCPClient::verifyServerResponce(const QString &responce) {
    const uchar HeaderFirstPart             = 0x55;
    const uchar HeaderSecondPart            = 0x55;
    const uchar Command                     = 0x01;
    const uchar OperationSuccessful         = 0x01;
    const uchar OperationUnsuccessful       = 0x02;

    Q_UNUSED(OperationUnsuccessful)

    const int HeaderFirstPartIndex          = 0;
    const int HeaderSecondPartIndex         = 1;
    const int CommandIndex                  = 2;
    const int OperationSuccessfulIndex     = 3;

    const int ServerResponceSize            = 4;

    if (responce.isEmpty())
        return false;
    if (responce.size() != ServerResponceSize)
        return false;

    for (int i = 0; i < responce.size(); i++) {
        uchar currentElement = responce.at(i).cell();
        switch(i){
            case HeaderFirstPartIndex : {
            if (currentElement != HeaderFirstPart)
                    return false;
                break;
            } case HeaderSecondPartIndex : {
                if (currentElement != HeaderSecondPart)
                    return false;
                break;
            } case CommandIndex : {
                if (currentElement != Command)
                    return false;
                break;
            } case OperationSuccessfulIndex : {
                if (currentElement != OperationSuccessful)
                    return false;
                break;
            } default : {
                return false;
            }
        }
    }
    return true;
}
