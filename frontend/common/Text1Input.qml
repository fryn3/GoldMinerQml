import QtQuick 2.15
import QtQuick.Controls 2.15

TextInput {
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontText1PixelSize
    font.weight: Theme.fontText1Weight
    color: enabled ? Theme.blackDark : Theme.blackLight
    focus: true
    clip: true
    cursorVisible: activeFocus

    MouseArea {
        id: ma
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        propagateComposedEvents: true

        onDoubleClicked: {
            parent.selectAll();
        }

        onPressed: {
            parent.forceActiveFocus(Qt.MouseFocusReason);
            if (mouse.button === Qt.LeftButton) {
                _p.pressedStart = parent.positionAt(mouse.x, mouse.y);
                parent.cursorPosition = parent.positionAt(mouse.x, mouse.y);
            }
        }
        onPositionChanged: {
            if (mouse.buttons && _p.pressedStart >= 0) {
                parent.select(_p.pressedStart, parent.positionAt(mouse.x, mouse.y));
            }
        }

        onReleased: {
            switch (mouse.button) {
            case Qt.LeftButton:
                _p.pressedStart = -1;
                break;
            case Qt.RightButton:
                if (parent.selectedText.length) {
                    parent.copy();
                    ToolTip.show("copied: " + parent.selectedText, 2000);
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
