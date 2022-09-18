import QtQuick 2.15

FocusScope {
    id: root

    property bool checked: radioGroup ? radioGroup.selected === root : false
    property alias text: t.text
    property RadioGroup radioGroup

    readonly property var defaultClickedBehavior: setChecked
    property var clickedBehavior: defaultClickedBehavior

    signal clicked()

    onClicked: {
        clickedBehavior();
    }

    implicitWidth: imgRect.implicitWidth + t.implicitWidth + t.anchors.leftMargin
    implicitHeight: Math.max(imgRect.implicitHeight, t.implicitHeight)
    activeFocusOnTab: true

    function setChecked() {
        if (radioGroup) {
            radioGroup.selected = root;
        }
    }

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
            root.clicked();
        }
        Keys.onSpacePressed: {
            root.clicked();
        }
    }
}
