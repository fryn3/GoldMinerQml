import QtQuick 2.15
import QtGraphicalEffects 1.15

FocusScope {
    id: root

    property bool checked: false
    property alias text: t.text

    implicitWidth: imgRect.implicitWidth + t.implicitWidth + t.anchors.leftMargin
    implicitHeight: Math.max(imgRect.implicitHeight, t.implicitHeight)
    activeFocusOnTab: true

    Rectangle {
        id: imgRect
        implicitWidth: 24
        implicitHeight: implicitWidth

        color: enabled ? Theme.cbBgNormal : Theme.cbBgDisabled

        border {
            color: {
                if (!enabled) {
                    return Theme.borderDisabled;
                }
                if (ma.activeFocus) {
                    return Theme.borderFocused;
                }
                return ma.containsMouse ? Theme.borderHover : Theme.borderNormal;
            }

            width: 1
        }
        radius: 4

        Image {
            id: img
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: root.checked ? "qrc:/frontend/common/icons/check.svg" : ""
        }

        ColorOverlay {
            anchors.fill: img
            source: img
            color: enabled ? Theme.cbImageNormal : Theme.cbImageDisabled
        }
    }

    Text1 {
        id: t
        anchors {
            left: imgRect.right
            leftMargin: 8
            verticalCenter: imgRect.verticalCenter
        }
    }

    MouseArea {
        id: ma

        anchors.fill: parent
        focus: true
        hoverEnabled: true
        onClicked: {
            root.forceActiveFocus();
            root.checked = !root.checked;
        }
        Keys.onSpacePressed: {
            root.checked = !root.checked;
        }
    }
}
