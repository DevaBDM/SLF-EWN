import QtQuick

Item {
    id: root
    property int deviceId: 0
    property string idString: ""
    property string zone: ""
    property int status: 0
    property bool acknowledged: false
    property real moisture: 0
    property real tiltX: 0
    property real tiltY: 0
    property real tiltZ: 0
    property string relativeTime: ""
    property var sparkline: []

    property real shownMoisture: 0
    property real shownX: 0
    property real shownY: 0
    property real shownZ: 0
    property color chipColor: Theme.statusColor(status)
    property real lift: 0
    property real statusGlow: 0
    property real breath: 0.4
    property bool ready: false

    readonly property bool criticalLive: status === 3 && !acknowledged

    Behavior on shownMoisture { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
    Behavior on shownX { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
    Behavior on shownY { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
    Behavior on shownZ { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
    Behavior on chipColor { ColorAnimation { duration: 300 } }
    Behavior on lift { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

    onMoistureChanged: shownMoisture = moisture
    onTiltXChanged: shownX = tiltX
    onTiltYChanged: shownY = tiltY
    onTiltZChanged: shownZ = tiltZ
    onStatusChanged: {
        chipColor = Theme.statusColor(status)
        if (ready)
            glowAnim.restart()
    }

    SequentialAnimation {
        id: glowAnim
        NumberAnimation { target: root; property: "statusGlow"; to: 1; duration: 140; easing.type: Easing.OutQuad }
        NumberAnimation { target: root; property: "statusGlow"; to: 0; duration: 260; easing.type: Easing.InQuad }
    }

    SequentialAnimation on breath {
        running: root.criticalLive
        loops: Animation.Infinite
        NumberAnimation { from: 0.4; to: 1.0; duration: 800; easing.type: Easing.InOutSine }
        NumberAnimation { from: 1.0; to: 0.4; duration: 800; easing.type: Easing.InOutSine }
    }

    transform: Translate { id: mount; y: 12 }
    opacity: 0

    Component.onCompleted: {
        shownMoisture = moisture
        shownX = tiltX
        shownY = tiltY
        shownZ = tiltZ
        appear.start()
        Qt.callLater(function() { ready = true })
    }

    ParallelAnimation {
        id: appear
        NumberAnimation { target: root; property: "opacity"; to: 1; duration: 250; easing.type: Easing.OutCubic }
        NumberAnimation { target: mount; property: "y"; to: 0; duration: 250; easing.type: Easing.OutCubic }
    }

    // Drop shadow
    Rectangle {
        anchors.fill: card
        anchors.topMargin: 4 + root.lift
        anchors.bottomMargin: -4 - root.lift * 1.5
        anchors.leftMargin: -1
        anchors.rightMargin: -1
        radius: 7
        color: "#000000"
        opacity: mouse.containsMouse ? 0.42 : 0.22
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    // Critical / status glow ring
    Rectangle {
        anchors.fill: card
        anchors.margins: -3
        radius: 9
        color: "transparent"
        border.width: 2
        border.color: root.chipColor
        opacity: root.criticalLive ? root.breath * 0.55 : root.statusGlow * 0.7
        visible: opacity > 0.02
    }

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 4
        anchors.topMargin: 4 - root.lift
        anchors.bottomMargin: 4 + root.lift
        radius: 6
        color: mouse.containsMouse ? Theme.bgCardHover : Theme.bgCard
        border.width: 1
        border.color: {
            if (root.criticalLive)
                return Qt.rgba(chipColor.r, chipColor.g, chipColor.b, 0.45 + breath * 0.4)
            if (root.statusGlow > 0.05)
                return Qt.rgba(chipColor.r, chipColor.g, chipColor.b, 0.35 + statusGlow * 0.45)
            return Theme.border
        }
        Behavior on color { ColorAnimation { duration: 150 } }

        Rectangle {
            width: parent.width - 2
            height: 1
            x: 1
            y: 1
            color: "#FFFFFF"
            opacity: 0.045
            radius: 1
        }

        Rectangle {
            width: 8
            height: 8
            radius: 4
            x: 14
            y: 14
            color: root.chipColor
        }

        Text {
            anchors.right: parent.right
            anchors.rightMargin: 14
            y: 12
            text: root.relativeTime
            color: Theme.textDim
            font.family: App.monoFont
            font.pixelSize: 11
        }

        Text {
            x: 14
            y: 30
            text: root.idString
            color: Theme.text
            font.family: App.monoFont
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        Text {
            x: 14
            y: 48
            width: parent.width - 28
            text: root.zone
            elide: Text.ElideRight
            color: Theme.textMuted
            font.family: App.uiFont
            font.pixelSize: 12
        }

        Text {
            x: 14
            y: 74
            text: root.shownMoisture.toFixed(1) + "%"
            color: Theme.text
            font.family: App.monoFont
            font.pixelSize: 28
            font.weight: Font.Medium
        }

        Text {
            x: 14
            y: 108
            text: "MOISTURE"
            color: Theme.textDim
            font.family: App.uiFont
            font.pixelSize: 9
            font.letterSpacing: 1.3
            font.weight: Font.DemiBold
        }

        Row {
            x: 14
            y: 126
            spacing: 16
            Repeater {
                model: [
                    { k: "X", v: root.shownX },
                    { k: "Y", v: root.shownY },
                    { k: "Z", v: root.shownZ }
                ]
                delegate: Column {
                    spacing: 1
                    Text {
                        text: (modelData.v >= 0 ? "+" : "") + modelData.v.toFixed(1) + "°"
                        color: Theme.text
                        font.family: App.monoFont
                        font.pixelSize: 14
                    }
                    Text {
                        text: "TILT " + modelData.k
                        color: Theme.textDim
                        font.family: App.uiFont
                        font.pixelSize: 8
                        font.letterSpacing: 0.8
                    }
                }
            }
        }

        Sparkline {
            id: spark
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            anchors.bottomMargin: 10
            height: 28
            values: root.sparkline
            lineColor: root.chipColor
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onEntered: root.lift = 2
        onExited: root.lift = 0
        onClicked: App.openDetail(root.deviceId)
    }
}
