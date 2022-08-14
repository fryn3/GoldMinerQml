import QtQuick 2.15
import QtQuick.Controls 2.15

FocusScope {
    property alias text: t.text
    property alias readOnly: t.readOnly
    property alias textInput: t

    implicitWidth: rect.implicitWidth
    implicitHeight: rect.implicitHeight
    activeFocusOnTab: true

    Rectangle {
        id: rect
        anchors.fill: parent

        implicitWidth: t.implicitWidth + 48
        implicitHeight: t.implicitHeight + 24
        radius: 8

        color: {
            if (!enabled) {
                return Theme.leBgDisabled;
            }
            return readOnly ? Theme.leBgReadOnly : Theme.leBgNormal
        }
        border {
            width: 1
            color: {
                if (!enabled) {
                    return Theme.borderDisabled;
                }
                if (t.activeFocus) {
                    return Theme.borderFocused;
                }
                if (ma.containsMouse) {
                    return Theme.borderHover;
                }
                return readOnly ? Theme.borderReadOnly : Theme.borderNormal;
            }
        }

        Text1Input {
            id: t
            anchors {
                left: parent.left
                leftMargin: 12
                verticalCenter: parent.verticalCenter
                right: parent.right
            }
        }

        MouseArea {
            id: ma
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true
            propagateComposedEvents: true

            onDoubleClicked: {
                t.selectAll();
            }

            onPressed: {
                t.forceActiveFocus(Qt.MouseFocusReason);
                if (mouse.button === Qt.LeftButton) {
                    _p.pressedStart = t.positionAt(mouse.x - t.anchors.leftMargin, mouse.y);
                    t.cursorPosition = t.positionAt(mouse.x - t.anchors.leftMargin, mouse.y);
                }
            }
            onPositionChanged: {
                if (mouse.buttons && _p.pressedStart >= 0) {
                    t.select(_p.pressedStart, t.positionAt(mouse.x - t.anchors.leftMargin, mouse.y));
                }
            }

            onReleased: {
                switch (mouse.button) {
                case Qt.LeftButton:
                    _p.pressedStart = -1;
                    break;
                case Qt.RightButton:
                    if (t.selectedText.length) {
                        t.copy();
                        ToolTip.show("copied: " + t.selectedText, 2000);
                    }
                    break;
                }
            }

            QtObject {
                id: _p
                property int pressedStart: -1
            }
        }
    }
}
