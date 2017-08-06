#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include "serialporthandler.h"
#include "sqlhandler.h"
//#include "tcpserver.h"
#include "tcpclient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void signalSendControlsEnabled(bool enable);
    //void sendDataByNetwork(const QString &data);
private slots:
    void getSerialPortInformation(void);
    void readProximityCard(void);
    void addProximityCardToDatabase(void);
    void MyslErrorMessage(const QString &title, const QString &message);
private:

    QPushButton *getSerialPortInformationButton_;
    QPushButton *findReaderAndReadProximityCardButton_;
    QPushButton *stopProximityCardButton_;
    QPushButton *addProximityCardToDbButton_;
    QTextEdit *serialPortInformation_;
    QTextEdit *proximityCardInformation_;
    QComboBox *hostIpAdrressesComboBox_;
    QLineEdit *hostPortLineEdit_;
    QPushButton *findProximityCardReader_;
    QPushButton *addProximityCardToControllerButton_;
    QTextEdit *serialMessagesHistory_;
    std::unique_ptr<SerialPortHandler> serialPortHandler_;
    std::unique_ptr<SqlHandler> sqlHandler_;
    std::unique_ptr<TCPClient> tcpClient_;
    std::unique_ptr<Ui::MainWindow> ui_;
 };

#endif // MAINWINDOW_H
