// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SUBSCRIPTIONWINDOW_H
#define SUBSCRIPTIONWINDOW_H

#include <QWidget>
#include <QtMqtt/QMqttMessage>
#include <QtMqtt/QMqttSubscription>

QT_BEGIN_NAMESPACE
namespace Ui {
class SubscriptionWindow;
}
QT_END_NAMESPACE

class SubscriptionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SubscriptionWindow(QMqttSubscription *sub, QWidget *parent = nullptr);
    ~SubscriptionWindow();

public slots:
    void updateMessage(const QMqttMessage &msg);
    void updateStatus(QMqttSubscription::SubscriptionState state);
private:
    Ui::SubscriptionWindow *ui;
    QMqttSubscription *m_sub;
};

#endif // SUBSCRIPTIONWINDOW_H
