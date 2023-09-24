import QtQuick 2.15
import QtQuick.Controls 2.15

import "common" as Common
import cpp.Core 43.21

Item {
    id: root

    readonly property bool hideRtsp: true
    readonly property bool hideSettings: true

    enum TabIndex {
        TabFiles,
        TabRtsp,
        TabSettings
    }

    component GreenTabButton: Rectangle {
        required property int index

        property alias text: t.text

        signal clicked()

        implicitWidth: t.implicitWidth + 48
        implicitHeight: t.implicitHeight + 24

        color: {
            if (!enabled) {
                return Common.Theme.tabBtnDisabled;
            }
            if (index === root.currentIndex) {
                return Common.Theme.tabBtnSelected;
            }
            if (ma.containsPress) {
                return Common.Theme.tabBtnDown;
            }
            return ma.containsMouse ? Common.Theme.tabBtnHover : Common.Theme.tabBtnNormal;
        }

        Common.TextH1 {
            id: t
            anchors.centerIn: parent
        }
        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                parent.clicked();
                root.currentIndex = index;
            }
        }
    }

    property int currentIndex: 0
    property var currentTabItem: tabFiles

    onCurrentIndexChanged: {
        switch (currentIndex) {
        case 0: // TabFiles:
            currentTabItem = tabFiles;
            break;
        case 1: // TabRtsp:
            currentTabItem = tabRtsp;
            break;
        case 2: // TabSettings:
            currentTabItem = tabSettings;
            break;
        }
    }

    Row {
        id: row
        width: parent.width
        enabled: core.state === Core.State.None && core.devicesFound > 0
        GreenTabButton {
            index: 0
            text: "Файлы"

            Rectangle {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                visible: !hideSettings && !hideRtsp && enabled && root.currentIndex === TabArea.TabIndex.TabSettings
                color: Common.Theme.borderNormal
                width: 1
                height: parent.height - 16
            }
        }
        GreenTabButton {
            visible: !hideRtsp
            index: 1
            text: "RTSP"
            Rectangle {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                visible: enabled && root.currentIndex === TabArea.TabIndex.TabFiles
                color: Common.Theme.borderNormal
                width: 1
                height: parent.height - 16
            }
        }
        GreenTabButton {
            visible: !hideSettings
            index: 2
            text: "Настройки"
        }
    }

    Rectangle {
        id: bg
        anchors {
            left: parent.left
            right: parent.right
            top: row.bottom
            bottom: parent.bottom
        }
        color: Common.Theme.tabBg

        Flickable {
            id: tabs
            anchors {
                fill: parent
                margins: 24
            }

            contentHeight: Math.max(root.currentTabItem.implicitHeight, height)

            flickableDirection: Flickable.VerticalFlick
            clip: true


            TabFiles {
                id: tabFiles
                visible: root.currentTabItem == tabFiles
                anchors.fill: parent
            }
            TabRtsp {
                id: tabRtsp
                visible: !hideRtsp && root.currentTabItem == tabRtsp
                anchors.fill: parent
                enabled: core.state === Core.State.None && core.devicesFound > 0
            }
            TabSettings {
                id: tabSettings
                visible: !hideSettings && root.currentTabItem == tabSettings
                anchors.fill: parent
                enabled: core.state === Core.State.None && core.devicesFound > 0
            }
        }
    }
}
