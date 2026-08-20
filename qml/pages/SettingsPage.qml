import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property real probeM: 30
    property real probeT: 4

    Rectangle {
        anchors.fill: parent
        anchors.margins: 16
        color: Theme.bgCard
        border.color: Theme.border
        border.width: 1
        radius: 6

        TabBar {
            id: tabs
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40
            background: Rectangle { color: "transparent" }

            TabButton {
                id: tabThresh
                text: "THRESHOLDS"
                font.family: App.uiFont
                font.pixelSize: 12
                font.letterSpacing: 0.8
                contentItem: Text {
                    text: tabThresh.text
                    color: tabThresh.checked ? Theme.text : Theme.textMuted
                    font: tabThresh.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Item {
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: tabThresh.checked ? Theme.action : "transparent"
                    }
                }
            }
            TabButton {
                id: tabNodes
                text: "NODES"
                font.family: App.uiFont
                font.pixelSize: 12
                font.letterSpacing: 0.8
                contentItem: Text {
                    text: tabNodes.text
                    color: tabNodes.checked ? Theme.text : Theme.textMuted
                    font: tabNodes.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Item {
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: tabNodes.checked ? Theme.action : "transparent"
                    }
                }
            }
            TabButton {
                id: tabLink
                text: "LINK"
                font.family: App.uiFont
                font.pixelSize: 12
                font.letterSpacing: 0.8
                contentItem: Text {
                    text: tabLink.text
                    color: tabLink.checked ? Theme.text : Theme.textMuted
                    font: tabLink.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Item {
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: tabLink.checked ? Theme.action : "transparent"
                    }
                }
            }
        }

        Rectangle {
            anchors.top: tabs.bottom
            width: parent.width
            height: 1
            color: Theme.border
        }

        StackLayout {
            id: stack
            anchors.top: tabs.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 24
            currentIndex: tabs.currentIndex

            // --- Thresholds ---
            Flickable {
                clip: true
                contentHeight: threshCol.height
                Column {
                    id: threshCol
                    width: Math.min(560, stack.width)
                    spacing: 22

                    Text {
                        text: "ALARM LIMITS"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }

                    Grid {
                        columns: 2
                        columnSpacing: 28
                        rowSpacing: 18
                        width: parent.width

                        Column {
                            spacing: 6
                            width: 250
                            Text { text: "Moisture warning  %"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                            Row {
                                spacing: 10
                                Slider {
                                    id: mw
                                    width: 170
                                    from: 5; to: 90
                                    value: App.moistureWarning
                                    onMoved: App.moistureWarning = value
                                }
                                Text { text: mw.value.toFixed(1); color: Theme.text; font.family: App.monoFont; font.pixelSize: 13; width: 40 }
                                Rectangle { width: 8; height: 8; radius: 4; color: Theme.warning; anchors.verticalCenter: parent.verticalCenter }
                            }
                        }
                        Column {
                            spacing: 6
                            width: 250
                            Text { text: "Moisture critical  %"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                            Row {
                                spacing: 10
                                Slider {
                                    id: mc
                                    width: 170
                                    from: 10; to: 95
                                    value: App.moistureCritical
                                    onMoved: App.moistureCritical = value
                                }
                                Text { text: mc.value.toFixed(1); color: Theme.text; font.family: App.monoFont; font.pixelSize: 13; width: 40 }
                                Rectangle { width: 8; height: 8; radius: 4; color: Theme.critical; anchors.verticalCenter: parent.verticalCenter }
                            }
                        }
                        Column {
                            spacing: 6
                            width: 250
                            Text { text: "Tilt warning  °"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                            Row {
                                spacing: 10
                                Slider {
                                    id: tw
                                    width: 170
                                    from: 1; to: 45
                                    value: App.tiltWarning
                                    onMoved: App.tiltWarning = value
                                }
                                Text { text: tw.value.toFixed(1); color: Theme.text; font.family: App.monoFont; font.pixelSize: 13; width: 40 }
                                Rectangle { width: 8; height: 8; radius: 4; color: Theme.warning; anchors.verticalCenter: parent.verticalCenter }
                            }
                        }
                        Column {
                            spacing: 6
                            width: 250
                            Text { text: "Tilt critical  °"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                            Row {
                                spacing: 10
                                Slider {
                                    id: tc
                                    width: 170
                                    from: 2; to: 60
                                    value: App.tiltCritical
                                    onMoved: App.tiltCritical = value
                                }
                                Text { text: tc.value.toFixed(1); color: Theme.text; font.family: App.monoFont; font.pixelSize: 13; width: 40 }
                                Rectangle { width: 8; height: 8; radius: 4; color: Theme.critical; anchors.verticalCenter: parent.verticalCenter }
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.borderSoft }

                    Text {
                        text: "PROBE PREVIEW"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }
                    Row {
                        spacing: 18
                        Column {
                            spacing: 4
                            Text { text: "Moisture %"; color: Theme.textMuted; font.pixelSize: 11; font.family: App.uiFont }
                            Slider { width: 160; from: 0; to: 100; value: probeM; onMoved: probeM = value }
                        }
                        Column {
                            spacing: 4
                            Text { text: "Tilt °"; color: Theme.textMuted; font.pixelSize: 11; font.family: App.uiFont }
                            Slider { width: 160; from: 0; to: 40; value: probeT; onMoved: probeT = value }
                        }
                        Rectangle {
                            width: 96
                            height: 36
                            radius: 4
                            color: Theme.bgInset
                            border.color: Theme.statusColor(App.probeStatus(probeM, probeT))
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter
                            Row {
                                anchors.centerIn: parent
                                spacing: 8
                                Rectangle {
                                    width: 8; height: 8; radius: 4
                                    color: Theme.statusColor(App.probeStatus(probeM, probeT))
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: Theme.statusLabel(App.probeStatus(probeM, probeT))
                                    color: Theme.statusColor(App.probeStatus(probeM, probeT))
                                    font.family: App.uiFont
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                    font.letterSpacing: 0.8
                                }
                            }
                        }
                    }
                }
            }

            // --- Nodes ---
            ListView {
                clip: true
                model: App.mapNodes
                spacing: 8
                delegate: Rectangle {
                    width: Math.min(640, stack.width)
                    height: 48
                    color: Theme.bgInset
                    border.color: Theme.border
                    radius: 4
                    Row {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 14
                        Rectangle {
                            width: 8; height: 8; radius: 4
                            color: Theme.statusColor(modelData.status)
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            width: 64
                            text: modelData.idString
                            color: Theme.text
                            font.family: App.monoFont
                            font.pixelSize: 13
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        TextField {
                            width: 360
                            height: 28
                            text: modelData.zone
                            color: Theme.text
                            font.family: App.uiFont
                            font.pixelSize: 12
                            background: Rectangle { color: Theme.bgCard; border.color: Theme.border; radius: 4 }
                            onEditingFinished: App.setZoneLabel(modelData.deviceId, text)
                        }
                    }
                }
            }

            // --- Link ---
            Column {
                spacing: 18
                Text {
                    text: "RECEIVER"
                    color: Theme.textDim
                    font.family: App.uiFont
                    font.pixelSize: 10
                    font.letterSpacing: 1.2
                    font.weight: Font.DemiBold
                }
                Row {
                    spacing: 16
                    Text { text: "Baud"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "115200"; color: Theme.text; font.family: App.monoFont; font.pixelSize: 13; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "8N1  ·  CRC-8/SMBUS over 11-byte payload"; color: Theme.textDim; font.pixelSize: 11; font.family: App.uiFont; anchors.verticalCenter: parent.verticalCenter }
                }
                Row {
                    spacing: 12
                    CheckBox {
                        id: simBox
                        checked: App.simulationEnabled
                        onToggled: App.simulationEnabled = checked
                        text: "Simulated telemetry (no radio)"
                        font.family: App.uiFont
                        font.pixelSize: 12
                        contentItem: Text {
                            text: simBox.text
                            color: Theme.text
                            font: simBox.font
                            leftPadding: simBox.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
                Text {
                    text: "Frame  [ ID 2B | Moisture 2B | TiltX 2B | TiltY 2B | TiltZ 2B | Status 1B | CRC 1B ]"
                    color: Theme.textDim
                    font.family: App.monoFont
                    font.pixelSize: 11
                }
            }
        }
    }
}
