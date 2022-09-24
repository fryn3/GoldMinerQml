import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

import Qt.labs.settings 1.1

import "common" as Common

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
        property string modelPath: model.path
        Layout.fillWidth: true
        Layout.fillHeight: true

        model: core.ftpModel
        delegate: Common.ListDelegate {
            height: 40
            width: ListView.view.width
            textInput.text: name
        }

        onCurrentIndexChanged: {
            console.info("ZAQAZ currentIndex =", currentIndex);
        }

        onModelPathChanged: {
            console.info("ZAQAZ modelPath =", modelPath);
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
            text: "Выбрать папку загрузки"
        }
        RowLayout {
            width: parent.width
            spacing: 16

            Common.LineEdit {
                id: path
                Layout.fillWidth: true
                Layout.fillHeight: true

                Binding {
                    target: core.deviceController
                    property: "downloadFolder"
                    value: path.text
                }
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
                text: "Удалить после загрузки"
            }
            RowLayout {
                width: parent.width
                spacing: 16
                Common.Button {
                    Layout.fillWidth: true
                    text: "Загрузить файлы"
                }
                Common.Button {
                    Layout.fillWidth: true
                    text: "Очистить память устройства"
                }
            } // RowLayout
        } // Column
    } // Column
}
