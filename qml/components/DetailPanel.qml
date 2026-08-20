import QtQuick

Item {
    id: root
    visible: overlay.opacity > 0.001 || panel.x < width - 2

    readonly property real panelWidth: Math.round(width * 0.60)
    property var node: App.selectedNode
    property int range: 0
    property var moisturePts: []
    property var tiltPts: []

    function reload() {
        moisturePts = App.moistureSeries(range)
        tiltPts = App.tiltSeries(range)
        mChart.playReveal()
        tChart.playReveal()
    }

    Connections {
        target: App
        function onSelectedChanged() {
            if (App.detailOpen)
                reload()
        }
        function onChartNeedsReload() {
            if (App.detailOpen)
                reload()
        }
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "#000000"
        opacity: App.detailOpen ? 0.15 : 0
        Behavior on opacity { NumberAnimation { duration: App.detailOpen ? 320 : 280; easing.type: Easing.InOutQuad } }
        MouseArea {
            anchors.fill: parent
            enabled: App.detailOpen
            onClicked: App.closeDetail()
        }
    }

    Rectangle {
        id: panel
        width: root.panelWidth
        height: parent.height
        x: App.detailOpen ? root.width - width : root.width
        color: Theme.bgCard
        Behavior on x {
            NumberAnimation {
                duration: App.detailOpen ? 320 : 280
                easing.type: Easing.InOutQuad
            }
        }

        Rectangle {
            anchors.left: parent.left
            width: 1
            height: parent.height
            color: Theme.border
        }

        Column {
            id: header
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 22
            spacing: 6

            Row {
                spacing: 12
                width: parent.width
                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: Theme.statusColor(node.status || 0)
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: node.idString || ""
                    color: Theme.text
                    font.family: App.monoFont
                    font.pixelSize: 18
                    font.weight: Font.DemiBold
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: Theme.statusLabel(node.status || 0)
                    color: Theme.statusColor(node.status || 0)
                    font.family: App.uiFont
                    font.pixelSize: 11
                    font.letterSpacing: 1.1
                    font.weight: Font.DemiBold
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { width: 1; height: 1 }
                Item {
                    width: header.width - 280
                    height: 1
                }
            }

            Text {
                text: node.zone || ""
                color: Theme.textMuted
                font.family: App.uiFont
                font.pixelSize: 13
            }

            Row {
                spacing: 10
                IndustrialButton {
                    visible: (node.status || 0) >= 2 && !node.acknowledged
                    primary: true
                    text: "Acknowledge Alert"
                    onClicked: App.acknowledgeSelected()
                }
                IndustrialButton {
                    text: "Close"
                    onClicked: App.closeDetail()
                }
            }
        }

        SegmentedControl {
            id: seg
            anchors.top: header.bottom
            anchors.topMargin: 18
            anchors.right: parent.right
            anchors.rightMargin: 22
            currentIndex: root.range
            onIndexChanged: function(i) {
                root.range = i
                reload()
            }
        }

        Column {
            anchors.top: seg.bottom
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 10
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            spacing: 8

            Text {
                text: "MOISTURE  %"
                color: Theme.textDim
                font.family: App.uiFont
                font.pixelSize: 10
                font.letterSpacing: 1.2
                font.weight: Font.DemiBold
                leftPadding: 12
            }
            TelemetryChart {
                id: mChart
                width: parent.width
                height: (parent.height - 48) / 2
                mode: "moisture"
                points: root.moisturePts
                warnY: App.moistureWarning
                critY: App.moistureCritical
                lineColor: Theme.chartTeal
            }
            Row {
                spacing: 14
                leftPadding: 12
                Text { text: "TILT  °"; color: Theme.textDim; font.family: App.uiFont; font.pixelSize: 10; font.letterSpacing: 1.2; font.weight: Font.DemiBold }
                Text { text: "X"; color: Theme.chartX; font.family: App.monoFont; font.pixelSize: 10 }
                Text { text: "Y"; color: Theme.chartY; font.family: App.monoFont; font.pixelSize: 10 }
                Text { text: "Z"; color: Theme.chartZ; font.family: App.monoFont; font.pixelSize: 10 }
                Text { text: "MAG"; color: Theme.chartMag; font.family: App.monoFont; font.pixelSize: 10 }
            }
            TelemetryChart {
                id: tChart
                width: parent.width
                height: (parent.height - 48) / 2
                mode: "tilt"
                points: root.tiltPts
            }
        }
    }
}
