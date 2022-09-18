import QtQuick 2.0
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import "common" as Common

import cpp.Core 43.21

/// Список фтп устройств, кнопка поиска
Item {

    property int currentIndex: core.deviceModel.rowCount ? ftpDevListView.currentIndex : -1

    implicitWidth: col.implicitWidth
    implicitHeight: col.implicitHeight

    onCurrentIndexChanged: {
        core.devModelCurrentIndex = currentIndex;
    }

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
        }
        Common.Button {
            id: btnFindDevs
            Layout.fillWidth: true
            height: 40
            text: "Поиск устройств"
            onClicked: core.findDev()
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
