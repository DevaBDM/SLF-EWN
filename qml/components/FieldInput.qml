import QtQuick
import QtQuick.Controls

Item {
    id: root
    property string label: ""
    property alias text: field.text
    property alias placeholder: field.placeholderText
    property bool password: false
    property alias input: field
    signal submitted()

    implicitHeight: 56
    implicitWidth: 280

    Text {
        id: caption
        text: root.label
        color: field.activeFocus ? Theme.text : Theme.textDim
        font.family: App.uiFont
        font.pixelSize: 10
        font.letterSpacing: 1.1
        font.weight: Font.DemiBold
        Behavior on color { ColorAnimation { duration: 120 } }
    }

    Rectangle {
        anchors.top: caption.bottom
        anchors.topMargin: 6
        anchors.left: parent.left
        anchors.right: parent.right
        height: 34
        radius: 4
        color: Theme.bgInset
        border.width: 1
        border.color: field.activeFocus ? Theme.action : Theme.border
        Behavior on border.color { ColorAnimation { duration: 120 } }

        TextField {
            id: field
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            verticalAlignment: Text.AlignVCenter
            color: Theme.text
            font.family: root.password ? App.monoFont : App.uiFont
            font.pixelSize: 13
            echoMode: root.password ? TextInput.Password : TextInput.Normal
            selectByMouse: true
            placeholderTextColor: Theme.textFaint
            background: Item {}
            onAccepted: root.submitted()
        }
    }
}
