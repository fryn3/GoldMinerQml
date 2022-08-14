import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "common" as Common

Item {
    id: root

    property alias selectDevText: selectDevice.text

    implicitWidth: Math.max(selectDevice.implicitWidth
                            , connectFtp.implicitWidth
                            , ftpFilesLv.implicitWidth
                            , columnControls.implicitWidth)
    implicitHeight: mainColumn.implicitHeight

    Column {
        id: mainColumn
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        Common.TextH1 {
            id: selectDevice

            text: "Выбранное устройство"
        }

        Item { height: 16; width: 1; }

        Common.Button {
            id: connectFtp
            width: parent.width

            text: "Посмотреть файлы"
        }

        Item { height: 16; width: 1; }

        Common.GreenListView {
            id: ftpFilesLv
            width: parent.width

            model: Qt.fontFamilies()
            delegate: Common.ListDelegate {
                height: 40
                width: ListView.view.width
                textInput.text: modelData
                progressBar.value: model.index / Qt.fontFamilies().length
            }
        }

        Item { height: 16; width: 1; }

        Column {
            id: columnControls
            width: parent.width

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
}
