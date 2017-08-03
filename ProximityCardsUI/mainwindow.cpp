#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGridLayout>
#include <QMessageBox>
#include <QDebug>

#define GRID_LAYOUT_FIRST_ROW_INDEX         (0)
#define GRID_LAYOUT_SECOND_ROW_INDEX        (1)
#define GRID_LAYOUT_THIRD_ROW_INDEX         (2)
#define GRID_LAYOUT_FOURTH_ROW_INDEX        (3)

#define GRID_LAYOUT_FIRST_COLUMN_INDEX      (0)
#define GRID_LAYOUT_SECOND_COLUMN_INDEX     (1)
#define GRID_LAYOUT_THIRD_COLUMN_INDEX      (2)

#define NO_ROWS_SPAN                        (1)
#define TWO_COLUMNS_SPAN                    (2)
#define THREE_COLUMNS_SPAN                  (3)

#define SHIFT_WINDOW_WIDTH                  (10)
#define SHIFT_WINDOW_HEIGHT                 (30)

#define WINDOW_WIDTH                        (500)
#define WINDOW_HEIGHT                       (700)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    findReaderButton_(new QPushButton(tr("Find proximity card reader"))),
    readProximityCardButton_(new QPushButton(tr("Read proximity card"))),
    stopProximityCardButton_(new QPushButton()),
    addProximityCardToDbButton_(new QPushButton(tr("Add to DB"))),
    serialPortInformation_(new /*QLabel*/QTextEdit(tr("Serial port information"))),
    proximityCardInformation_(new QTextEdit("HELLO EVERYONE")),
    ipAddressPort_(new QTextEdit),
    serialMessagesHistory_(new QTextEdit),
    serialPortHandler_(new SerialPortHandler),
    sqlHandler_(new SqlHandler("localhost", "root", "olhasham2012", "ProximityCardsDatabase", "ProximityCardsTable")),
    tcpServer_(new TCPServer),
    ui_(new Ui::MainWindow)
{
    if (ui_) {
        ui_->setupUi(this);
        QWidget *mainWidget = new QWidget;
        QGridLayout *mainLayout = new QGridLayout;

        if (mainWidget && mainLayout) {

            if (findReaderButton_) {
                findReaderButton_->setToolTip(tr("Find proximity card reader"));
                findReaderButton_->setMinimumWidth(150);
                connect(findReaderButton_, SIGNAL(pressed()), SLOT(getSerialPortInformation()));
                mainLayout->addWidget(findReaderButton_,
                                                      GRID_LAYOUT_FIRST_ROW_INDEX,
                                                      GRID_LAYOUT_FIRST_COLUMN_INDEX);
            }

            QWidget *proximityCardsButtonswidget = new QWidget;
            QVBoxLayout *proximityCardsButtonsLayout = new QVBoxLayout;
            if (proximityCardsButtonswidget && proximityCardsButtonsLayout) {
                if (readProximityCardButton_) {
                    readProximityCardButton_->setToolTip(tr("Read proximity card"));
                    readProximityCardButton_->setMinimumWidth(150);
                    connect(readProximityCardButton_, SIGNAL(pressed()), SLOT(readProximityCard()));
                    proximityCardsButtonsLayout->addWidget(readProximityCardButton_);
                }
                if (stopProximityCardButton_) {
                    stopProximityCardButton_->setToolTip(tr("Stop proximity card reading"));
                    stopProximityCardButton_->setIcon(QIcon("://Images/cancel.ico"));
                    stopProximityCardButton_->setMaximumWidth(20);
                    if (serialPortHandler_)
                        connect(stopProximityCardButton_, SIGNAL(pressed()), serialPortHandler_.get(), SLOT(stopProximityCard()));
                     proximityCardsButtonsLayout->addWidget(stopProximityCardButton_);
                }
                proximityCardsButtonswidget->setLayout(proximityCardsButtonsLayout);
                mainLayout->addWidget(proximityCardsButtonswidget,
                                                      GRID_LAYOUT_SECOND_ROW_INDEX,
                                                      GRID_LAYOUT_FIRST_COLUMN_INDEX);
            }
            if (serialPortInformation_) {
                serialPortInformation_->setReadOnly(true);
                serialPortInformation_->setToolTip(tr("Serial port information"));
                mainLayout->addWidget(serialPortInformation_,
                                      GRID_LAYOUT_FIRST_ROW_INDEX,
                                      GRID_LAYOUT_SECOND_COLUMN_INDEX,
                                      NO_ROWS_SPAN,
                                      TWO_COLUMNS_SPAN);
            }
            if (proximityCardInformation_) {
                proximityCardInformation_->setReadOnly(true);
                proximityCardInformation_->setToolTip(tr("Proximity card information"));
                mainLayout->addWidget(proximityCardInformation_,
                                                      GRID_LAYOUT_SECOND_ROW_INDEX,
                                                      GRID_LAYOUT_SECOND_COLUMN_INDEX);
            }
            if (addProximityCardToDbButton_) {
                addProximityCardToDbButton_->setToolTip(tr("Add proximity card to database"));
                addProximityCardToDbButton_->setMinimumWidth(100);
                connect(addProximityCardToDbButton_, SIGNAL(pressed()), SLOT(addProximityCardToDatabase()));
                mainLayout->addWidget(addProximityCardToDbButton_,
                                                      GRID_LAYOUT_SECOND_ROW_INDEX,
                                                      GRID_LAYOUT_THIRD_COLUMN_INDEX);
            }
            if (ipAddressPort_) {
                ipAddressPort_->setReadOnly(true);
                ipAddressPort_->setToolTip(tr("IP addres and port"));
                if (tcpServer_)
                    connect(tcpServer_.get(), &TCPServer::signalSendStatusBarMessage, ipAddressPort_, &QTextEdit::append);
                mainLayout->addWidget(ipAddressPort_,
                                      GRID_LAYOUT_THIRD_ROW_INDEX,
                                      GRID_LAYOUT_FIRST_COLUMN_INDEX,
                                      NO_ROWS_SPAN,
                                      THREE_COLUMNS_SPAN);
            }
            if (serialMessagesHistory_) {
                serialMessagesHistory_->setReadOnly(true);
                serialMessagesHistory_->setToolTip(tr("Serial ports messages history"));
                if (serialPortHandler_)
                    connect(serialPortHandler_.get(), &SerialPortHandler::signalSendStatusBarMessage, serialMessagesHistory_, &QTextEdit::append);
                mainLayout->addWidget(serialMessagesHistory_,
                                      GRID_LAYOUT_FOURTH_ROW_INDEX,
                                      GRID_LAYOUT_FIRST_COLUMN_INDEX,
                                      NO_ROWS_SPAN,
                                      THREE_COLUMNS_SPAN);
            }
            mainWidget->setLayout(mainLayout);
            setCentralWidget(mainWidget);
            resize(QSize(WINDOW_WIDTH, WINDOW_HEIGHT));
            setWindowTitle(tr("Proximity cards UI"));
            setWindowIcon(QIcon("://Images/cardMachine.ico"));
        }

        getSerialPortInformation();
        if (serialPortHandler_) {
            serialPortHandler_->setSerialSettings(QSerialPort::NoParity, QSerialPort::Baud19200, QSerialPort::Data8, QSerialPort::OneStop);
            connect(serialPortHandler_.get(), &SerialPortHandler::signalSendControlsEnabled, [this](bool enable) {
                if (findReaderButton_ && readProximityCardButton_) {
                    findReaderButton_->setEnabled(enable);
                    readProximityCardButton_->setEnabled(enable);
                    addProximityCardToDbButton_->setEnabled(enable);
                }
            });
            connect(this, &MainWindow::signalSendControlsEnabled, [this](bool enable) {
                if (findReaderButton_ && readProximityCardButton_) {
                    findReaderButton_->setEnabled(enable);
                    readProximityCardButton_->setEnabled(enable);
                    addProximityCardToDbButton_->setEnabled(enable);
                }
            });
            connect(serialPortHandler_.get(), &SerialPortHandler::signalSendStatusBarMessage, [this](QString message){
                statusBar()->clearMessage();
                statusBar()->showMessage(message);
            });
            connect(serialPortHandler_.get(), &SerialPortHandler::signalSendProximityCardInformation, [this](const QString &cardData){
                if (proximityCardInformation_) {
                    proximityCardInformation_->setText(cardData);
                }
            });
        }
        if (sqlHandler_) {
            connect(sqlHandler_.get(), &SqlHandler::signalErrorMessage, this, &MainWindow::MyslErrorMessage);
            sqlHandler_->init();
        }
        if (tcpServer_) {
            connect(tcpServer_.get(), &TCPServer::signalSendStatusBarMessage, [this](QString message){
                statusBar()->clearMessage();
                statusBar()->showMessage(message);
            });
            tcpServer_->init();
        }
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::getSerialPortInformation(void) {
    if (serialPortInformation_ && serialPortHandler_) {
        statusBar()->clearMessage();
        statusBar()->showMessage("Getting serial ports information ...");
        serialPortInformation_->setText(serialPortHandler_->getSerialPortInformation());
        statusBar()->showMessage("Reseived serial ports information!");
    }
}

void MainWindow::readProximityCard(void) {
    if (proximityCardInformation_ && serialPortHandler_) {
        serialPortHandler_->proximityCardInformation();
    }
}

void MainWindow::MyslErrorMessage(const QString &title, const QString &message) {
    QMessageBox::warning(this, title, message);
}

void MainWindow::addProximityCardToDatabase(void) {
    emit signalSendControlsEnabled(false);
    statusBar()->showMessage(tr("Adding the card data..."));
    QString proximityCardData;
    if (serialPortHandler_)
        proximityCardData = serialPortHandler_->getLastCardData();
    if (proximityCardData.isEmpty()) {
        QMessageBox::warning(this, tr("Can't add the card!")
                             , tr("Nothing to add"));
        statusBar()->showMessage(tr("Didn't add the card data!"));
        emit signalSendControlsEnabled(true);
        return;
    }
    if (proximityCardInformation_ && sqlHandler_) {
        if (!sqlHandler_->isSameDataInTable(proximityCardData)) {
            if (sqlHandler_->setDataToTable(proximityCardData))
                statusBar()->showMessage(tr("The card data successfully added!"));
            else
                statusBar()->showMessage(tr("Didn't add the card data!"));
        }
        else {
            QMessageBox::warning(this, tr("Can't add the card!"),
                                 tr("The card with data %1 already exists").arg(proximityCardData));
            statusBar()->showMessage(tr("Didn't add the card data!"));
        }
    }
    emit signalSendControlsEnabled(true);
}
