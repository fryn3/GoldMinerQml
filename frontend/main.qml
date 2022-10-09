import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import Qt.labs.settings 1.1

import "common" as Common
import cpp.Core 43.21

Window {
    id: root
    width: 980
    height: 700
    visible: true
    title: qsTr("Atest qml")
    color: Common.Theme.bg

    Settings {
        category: "Window"
        property alias x: root.x
        property alias y: root.y
        property alias width: root.width
        property alias height: root.height
    }

    Common.Text1Input {
        id: stateBar
        anchors {
            right: parent.right
            top: parent.top
            margins: 8
        }
        readOnly: true

        text: core.state
    }
    FtpDevices {
        id: ftpDevices
        anchors {
            left: parent.left
            leftMargin: 40
            top: parent.top
            topMargin: 32
            bottom: parent.bottom
            bottomMargin: 40
        }
        width: parent.width / 3

        controlState: {
            switch(core.state) {
            case Core.State.ProcessAutoDownloading:
                return FtpDevices.ControlState.Downloading;
            case Core.State.None:
                return FtpDevices.ControlState.None;
            default:
                return FtpDevices.ControlState.Processing;
            }
        }

        onBtnFindDevsClicked: core.findDev()
        onBtnDevAutoStartClicked: core.runAutoDownloading()
        onBtnDevAutoStopClicked: core.stopAutoDownloading()
    }
    TabArea {
        anchors {
            left: ftpDevices.right
            leftMargin: 32
            right: parent.right
            rightMargin: 40
            top: parent.top
            topMargin: 32
            bottom: parent.bottom
            bottomMargin: 40
        }
    }
    Common.Text1Input {
        id: statusBar
        anchors {
            left: parent.left
            bottom: parent.bottom
            margins: 8
        }
        readOnly: true

        text: "Status message"

        Connections {
            target: core
            function onShowMessage(msg, timeoutMilisec) {
                statusBarTimer.stop();
                console.info("[showMessage]: \"", msg, "\",", timeoutMilisec, "ms")
                if (timeoutMilisec > 0) {
                    statusBarTimer.interval = timeoutMilisec;
                    statusBarTimer.restart();
                }
                statusBar.text = msg;
            }
        }

        Timer {
            id: statusBarTimer

            onTriggered: {
                statusBar.text = "";
            }
        }
    }
}
