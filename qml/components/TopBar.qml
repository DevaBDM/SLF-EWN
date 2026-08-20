import QtQuick

Rectangle {
    id: root
    color: Theme.bgTopBar
    height: 52

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: Theme.borderSoft
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        spacing: 18

        Rectangle {
            id: pill
            width: pillRow.implicitWidth + 18
            height: 26
            radius: 13
            color: Theme.bgCard
            border.color: Theme.border
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter

            Row {
                id: pillRow
                anchors.centerIn: parent
                spacing: 8
                StatusDot {
                    linkState: App.connectionState
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: {
                        if (App.connectionState === 4)
                            return "SIMULATION"
                        if (App.connectionState === 2)
                            return App.portName
                        if (App.connectionState === 1)
                            return "LINKING"
                        if (App.connectionState === 3)
                            return "LINK FAIL"
                        return "OFFLINE"
                    }
                    color: Theme.text
                    font.family: App.monoFont
                    font.pixelSize: 11
                    anchors.verticalCenter: parent.verticalCenter
                }
                Rectangle {
                    visible: App.connectionState === 4
                    width: simLabel.implicitWidth + 10
                    height: 14
                    radius: 3
                    color: "#2A2416"
                    border.color: "#5A4A22"
                    border.width: 1
                    Text {
                        id: simLabel
                        anchors.centerIn: parent
                        text: "SIM"
                        color: Theme.warning
                        font.family: App.uiFont
                        font.pixelSize: 9
                        font.weight: Font.DemiBold
                        font.letterSpacing: 0.8
                    }
                }
            }
        }

        Column {
            spacing: 1
            anchors.verticalCenter: parent.verticalCenter
            Text {
                text: (App.liveNodeCount < 10 ? "0" : "") + App.liveNodeCount
                color: Theme.text
                font.family: App.monoFont
                font.pixelSize: 13
            }
            Text {
                text: "NODES"
                color: Theme.textDim
                font.family: App.uiFont
                font.pixelSize: 10
                font.letterSpacing: 1.1
            }
        }

        Rectangle {
            visible: App.alertCount > 0
            width: alertRow.implicitWidth + 14
            height: 22
            radius: 9
            color: "#2A1A18"
            border.color: "#5A2C28"
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter
            Row {
                id: alertRow
                anchors.centerIn: parent
                spacing: 6
                Rectangle { width: 6; height: 6; radius: 3; color: Theme.critical; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    text: App.alertCount + " ALERT" + (App.alertCount === 1 ? "" : "S")
                    color: Theme.critical
                    font.family: App.monoFont
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    Text {
        id: clock
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        text: App.clock
        color: Theme.text
        font.family: App.monoFont
        font.pixelSize: 13
    }
}
