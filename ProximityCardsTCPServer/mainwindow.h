#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <memory>
#include "tcpserver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void init(void);
private slots:

signals:
    void signalSendControlsEnabled(bool enable);
private:
    std::unique_ptr<TCPServer> tcpServer_;
    std::unique_ptr<Ui::MainWindow> ui_;
};

#endif // MAINWINDOW_H
