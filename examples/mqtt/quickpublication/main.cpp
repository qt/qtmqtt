// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "qmlmqttclient.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLoggingCategory>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<QmlMqttClient>("MqttClient", 1, 0, "MqttClient");

    engine.load(QUrl(u"qrc:/main.qml"_s));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
