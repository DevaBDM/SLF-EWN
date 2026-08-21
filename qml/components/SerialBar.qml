import QtQuick
import QtQuick.Controls

Item {
    id: root
    property bool expanded: true

    // The sidebar reads this so it stays open while the dropdown is on screen.
    // Without it the popup dies the instant the pointer leaves the rail.
    property bool popupOpen: portBox.popup.visible

    height: expanded ? 186 : 64
    Behavior on height { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }

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
                opacity: root.expanded ? 1 : 0
                visible: opacity > 0.02
                Behavior on opacity { NumberAnimation { duration: 140 } }
                text: App.connectionState === 4 ? "SIMULATED" : "SERIAL"
                color: App.connectionState === 4 ? Theme.warning : Theme.textDim
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
            enabled: Auth.isAdmin && App.connectionState !== 2
            width: parent.width
            height: 28
            model: App.availablePorts
            font.family: App.monoFont
            font.pixelSize: 11
            onActivated: function(index) { App.portName = App.availablePorts[index] }

            function selectCurrent() {
                const ports = App.availablePorts
                for (let i = 0; i < ports.length; ++i) {
                    if (ports[i] === App.portName) {
                        if (currentIndex !== i)
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
                border.color: portBox.popup.visible ? Theme.action : Theme.border
                border.width: 1
                radius: 4
            }
            contentItem: Text {
                leftPadding: 8
                rightPadding: 20
                elide: Text.ElideRight
                text: {
                    if (!App.availablePorts.length)
                        return "No ports found"
                    const i = portBox.currentIndex
                    if (i >= 0 && i < App.availablePortLabels.length)
                        return App.availablePortLabels[i]
                    return portBox.displayText
                }
                color: App.availablePorts.length ? Theme.text : Theme.textDim
                font: portBox.font
                verticalAlignment: Text.AlignVCenter
            }
            popup: Popup {
                y: portBox.height + 2
                width: portBox.width
                implicitHeight: Math.min(contentItem.implicitHeight + 2, 180)
                padding: 1
                background: Rectangle {
                    color: Theme.bgCard
                    border.color: Theme.border
                    border.width: 1
                    radius: 4
                }
                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: portBox.popup.visible ? portBox.delegateModel : null
                    currentIndex: portBox.highlightedIndex
                    ScrollIndicator.vertical: ScrollIndicator {}
                }
            }
            delegate: ItemDelegate {
                width: portBox.width
                height: 26
                highlighted: portBox.highlightedIndex === index
                background: Rectangle {
                    color: highlighted ? "#1A2430" : "transparent"
                }
                contentItem: Text {
                    leftPadding: 8
                    text: (index >= 0 && index < App.availablePortLabels.length)
                          ? App.availablePortLabels[index] : modelData
                    color: Theme.text
                    font.family: App.monoFont
                    font.pixelSize: 11
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Row {
            visible: root.expanded
            width: parent.width
            spacing: 8

            Text {
                text: "115200  8N1"
                color: Theme.textDim
                font.family: App.monoFont
                font.pixelSize: 10
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: "· rescan"
                color: rescanMouse.containsMouse ? Theme.action : Theme.textDim
                font.family: App.uiFont
                font.pixelSize: 10
                anchors.verticalCenter: parent.verticalCenter
                MouseArea {
                    id: rescanMouse
                    anchors.fill: parent
                    anchors.margins: -4
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: App.refreshPorts()
                }
            }
        }

        IndustrialButton {
            visible: root.expanded
            enabled: Auth.isAdmin
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
