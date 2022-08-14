import QtQuick 2.15
import QtQuick.Layouts 1.15

FocusScope {
    id: root

    property int index: model.index
    property alias textInput: _textInput
    property alias progressBar: _progressBar
    property alias iconImage: _icon
    property alias mouseArea: ma

    implicitWidth: rect.implicitWidth
    implicitHeight: rect.implicitHeight

    Rectangle {
        id: rect
        anchors.fill: parent
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight
        focus: true

        color: {
            if (ma.containsPress) {
                return Theme.listMouseDown;
            }
            if (root.index === root.ListView.view.currentIndex) {
                return Theme.listRowSelected;
            }
            return ma.containsMouse ? Theme.listMouseHover : Theme.listMouseNormal;
        }

        RowLayout {
            id: row
            anchors {
                leftMargin: 17
                rightMargin: 17
                fill: parent
            }
            spacing: 5
            Image {
                id: _icon
                visible: !!('' + source)
                width: 14
                height: width
                fillMode: Image.PreserveAspectFit
                Layout.minimumWidth: width
                Layout.alignment: Qt.AlignVCenter
            }
            Text1 {
                id: _textInput
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
        }
        ListProgressBar {
            id: _progressBar
            anchors {
                right: parent.right
                rightMargin: 17
                verticalCenter: parent.verticalCenter
            }
            width: 160
        }
        Rectangle {
            anchors {
                left: parent.left
                leftMargin: 16
                right: parent.right
                rightMargin: 16
                bottom: parent.bottom
            }
            color: Theme.borderNormal
            height: 1
        }
    }
    MouseArea {
        id: ma
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            root.ListView.view.currentIndex = root.index;
        }
    }
}
