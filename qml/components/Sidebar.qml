import QtQuick

Rectangle {
    id: root
    color: Theme.bgSidebar
    width: hovered ? 240 : 72
    clip: true

    property bool hovered: hover.containsMouse || serialHover.containsMouse

    Behavior on width { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }

    Rectangle {
        anchors.right: parent.right
        width: 1
        height: parent.height
        color: Theme.borderSoft
    }

    MouseArea {
        id: hover
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }

    Column {
        id: brand
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 18
        anchors.leftMargin: root.hovered ? 20 : 0
        spacing: 2

        Text {
            text: root.hovered ? "SLF-EWN" : "SLF"
            color: Theme.text
            font.family: App.uiFont
            font.pixelSize: 13
            font.weight: Font.DemiBold
            font.letterSpacing: 1.4
            anchors.horizontalCenter: root.hovered ? undefined : parent.horizontalCenter
        }
        Text {
            visible: root.hovered
            text: "SUBSURFACE MONITOR"
            color: Theme.textDim
            font.family: App.uiFont
            font.pixelSize: 10
            font.letterSpacing: 0.6
        }
    }

    Column {
        anchors.top: brand.bottom
        anchors.topMargin: 28
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 4

        NavItem { pageIndex: 0; title: "Dashboard"; icon: "dashboard"; expanded: root.hovered; onClicked: App.page = 0 }
        NavItem { pageIndex: 1; title: "Alert Log"; icon: "alerts"; expanded: root.hovered; onClicked: App.page = 1 }
        NavItem { pageIndex: 2; title: "Map View"; icon: "map"; expanded: root.hovered; onClicked: App.page = 2 }
        NavItem { pageIndex: 3; title: "Settings"; icon: "settings"; expanded: root.hovered; onClicked: App.page = 3 }
    }

    MouseArea {
        id: serialHover
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: serial.height + 8
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }

    SerialBar {
        id: serial
        expanded: root.hovered
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
