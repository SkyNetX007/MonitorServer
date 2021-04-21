#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QRegExpValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket();
    server = new QTcpServer();

    // 获取本地的IP
    for (int i = 0; i < QNetworkInterface().allAddresses().length(); ++i) {
        ui->IPComboBox->addItem(QNetworkInterface().allAddresses().at(i).toString());
    }
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::blue);
    ui->serverStateLabel->setPalette(pe);
    ui->serverStateLabel->setText("服务器未启动");

    // 设置tableWidget不可编辑
    ui->sessionList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 关联客户端连接信号newConnection
    connect(server, &QTcpServer::newConnection, this, &MainWindow::serverNewConnect);

    // 端口输入限制
    QRegExpValidator *pRevalidotor = new QRegExpValidator(QRegExp("[0-9]{5}"), this);
    ui->localPortEdit->setValidator(pRevalidotor);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::serverNewConnect()
{
    // 获取客户端连接
    socket = server->nextPendingConnection();
    clientSocket.append(socket);

    // 把连接到的客户端添加入tableWidget中
    int currentRow = ui->sessionList->rowCount();
    ui->sessionList->insertRow(currentRow);
    QTableWidgetItem *item_1 = new QTableWidgetItem();
    QTableWidgetItem *item_2 = new QTableWidgetItem();
    QTableWidgetItem *item_3 = new QTableWidgetItem();
    QTableWidgetItem *item_4 = new QTableWidgetItem();
    item_1->setText(tr("0000000%1").arg(QString::number(ui->sessionList->rowCount())));
    item_2->setText(clientSocket[currentRow]->peerAddress().toString().mid(7));
    item_3->setText(QString::number(clientSocket[currentRow]->peerPort()));
    item_4->setText("在线");
    ui->sessionList->setItem(currentRow, 0, item_1);
    ui->sessionList->setItem(currentRow, 1, item_2);
    ui->sessionList->setItem(currentRow, 2, item_3);
    ui->sessionList->setItem(currentRow, 3, item_4);

    // 连接QTcpSocket的信号槽，以读取新数据
    connect(socket, SIGNAL(readyRead()), this, SLOT(Read_Data()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disConnected()));
}

void MainWindow::on_socketListen_clicked()
{
    if (ui->socketListen->text() == tr("侦听")) {
        // 从输入端获取端口号
        int port = ui->localPortEdit->text().toInt();

        // 侦听指定的端口
        if(!server->listen(QHostAddress::Any, port)) {
            // 若出错，则输出错误信息
            QMessageBox::information(this, tr("错误"), server->errorString(), QMessageBox::Yes);
            return;
        }
        else {
            // 修改按键文字
            ui->socketListen->setText("取消侦听");
            QPalette pe;
            pe.setColor(QPalette::WindowText, Qt::red);
            ui->serverStateLabel->setPalette(pe);
            ui->serverStateLabel->setText("服务器运行中...");
        }
    }
    else {
        // 如果正在连接......
        if(socket->state() == QAbstractSocket::ConnectedState) {
            // 关闭连接
            socket->disconnectFromHost();
        }
        // 取消侦听
        server->close();
        // 修改按键文字
        ui->socketListen->setText("侦听");
        QPalette pe;
        pe.setColor(QPalette::WindowText, Qt::blue);
        ui->serverStateLabel->setPalette(pe);
        ui->serverStateLabel->setText("服务器未打开");
    }
}

// 从客户端接收到的消息
void MainWindow::ReadData()
{
    // 由于readyRead信号并未提供SocketDecriptor，所以需要遍历所有客户端
    for (int i = 0; i < clientSocket.length(); ++i) {
        // 读取缓冲区数据
        QByteArray buffer = clientSocket[i]->readAll();
        if(buffer.isEmpty()) {
            continue;
        }

        static QString IP_Port, IP_Port_Pre;
        IP_Port = tr("[%1:%2]:").arg(clientSocket[i]->peerAddress().toString().mid(7)).arg(clientSocket[i]->peerPort());

        // 若此次消息的地址与上次不同，则需显示此次消息的客户端地址
        if (IP_Port != IP_Port_Pre) {
            ui->recievedData->append(IP_Port);
        }

        ui->recievedData->append(buffer);

        // 更新ip_port
        IP_Port_Pre = IP_Port;
    }
}

void MainWindow::DisConnected()
{
    // 遍历寻找断开连接的是哪一个客户端
    for(int i = 0; i < clientSocket.length(); ++i) {
        if(clientSocket[i]->state() == QAbstractSocket::UnconnectedState)
        {
            // 删除存储在tableWidget中的该客户端信息
            for (int j = 0; j < ui->sessionList->rowCount(); ++j) {
                if (clientSocket[i]->peerAddress().toString().mid(7) == ui->sessionList->item(j, 1)->text()) {
                    ui->sessionList->removeRow(j);
                }
            }
            // 删除存储在clientSocket列表中的客户端信息
            clientSocket[i]->destroyed();
            clientSocket.removeAt(i);
        }
    }
}

void MainWindow::on_clearLog_clicked()
{
    ui->recievedData->clear();
}

void MainWindow::on_Send_clicked()
{
    // 获取选中的行
    int row = ui->sessionList->currentRow();

    QString data = ui->Send->text();
    if (data.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入发送内容！", QMessageBox::Yes);
    }
    else {
        if (row >= 0) {
            for (int i = 0; i < clientSocket.length(); ++i) {
                if (QString::number(clientSocket[i]->peerPort()) == ui->sessionList->item(row, 2)->text()) {
                    //以ASCII码形式发送文本框内容
                    clientSocket[i]->write(data.toLatin1());
                }
            }
        }
        else {
            socket->write(data.toLatin1());
        }
    }
}
