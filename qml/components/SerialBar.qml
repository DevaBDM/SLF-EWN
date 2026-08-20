import QtQuick
import QtQuick.Controls

Item {
    id: root
    property bool expanded: true
    height: expanded ? 158 : 64

    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: Theme.borderSoft
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: expanded ? 12 : 8
        anchors.rightMargin: expanded ? 12 : 8
        anchors.topMargin: 12
        spacing: 8

        Row {
            spacing: 8
            anchors.horizontalCenter: expanded ? undefined : parent.horizontalCenter
            StatusDot {
                id: dot
                linkState: App.connectionState
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                visible: root.expanded
                text: "SERIAL"
                color: Theme.textDim
                font.family: App.uiFont
                font.pixelSize: 10
                font.letterSpacing: 1.1
                font.weight: Font.DemiBold
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        ComboBox {
            id: portBox
            visible: root.expanded
            width: parent.width
            height: 28
            model: App.availablePorts
            font.family: App.monoFont
            font.pixelSize: 11
            onActivated: App.portName = currentText
            function selectCurrent() {
                const ports = App.availablePorts
                for (let i = 0; i < ports.length; ++i) {
                    if (ports[i] === App.portName) {
                        currentIndex = i
                        return
                    }
                }
            }
            Component.onCompleted: selectCurrent()
            transform: Translate { id: shakeX }
            SequentialAnimation {
                id: shake
                running: false
                NumberAnimation { target: shakeX; property: "x"; to: 4; duration: 40 }
                NumberAnimation { target: shakeX; property: "x"; to: -4; duration: 40 }
                NumberAnimation { target: shakeX; property: "x"; to: 4; duration: 40 }
                NumberAnimation { target: shakeX; property: "x"; to: -4; duration: 40 }
                NumberAnimation { target: shakeX; property: "x"; to: 4; duration: 40 }
                NumberAnimation { target: shakeX; property: "x"; to: 0; duration: 40 }
            }
            Connections {
                target: App
                function onPortsChanged() { portBox.selectCurrent() }
                function onConnectionChanged() { portBox.selectCurrent() }
                function onSerialShake() { shake.restart() }
            }

            background: Rectangle {
                implicitHeight: 28
                color: Theme.bgInset
                border.color: Theme.border
                border.width: 1
                radius: 4
            }
            contentItem: Text {
                leftPadding: 8
                text: portBox.displayText.length ? portBox.displayText : "No ports"
                color: Theme.text
                font: portBox.font
                verticalAlignment: Text.AlignVCenter
            }
        }

        Text {
            visible: root.expanded
            text: "115200  8N1"
            color: Theme.textDim
            font.family: App.monoFont
            font.pixelSize: 10
        }

        IndustrialButton {
            visible: root.expanded
            width: parent.width
            compact: true
            primary: App.connectionState !== 2
            destructive: App.connectionState === 2
            text: App.connectionState === 2 ? "Disconnect" : (App.connectionState === 1 ? "Connecting…" : "Connect")
            onClicked: {
                if (App.connectionState === 2)
                    App.disconnectSerial()
                else {
                    App.refreshPorts()
                    App.connectSerial()
                }
            }
        }
    }
}
