/******************************************************************************
**
** Copyright (C) 2017 Lorenz Haas
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtMqtt module.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QVector>
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
    void usableWithQVector();
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

void Tst_QMqttTopicName::usableWithQVector()
{
    const QMqttTopicName topic{"a/b"};
    QVector<QMqttTopicName> names;
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
