import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Item {
    id: root

    Rectangle {
        id: filterBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 16
        height: 44
        color: Theme.bgCard
        border.color: Theme.border
        border.width: 1
        radius: 6

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 12
            spacing: 10

            ComboBox {
                id: typeBox
                width: 190
                height: 28
                model: ["All events", "Warning", "Critical", "Emergency-Interrupt"]
                font.family: App.uiFont
                font.pixelSize: 12
                onActivated: App.alerts.typeFilter = index === 0 ? -1 : index - 1
                background: Rectangle { color: Theme.bgInset; border.color: Theme.border; radius: 4 }
                contentItem: Text {
                    text: typeBox.displayText
                    color: Theme.text
                    font: typeBox.font
                    leftPadding: 8
                    verticalAlignment: Text.AlignVCenter
                }
            }
            ComboBox {
                id: zoneBox
                width: 240
                height: 28
                model: App.zoneNames
                font.family: App.uiFont
                font.pixelSize: 12
                displayText: currentIndex <= 0 ? "All zones" : currentText
                onActivated: App.alerts.zoneFilter = index <= 0 ? "" : currentText
                background: Rectangle { color: Theme.bgInset; border.color: Theme.border; radius: 4 }
                contentItem: Text {
                    text: zoneBox.displayText
                    color: Theme.text
                    font: zoneBox.font
                    leftPadding: 8
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
            ComboBox {
                id: dateBox
                width: 140
                height: 28
                model: ["Any time", "Last 24h", "Last 7d"]
                font.family: App.uiFont
                font.pixelSize: 12
                onActivated: App.alerts.dateFilter = index
                background: Rectangle { color: Theme.bgInset; border.color: Theme.border; radius: 4 }
                contentItem: Text {
                    text: dateBox.displayText
                    color: Theme.text
                    font: dateBox.font
                    leftPadding: 8
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 12
            spacing: 8
            IndustrialButton {
                destructive: true
                text: "Clear Log"
                onClicked: App.clearAlerts()
            }
            IndustrialButton {
                primary: true
                text: "Export CSV"
                onClicked: saveDialog.open()
            }
        }
    }

    FileDialog {
        id: saveDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["CSV files (*.csv)"]
        defaultSuffix: "csv"
        onAccepted: App.exportAlerts(selectedFile)
    }

    Rectangle {
        anchors.top: filterBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16
        anchors.topMargin: 10
        color: Theme.bgCard
        border.color: Theme.border
        border.width: 1
        radius: 6
        clip: true

        Row {
            id: header
            x: 12
            y: 0
            height: 36
            spacing: 0
            Repeater {
                model: [
                    { t: "TIMESTAMP", w: 190 },
                    { t: "DEVICE", w: 90 },
                    { t: "ZONE", w: 240 },
                    { t: "EVENT", w: 170 },
                    { t: "TRIGGER", w: 180 },
                    { t: "ACK", w: 50 }
                ]
                delegate: Text {
                    width: modelData.w
                    height: 36
                    text: modelData.t
                    color: Theme.textMuted
                    font.family: App.uiFont
                    font.pixelSize: 10
                    font.letterSpacing: 0.8
                    font.weight: Font.DemiBold
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Rectangle {
            anchors.top: header.bottom
            width: parent.width
            height: 1
            color: Theme.border
        }

        ListView {
            id: list
            anchors.top: header.bottom
            anchors.topMargin: 1
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            clip: true
            model: App.alerts
            spacing: 0
            boundsBehavior: Flickable.StopAtBounds

            add: Transition {
                NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 250 }
                NumberAnimation { property: "y"; duration: 250; easing.type: Easing.OutCubic }
            }
            addDisplaced: Transition {
                NumberAnimation { property: "y"; duration: 250; easing.type: Easing.OutCubic }
            }

            opacity: 1
            Behavior on opacity { NumberAnimation { duration: 150 } }
            Connections {
                target: App.alerts
                function onFilterChanged() {
                    list.opacity = 0
                    fadeBack.start()
                }
            }
            PauseAnimation {
                id: fadeBack
                duration: 80
                onStopped: list.opacity = 1
            }

            delegate: Item {
                id: row
                width: list.width
                height: 40
                property real flash: fresh ? 0.35 : 0

                NumberAnimation on flash {
                    running: fresh
                    from: 0.35
                    to: 0
                    duration: 1000
                    easing.type: Easing.OutQuad
                }

                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(Theme.action.r, Theme.action.g, Theme.action.b, row.flash)
                }
                Rectangle {
                    anchors.fill: parent
                    color: index % 2 ? "#181D25" : "transparent"
                    opacity: 1 - row.flash
                }

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    spacing: 0
                    Text {
                        width: 190; height: parent.height
                        text: timestampText
                        color: Theme.text
                        font.family: App.monoFont
                        font.pixelSize: 12
                        verticalAlignment: Text.AlignVCenter
                    }
                    Text {
                        width: 90; height: parent.height
                        text: idString
                        color: Theme.text
                        font.family: App.monoFont
                        font.pixelSize: 12
                        verticalAlignment: Text.AlignVCenter
                    }
                    Text {
                        width: 240; height: parent.height
                        text: zone
                        elide: Text.ElideRight
                        color: Theme.textMuted
                        font.family: App.uiFont
                        font.pixelSize: 12
                        verticalAlignment: Text.AlignVCenter
                    }
                    Item {
                        width: 170; height: parent.height
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: tag.implicitWidth + 12
                            height: 18
                            radius: 3
                            color: eventType === 0 ? "#2A2114" : "#2A1A18"
                            border.color: Theme.statusColor(eventType === 0 ? 2 : 3)
                            border.width: 1
                            Text {
                                id: tag
                                anchors.centerIn: parent
                                text: typeLabel
                                color: Theme.statusColor(eventType === 0 ? 2 : 3)
                                font.family: App.uiFont
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                            }
                        }
                    }
                    Text {
                        width: 180; height: parent.height
                        text: triggerValue
                        color: Theme.text
                        font.family: App.monoFont
                        font.pixelSize: 12
                        verticalAlignment: Text.AlignVCenter
                    }
                    Item {
                        width: 50; height: parent.height
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 14
                            height: 14
                            radius: 3
                            color: acknowledged ? Theme.action : Theme.bgInset
                            border.color: acknowledged ? Theme.action : "#3A4250"
                            border.width: 1
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: App.acknowledgeAlert(alertId)
                            }
                        }
                    }
                }
            }
        }
    }
}
