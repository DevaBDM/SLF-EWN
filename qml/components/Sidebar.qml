import QtQuick

// Overlay rail. The collapsed width is reserved once by the shell; expanding
// draws on top of the canvas instead of resizing it, so hovering the nav never
// reflows the dashboard underneath or moves what the operator was aiming at.
Rectangle {
    id: root
    color: Theme.bgSidebar
    clip: true

    readonly property int railWidth: 72
    readonly property int panelWidth: 240

    // Pinned wins over hover so the panel can be locked open while working in
    // it. Popup tracking keeps it open when the cursor moves onto a dropdown
    // that renders outside the sidebar's own bounds.
    property bool pinned: false
    property bool expanded: pinned || hover.containsMouse || serial.popupOpen

    width: expanded ? panelWidth : railWidth
    Behavior on width { NumberAnimation { duration: 170; easing.type: Easing.OutCubic } }

    // Depth cue only while floating over content.
    Rectangle {
        anchors.left: parent.right
        width: 12
        height: parent.height
        opacity: root.expanded && !root.pinned ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 170 } }
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#66000000" }
            GradientStop { position: 1.0; color: "#00000000" }
        }
    }

    Rectangle {
        anchors.right: parent.right
        width: 1
        height: parent.height
        color: Theme.borderSoft
    }

    // Hover sensor only: it never accepts buttons, so every click falls through
    // to the controls stacked above it.
    MouseArea {
        id: hover
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }

    Item {
        id: brand
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 18
        height: 34

        Column {
            anchors.left: parent.left
            anchors.leftMargin: root.expanded ? 20 : 0
            anchors.right: parent.right
            Behavior on anchors.leftMargin { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }
            spacing: 2

            Text {
                text: root.expanded ? "SLF-EWN" : "SLF"
                color: Theme.text
                font.family: App.uiFont
                font.pixelSize: 13
                font.weight: Font.DemiBold
                font.letterSpacing: 1.4
                anchors.horizontalCenter: root.expanded ? undefined : parent.horizontalCenter
            }
            Text {
                opacity: root.expanded ? 1 : 0
                visible: opacity > 0.02
                Behavior on opacity { NumberAnimation { duration: 140 } }
                text: "SUBSURFACE MONITOR"
                color: Theme.textDim
                font.family: App.uiFont
                font.pixelSize: 10
                font.letterSpacing: 0.6
            }
        }

        // Lock the panel open. Without this the rail is hover-only, and any
        // control inside it disappears the moment you reach for something else.
        Rectangle {
            id: pinBtn
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.top: parent.top
            width: 24
            height: 24
            radius: 4
            opacity: root.expanded ? 1 : 0
            visible: opacity > 0.02
            Behavior on opacity { NumberAnimation { duration: 140 } }
            color: pinMouse.containsMouse ? "#1A2430" : "transparent"
            border.color: root.pinned ? Theme.action : "transparent"
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: root.pinned ? "\u00AB" : "\u00BB"
                color: root.pinned ? Theme.action : Theme.textMuted
                font.family: App.uiFont
                font.pixelSize: 13
            }

            MouseArea {
                id: pinMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.pinned = !root.pinned
            }

            ToolTipLite {
                show: pinMouse.containsMouse
                text: root.pinned ? "Unpin sidebar" : "Keep sidebar open"
            }
        }
    }

    Column {
        anchors.top: brand.bottom
        anchors.topMargin: 28
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 4

        NavItem { pageIndex: 0; title: "Dashboard"; icon: "dashboard"; expanded: root.expanded; onClicked: App.page = 0 }
        NavItem { pageIndex: 1; title: "Alert Log"; icon: "alerts"; expanded: root.expanded; onClicked: App.page = 1 }
        NavItem { pageIndex: 2; title: "Map View"; icon: "map"; expanded: root.expanded; onClicked: App.page = 2 }
        NavItem { pageIndex: 3; title: "Settings"; icon: "settings"; expanded: root.expanded; onClicked: App.page = 3 }
    }

    SerialBar {
        id: serial
        expanded: root.expanded
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
