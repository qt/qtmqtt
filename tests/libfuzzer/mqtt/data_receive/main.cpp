// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtMqtt/private/qmqttcontrolpacket_p.h>

#include <QtMqtt>
#include <QtTest>
#include <QtCore>

Q_DECLARE_METATYPE(QMqttSubscription::SubscriptionState)

class FakeServer : public QIODevice
{
    Q_OBJECT
public:
    explicit FakeServer(QObject *parent);

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    int m_messageState{0};
    QByteArray m_readBuffer;
};

FakeServer::FakeServer(QObject *parent)
    : QIODevice(parent)
{
    m_messageState = 0;
    open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}

qint64 FakeServer::readData(char *data, qint64 maxlen)
{
    const qint64 dataToWrite = qMax<qint64>(0, qMin<qint64>(maxlen, m_readBuffer.size()));
    memcpy(data, m_readBuffer.constData(), static_cast<size_t>(dataToWrite));
    m_readBuffer = m_readBuffer.mid(static_cast<int>(dataToWrite));
    return dataToWrite;
}

void bufferAppend(QByteArray &buffer, quint16 value)
{
    const quint16 msb = qToBigEndian<quint16>(value);
    const char * msb_c = reinterpret_cast<const char*>(&msb);
    buffer.append(msb_c, 2);
}

qint64 FakeServer::writeData(const char *data, qint64 len)
{
    if (m_messageState == 0) { // Received CONNECT
        QByteArray response;
        response += 0x20;
        response += quint8(2); // Payload size
        response += char(0); // ackFlags
        response += char(0); // result

        m_readBuffer = response;
        emit readyRead();
        m_messageState = 1;
    } else if (m_messageState == 1) { // Received SUBSCRIBE
        // Byte 0 == 0x82 (0x80 SUBSCRIBE 0x02 standard)
        quint8 msg = reinterpret_cast<const quint8 *>(data)[0];
        if (msg != quint8(0x82))
            qFatal("Expected subscribe message");
        // Byte 1+2 == ID
        const quint16 id_big = *reinterpret_cast<const quint16 *>(&data[2]);
        const quint16 id = qFromBigEndian<quint16>(id_big);

        QMqttControlPacket packet(QMqttControlPacket::SUBACK);
        packet.append(id);
        const quint8 qosLevel = 1;
        packet.append(static_cast<char>(qosLevel));
        m_readBuffer = packet.serialize();
        // We need to be async to have QMqttConnection prepare its internals
        QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);

        m_messageState = 2;
    }
    // Ignore any follow up data

    return len;
}

extern "C" int LLVMFuzzerTestOneInput(const char *Data, size_t Size)
{
    static int argc = 1;
    static char *argv_1[] = {qstrdup("fuzzprepare")};
    static char **argv = argv_1;
    static QCoreApplication a(argc, argv);

    FakeServer serv(&a);
    QMqttClient client;
    client.setHostname(QLatin1String("localhost"));
    client.setPort(1883);

    client.setTransport(&serv, QMqttClient::IODevice);
    client.connectToHost();

    QSignalSpy spy(&client, &QMqttClient::connected);
    spy.wait(5);

    if (client.state() != QMqttClient::Connected) {
        qFatal("Not able to run test propertly");
        return -1;
    }

    auto sub = client.subscribe(QLatin1String("a"), 1);
    qRegisterMetaType<QMqttSubscription::SubscriptionState>("SubscriptionState");
    QSignalSpy subSpy(sub, &QMqttSubscription::stateChanged);
    spy.wait(5);
    if (sub->state() != QMqttSubscription::Subscribed)
        spy.wait(10);
    if (sub->state() != QMqttSubscription::Subscribed)
        qFatal("Could not subscribe");

    serv.m_readBuffer = QByteArray(Data, static_cast<int>(Size));
    QMetaObject::invokeMethod(&serv, "readyRead");
    QCoreApplication::processEvents();
    return 0;
}

#include "main.moc"
