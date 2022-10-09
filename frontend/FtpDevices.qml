import QtQuick 2.0
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import "common" as Common

import cpp.Core 43.21

/// Список фтп устройств, кнопка поиска
Item {
    id: root

    enum ControlState {
        // Бездельничаем.
        None,
        // Когда идет какой то процес, кроме автоскачивания.
        Processing,
        // Идет процесс автоскачивания.
        Downloading
    }

    property int controlState: FtpDevices.ControlState.None
    readonly property int currentIndex: core.deviceModel.rowCount ? ftpDevListView.currentIndex : -1

    signal btnFindDevsClicked
    signal btnDevAutoStartClicked
    signal btnDevAutoStopClicked

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
            enabled: controlState === FtpDevices.ControlState.None

            model: core.deviceModel
            delegate: Common.ListDelegate {
                height: 40
                width: ListView.view.width
                textInput.text: model.display
                progressBar {
                    to: model.totalSize
                    value: model.doneSize
                }
            }
        }
        Common.Button {
            id: btnFindDevs
            Layout.fillWidth: true
            enabled: controlState === FtpDevices.ControlState.None
            height: 40
            text: "Поиск устройств"
            onClicked: root.btnFindDevsClicked()
        }
        Common.Button {
            Layout.fillWidth: true
            enabled: controlState === FtpDevices.ControlState.Downloading
                     || controlState === FtpDevices.ControlState.None
            height: 40
            text: controlState === FtpDevices.ControlState.Downloading
                  ? "Остановить скачивание" : "Автоскачивание"
            onClicked: controlState === FtpDevices.ControlState.Downloading
                       ? root.btnDevAutoStopClicked() : root.btnDevAutoStartClicked()
        }
    }
}
