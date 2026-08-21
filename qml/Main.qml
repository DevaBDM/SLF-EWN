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

    Item {
        id: shell
        anchors.fill: parent
        opacity: Auth.authenticated ? 1 : 0
        enabled: Auth.authenticated
        visible: opacity > 0.01
        Behavior on opacity { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

        // The layout reserves only the collapsed rail. Hover-expansion floats
        // over the canvas, so content never shifts under the pointer; pinning is
        // the one deliberate action that reflows.
        Item {
            id: railSpace
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: sidebar.pinned ? sidebar.panelWidth : sidebar.railWidth
            Behavior on width { NumberAnimation { duration: 170; easing.type: Easing.OutCubic } }
        }

        Sidebar {
            id: sidebar
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            z: 20
        }

        TopBar {
            id: topbar
            anchors.left: railSpace.right
            anchors.right: parent.right
            anchors.top: parent.top
            z: 1
        }

        Item {
            id: canvas
            anchors.left: railSpace.right
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

    Component {
        id: loginComponent
        LoginPage {}
    }

    Loader {
        id: loginLoader
        anchors.fill: parent
        z: 100
        active: true
        sourceComponent: loginComponent
        opacity: Auth.authenticated ? 0 : 1
        visible: opacity > 0.01
        Behavior on opacity { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }

        // Drop the credential form once it has faded out so a later sign-out
        // always starts from empty fields.
        onOpacityChanged: if (opacity < 0.01 && Auth.authenticated) active = false

        Connections {
            target: Auth
            function onLoggedOut() { loginLoader.active = true }
        }
    }

    // Read-only guests get a single explanatory toast instead of silent no-ops.
    Rectangle {
        id: toast
        z: 200
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        width: toastText.implicitWidth + 28
        height: toastText.implicitHeight + 16
        radius: 10
        color: Theme.warning
        opacity: 0
        visible: opacity > 0.01

        Text {
            id: toastText
            anchors.centerIn: parent
            text: "Read-only session — sign in as admin to change this"
            color: "#FFFFFF"
            font.family: App.uiFont
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        SequentialAnimation {
            id: toastAnim
            NumberAnimation { target: toast; property: "opacity"; to: 1; duration: 140 }
            PauseAnimation { duration: 2200 }
            NumberAnimation { target: toast; property: "opacity"; to: 0; duration: 260 }
        }

        Connections {
            target: App
            function onPermissionDenied() { toastAnim.restart() }
        }
    }
}
