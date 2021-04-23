#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QtCharts>
#include <QSound>
QT_CHARTS_USE_NAMESPACE

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

    void on_clearLog_clicked();

    void on_Send_clicked();

    void on_drawChart_clicked();

    void ServerNewConnect();

    void ReadData();

    void DisConnected();

private:
    Ui::MainWindow *ui;

    class Resistance;
    QTcpServer *server;
    QTcpSocket *socket;
    QList<QTcpSocket*> clientSocket;
    QChartView *chartView = new QChartView();
    QChart *chart = chartView->chart();
    QLineSeries *series = new QLineSeries();
    //QTimer timer;
    QVector<Resistance> DataList;
    int rate_value = 0;
    QImage cameraFrame;
    int dataLength = 10;

    bool Json2Instance(QString json, Resistance* data);
    void UpdateData(Resistance *instance);
};

#endif // MAINWINDOW_H
