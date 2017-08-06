#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
//    ipAddressPort_(new QLineEdit),
//    serialMessagesHistory_(new QTextEdit),
    tcpServer_(new TCPServer),
    ui_(new Ui::MainWindow)
{
    if (ui_) {

        ui_->setupUi(this);
        init();
        setWindowTitle(tr("Proximity cards TCP Server"));
        setWindowIcon(QIcon("://Images/network.ico"));
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::init(void) {
    if (tcpServer_) {
        connect(tcpServer_.get(), &TCPServer::sendStatusMessage, [this](QString message){
            statusBar()->clearMessage();
            statusBar()->showMessage(message);
        });
        connect(tcpServer_.get(), &TCPServer::sendStatusMessage, ui_->messagesHistory_, &QTextEdit::append);
        connect(tcpServer_.get(), &TCPServer::sendIpAddressAndPort, [this](QString message){
            ui_->ipAddressAndPort_->setText(message);
        });
        tcpServer_->init();
    }
}


