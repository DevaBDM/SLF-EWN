import QtQuick

Item {
    id: root
    property int pulseTick: 0

    Timer {
        interval: 80
        running: true
        repeat: true
        onTriggered: {
            pulseTick++
            canvas.requestPaint()
        }
    }

    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    Connections {
        target: App
        function onMapNodesChanged() { canvas.requestPaint() }
    }

    Text {
        x: 20
        y: 16
        text: "ZONE PLOT  ·  relative coordinates, not GIS"
        color: Theme.textDim
        font.family: App.uiFont
        font.pixelSize: 10
        font.letterSpacing: 1.0
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: 16
        anchors.topMargin: 40

        property var hitNodes: []

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const w = width
            const h = height

            ctx.fillStyle = Theme.bgCard
            ctx.strokeStyle = Theme.border
            ctx.lineWidth = 1
            roundFill(ctx, 0, 0, w, h, 6)
            ctx.fill()
            ctx.stroke()

            const zones = App.zones
            const nodes = App.mapNodes
            hitNodes = []

            ctx.setLineDash([6, 5])
            ctx.lineWidth = 1
            ctx.font = "11px " + App.uiFont
            for (let i = 0; i < zones.length; ++i) {
                const z = zones[i]
                const zx = 16 + z.x * (w - 32)
                const zy = 16 + z.y * (h - 32)
                const zw = z.w * (w - 32)
                const zh = z.h * (h - 32)
                ctx.strokeStyle = Theme.border
                ctx.strokeRect(zx, zy, zw, zh)
                ctx.fillStyle = Theme.textDim
                ctx.setLineDash([])
                ctx.fillText(z.name.toUpperCase(), zx + 8, zy + 16)
                ctx.setLineDash([6, 5])
            }
            ctx.setLineDash([])

            const breath = 0.4 + 0.6 * (0.5 + 0.5 * Math.sin(pulseTick / (1600 / 80) * Math.PI * 2))

            for (let i = 0; i < nodes.length; ++i) {
                const n = nodes[i]
                const nx = 16 + n.x * (w - 32)
                const ny = 16 + n.y * (h - 32)
                let col = Theme.statusColor(n.status)
                let radius = 7
                if (n.status === 3 && !n.acknowledged) {
                    radius = 7 * (1 + 0.15 * breath)
                    ctx.beginPath()
                    ctx.arc(nx, ny, radius + 6, 0, Math.PI * 2)
                    ctx.strokeStyle = Qt.rgba(col.r, col.g, col.b, breath * 0.55)
                    ctx.lineWidth = 2
                    ctx.stroke()
                }
                ctx.beginPath()
                ctx.arc(nx, ny, radius, 0, Math.PI * 2)
                ctx.fillStyle = col
                ctx.fill()
                ctx.lineWidth = 1
                ctx.strokeStyle = "#12161C"
                ctx.stroke()

                ctx.fillStyle = Theme.textMuted
                ctx.font = "11px " + App.monoFont
                ctx.fillText(n.idString, nx + 12, ny + 4)

                hitNodes.push({ id: n.deviceId, x: nx, y: ny, r: radius + 8 })
            }
        }

        function roundFill(ctx, x, y, w, h, r) {
            ctx.beginPath()
            ctx.moveTo(x + r, y)
            ctx.arcTo(x + w, y, x + w, y + h, r)
            ctx.arcTo(x + w, y + h, x, y + h, r)
            ctx.arcTo(x, y + h, x, y, r)
            ctx.arcTo(x, y, x + w, y, r)
            ctx.closePath()
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: function(mouse) {
                const hits = canvas.hitNodes
                for (let i = 0; i < hits.length; ++i) {
                    const dx = mouse.x - hits[i].x
                    const dy = mouse.y - hits[i].y
                    if (dx * dx + dy * dy <= hits[i].r * hits[i].r) {
                        App.openDetail(hits[i].id)
                        return
                    }
                }
            }
        }
    }
}
