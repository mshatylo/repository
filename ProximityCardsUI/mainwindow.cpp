#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGridLayout>
#include <QMessageBox>
#include <QDebug>

#define GRID_LAYOUT_FIRST_ROW_INDEX         (0)
#define GRID_LAYOUT_SECOND_ROW_INDEX        (1)
#define GRID_LAYOUT_THIRD_ROW_INDEX         (2)
#define GRID_LAYOUT_FOURTH_ROW_INDEX        (3)
#define GRID_LAYOUT_FIFTH_ROW_INDEX         (4)

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
    getSerialPortInformationButton_(new QPushButton(tr("Get serial port information"))),
    findReaderAndReadProximityCardButton_(new QPushButton(tr("Find reader and\nread proximity card"))),
    stopProximityCardButton_(new QPushButton()),
    addProximityCardToDbButton_(new QPushButton(tr("Add to DB"))),
    serialPortInformation_(new /*QLabel*/QTextEdit(tr("Serial port information"))),
    proximityCardInformation_(new QTextEdit("HELLO EVERYONE")),
    hostIpAdrressesComboBox_(new QComboBox),
    hostPortLineEdit_(new QLineEdit),
    addProximityCardToControllerButton_(new QPushButton("Add proximity card to controller")),
    findProximityCardReader_(new QPushButton("Find proximity card reader")),
    serialMessagesHistory_(new QTextEdit),
    serialPortHandler_(new SerialPortHandler),
    sqlHandler_(new SqlHandler("localhost", "root", "olhasham2012", "ProximityCardsDatabase", "ProximityCardsTable")),
    tcpClient_(new TCPClient),
    ui_(new Ui::MainWindow)
{
    if (ui_) {
        ui_->setupUi(this);
        QWidget *mainWidget = new QWidget;
        QGridLayout *mainLayout = new QGridLayout;

        if (mainWidget && mainLayout) {

            if (getSerialPortInformationButton_) {
                getSerialPortInformationButton_->setToolTip(tr("Get serial port information"));
                getSerialPortInformationButton_->setMinimumWidth(150);
                connect(getSerialPortInformationButton_, SIGNAL(pressed()), SLOT(getSerialPortInformation()));
                mainLayout->addWidget(getSerialPortInformationButton_,
                                                      GRID_LAYOUT_FIRST_ROW_INDEX,
                                                      GRID_LAYOUT_FIRST_COLUMN_INDEX);
            }

            QWidget *proximityCardsButtonswidget = new QWidget;
            QVBoxLayout *proximityCardsButtonsLayout = new QVBoxLayout;
            if (proximityCardsButtonswidget && proximityCardsButtonsLayout) {
                if (findReaderAndReadProximityCardButton_) {
                    findReaderAndReadProximityCardButton_->setToolTip(tr("Find reader and read proximity card"));
                    findReaderAndReadProximityCardButton_->setMinimumWidth(150);
                    connect(findReaderAndReadProximityCardButton_, SIGNAL(pressed()), SLOT(readProximityCard()));
                    proximityCardsButtonsLayout->addWidget(findReaderAndReadProximityCardButton_);
                }
                if (stopProximityCardButton_) {
                    stopProximityCardButton_->setToolTip(tr("Stop proximity card reading"));
                    stopProximityCardButton_->setIcon(QIcon("://Images/cancel.ico"));
                    stopProximityCardButton_->setMaximumWidth(20);
                    if (serialPortHandler_)
                        connect(stopProximityCardButton_, SIGNAL(pressed()), serialPortHandler_.get(), SLOT(stopProximityCardGetting()));
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
            if (hostIpAdrressesComboBox_) {
                hostIpAdrressesComboBox_->setToolTip(tr("Host IP address"));
                if (tcpClient_) {
                    connect(hostIpAdrressesComboBox_, &QComboBox::editTextChanged, tcpClient_.get(), &TCPClient::setHostIpAddress);
                    connect(tcpClient_.get(), &TCPClient::signalSendStatusBarMessage, serialMessagesHistory_, &QTextEdit::append);
                    connect(tcpClient_.get(), &TCPClient::signalSendStatusBarMessage, [this](QString message){
                        statusBar()->clearMessage();
                        statusBar()->showMessage(message);
                    });
                    tcpClient_->init(*hostIpAdrressesComboBox_);
                }
                mainLayout->addWidget(hostIpAdrressesComboBox_,
                                      GRID_LAYOUT_THIRD_ROW_INDEX,
                                      GRID_LAYOUT_FIRST_COLUMN_INDEX,
                                      NO_ROWS_SPAN,
                                      TWO_COLUMNS_SPAN);
            }
            if (hostPortLineEdit_) {
                hostPortLineEdit_->setToolTip(tr("Host port"));
                if (tcpClient_) {
                    connect(hostPortLineEdit_, &QLineEdit::textChanged, tcpClient_.get(), &TCPClient::setHostPort);
                }
                mainLayout->addWidget(hostPortLineEdit_,
                                      GRID_LAYOUT_THIRD_ROW_INDEX,
                                      GRID_LAYOUT_THIRD_COLUMN_INDEX);
            }
            if (findProximityCardReader_) {
                findProximityCardReader_->setToolTip(tr("Find proximity card reader"));
                if (serialPortHandler_) {
                    connect(findProximityCardReader_, &QPushButton::pressed, serialPortHandler_.get(), &SerialPortHandler::findCardReader);
                }
                mainLayout->addWidget(findProximityCardReader_,
                                      GRID_LAYOUT_FOURTH_ROW_INDEX,
                                      GRID_LAYOUT_FIRST_COLUMN_INDEX);
            }
            if (addProximityCardToControllerButton_) {
                addProximityCardToControllerButton_->setToolTip(tr("Add proximity card to controller"));
                addProximityCardToControllerButton_->setEnabled(false);
                if (serialPortHandler_) {
                    connect(addProximityCardToControllerButton_, &QPushButton::pressed, serialPortHandler_.get(), &SerialPortHandler::readCardData);
                }
                mainLayout->addWidget(addProximityCardToControllerButton_,
                                      GRID_LAYOUT_FOURTH_ROW_INDEX,
                                      GRID_LAYOUT_THIRD_COLUMN_INDEX,
                                      NO_ROWS_SPAN,
                                      TWO_COLUMNS_SPAN);
            }
            if (serialMessagesHistory_) {
                serialMessagesHistory_->setReadOnly(true);
                serialMessagesHistory_->setToolTip(tr("Serial ports messages history"));
                if (serialPortHandler_)
                    connect(serialPortHandler_.get(), &SerialPortHandler::signalSendStatusBarMessage, serialMessagesHistory_, &QTextEdit::append);
                mainLayout->addWidget(serialMessagesHistory_,
                                      GRID_LAYOUT_FIFTH_ROW_INDEX,
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
                if (getSerialPortInformationButton_ && findReaderAndReadProximityCardButton_
                    && addProximityCardToDbButton_ && findProximityCardReader_) {
                    getSerialPortInformationButton_->setEnabled(enable);
                    findReaderAndReadProximityCardButton_->setEnabled(enable);
                    addProximityCardToDbButton_->setEnabled(enable);
                    findProximityCardReader_->setEnabled(enable);
                }
            });
            connect(this, &MainWindow::signalSendControlsEnabled, [this](bool enable) {
                if (getSerialPortInformationButton_ && findReaderAndReadProximityCardButton_
                    && addProximityCardToDbButton_ && findProximityCardReader_) {
                    getSerialPortInformationButton_->setEnabled(enable);
                    findReaderAndReadProximityCardButton_->setEnabled(enable);
                    addProximityCardToDbButton_->setEnabled(enable);
                    findProximityCardReader_->setEnabled(enable);
                }
            });
            connect(serialPortHandler_.get(), &SerialPortHandler::signalSendAddProximityCardToControllerButtonEnabled, [this](bool enable) {
                if (addProximityCardToControllerButton_) {
                    addProximityCardToControllerButton_->setEnabled(enable);
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

        if (serialPortHandler_ && tcpClient_) {
            connect(serialPortHandler_.get(), &SerialPortHandler::sendDataByNetwork, tcpClient_.get(), &TCPClient::sendMessage);
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
