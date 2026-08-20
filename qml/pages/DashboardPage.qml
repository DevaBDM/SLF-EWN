import QtQuick

Item {
    id: root

    readonly property int gap: 12
    readonly property int minCard: 258
    readonly property int cols: Math.max(1, Math.floor((width - 32 + gap) / (minCard + gap)))
    readonly property int cardW: Math.max(240, Math.floor((width - 32 - gap * (cols - 1)) / cols))

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: grid.height + 32
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 2500

        Grid {
            id: grid
            x: 16
            y: 16
            width: root.width - 32
            columns: root.cols
            columnSpacing: root.gap
            rowSpacing: root.gap

            Repeater {
                model: App.nodes
                delegate: NodeCard {
                    width: root.cardW
                    height: 196
                    deviceId: model.deviceId
                    idString: model.idString
                    zone: model.zone
                    status: model.status
                    acknowledged: model.acknowledged
                    moisture: model.moisture
                    tiltX: model.tiltX
                    tiltY: model.tiltY
                    tiltZ: model.tiltZ
                    relativeTime: model.relativeTime
                    sparkline: model.sparkline
                }
            }
        }
    }

    Text {
        visible: App.liveNodeCount === 0
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 24
        anchors.topMargin: 28
        text: "Waiting for telemetry…"
        color: Theme.textDim
        font.family: App.uiFont
        font.pixelSize: 13
    }
}
