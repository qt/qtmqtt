// Copyright (C) 2017 Lorenz Haas
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtMqtt/QMqttTopicName>
#include <QtTest/QtTest>

class Tst_QMqttTopicName : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void checkValidity();
    void checkLevelCount();
    void checkLevels_data();
    void checkLevels();
    void usableWithQList();
    void usableWithQMap();
    void usableWithQHash();
};

void Tst_QMqttTopicName::checkValidity()
{
    QVERIFY(QMqttTopicName("a").isValid());
    QVERIFY(QMqttTopicName("/").isValid());
    QVERIFY(QMqttTopicName("a b").isValid());

    QVERIFY(!QMqttTopicName("").isValid());
    QVERIFY(!QMqttTopicName("/a/#").isValid());
    QVERIFY(!QMqttTopicName("/+/a").isValid());
    QVERIFY(!QMqttTopicName(QString(3, QChar(QChar::Null))).isValid());
}

void Tst_QMqttTopicName::checkLevelCount()
{
    QCOMPARE(QMqttTopicName("a").levelCount(), 1);
    QCOMPARE(QMqttTopicName("/").levelCount(), 2);
    QCOMPARE(QMqttTopicName("/a").levelCount(), 2);
    QCOMPARE(QMqttTopicName("a/").levelCount(), 2);
    QCOMPARE(QMqttTopicName("a/b").levelCount(), 2);
    QCOMPARE(QMqttTopicName("a/b/").levelCount(), 3);
}

void Tst_QMqttTopicName::checkLevels_data()
{
    QTest::addColumn<QMqttTopicName>("name");
    QTest::addColumn<QStringList>("levels");

    QTest::newRow("1") << QMqttTopicName("a") << QStringList{"a"};
    QTest::newRow("2") << QMqttTopicName("/") << QStringList{"", ""};
    QTest::newRow("3") << QMqttTopicName("//") << QStringList{"", "", ""};
    QTest::newRow("4") << QMqttTopicName("a/") << QStringList{"a", ""};
    QTest::newRow("5") << QMqttTopicName("/a") << QStringList{"", "a"};
    QTest::newRow("6") << QMqttTopicName("a/b") << QStringList{"a", "b"};
    QTest::newRow("7") << QMqttTopicName("a/b/") << QStringList{"a", "b", ""};
    QTest::newRow("8") << QMqttTopicName("/a/b") << QStringList{"", "a", "b"};
}

void Tst_QMqttTopicName::checkLevels()
{
    QFETCH(QMqttTopicName, name);
    QFETCH(QStringList, levels);

    QCOMPARE(name.levels(), levels);
}

void Tst_QMqttTopicName::usableWithQList()
{
    const QMqttTopicName topic{"a/b"};
    QList<QMqttTopicName> names;
    names.append(topic);
    QCOMPARE(topic, names.constFirst());
}

void Tst_QMqttTopicName::usableWithQMap()
{
    const QMqttTopicName topic{"a/b"};
    QMap<QMqttTopicName, int> names;
    names.insert(topic, 42);
    QCOMPARE(names[topic], 42);
}

void Tst_QMqttTopicName::usableWithQHash()
{
    const QMqttTopicName topic{"a/b"};
    QHash<QMqttTopicName, int> names;
    names.insert(topic, 42);
    QCOMPARE(names[topic], 42);
}

QTEST_MAIN(Tst_QMqttTopicName)

#include "tst_qmqtttopicname.moc"
