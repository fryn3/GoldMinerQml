import QtQuick 2.15

FocusScope {
    id: root

    readonly property bool checked: radioGroup.selected === root
    property alias text: t.text
    property RadioGroup radioGroup

    function setChecked() {
        radioGroup.selected = root;
    }

    implicitWidth: imgRect.implicitWidth + t.implicitWidth + t.anchors.leftMargin
    implicitHeight: Math.max(imgRect.implicitHeight, t.implicitHeight)
    activeFocusOnTab: true

    Rectangle {
        id: imgRect
        implicitWidth: 24
        implicitHeight: implicitWidth
        radius: width / 2
        color: root.enabled ? Theme.cbBgNormal : Theme.cbBgNormal
        border {
            color: {
                if (!root.enabled) {
                    return Theme.borderDisabled;
                }
                if (root.activeFocus) {
                    return Theme.borderFocused;
                }
                return ma.containsMouse ? Theme.borderHover : Theme.borderNormal;
            }
            width: 1
        }
        Rectangle {
            anchors {
                fill: parent
                margins: 7
            }
            visible: root.checked
            radius: width / 2
            color: root.enabled ? Theme.cbImageNormal : Theme.cbImageDisabled;
        }
    }
    Text1 {
        id: t
        text: root.text
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
            root.radioGroup.selected = root;
        }
        Keys.onSpacePressed: {
            root.radioGroup.selected = root;
        }
    }
}
