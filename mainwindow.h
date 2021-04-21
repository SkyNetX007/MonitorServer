#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_socketListen_clicked();

    void serverNewConnect();

    void ReadData();

    void DisConnected();

    void on_clearLog_clicked();

    void on_Send_clicked();

private:
    Ui::MainWindow *ui;

    QTcpServer * server;
    QTcpSocket * socket;
    QList<QTcpSocket*> clientSocket;


};

class Resistance;

#endif // MAINWINDOW_H
