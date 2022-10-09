import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

import Qt.labs.settings 1.1

import "common" as Common
import cpp.Core 43.21

ColumnLayout {
    id: root

    property alias selectDevText: selectDevice.text

    spacing: 0

    Settings {
        category: "Window"
        property alias pathDir: path.text
        property alias deleteAfterDownloading: cbDeleteAfterDownloading.checked
    }

    FileDialog {
        id: fileDialog
        title: "Выбери место для сохранения"
        selectFolder: true
        onAccepted: {
            path.text = (fileDialog.fileUrl + "").replace("file:///", "");
        }
    }

    Common.TextH1 {
        id: selectDevice
        enabled: core.devModelCurrentIndex >= 0
        text: core.currentDeviceCam.ip
              ? "Выбранное устройство: " + core.currentDeviceCam.ip
              : "Устройство не выбранно"
    }

    Item {
        Layout.minimumHeight: 16
        Layout.fillWidth: true
    }

    Common.Button {
        id: connectFtp
        Layout.fillWidth: true
        enabled: core.state === Core.State.None && core.devModelCurrentIndex >= 0

        text: "Посмотреть файлы"
        onClicked: {
            core.showFtpFiles();
        }
    }

    Item {
        Layout.minimumHeight: 16
        Layout.fillWidth: true
    }

    Common.GreenListView {
        id: ftpFilesLv
        Layout.fillWidth: true
        Layout.fillHeight: true
        enabled: core.devModelCurrentIndex >= 0

        model: core.ftpModel
        delegate: Common.ListDelegate {
            height: 40
            width: ListView.view.width
            textInput.text: name
            progressBar {
                to: model.progressTotal
                value: model.progressDone
            }
        }

        onCurrentIndexChanged: {
            console.info("ZAQAZ currentIndex =", currentIndex);
        }
    }

    Item {
        Layout.minimumHeight: 16
        Layout.fillWidth: true
    }

    Column {
        id: columnControls
        Layout.fillWidth: true

        spacing: 16

        Common.TextH1 {
            enabled: core.state === Core.State.None && core.devModelCurrentIndex >= 0
            text: "Выбрать папку загрузки"
        }
        RowLayout {
            width: parent.width
            spacing: 16
            enabled: core.state === Core.State.None && core.devModelCurrentIndex >= 0

            Common.LineEdit {
                id: path
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Common.Button {
                id: selectDirBtn
                Layout.alignment: Qt.AlignRight
                text: "Выбрать"
                onClicked: fileDialog.open()
            }
        } // RowLayout

        Column {
            spacing: 8
            width: parent.width
            Common.CheckBox {
                id: cbDeleteAfterDownloading
                enabled: core.state === Core.State.None && core.devModelCurrentIndex >= 0
                text: "Удалить после загрузки"
            }
            RowLayout {
                width: parent.width
                spacing: 16
                Common.Button {
                    id: btnDownloading
                    Layout.fillWidth: true
                    enabled: core.ftpModel.rowCount > 0
                             && (core.state === Core.State.None
                                 || core.state === Core.State.ProcessDownloading)
                    text: "Загрузить файлы"
                    onClicked: {
                        if (core.state === Core.State.None) {
                            core.downloadDevice(path.text, cbDeleteAfterDownloading.checked);
                        } else if (core.state === Core.State.ProcessDownloading) {
                            core.stopDownloadDevice();
                        } else {
                            console.info("ZAQAZ wtf core.state", core.state);
                        }
                    }

                    states: [
                        State {
                            when: core.state === Core.State.ProcessDownloading
                            PropertyChanges {
                                target: btnDownloading
                                text: "Остановить загрузку"
                            }
                        },
                        State {
                            when: core.state === Core.State.ProcessStoping
                            PropertyChanges {
                                target: btnDownloading
                                text: "Идет остановка загрузки"
                            }
                        }

                    ]

                }
                Common.Button {
                    id: btnRemoveAll
                    Layout.fillWidth: true
                    enabled: core.ftpModel.rowCount > 0
                             && (core.state === Core.State.None
                                 || core.state === Core.State.ProcessRemoving)
                    text: "Очистить память устройства"
                    onClicked: {
                        if (core.state === Core.State.None) {
                            core.cleanDevice();
                        } else if (core.state === Core.State.ProcessRemoving) {
                            core.stopCleanDevice();
                        } else {
                            console.info("ZAQAZ wtf core.state", core.state);
                        }
                    }

                    states: [
                        State {
                            when: core.state === Core.State.ProcessRemoving
                            PropertyChanges {
                                target: btnRemoveAll
                                text: "Остановить очистку"
                            }
                        }
                    ]
                }
            } // RowLayout
        } // Column
    } // Column
}
