import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

FocusScope {
    id: root

    property alias delegate: lv.delegate
    property alias model: lv.model
    property alias currentIndex: lv.currentIndex

    implicitWidth: rect.implicitWidth
    implicitHeight: rect.implicitHeight
    activeFocusOnTab: true

    Rectangle {
        id: rect

        anchors.fill: parent
        implicitWidth: 100//lv.implicitWidth
        implicitHeight: 200//lv.implicitHeight

        color: Theme.listBg
        border {
            color: {
                if (!enabled) {
                    return Theme.borderDisabled;
                }
                if (lv.activeFocus) {
                    return Theme.borderFocused;
                }
                return ma.containsMouse ? Theme.borderHover : Theme.borderNormal;
            }
            width: 1
        }
        radius: 8
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                width: rect.width
                height: rect.height
                radius: rect.radius
            }
        }

        ListView {
            id: lv
            anchors {
                fill: parent
                margins: parent.border.width
            }
            focus: true
            clip: true

            ScrollBar.vertical: ScrollBar { }
        }
    }
    MouseArea {
        id: ma
        anchors.fill: parent
        propagateComposedEvents: true
        hoverEnabled: true
        onClicked: {
            parent.forceActiveFocus();
            mouse.accepted = false;
        }
    }
}
