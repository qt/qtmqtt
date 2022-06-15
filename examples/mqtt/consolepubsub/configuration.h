// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QByteArray>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QString>
#include <QMqttClient>
#include <QSslConfiguration>
#include <QSslSocket>

struct Configuration
{
    QString topic;
    QByteArray content;
    quint8 qos;
    bool retain{false};
    bool useEncryption{false};
#ifndef QT_NO_SSL
    QSslConfiguration sslConfiguration;
#endif
};

QMqttClient *createClientWithConfiguration(QCoreApplication *app,
                                           Configuration *msg,
                                           bool publish)
{
    QCommandLineParser parser;
    if (publish)
        parser.setApplicationDescription("Qt MQTT publish tool");
    else
        parser.setApplicationDescription("Qt MQTT subscription tool");

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption optionDebug("d",
                                   QLatin1String("Enable debug messages / logging categories"));
    parser.addOption(optionDebug);

    QCommandLineOption optionFile("f",
                                  QLatin1String("Specify the content of a file as message."),
                                  QLatin1String("filename"));
    if (publish)
        parser.addOption(optionFile);

    QCommandLineOption optionClientId("i",
                                      QLatin1String("Specify a client ID. Defaults to random value."),
                                      QLatin1String("clientid"));
    parser.addOption(optionClientId);

    QCommandLineOption optionKeepAlive("k",
                                       QLatin1String("Specify the keep-alive value in seconds."),
                                       QLatin1String("keepAlive"));
    parser.addOption(optionKeepAlive);

    QCommandLineOption optionMessageContent("m",
                                            QLatin1String("Specify the message content. Defaults to"
                                                          " a null message."),
                                            QLatin1String("messageContent"));
    if (publish)
        parser.addOption(optionMessageContent);

    QCommandLineOption optionPassword("P",
                                      QLatin1String("Provide a password."),
                                      QLatin1String("password"));
    parser.addOption(optionPassword);

    QCommandLineOption optionPort("p",
                                  QLatin1String("Network port to connect to. Defaults to 1883."),
                                  QLatin1String("hostPort"),
                                  QLatin1String("1883"));
    parser.addOption(optionPort);

    QCommandLineOption optionQos("q",
                                 QLatin1String("Quality of service level to use for all messages."
                                               "Defaults to 0."),
                                 QLatin1String("qos"),
                                 QLatin1String("0"));
    parser.addOption(optionQos);

    QCommandLineOption optionRetain("r",
                                    QLatin1String("Specify the retain flag for a message."));
    if (publish)
        parser.addOption(optionRetain);

    QCommandLineOption optionHost("s",
                                  QLatin1String("MQTT server to connect to. Defaults to localhost."),
                                  QLatin1String("hostName"),
                                  QLatin1String("localhost"));
    parser.addOption(optionHost);

    QCommandLineOption optionMessageTopic("t",
                                          QLatin1String("Specify the message topic."),
                                          QLatin1String("messageTopic"));
    parser.addOption(optionMessageTopic);

    QCommandLineOption optionUser("u",
                                  QLatin1String("Provide a username."),
                                  QLatin1String("username"));
    parser.addOption(optionUser);

    QCommandLineOption optionVersion("V",
                                     QLatin1String("Specify the protocol version. Options are "
                                                   "mqtt31, mqtt311, mqtt5. Defaults to mqtt311."),
                                     QLatin1String("protocolVersion"),
                                     QLatin1String("mqtt311"));
    parser.addOption(optionVersion);

    QCommandLineOption optionCaFile("cafile",
                                    QLatin1String("Specify a file containing trusted CA "
                                                  "certificates to enable encrypted communication."),
                                    QLatin1String("cafile"));
    parser.addOption(optionCaFile);

    QCommandLineOption optionCaPath("capath",
                                    QLatin1String("Specify a directory containing trusted CA "
                                                  "certificates to enable encrypted communication."),
                                    QLatin1String("capath"));
    parser.addOption(optionCaPath);

    parser.process(*app);

    auto *client = new QMqttClient(app);

    client->setHostname(parser.value(optionHost));
    bool ok = true;
    quint16 port = static_cast<quint16>(parser.value(optionPort).toInt(&ok));
    if (!ok) {
        qWarning() << "Invalid port specified:" << parser.value(optionPort);
        return nullptr;
    }
    client->setPort(port);

    if (parser.isSet(optionUser))
        client->setUsername(parser.value(optionUser));
    if (parser.isSet(optionPassword))
        client->setPassword(parser.value(optionPassword));
    if (parser.isSet(optionClientId))
        client->setClientId(parser.value(optionClientId));
    if (parser.isSet(optionVersion)) {
        const QString version = parser.value(optionVersion);
        if (version == QLatin1String("mqtt31"))
            client->setProtocolVersion(QMqttClient::MQTT_3_1);
        else if (version == QLatin1String("mqtt311"))
            client->setProtocolVersion(QMqttClient::MQTT_3_1_1);
        else if (version == QLatin1String("mqtt5"))
            client->setProtocolVersion(QMqttClient::MQTT_5_0);
        else {
            qWarning() << "Invalid protocol version specified:" << version;
            return nullptr;
        }
    }

    if (parser.isSet(optionCaFile) || parser.isSet(optionCaPath)) {
#ifdef QT_NO_SSL
        qWarning() << "Qt has not been compiled with SSL support.";
        return nullptr;
#else
        QList<QString> fileNames;
        if (parser.isSet(optionCaFile))
            fileNames.append(parser.value(optionCaFile));

        if (parser.isSet(optionCaPath)) {
            QFileInfo path(parser.value(optionCaPath));
            if (!path.isDir()) {
                qWarning() << "Specified capath is not a directory";
                return nullptr;
            }
            auto entries = QDir(parser.value(optionCaPath)).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
            for (auto entry : entries)
                fileNames.append(entry.absoluteFilePath());
        }
        if (fileNames.isEmpty()) {
            qWarning() << "No certificate file found.";
            return nullptr;
        }

        QList<QSslCertificate> defaultCerts;
        for (auto it : fileNames) {
            auto certificates = QSslCertificate::fromPath(it);
            if (certificates.isEmpty() && parser.isSet(optionDebug))
                qWarning() << "File " << it << " does not contain any certificates";
            defaultCerts.append(certificates);
        }
        if (defaultCerts.isEmpty()) {
            qWarning() << "No certificate could be loaded.";
            return nullptr;
        }

        msg->sslConfiguration.setCaCertificates(defaultCerts);
        msg->useEncryption = true;
#endif
    }

    if (parser.isSet(optionDebug))
        QLoggingCategory::setFilterRules(QLatin1String("qt.mqtt.*=true"));

    msg->qos = static_cast<quint8>(parser.value(optionQos).toInt(&ok));
    if (!ok || msg->qos > 2) {
        qWarning() << "Invalid quality of service for message specified:" << msg->qos;
        return nullptr;
    }

    if (parser.isSet(optionKeepAlive)) {
        const quint16 keep = static_cast<quint16>(parser.value(optionKeepAlive).toUInt(&ok));
        if (!ok) {
            qWarning() << "Invalid keep alive value specified";
            return nullptr;
        }
        client->setKeepAlive(keep);
    }

    if (publish) {
        if (parser.isSet(optionFile) && parser.isSet(optionMessageContent)) {
            qWarning() << "You cannot specify a file and a text as message.";
            return nullptr;
        }
        if (parser.isSet(optionFile)) {
            QFile file(parser.value(optionFile));
            if (!file.open(QIODevice::ReadOnly)) {
                qWarning() << "Could not open specified file for reading.";
                return nullptr;
            }
            msg->content = file.readAll();
            file.close();
        }
        if (parser.isSet(optionMessageContent))
            msg->content = parser.value(optionMessageContent).toUtf8();
    }

    if (!parser.isSet(optionMessageTopic)) {
        qWarning() << "You must specify a topic to publish a message.";
        return nullptr;
    }
    msg->topic = parser.value(optionMessageTopic);

    if (publish && !QMqttTopicName(msg->topic).isValid()) {
        qWarning() << "The specified message topic is invalid.";
        return nullptr;
    }

    if (!publish && !QMqttTopicFilter(msg->topic).isValid()) {
        qWarning() << "The specified subscription topic is invalid.";
        return nullptr;
    }

    if (publish && parser.isSet(optionRetain))
        msg->retain = true;

    // Output:
    qInfo() << "Client configuration:";
    qInfo() << "  Host:" << client->hostname() << " Port:" << client->port()
            << " Protocol:" << client->protocolVersion();
    qInfo() << "  Username:" << client->username() << " Password:" << !client->password().isEmpty();
    qInfo() << "  Client ID:" << client->clientId() << "Keep Alive:" << client->keepAlive();

    return client;
}

