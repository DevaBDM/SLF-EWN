import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property real probeM: 30
    property real probeT: 4
    property var userRows: []

    readonly property bool admin: Auth.isAdmin

    function reloadUsers() {
        userRows = Auth.userList()
    }

    Component.onCompleted: reloadUsers()

    Connections {
        target: Auth
        function onUsersChanged() { root.reloadUsers() }
        function onSessionChanged() { root.reloadUsers() }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 16
        color: Theme.bgCard
        border.color: Theme.border
        border.width: 1
        radius: 6

        Row {
            id: tabs
            property int currentIndex: 0
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 8
            height: 40
            spacing: 2

            Repeater {
                model: ["THRESHOLDS", "NODES", "LINK", "ACCESS"]
                delegate: Item {
                    width: tabLabel.implicitWidth + 34
                    height: 40

                    Text {
                        id: tabLabel
                        anchors.centerIn: parent
                        text: modelData
                        color: tabs.currentIndex === index ? Theme.text
                                                           : (tabMouse.containsMouse ? Theme.text : Theme.textMuted)
                        font.family: App.uiFont
                        font.pixelSize: 12
                        font.letterSpacing: 0.8
                        font.weight: tabs.currentIndex === index ? Font.DemiBold : Font.Normal
                        Behavior on color { ColorAnimation { duration: 110 } }
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width - 16
                        height: 2
                        color: tabs.currentIndex === index ? Theme.action : "transparent"
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }

                    MouseArea {
                        id: tabMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: tabs.currentIndex = index
                    }
                }
            }
        }

        Rectangle {
            anchors.top: tabs.bottom
            width: parent.width
            height: 1
            color: Theme.border
        }

        Rectangle {
            id: readOnlyBanner
            visible: !root.admin
            anchors.top: tabs.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 12
            anchors.topMargin: 10
            height: 36
            radius: 10
            color: Theme.warning
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                text: "Read-only session — configuration is locked for guest operators."
                color: "#FFFFFF"
                font.family: App.uiFont
                font.pixelSize: 12
                font.weight: Font.DemiBold
            }
        }

        StackLayout {
            id: stack
            anchors.top: readOnlyBanner.visible ? readOnlyBanner.bottom : tabs.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 24
            currentIndex: tabs.currentIndex

            // ---------------- Thresholds ----------------
            Flickable {
                clip: true
                contentHeight: threshCol.height
                Column {
                    id: threshCol
                    width: Math.min(560, stack.width)
                    spacing: 22
                    enabled: root.admin
                    opacity: root.admin ? 1 : 0.55

                    Text {
                        text: "ALARM LIMITS"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }

                    Grid {
                        columns: 2
                        columnSpacing: 28
                        rowSpacing: 18
                        width: parent.width

                        Column {
                            spacing: 6
                            width: 250
                            Text { text: "Moisture warning  %"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                            Row {
                                spacing: 10
                                Slider {
                                    id: mw
                                    width: 170
                                    from: 5; to: 90
                                    value: App.moistureWarning
                                    onMoved: App.moistureWarning = value
                                }
                                Text { text: mw.value.toFixed(1); color: Theme.text; font.family: App.monoFont; font.pixelSize: 13; width: 40 }
                                Rectangle { width: 8; height: 8; radius: 4; color: Theme.warning; anchors.verticalCenter: parent.verticalCenter }
                            }
                        }
                        Column {
                            spacing: 6
                            width: 250
                            Text { text: "Moisture critical  %"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                            Row {
                                spacing: 10
                                Slider {
                                    id: mc
                                    width: 170
                                    from: 10; to: 95
                                    value: App.moistureCritical
                                    onMoved: App.moistureCritical = value
                                }
                                Text { text: mc.value.toFixed(1); color: Theme.text; font.family: App.monoFont; font.pixelSize: 13; width: 40 }
                                Rectangle { width: 8; height: 8; radius: 4; color: Theme.critical; anchors.verticalCenter: parent.verticalCenter }
                            }
                        }
                        Column {
                            spacing: 6
                            width: 250
                            Text { text: "Tilt warning  °"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                            Row {
                                spacing: 10
                                Slider {
                                    id: tw
                                    width: 170
                                    from: 1; to: 45
                                    value: App.tiltWarning
                                    onMoved: App.tiltWarning = value
                                }
                                Text { text: tw.value.toFixed(1); color: Theme.text; font.family: App.monoFont; font.pixelSize: 13; width: 40 }
                                Rectangle { width: 8; height: 8; radius: 4; color: Theme.warning; anchors.verticalCenter: parent.verticalCenter }
                            }
                        }
                        Column {
                            spacing: 6
                            width: 250
                            Text { text: "Tilt critical  °"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                            Row {
                                spacing: 10
                                Slider {
                                    id: tc
                                    width: 170
                                    from: 2; to: 60
                                    value: App.tiltCritical
                                    onMoved: App.tiltCritical = value
                                }
                                Text { text: tc.value.toFixed(1); color: Theme.text; font.family: App.monoFont; font.pixelSize: 13; width: 40 }
                                Rectangle { width: 8; height: 8; radius: 4; color: Theme.critical; anchors.verticalCenter: parent.verticalCenter }
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.borderSoft }

                    Text {
                        text: "PROBE PREVIEW"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }
                    Row {
                        spacing: 18
                        Column {
                            spacing: 4
                            Text { text: "Moisture %"; color: Theme.textMuted; font.pixelSize: 11; font.family: App.uiFont }
                            Slider { width: 160; from: 0; to: 100; value: probeM; onMoved: probeM = value }
                        }
                        Column {
                            spacing: 4
                            Text { text: "Tilt °"; color: Theme.textMuted; font.pixelSize: 11; font.family: App.uiFont }
                            Slider { width: 160; from: 0; to: 40; value: probeT; onMoved: probeT = value }
                        }
                        MessageBanner {
                            width: 110
                            compact: false
                            kind: {
                                const s = App.probeStatus(probeM, probeT)
                                if (s === 3) return "critical"
                                if (s === 2) return "warning"
                                if (s === 1) return "safe"
                                return "info"
                            }
                            text: Theme.statusLabel(App.probeStatus(probeM, probeT))
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            // ---------------- Nodes ----------------
            ListView {
                clip: true
                model: App.mapNodes
                spacing: 8
                delegate: Rectangle {
                    width: Math.min(640, stack.width)
                    height: 48
                    color: Theme.bgInset
                    border.color: Theme.border
                    radius: 4
                    Row {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 14
                        Rectangle {
                            width: 8; height: 8; radius: 4
                            color: Theme.statusColor(modelData.status)
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            width: 64
                            text: modelData.idString
                            color: Theme.text
                            font.family: App.monoFont
                            font.pixelSize: 13
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        TextField {
                            width: 360
                            height: 28
                            enabled: root.admin
                            text: modelData.zone
                            color: Theme.text
                            font.family: App.uiFont
                            font.pixelSize: 12
                            background: Rectangle { color: Theme.bgCard; border.color: Theme.border; radius: 4 }
                            onEditingFinished: App.setZoneLabel(modelData.deviceId, text)
                        }
                    }
                }
            }

            // ---------------- Link ----------------
            Flickable {
                clip: true
                contentHeight: linkCol.height

                Column {
                    id: linkCol
                    width: Math.min(680, stack.width)
                    spacing: 18

                    // --- where the numbers on screen are actually coming from ---
                    Text {
                        text: "DATA SOURCE"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }

                    Rectangle {
                        width: parent.width
                        height: 40
                        radius: 4
                        color: App.connectionState === 4 ? "#1F1A12" : Theme.bgInset
                        border.width: 1
                        border.color: App.connectionState === 2 ? Theme.safe
                                    : (App.connectionState === 4 ? Theme.warning : Theme.border)

                        Row {
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 10
                            Rectangle {
                                width: 8; height: 8; radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                color: App.connectionState === 2 ? Theme.safe
                                     : (App.connectionState === 4 ? Theme.warning : Theme.textMuted)
                            }
                            Text {
                                text: App.sourceLabel
                                color: App.connectionState === 4 ? Theme.warning : Theme.text
                                font.family: App.uiFont
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }

                    CheckBox {
                        id: simBox
                        enabled: root.admin
                        checked: App.simulationEnabled
                        onClicked: App.simulationEnabled = checked
                        text: "Use simulated telemetry (demo only — live port data is paused)"
                        font.family: App.uiFont
                        font.pixelSize: 12
                        contentItem: Text {
                            text: simBox.text
                            color: Theme.text
                            font: simBox.font
                            leftPadding: simBox.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "Off by default. When unchecked, the dashboard auto-opens the USB-UART receiver at 115200 8N1 and stores every decoded frame."
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 11
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.borderSoft }

                    // --- port selection, reachable without hovering the sidebar ---
                    Text {
                        text: "RECEIVER PORT"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }

                    Row {
                        spacing: 10

                        ComboBox {
                            id: settingsPort
                            enabled: root.admin && App.connectionState !== 2
                            width: 220
                            height: 32
                            model: App.availablePorts
                            font.family: App.monoFont
                            font.pixelSize: 12
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
                            Connections {
                                target: App
                                function onPortsChanged() { settingsPort.selectCurrent() }
                                function onConnectionChanged() { settingsPort.selectCurrent() }
                            }

                            background: Rectangle {
                                implicitHeight: 32
                                color: Theme.bgInset
                                border.color: settingsPort.popup.visible ? Theme.action : Theme.border
                                border.width: 1
                                radius: 4
                            }
                            contentItem: Text {
                                leftPadding: 10
                                rightPadding: 22
                                elide: Text.ElideRight
                                text: {
                                    if (!App.availablePorts.length)
                                        return "No serial ports detected"
                                    const i = settingsPort.currentIndex
                                    if (i >= 0 && i < App.availablePortLabels.length)
                                        return App.availablePortLabels[i]
                                    return settingsPort.displayText
                                }
                                color: App.availablePorts.length ? Theme.text : Theme.textDim
                                font: settingsPort.font
                                verticalAlignment: Text.AlignVCenter
                            }
                            popup: Popup {
                                y: settingsPort.height + 2
                                width: settingsPort.width
                                implicitHeight: Math.min(contentItem.implicitHeight + 2, 200)
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
                                    model: settingsPort.popup.visible ? settingsPort.delegateModel : null
                                    currentIndex: settingsPort.highlightedIndex
                                    ScrollIndicator.vertical: ScrollIndicator {}
                                }
                            }
                            delegate: ItemDelegate {
                                width: settingsPort.width
                                height: 28
                                highlighted: settingsPort.highlightedIndex === index
                                background: Rectangle { color: highlighted ? "#1A2430" : "transparent" }
                                contentItem: Text {
                                    leftPadding: 10
                                    text: (index >= 0 && index < App.availablePortLabels.length)
                                          ? App.availablePortLabels[index] : modelData
                                    color: Theme.text
                                    font.family: App.monoFont
                                    font.pixelSize: 12
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }

                        IndustrialButton {
                            text: "Rescan"
                            compact: true
                            onClicked: App.refreshPorts()
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        IndustrialButton {
                            enabled: root.admin
                            primary: App.connectionState !== 2
                            destructive: App.connectionState === 2
                            compact: true
                            text: App.connectionState === 2 ? "Disconnect"
                                : (App.connectionState === 1 ? "Connecting…" : "Connect")
                            onClicked: {
                                if (App.connectionState === 2)
                                    App.disconnectSerial()
                                else {
                                    App.refreshPorts()
                                    App.connectSerial()
                                }
                            }
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Text {
                        text: "115200  8N1  ·  binary 12-byte frame  or  ASCII CSV  ID,moisture,tiltX,tiltY,tiltZ"
                        color: Theme.textDim
                        font.family: App.monoFont
                        font.pixelSize: 11
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.borderSoft }

                    // --- proof that bytes are arriving and decoding ---
                    Text {
                        text: "LINK HEALTH"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }

                    Grid {
                        columns: 4
                        columnSpacing: 12
                        rowSpacing: 10
                        width: parent.width

                        Repeater {
                            model: [
                                { k: "BYTES IN", v: App.rxBytes.toString() },
                                { k: "FRAMES OK", v: App.rxFrames.toString() },
                                { k: "BYTES DROPPED", v: App.rxDiscarded.toString() },
                                { k: "LAST FRAME", v: App.rxLastFrame }
                            ]
                            delegate: Rectangle {
                                width: (linkCol.width - 36) / 4
                                height: 52
                                radius: 4
                                color: Theme.bgInset
                                border.color: Theme.border
                                border.width: 1
                                Column {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 10
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 3
                                    Text {
                                        text: modelData.k
                                        color: Theme.textDim
                                        font.family: App.uiFont
                                        font.pixelSize: 9
                                        font.letterSpacing: 0.9
                                        font.weight: Font.DemiBold
                                    }
                                    Text {
                                        text: modelData.v
                                        color: Theme.text
                                        font.family: App.monoFont
                                        font.pixelSize: 15
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: diagText.implicitHeight + 20
                        radius: 4
                        color: Theme.bgInset
                        border.width: 1
                        border.color: App.connectionState === 2 && App.rxFrames > 0 ? Theme.safe
                                    : (App.connectionState === 2 ? Theme.warning : Theme.border)
                        Text {
                            id: diagText
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            wrapMode: Text.WordWrap
                            text: App.linkDiagnosis
                            color: App.connectionState === 2 && App.rxFrames > 0 ? Theme.safe : Theme.textMuted
                            font.family: App.uiFont
                            font.pixelSize: 12
                        }
                    }

                    Column {
                        width: parent.width
                        spacing: 6
                        visible: App.rxTail.length > 0
                        Text {
                            text: "LAST BYTES ON THE WIRE"
                            color: Theme.textDim
                            font.family: App.uiFont
                            font.pixelSize: 9
                            font.letterSpacing: 0.9
                            font.weight: Font.DemiBold
                        }
                        Rectangle {
                            width: parent.width
                            height: hexText.implicitHeight + 16
                            radius: 4
                            color: "#0E1117"
                            border.color: Theme.border
                            border.width: 1
                            Text {
                                id: hexText
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.margins: 8
                                anchors.verticalCenter: parent.verticalCenter
                                text: App.rxTail
                                color: Theme.textMuted
                                wrapMode: Text.WrapAnywhere
                                font.family: App.monoFont
                                font.pixelSize: 11
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.borderSoft }

                    // --- local store ---
                    Text {
                        text: "LOCAL STORE"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }

                    Grid {
                        columns: 2
                        columnSpacing: 40
                        rowSpacing: 8
                        width: parent.width

                        Text { text: "Engine"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                        Text { text: App.storeStatus; color: Theme.text; font.family: App.monoFont; font.pixelSize: 12 }

                        Text { text: "Stored readings"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                        Text { text: App.storedSamples + " rows  ·  30-day retention"; color: Theme.text; font.family: App.monoFont; font.pixelSize: 12 }

                        Text { text: "Stored alerts"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                        Text { text: App.storedAlerts + " rows"; color: Theme.text; font.family: App.monoFont; font.pixelSize: 12 }

                        Text { text: "Written this session"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                        Text {
                            text: App.storeRows + " rows  ·  " + App.storePending + " buffered"
                            color: App.storeRows > 0 ? Theme.safe : Theme.textMuted
                            font.family: App.monoFont
                            font.pixelSize: 12
                        }

                        Text { text: "Last write"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                        Text { text: App.storeLastWrite; color: Theme.text; font.family: App.monoFont; font.pixelSize: 12 }

                        Text { text: "File size"; color: Theme.textMuted; font.pixelSize: 12; font.family: App.uiFont }
                        Text { text: App.storeSize; color: Theme.text; font.family: App.monoFont; font.pixelSize: 12 }
                    }

                    Text {
                        width: parent.width
                        text: App.storePath
                        color: Theme.textDim
                        font.family: App.monoFont
                        font.pixelSize: 11
                        wrapMode: Text.WrapAnywhere
                    }

                    Row {
                        spacing: 10
                        IndustrialButton {
                            compact: true
                            text: "Copy path"
                            onClicked: App.copyToClipboard(App.storePath)
                        }
                        IndustrialButton {
                            compact: true
                            text: "Flush to disk now"
                            onClicked: App.flushStore()
                        }
                    }
                }
            }

            // ---------------- Access ----------------
            Flickable {
                clip: true
                contentHeight: accessCol.height
                Column {
                    id: accessCol
                    width: Math.min(660, stack.width)
                    spacing: 20

                    Text {
                        text: "SESSION"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }

                    Row {
                        spacing: 14
                        Rectangle {
                            width: sessionRole.implicitWidth + 16
                            height: 22
                            radius: 3
                            color: root.admin ? "#16283F" : "#1F242C"
                            border.color: root.admin ? Theme.action : Theme.border
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter
                            Text {
                                id: sessionRole
                                anchors.centerIn: parent
                                text: Auth.roleLabel
                                color: root.admin ? Theme.action : Theme.textMuted
                                font.family: App.uiFont
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                                font.letterSpacing: 0.9
                            }
                        }
                        Text {
                            text: "signed in as " + Auth.username
                            color: Theme.textMuted
                            font.family: App.uiFont
                            font.pixelSize: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        IndustrialButton {
                            destructive: true
                            compact: true
                            text: "Sign out"
                            onClicked: Auth.logout()
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.borderSoft }

                    // --- change own password ---
                    Text {
                        text: "CHANGE MY PASSWORD"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }
                    Row {
                        spacing: 12
                        FieldInput { id: curPass; width: 180; label: "CURRENT"; password: true }
                        FieldInput { id: newPass; width: 180; label: "NEW"; password: true }
                        FieldInput { id: confirmPass; width: 180; label: "CONFIRM"; password: true }
                    }
                    Row {
                        spacing: 12
                        IndustrialButton {
                            primary: true
                            text: "Update password"
                            onClicked: {
                                if (Auth.changeOwnPassword(curPass.text, newPass.text, confirmPass.text)) {
                                    curPass.text = ""; newPass.text = ""; confirmPass.text = ""
                                    passNotice.kind = "safe"
                                    passNotice.text = "Password updated"
                                } else {
                                    passNotice.kind = "critical"
                                    passNotice.text = Auth.errorMessage
                                }
                            }
                        }
                        MessageBanner {
                            id: passNotice
                            compact: true
                            kind: "safe"
                            text: ""
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.borderSoft; visible: root.admin }

                    // --- operator directory (admin only) ---
                    Text {
                        visible: root.admin
                        text: "OPERATORS"
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                        font.weight: Font.DemiBold
                    }

                    Column {
                        visible: root.admin
                        width: parent.width
                        spacing: 6

                        Repeater {
                            model: root.userRows
                            delegate: Rectangle {
                                width: accessCol.width
                                height: 40
                                radius: 4
                                color: Theme.bgInset
                                border.color: Theme.border
                                border.width: 1

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 12
                                    spacing: 14

                                    Text {
                                        width: 130
                                        height: parent.height
                                        text: modelData.username
                                        color: Theme.text
                                        font.family: App.monoFont
                                        font.pixelSize: 12
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    Item {
                                        width: 70
                                        height: parent.height
                                        Rectangle {
                                            anchors.verticalCenter: parent.verticalCenter
                                            width: roleTag.implicitWidth + 14
                                            height: 18
                                            radius: 3
                                            color: modelData.role === 2 ? "#16283F" : "#1F242C"
                                            border.color: modelData.role === 2 ? Theme.action : Theme.border
                                            border.width: 1
                                            Text {
                                                id: roleTag
                                                anchors.centerIn: parent
                                                text: modelData.roleLabel
                                                color: modelData.role === 2 ? Theme.action : Theme.textMuted
                                                font.family: App.uiFont
                                                font.pixelSize: 9
                                                font.weight: Font.DemiBold
                                            }
                                        }
                                    }
                                    Text {
                                        width: 150
                                        height: parent.height
                                        text: "last: " + modelData.lastLogin
                                        color: Theme.textDim
                                        font.family: App.monoFont
                                        font.pixelSize: 11
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    MessageBanner {
                                        visible: modelData.defaultPassword
                                        compact: true
                                        kind: "warning"
                                        text: "DEFAULT PW"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                IndustrialButton {
                                    anchors.right: parent.right
                                    anchors.rightMargin: 10
                                    anchors.verticalCenter: parent.verticalCenter
                                    compact: true
                                    destructive: true
                                    text: "Remove"
                                    enabled: modelData.username !== "admin" && modelData.username !== Auth.username
                                    onClicked: Auth.removeUser(modelData.username)
                                }
                            }
                        }
                    }

                    // --- add operator ---
                    Column {
                        visible: root.admin
                        spacing: 12
                        Text {
                            text: "ADD OPERATOR"
                            color: Theme.textDim
                            font.family: App.uiFont
                            font.pixelSize: 10
                            font.letterSpacing: 1.2
                            font.weight: Font.DemiBold
                        }
                        Row {
                            spacing: 12
                            FieldInput { id: newUser; width: 180; label: "OPERATOR"; placeholder: "e.g. shift-b" }
                            FieldInput { id: newUserPass; width: 180; label: "PASSWORD"; password: true }
                            Column {
                                spacing: 6
                                Text {
                                    text: "ROLE"
                                    color: Theme.textDim
                                    font.family: App.uiFont
                                    font.pixelSize: 10
                                    font.letterSpacing: 1.1
                                    font.weight: Font.DemiBold
                                }
                                SegmentedControl {
                                    id: roleSeg
                                    model: ["GUEST", "ADMIN"]
                                    currentIndex: 0
                                }
                            }
                        }
                        Row {
                            spacing: 12
                            IndustrialButton {
                                primary: true
                                text: "Create operator"
                                onClicked: {
                                    if (Auth.createUser(newUser.text, newUserPass.text, roleSeg.currentIndex === 1 ? 2 : 1)) {
                                        newUser.text = ""; newUserPass.text = ""
                                        userNotice.kind = "safe"
                                        userNotice.text = "Operator added"
                                    } else {
                                        userNotice.kind = "critical"
                                        userNotice.text = Auth.errorMessage
                                    }
                                }
                            }
                            MessageBanner {
                                id: userNotice
                                compact: true
                                kind: "safe"
                                text: ""
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }
    }
}
