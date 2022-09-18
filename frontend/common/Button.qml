import QtQuick 2.15

FocusScope {
    id: root

    property alias text: t.text
    signal clicked

    implicitWidth: rect.implicitWidth
    implicitHeight: rect.implicitHeight
    activeFocusOnTab: true

    Rectangle {
        id: rect

        property bool spacePressed: false

        anchors.fill: parent
        implicitWidth: t.implicitWidth + 48
        implicitHeight: t.implicitHeight + 24
        focus: true
        clip: true
        radius: 8
        color: {
            if (!enabled) {
                return Theme.buttonDisabledColor;
            }
            if (ma.containsPress || spacePressed) {
                return Theme.buttonDownColor;
            }
            return ma.containsMouse ? Theme.buttonHoverColor : Theme.buttonNormalColor;
        }
        border {
            width: activeFocus ? 1 : 0
            color: Theme.borderFocused
        }

        Keys.onSpacePressed: {
            spacePressed = true;
            root.clicked();
        }
        Keys.onReleased: {
            spacePressed = false;
        }

        TextBtn {
            id: t
            anchors.centerIn: parent
        }

        MouseArea {
            id: ma

            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                root.forceActiveFocus();
                root.clicked();
            }
        }
    }
}
