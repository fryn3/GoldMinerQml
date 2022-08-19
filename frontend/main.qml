import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15


import "common" as Common

Window {
    width: 980
    height: 600
    visible: true
    title: qsTr("Hello World")
    color: Common.Theme.bg
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
