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
