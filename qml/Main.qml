import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: win
    visible: true
    width: 1440
    height: 900
    minimumWidth: 1080
    minimumHeight: 680
    title: "SLF-EWN  ·  Subsurface Early Warning"
    color: Theme.bgDeep
    font.family: App.uiFont
    palette.window: Theme.bgDeep
    palette.windowText: Theme.text
    palette.base: Theme.bgInset
    palette.text: Theme.text
    palette.button: Theme.bgCard
    palette.buttonText: Theme.text
    palette.highlight: "#243044"
    palette.highlightedText: Theme.text
    palette.placeholderText: Theme.textDim

    Sidebar {
        id: sidebar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        z: 2
    }

    TopBar {
        id: topbar
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.top: parent.top
        z: 1
    }

    Item {
        id: canvas
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.top: topbar.bottom
        anchors.bottom: parent.bottom

        DashboardPage {
            anchors.fill: parent
            opacity: App.page === 0 ? 1 : 0
            enabled: App.page === 0
            visible: opacity > 0.01
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
        AlertLogPage {
            anchors.fill: parent
            opacity: App.page === 1 ? 1 : 0
            enabled: App.page === 1
            visible: opacity > 0.01
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
        MapPage {
            anchors.fill: parent
            opacity: App.page === 2 ? 1 : 0
            enabled: App.page === 2
            visible: opacity > 0.01
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
        SettingsPage {
            anchors.fill: parent
            opacity: App.page === 3 ? 1 : 0
            enabled: App.page === 3
            visible: opacity > 0.01
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }

        DetailPanel {
            anchors.fill: parent
            z: 10
        }
    }
}
