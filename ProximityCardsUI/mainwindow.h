#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include "serialporthandler.h"
#include "sqlhandler.h"
#include <tcpserver.h>

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
private slots:
    void getSerialPortInformation(void);
    void readProximityCard(void);
    void addProximityCardToDatabase(void);
    void MyslErrorMessage(const QString &title, const QString &message);
private:

    QPushButton *findReaderButton_;
    QPushButton *readProximityCardButton_;
    QPushButton *stopProximityCardButton_;
    QPushButton *addProximityCardToDbButton_;
    QTextEdit *serialPortInformation_;
    QTextEdit *proximityCardInformation_;
    QTextEdit *ipAddressPort_;
    QTextEdit *serialMessagesHistory_;
    std::unique_ptr<SerialPortHandler> serialPortHandler_;
    std::unique_ptr<SqlHandler> sqlHandler_;
    std::unique_ptr<TCPServer> tcpServer_;
    std::unique_ptr<Ui::MainWindow> ui_;
 };

#endif // MAINWINDOW_H
