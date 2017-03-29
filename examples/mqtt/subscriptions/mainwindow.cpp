#include "mainwindow.h"
#include "subscriptionwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QDateTime>
#include <QtMqtt/QMqttClient>
#include <QtWidgets/QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_client = new QMqttClient(this);
    m_client->setHostname(ui->lineEditHost->text());
    m_client->setPort(ui->spinBoxPort->value());

    connect(m_client, &QMqttClient::stateChanged, this, &MainWindow::updateLogStateChange);
    connect(m_client, &QMqttClient::disconnected, this, &MainWindow::brokerDisconnected);

    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QString &topic) {
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" Received Topic: ")
                    + topic
                    + QLatin1String(" Message: ")
                    + message
                    + QLatin1Char('\n');
        ui->editLog->insertPlainText(content);
    });

    connect(m_client, &QMqttClient::pingResponse, this, [this]() {
        ui->buttonPing->setEnabled(true);
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" PingResponse")
                    + QLatin1Char('\n');
        ui->editLog->insertPlainText(content);
    });

    connect(ui->lineEditHost, &QLineEdit::textChanged, m_client, &QMqttClient::setHostname);
    connect(ui->spinBoxPort, SIGNAL(valueChanged(int)), this, SLOT(setClientPort(int)));
    updateLogStateChange();
}

MainWindow::~MainWindow()
{
    delete ui;
    qApp->quit();
}

void MainWindow::on_buttonConnect_clicked()
{
    if (m_client->state() == QMqttClient::Disconnected) {
        ui->lineEditHost->setEnabled(false);
        ui->spinBoxPort->setEnabled(false);
        ui->buttonConnect->setText(tr("Disconnect"));
        m_client->connectToHost();
    } else {
        ui->lineEditHost->setEnabled(true);
        ui->spinBoxPort->setEnabled(true);
        ui->buttonConnect->setText(tr("Connect"));
        m_client->disconnectFromHost();
    }
}

void MainWindow::on_buttonQuit_clicked()
{
    QApplication::quit();
}

void MainWindow::updateLogStateChange()
{
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_client->state())
                    + QLatin1Char('\n');
    ui->editLog->insertPlainText(content);
}

void MainWindow::brokerDisconnected()
{
    ui->lineEditHost->setEnabled(true);
    ui->spinBoxPort->setEnabled(true);
    ui->buttonConnect->setText(tr("Connect"));
}

void MainWindow::setClientPort(int p)
{
    m_client->setPort(p);
}

void MainWindow::on_buttonPublish_clicked()
{
    if (m_client->publish(ui->lineEditTopic->text(), ui->lineEditMessage->text().toUtf8(), ui->spinQoS_2->text().toUInt()) == -1)
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
}

void MainWindow::on_buttonSubscribe_clicked()
{
    auto subscription = m_client->subscribe(ui->lineEditTopic->text(), ui->spinQoS->text().toUInt());
    if (!subscription) {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
    auto subWindow = new SubscriptionWindow(subscription);
    subWindow->setWindowTitle(subscription->topic());
    subWindow->show();
}

void MainWindow::on_buttonPing_clicked()
{
    ui->buttonPing->setEnabled(false);
    m_client->requestPing();
}
