import QtQuick 2.0
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import "common" as Common

import cpp.Core 43.21

/// Список фтп устройств, кнопка поиска
Item {
    implicitWidth: col.implicitWidth
    implicitHeight: col.implicitHeight

    ColumnLayout {
        id: col
        anchors.fill: parent
        spacing: 16

        Common.TextH1 {
            Layout.fillWidth: true
            text: "Найденные устройства"
        }
        Common.GreenListView {
            id: ftpDevListView
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: core.deviceModel
            delegate: Common.ListDelegate {
                height: 40
                width: ListView.view.width
                textInput.text: model.name
                progressBar.value: Math.random() //model.index
            }

            onCurrentIndexChanged: {
                core.devModelCurrentIndex = currentIndex;
            }
        }
        Common.Button {
            id: btnFindDevs
            Layout.fillWidth: true
            height: 40
            text: "Поиск устройств"
            onClicked: core.findDevices()
        }
        Common.Button {
            Layout.fillWidth: true
            height: 40
            text: "Авто скачивание"
        }
    }
    states: [
        State {
            when: core.state === Core.State.FindingDevices
            PropertyChanges {
                target: btnFindDevs
                enabled: false
            }
        }

    ]
}
