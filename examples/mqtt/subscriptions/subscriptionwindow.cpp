// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "subscriptionwindow.h"
#include "ui_subscriptionwindow.h"

using namespace Qt::StringLiterals;

SubscriptionWindow::SubscriptionWindow(QMqttSubscription *sub, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubscriptionWindow),
    m_sub(sub)
{
    ui->setupUi(this);

    ui->labelSub->setText(m_sub->topic().filter());
    ui->labelQoS->setText(QString::number(m_sub->qos()));
    updateStatus(m_sub->state());
    connect(m_sub, &QMqttSubscription::messageReceived, this, &SubscriptionWindow::updateMessage);
    connect(m_sub, &QMqttSubscription::stateChanged, this, &SubscriptionWindow::updateStatus);
    connect(m_sub, &QMqttSubscription::qosChanged, [this](quint8 qos) {
        ui->labelQoS->setText(QString::number(qos));
    });
    connect(ui->pushButton, &QAbstractButton::clicked, m_sub, &QMqttSubscription::unsubscribe);
}

SubscriptionWindow::~SubscriptionWindow()
{
    m_sub->unsubscribe();
    delete ui;
}

void SubscriptionWindow::updateMessage(const QMqttMessage &msg)
{
    ui->listWidget->addItem(msg.payload());
}

void SubscriptionWindow::updateStatus(QMqttSubscription::SubscriptionState state)
{
    switch (state) {
    case QMqttSubscription::Unsubscribed:
        ui->labelStatus->setText(u"Unsubscribed"_s);
        break;
    case QMqttSubscription::SubscriptionPending:
        ui->labelStatus->setText(u"Pending"_s);
        break;
    case QMqttSubscription::Subscribed:
        ui->labelStatus->setText(u"Subscribed"_s);
        break;
    case QMqttSubscription::Error:
        ui->labelStatus->setText(u"Error"_s);
        break;
    case QMqttSubscription::UnsubscriptionPending:
        ui->labelStatus->setText(u"Pending Unsubscription"_s);
        break;
    default:
        ui->labelStatus->setText(u"--Unknown--"_s);
        break;
    }
}
