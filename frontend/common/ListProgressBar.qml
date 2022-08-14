import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property real from: 0
    property real to: 1
    property real value: 0.3
    property string suffix: "%"
    readonly property real progress: from >= to ? 1 : (value - from) / (to - from)

    implicitWidth: t.implicitWidth
    implicitHeight: 24

    Text1 {
        id: t
        anchors {
            top: parent.top
            right: parent.right
        }
        text: Math.floor(100 * progress) + suffix
    }

    Rectangle {
        id: bg
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 4
        radius: height
        color: Theme.progressBarBg

        Item {
            id: itemValue
            height: parent.height
            width: from >= to ? parent.width
                              : parent.width * (value - from) / (to - from)
            clip: true
            Rectangle {
                id: rectValue
                width: bg.width
                height: bg.height
                radius: bg.radius
                color: Theme.progressBarValue
            }
        }
    }
}
