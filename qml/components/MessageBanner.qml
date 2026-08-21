import QtQuick

// Rounded, filled severity chip. Used for alerts, login errors, and toasts so
// every message reads as a color block with white type rather than outlined text.
Rectangle {
    id: root

    property string text: ""
    property string kind: "warning"   // warning | critical | emergency | safe | info
    property int eventType: -1        // Alert EventType; overrides kind when >= 0
    property bool compact: false
    property bool fillWidth: false

    readonly property color fill: eventType >= 0 ? Theme.eventFill(eventType) : Theme.kindFill(kind)

    implicitWidth: fillWidth ? (parent ? parent.width : label.implicitWidth + hPad * 2)
                             : label.implicitWidth + hPad * 2
    implicitHeight: Math.max(compact ? 22 : 32, label.implicitHeight + vPad * 2)
    width: fillWidth && parent ? parent.width : implicitWidth
    radius: compact ? 8 : 10
    color: fill
    visible: text.length > 0
    clip: true

    readonly property int hPad: compact ? 10 : 14
    readonly property int vPad: compact ? 4 : 8

    Text {
        id: label
        anchors.left: parent.left
        anchors.right: fillWidth ? parent.right : undefined
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: root.hPad
        anchors.rightMargin: fillWidth ? root.hPad : 0
        width: fillWidth ? parent.width - root.hPad * 2 : implicitWidth
        text: root.text
        color: "#FFFFFF"
        wrapMode: fillWidth ? Text.WordWrap : Text.NoWrap
        elide: fillWidth ? Text.ElideNone : Text.ElideRight
        font.family: App.uiFont
        font.pixelSize: compact ? 10 : 12
        font.weight: Font.DemiBold
        font.letterSpacing: compact ? 0.4 : 0.2
    }
}
