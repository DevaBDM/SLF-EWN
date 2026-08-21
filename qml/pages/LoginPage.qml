import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: Theme.bgDeep

    property real cardOffset: 14
    property real cardOpacity: 0

    function submit() {
        if (Auth.login(userField.text, passField.text)) {
            passField.text = ""
        }
    }

    Component.onCompleted: {
        mount.start()
        userField.input.forceActiveFocus()
    }

    ParallelAnimation {
        id: mount
        NumberAnimation { target: root; property: "cardOpacity"; to: 1; duration: 260; easing.type: Easing.OutCubic }
        NumberAnimation { target: root; property: "cardOffset"; to: 0; duration: 260; easing.type: Easing.OutCubic }
    }

    Connections {
        target: Auth
        function onLoginRejected() { shake.restart() }
    }

    // ---------- left identity panel ----------
    Rectangle {
        id: brandPanel
        width: Math.max(360, parent.width * 0.42)
        height: parent.height
        color: Theme.bgSidebar

        Canvas {
            anchors.fill: parent
            opacity: 0.5
            onPaint: {
                const ctx = getContext("2d")
                ctx.reset()
                ctx.strokeStyle = Theme.borderSoft
                ctx.lineWidth = 1
                for (let x = 0; x < width; x += 28) {
                    ctx.beginPath()
                    ctx.moveTo(x + 0.5, 0)
                    ctx.lineTo(x + 0.5, height)
                    ctx.stroke()
                }
                for (let y = 0; y < height; y += 28) {
                    ctx.beginPath()
                    ctx.moveTo(0, y + 0.5)
                    ctx.lineTo(width, y + 0.5)
                    ctx.stroke()
                }
            }
            Component.onCompleted: requestPaint()
        }

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: Theme.border
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 52
            anchors.rightMargin: 52
            spacing: 14

            Row {
                spacing: 10
                Rectangle {
                    width: 3
                    height: 26
                    color: Theme.safe
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "SLF-EWN"
                    color: Theme.text
                    font.family: App.uiFont
                    font.pixelSize: 26
                    font.weight: Font.DemiBold
                    font.letterSpacing: 3
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Text {
                text: "Subterranean Landslide & Flood\nEarly-Warning Network"
                color: Theme.textMuted
                font.family: App.uiFont
                font.pixelSize: 14
                lineHeight: 1.35
            }

            Item { width: 1; height: 18 }

            Repeater {
                model: [
                    { k: "LINK", v: App.sourceLabel },
                    { k: "NODES", v: App.nodeCount + " configured" },
                    { k: "STORE", v: App.storeStatus }
                ]
                delegate: Row {
                    spacing: 14
                    Text {
                        width: 62
                        text: modelData.k
                        color: Theme.textFaint
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }
                    Text {
                        text: modelData.v
                        color: Theme.textDim
                        font.family: App.monoFont
                        font.pixelSize: 11
                    }
                }
            }
        }

        Text {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 52
            text: "Authorised personnel only · all sessions are logged"
            color: Theme.textFaint
            font.family: App.uiFont
            font.pixelSize: 11
        }
    }

    // ---------- credential card ----------
    Item {
        id: cardWrap
        anchors.left: brandPanel.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        opacity: root.cardOpacity

        Column {
            id: card
            width: 320
            anchors.centerIn: parent
            spacing: 16
            transform: Translate { id: shakeT; y: root.cardOffset }

            SequentialAnimation {
                id: shake
                NumberAnimation { target: shakeT; property: "x"; to: 5; duration: 45 }
                NumberAnimation { target: shakeT; property: "x"; to: -5; duration: 45 }
                NumberAnimation { target: shakeT; property: "x"; to: 4; duration: 45 }
                NumberAnimation { target: shakeT; property: "x"; to: 0; duration: 45 }
            }

            Column {
                spacing: 4
                Text {
                    text: "OPERATOR SIGN-IN"
                    color: Theme.textDim
                    font.family: App.uiFont
                    font.pixelSize: 10
                    font.letterSpacing: 1.4
                    font.weight: Font.DemiBold
                }
                Text {
                    text: "Monitoring console access"
                    color: Theme.text
                    font.family: App.uiFont
                    font.pixelSize: 18
                }
            }

            FieldInput {
                id: userField
                width: parent.width
                label: "OPERATOR"
                placeholder: "admin"
                onSubmitted: passField.input.forceActiveFocus()
            }

            FieldInput {
                id: passField
                width: parent.width
                label: "PASSWORD"
                placeholder: "••••••"
                password: true
                onSubmitted: root.submit()
            }

            Item {
                width: parent.width
                height: Auth.errorMessage.length ? errBanner.height : 0
                Behavior on height { NumberAnimation { duration: 140 } }

                MessageBanner {
                    id: errBanner
                    fillWidth: true
                    kind: "critical"
                    text: Auth.errorMessage
                    opacity: Auth.errorMessage.length ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: 140 } }
                }
            }

            IndustrialButton {
                width: parent.width
                primary: true
                enabled: !Auth.locked
                text: Auth.locked ? ("Locked · " + Auth.lockRemaining + " s") : "Sign in as Admin"
                onClicked: root.submit()
            }

            Row {
                width: parent.width
                spacing: 10
                Rectangle { width: (parent.width - 30) / 2; height: 1; color: Theme.borderSoft; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    text: "OR"
                    color: Theme.textFaint
                    font.family: App.uiFont
                    font.pixelSize: 10
                    font.letterSpacing: 1
                    anchors.verticalCenter: parent.verticalCenter
                }
                Rectangle { width: (parent.width - 30) / 2; height: 1; color: Theme.borderSoft; anchors.verticalCenter: parent.verticalCenter }
            }

            IndustrialButton {
                width: parent.width
                destructive: true
                enabled: !Auth.locked
                text: "Continue as Guest — read only"
                onClicked: Auth.loginAsGuest()
            }

            Text {
                width: parent.width
                text: "Guests can watch live telemetry, charts and the alert log, but cannot acknowledge alarms or change thresholds."
                color: Theme.textFaint
                font.family: App.uiFont
                font.pixelSize: 10
                wrapMode: Text.WordWrap
                lineHeight: 1.3
            }

            MessageBanner {
                visible: Auth.defaultAdminPassword
                fillWidth: true
                kind: "warning"
                text: "First run — sign in with admin / admin, then change the password in Settings → Access."
            }
        }
    }
}
