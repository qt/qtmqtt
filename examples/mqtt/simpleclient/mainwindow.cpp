#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtMqtt/QMqttClient>

// http://test.mosquitto.org/
//const QString TEST_HOST("test.mosquitto.org");
const QString TEST_HOST("hostprocess.de");
const quint16 TEST_PORT = 1883;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_client = new QMqttClient(this);
    m_client->setHostname(TEST_HOST);
    m_client->setPort(TEST_PORT);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonConnect_clicked()
{
    m_client->connectToHost();
}

void MainWindow::on_buttonQuit_clicked()
{
    QApplication::quit();
}
