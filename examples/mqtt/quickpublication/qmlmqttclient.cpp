// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "qmlmqttclient.h"
#include <QDebug>
#include <QMqttTopicName>

QmlMqttClient::QmlMqttClient(QObject *parent)
    : QMqttClient(parent)
{
}

int QmlMqttClient::publish(const QString &topic, const QString &message, int qos, bool retain)
{
    auto result = QMqttClient::publish(QMqttTopicName(topic), message.toUtf8(), qos, retain);
    return result;
}
