import QtQuick 2.15
import QtGraphicalEffects 1.15

LineEdit {
    id: root

    property real stepSize: 1
    readonly property real value: +text

    component SpinBtn: MouseArea {

        implicitWidth: img.implicitWidth + 8
        implicitHeight: img.implicitHeight + 8
        hoverEnabled: true

        Image {
            id: img
            anchors.centerIn: parent
            source: "qrc:/frontend/common/icons/spinButtonUpIcon.svg"
            sourceSize.width: 12
            sourceSize.height: 7
        }
        ColorOverlay {
            anchors.fill: img
            source: img
            color: {
                if (!enabled) {
                    return Theme.buttonDisabledColor;
                }
                if (parent.containsPress) {
                    return Theme.buttonDownColor;
                }
                return parent.containsMouse || activeFocus ? Theme.buttonHoverColor : Theme.buttonNormalColor;
            }
        }
    }

    Column {
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
            topMargin: 9 - 4
            rightMargin: 12 - 4
            bottomMargin: 9 - 4
        }

        SpinBtn {
            onClicked: {
                root.forceActiveFocus();
                root.text = value + stepSize;
            }
        }
        SpinBtn {
            rotation: 180
            onClicked: {
                root.forceActiveFocus();
                root.text = value - stepSize;
            }
        }

    }
}
