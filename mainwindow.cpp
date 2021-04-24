#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    // 设置sessionList不可编辑
    ui->sessionList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 关联客户端连接信号newConnection
    connect(server, &QTcpServer::newConnection, this, &MainWindow::ServerNewConnect);

    // 端口输入限制
    QRegExpValidator *pRevalidotor = new QRegExpValidator(QRegExp("[0-9]{5}"), this);
    ui->localPortEdit->setValidator(pRevalidotor);

    // 将图表绑定到视图
    ui->chartView->setChart(chart);

    // 历史数据保存
    ResistanceDataFile.setFileName("ResistanceData.txt");
}

MainWindow::~MainWindow()
{
    delete ui;
}

class MainWindow::Resistance
{
private:

public:
    double r = 0.0;
    time_t TimeStamp = 0;

    Resistance();
    Resistance(double _r, time_t _TimeStamp);
    //~Resistance();
};

MainWindow::Resistance::Resistance(double _r, time_t _TimeStamp)
{
    r = _r;
    TimeStamp = _TimeStamp;
}

void MainWindow::ServerNewConnect()
{
    // 获取客户端连接
    socket = server->nextPendingConnection();
    clientSocket.append(socket);

    // 把连接到的客户端添加入sessionList
    int currentRow = ui->sessionList->rowCount();
    ui->sessionList->insertRow(currentRow);
    QTableWidgetItem *item_1 = new QTableWidgetItem();
    QTableWidgetItem *item_2 = new QTableWidgetItem();
    QTableWidgetItem *item_3 = new QTableWidgetItem();
    QTableWidgetItem *item_4 = new QTableWidgetItem();
    item_1->setText(tr("#%1").arg(QString::number(ui->sessionList->rowCount())));
    item_2->setText(clientSocket[currentRow]->peerAddress().toString().mid(7));
    item_3->setText(QString::number(clientSocket[currentRow]->peerPort()));
    item_4->setText("在线");
    ui->sessionList->setItem(currentRow, 0, item_1);
    ui->sessionList->setItem(currentRow, 1, item_2);
    ui->sessionList->setItem(currentRow, 2, item_3);
    ui->sessionList->setItem(currentRow, 3, item_4);

    // 日志输出
    static QString connectMessage;
    connectMessage = tr("[%1:%2] connected.").arg(socket->peerAddress().toString().mid(7)).arg(socket->peerPort());
    ui->recievedData->append(connectMessage);
    timer.start(3000);

    // 连接QTcpSocket的信号槽，以读取新数据
    connect(socket, SIGNAL(readyRead()), this, SLOT(ReadData()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(DisConnected()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(Heartbeat()));
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
    QByteArray buffer;

    // 由于readyRead信号并未提供SocketDecriptor，所以需要遍历所有客户端
    for (int i = 0; i < clientSocket.length(); ++i)
    {
        buffer = clientSocket[i]->readAll();
        // 读取缓冲区数据
        if (buffer.isEmpty()) {
            continue;
        }
        qDebug()<<buffer<<"\n";

        // 解析数据
        QString buffer_string = buffer, jsonparse;

        if (buffer_string[0]=="0")
        {
            timer.start(3000);
            //disconnect(socket, SIGNAL(readyRead()), this, SLOT(ReadData()));
            qDebug()<<"Recieving picture...";
            QByteArray image_buffer;
            image_buffer.resize(buffer.size());
            image_buffer = buffer.mid(1,buffer.size()-1);

            packageBuffer.clear();
            packageBuffer += image_buffer;
            //socket->write("s");
            //socket->waitForBytesWritten();
            //qDebug()<<"Package recieved:"<<image_buffer.size()<<"/"<<packageBuffer.size()<<"\n"<<"Package:"<<image_buffer<<"\n";
            while (true)
            {
                buffer = clientSocket[i]->readAll();

                QString buffer_string = buffer;

                if (!socket->waitForReadyRead(500))
                {
                    break;
                }

                if (buffer_string[0]=="E")
                {
                    //connect(socket, SIGNAL(readyRead()), this, SLOT(ReadData()));
                    /*
                    QBuffer camera_buffer(&packageBuffer);
                    camera_buffer.open(QIODevice::ReadOnly);
                    QImageReader image_reader(&camera_buffer);
                    cameraFrame = image_reader.read();*/
                    break;
                }

                image_buffer = buffer.mid(1,buffer.size()-1);

                if (image_buffer.size() == 0)
                {
                    //socket->write("s");
                    //socket->waitForBytesWritten();
                    continue;
                }
                packageBuffer += image_buffer;

                //socket->write("s");
                //socket->waitForBytesWritten();
                //qDebug()<<"Package recieved:"<<image_buffer.size()<<"/"<<packageBuffer.size()<<"\n"<<"Package:"<<image_buffer<<"\n";
            }
            qDebug()<<"CAMERA data recieved:"<<packageBuffer.size();
            //qDebug()<<packageBuffer;

            QBuffer camera_buffer(&packageBuffer);
            camera_buffer.open(QIODevice::ReadOnly);
            QImageReader image_reader(&camera_buffer);
            image_reader.setFormat("bmp");
            cameraFrame = image_reader.read();
            /*
            cameraFrame = QImage((unsigned char*)packageBuffer.data(), 640, 480, QImage::Format_BGR888);
            cameraFrame.save("cameraFrame.jpg");*/

            //socket->write("s");
            //socket->waitForBytesWritten();

            packageBuffer.clear();

            if(!cameraFrame.isNull()){
                QPixmap pix = QPixmap::fromImage(cameraFrame);
                ui->cameraView->setPixmap(pix.scaled(ui->cameraView->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
            }

        }
        if (buffer_string[0]=="1")
        {
            buffer_string = buffer_string.mid(1,buffer_string.size()-1);
            Resistance instance(0, 0);
            if (Json2Instance(buffer_string, &instance))
            {
                jsonparse = tr("Resistence:%1 Time:%2").arg(instance.r).arg(instance.TimeStamp);
                UpdateData(&instance);
                if ((instance.r<6000.0)||(instance.r>7000.0))
                {
                    ui->recievedData->append("Warning! Resistance out of safety range!");
                    QSound::play("./media/bell.wav");
                }
                if (ResistanceDataFile.open(QIODevice::WriteOnly | QIODevice::Append)==true)
                {
                    QDataStream data_stream(&ResistanceDataFile);
                    QString data = tr("Resistence:%1 Time:%2\n").arg(instance.r).arg(instance.TimeStamp);
                    data_stream << data;
                    ResistanceDataFile.close();
                }

                ui->rateLabel->setText(QString("%1").arg(rate_value));
            }
            else
            {
                jsonparse = tr("%1 %2").arg("Json parse error!\nRecieved data:").arg(buffer_string);
            }

            // 日志输出
            static QString IP_Port, IP_Port_Pre;
            IP_Port = tr("[%1:%2] messages:").arg(clientSocket[i]->peerAddress().toString().mid(7)).arg(clientSocket[i]->peerPort());

            // 若此次消息的地址与上次不同，则需显示此次消息的客户端地址
            if (IP_Port != IP_Port_Pre) {
                ui->recievedData->append(IP_Port);
            }

            ui->recievedData->append(jsonparse);

            // 更新ip_port
            IP_Port_Pre = IP_Port;
        }

        if (buffer_string[0]=="n")
        {
            ui->recievedData->append("Error!");
        }
        if (buffer_string[0]=="h")
        {
            timer.start(3000);
            ui->recievedData->append("Heartbeat recieved!");
            socket->write("h");
            socket->waitForBytesWritten();
            qDebug()<<"Heartbeat recieved!";
        }

    }
    return;
}

void MainWindow::DisConnected()
{
    // 遍历寻找断开连接的是哪一个客户端
    for(int i = 0; i < clientSocket.length(); ++i) {
        if(clientSocket[i]->state() == QAbstractSocket::UnconnectedState)
        {
            // 删除存储在sessionList中的该客户端信息
            for (int j = 0; j < ui->sessionList->rowCount(); ++j) {
                if (clientSocket[i]->peerAddress().toString().mid(7) == ui->sessionList->item(j, 1)->text()) {
                    ui->sessionList->removeRow(j);
                }
            }
            // 日志输出
            static QString disconnectMessage;
            disconnectMessage = tr("[%1:%2] disconnected.").arg(clientSocket[i]->peerAddress().toString().mid(7)).arg(clientSocket[i]->peerPort());
            ui->recievedData->append(disconnectMessage);
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

    QString data = ui->Message->text();
    if (data.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入发送内容！", QMessageBox::Yes);
    }
    else {
        if (row >= 0) {
            for (int i = 0; i < clientSocket.length(); ++i) {
                if (QString::number(clientSocket[i]->peerPort()) == ui->sessionList->item(row, 2)->text()) {
                    //以ASCII码形式发送文本框内容
                    clientSocket[i]->write(data.toUtf8());
                }
            }
        }
        else {
            socket->write(data.toUtf8());
            socket->waitForBytesWritten();
        }
    }
}

bool MainWindow::Json2Instance(QString json, Resistance* data)
{
    int r_value = 0, timestamp_value = 0;
    QJsonParseError jsonerror;
    QJsonDocument doc = QJsonDocument::fromJson(json.toLatin1(), &jsonerror);
    if (!doc.isNull() && jsonerror.error == QJsonParseError::NoError)
    {
        if(doc.isObject())
        {
            // 开始解析json对象
            QJsonObject object = doc.object();
            //如果包含 RESISTANCE
            if (object.contains("RESISTANCE"))
            {
                //获取 RESISTANCE
                QJsonValue RESISTANCE_VALUE = object.take("RESISTANCE");
                //转换 RESISTANCE
                r_value = RESISTANCE_VALUE.toVariant().toInt();
            }
            if (object.contains("TIMESTAMP"))
            {
                QJsonValue TIMESTAMP_VALUE = object.take("TIMESTAMP");
                timestamp_value = TIMESTAMP_VALUE.toVariant().toInt();
            }
            if (object.contains("RATE"))
            {
                QJsonValue RATE_VALUE = object.take("RATE");
                rate_value = RATE_VALUE.toVariant().toInt();
            }
            /*
            if (object.contains("CAMERA"))
            {
                QJsonValue CAMERA_VALUE = object.take("CAMERA");
                QByteArray camera_array = CAMERA_VALUE.toVariant().toByteArray();
                camera_array = QByteArray::fromHex(camera_array);
                QBuffer buffer(&camera_array);
                buffer.open(QIODevice::ReadOnly);
                QImageReader reader(&buffer);
                cameraFrame = reader.read();
            }
            */
        }
        data->r = r_value;
        data->TimeStamp = timestamp_value;
        //qDebug()<<r_value<<timestamp_value;
        return true;
    }
    return false;
}

void MainWindow::on_drawChart_clicked()
{
    if (ui->drawChart->text() == tr("绘制"))
    {
        // 将系列添加到图表
        chart->addSeries(series);

        ui->drawChart->setText("清除");
        chartView->setRubberBand(QChartView::HorizontalRubberBand);
        chartView->setRenderHint(QPainter::Antialiasing);
        chart->setTitle("Resistance Monitor");

        // 基于已添加到图表的 series 来创建默认的坐标轴
        chart->createDefaultAxes();
        chart->axes().back()->setGridLineVisible(false);
        chart->axes(Qt::Vertical).back()->setTitleText("Resistance/Ohm");
        chart->axes(Qt::Horizontal).back()->setTitleText("Time");
        chart->axes(Qt::Horizontal).back()->setRange(0, 10);
        chart->axes(Qt::Vertical).back()->setRange(5000, 8000);
    }
    else
    {
        ui->drawChart->setText("绘制");
        chart->axes().clear();
        series->clear();
        chart->removeAxis(chart->axes().back());
        chart->removeSeries(series);
    }
    if (clientSocket.length()==0)
    {
        ui->drawChart->setText("绘制");
        chart->axes().clear();
        series->clear();
        chart->removeAxis(chart->axes().back());
        chart->removeSeries(series);
    }
}

void MainWindow::UpdateData(Resistance *instance)
{
    if (ui->drawChart->text() == tr("绘制")) return;

    if (clientSocket.length()==0)
    {
        ui->drawChart->setText("绘制");
        chart->axes().clear();
        series->clear();
        chart->removeAxis(chart->axes().back());
        chart->removeSeries(series);
    }

    DataList.push_back(*instance);
    //qDebug()<<"Updating.."<<DataList.size();

    if (DataList.size() > dataLength)
    {
        DataList.remove(0);
    }

    // 基于已添加到图表的 series 来创建默认的坐标轴
    chart->createDefaultAxes();
    chart->axes().back()->setGridLineVisible(false);
    chart->axes(Qt::Vertical).back()->setTitleText("Resistance/Ohm");
    chart->axes(Qt::Horizontal).back()->setTitleText("Time");
    chart->axes(Qt::Vertical).back()->setRange(5000, 8000);
    chart->axes(Qt::Horizontal).back()->setRange((int)DataList.begin()->TimeStamp, (int)DataList.end()->TimeStamp);

    series->clear();
    for (int i = 0; i < DataList.size(); i++)
    {
        series->append(DataList[i].TimeStamp, DataList[i].r);
    }
    /*
    QPixmap pixchart = ui->chartView->grab();
    chartImage = pixchart.toImage();
    chartImage.save("chart.png");
    QByteArray chartImageArray = QByteArray::fromRawData((const char*)chartImage.bits(), chartImage.sizeInBytes());
    socket->write(chartImageArray);*/
    return;
}

void MainWindow::Heartbeat()
{
    qDebug()<<"Heartbeat lost!";
    ui->recievedData->append("Heartbeat lost!");
    timer.start(3000);
}

void MainWindow::on_refreshFrame_clicked()
{
    socket->write("c");
    socket->waitForBytesWritten();
}
