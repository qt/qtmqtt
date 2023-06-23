// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Qt Quick Publication")
    id: root

    MqttClient {
        id: client
        hostname: hostnameField.text
        port: portField.text
    }

    ListModel {
        id: messageModel
    }

    function addMessage(payload)
    {
        messageModel.insert(0, {"payload" : payload})

        if (messageModel.count >= 100)
            messageModel.remove(99)
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 10
        columns: 2

        Label {
            text: "Hostname:"
            enabled: client.state === MqttClient.Disconnected
        }

        TextField {
            id: hostnameField
            Layout.fillWidth: true
            text: "test.mosquitto.org"
            placeholderText: "<Enter host running MQTT broker>"
            enabled: client.state === MqttClient.Disconnected
        }

        Label {
            text: "Port:"
            enabled: client.state === MqttClient.Disconnected
        }

        TextField {
            id: portField
            Layout.fillWidth: true
            text: "1883"
            placeholderText: "<Port>"
            inputMethodHints: Qt.ImhDigitsOnly
            enabled: client.state === MqttClient.Disconnected
        }

        Button {
            id: connectButton
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: client.state === MqttClient.Connected ? "Disconnect" : "Connect"
            onClicked: {
                if (client.state === MqttClient.Connected) {
                    client.disconnectFromHost()
                    messageModel.clear()
                } else
                    client.connectToHost()
            }
        }

        GridLayout {
            enabled: client.state === MqttClient.Connected
            Layout.columnSpan: 2
            Layout.fillWidth: true
            columns: 4

            Label {
                text: "Topic:"
            }

            TextField {
                id: pubField
                Layout.fillWidth: true
                placeholderText: "<Publication topic>"
            }

            Label {
                text: "Message:"
            }

            TextField {
                id: msgField
                Layout.fillWidth: true
                placeholderText: "<Publication message>"
            }

            Label {
                text: "QoS:"
            }

            ComboBox {
                id: qosItems
                Layout.fillWidth: true
                editable: false
                model: [0, 1, 2]
            }

            CheckBox {
                id: retain
                checked: false
                text: "Retain"
            }

            Button {
                id: pubButton
                Layout.fillWidth: true
                text: "Publish"
                onClicked: {
                    if (pubField.text.length === 0) {
                        console.log("No payload to send. Skipping publish...")
                        return
                    }
                    client.publish(pubField.text, msgField.text, qosItems.currentText, retain.checked)
                    root.addMessage(msgField.text)
                }
            }
        }

        ListView {
            id: messageView
            model: messageModel
            implicitHeight: 300
            implicitWidth: 200
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: 2
            clip: true
            delegate: Rectangle {
                id: delegatedRectangle
                required property int index
                required property string payload
                width: ListView.view.width
                height: 30
                color: index % 2 ? "#DDDDDD" : "#888888"
                radius: 5
                Text {
                    text: delegatedRectangle.payload
                    anchors.centerIn: parent
                }
            }
        }

        Label {
            function stateToString(value) {
                if (value === 0)
                    return "Disconnected"
                else if (value === 1)
                    return "Connecting"
                else if (value === 2)
                    return "Connected"
                else
                    return "Unknown"
            }

            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: "#333333"
            text: "Status:" + stateToString(client.state) + "(" + client.state + ")"
            enabled: client.state === MqttClient.Connected
        }
    }
}
