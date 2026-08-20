import QtQuick

Item {
    id: root
    property bool primary: false
    property bool destructive: false
    property bool compact: false
    property alias text: label.text
    property bool hovered: mouse.containsMouse
    signal clicked()

    implicitWidth: Math.max(compact ? 72 : 96, label.implicitWidth + (compact ? 16 : 22))
    implicitHeight: compact ? 28 : 32

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 4
        color: {
            if (root.primary)
                return mouse.containsMouse ? "#6BB0FF" : Theme.action
            if (mouse.containsMouse)
                return "#1E242E"
            return root.destructive ? "transparent" : Theme.bgCard
        }
        border.width: 1
        border.color: {
            if (root.primary)
                return bg.color
            if (root.destructive)
                return mouse.containsMouse ? "#5C6573" : "#3A4250"
            return Theme.border
        }

        Behavior on color { ColorAnimation { duration: 100 } }
        Behavior on border.color { ColorAnimation { duration: 100 } }
    }

    Text {
        id: label
        anchors.centerIn: parent
        color: root.primary ? "#0E1218" : Theme.text
        font.family: App.uiFont
        font.pixelSize: compact ? 11 : 12
        font.weight: root.primary ? Font.DemiBold : Font.Normal
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
