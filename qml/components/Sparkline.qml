import QtQuick

Item {
    id: root
    property var values: []
    property color lineColor: Theme.safe
    property real slide: 0
    readonly property int maxPoints: 20

    onValuesChanged: {
        if (values && values.length >= maxPoints) {
            slide = 1
            slideAnim.restart()
        }
        canvas.requestPaint()
    }
    onLineColorChanged: canvas.requestPaint()
    onSlideChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    NumberAnimation {
        id: slideAnim
        target: root
        property: "slide"
        to: 0
        duration: 200
        easing.type: Easing.OutCubic
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const pts = root.values
            if (!pts || pts.length < 2)
                return

            let minV = pts[0], maxV = pts[0]
            for (let i = 0; i < pts.length; ++i) {
                minV = Math.min(minV, pts[i])
                maxV = Math.max(maxV, pts[i])
            }
            if (Math.abs(maxV - minV) < 0.01) {
                minV -= 1
                maxV += 1
            }
            const pad = (maxV - minV) * 0.12
            minV -= pad
            maxV += pad

            const n = pts.length
            const dx = (width - 2) / (root.maxPoints - 1)
            const shift = -(1 - root.slide) * dx
            const y0 = 2
            const y1 = height - 2
            const extra = n > root.maxPoints ? 0 : (root.maxPoints - n)

            function X(i) { return 1 + (i - (n - root.maxPoints) + extra) * dx + shift }
            function Y(i) { return y1 - ((pts[i] - minV) / (maxV - minV)) * (y1 - y0) }

            ctx.beginPath()
            ctx.moveTo(X(0), Y(0))
            for (let i = 1; i < n; ++i)
                ctx.lineTo(X(i), Y(i))
            ctx.strokeStyle = root.lineColor
            ctx.lineWidth = 1.35
            ctx.lineJoin = "round"
            ctx.lineCap = "round"

            ctx.lineTo(X(n - 1), y1)
            ctx.lineTo(X(0), y1)
            ctx.closePath()
            const fill = Qt.rgba(root.lineColor.r, root.lineColor.g, root.lineColor.b, 0.16)
            ctx.fillStyle = fill
            ctx.fill()

            ctx.beginPath()
            ctx.moveTo(X(0), Y(0))
            for (let i = 1; i < n; ++i)
                ctx.lineTo(X(i), Y(i))
            ctx.stroke()
        }
    }
}
