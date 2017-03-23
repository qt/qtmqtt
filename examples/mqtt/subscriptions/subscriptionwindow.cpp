#include "subscriptionwindow.h"
#include "ui_subscriptionwindow.h"

SubscriptionWindow::SubscriptionWindow(QSharedPointer<QMqttSubscription> &sub, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubscriptionWindow)
{
    m_sub = sub;
    ui->setupUi(this);

    ui->labelSub->setText(m_sub->topic());
    updateStatus(m_sub->state());
    connect(m_sub.data(), &QMqttSubscription::messageReceived, this, &SubscriptionWindow::updateMessage);
    connect(m_sub.data(), &QMqttSubscription::stateChanged, this, &SubscriptionWindow::updateStatus);
    connect(ui->pushButton, &QAbstractButton::clicked, m_sub.data(), &QMqttSubscription::unsubscribe);
}

SubscriptionWindow::~SubscriptionWindow()
{
    delete ui;
}

void SubscriptionWindow::updateMessage(const QString &text)
{
    ui->listWidget->addItem(text);
}

void SubscriptionWindow::updateStatus(QMqttSubscription::SubscriptionState state)
{
    switch (state) {
    case QMqttSubscription::Unsubscribed:
        ui->labelStatus->setText(QLatin1String("Unsubscribed"));
        break;
    case QMqttSubscription::Pending:
        ui->labelStatus->setText(QLatin1String("Pending"));
        break;
    case QMqttSubscription::Subscribed:
        ui->labelStatus->setText(QLatin1String("Subscribed"));
        break;
    case QMqttSubscription::Error:
        ui->labelStatus->setText(QLatin1String("Error"));
        break;
    default:
        ui->labelStatus->setText(QLatin1String("--Unknown--"));
        break;
    }
}
