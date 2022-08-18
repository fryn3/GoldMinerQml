import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "common" as Common

ColumnLayout {
    id: root

    property alias selectDevText: selectDevice.text

    spacing: 0

    Common.TextH1 {
        id: selectDevice

        text: "Выбранное устройство"
    }

    Item {
        Layout.minimumHeight: 16
        Layout.fillWidth: true
    }

    Common.Button {
        id: connectFtp
        Layout.fillWidth: true

        text: "Посмотреть файлы"
    }

    Item {
        Layout.minimumHeight: 16
        Layout.fillWidth: true
    }

    Common.GreenListView {
        id: ftpFilesLv
        Layout.fillWidth: true
        Layout.fillHeight: true

        model: Qt.fontFamilies()
        delegate: Common.ListDelegate {
            height: 40
            width: ListView.view.width
            textInput.text: modelData
            progressBar.value: model.index / Qt.fontFamilies().length
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
                text: "Some text"
            }

            Common.Button {
                id: selectDirBtn
                Layout.alignment: Qt.AlignRight
                text: "Выбрать"
            }
        } // RowLayout

        Column {
            spacing: 8
            width: parent.width
            Common.CheckBox {
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
