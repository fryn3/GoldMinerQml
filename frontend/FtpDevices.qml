import QtQuick 2.0
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import "common" as Common
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

            model: Qt.fontFamilies()
            delegate: Common.ListDelegate {
                height: 40
                width: ListView.view.width
                textInput.text: modelData
                progressBar.value: model.index / Qt.fontFamilies().length
            }
        }
        Common.Button {
            Layout.fillWidth: true
            height: 40
            text: "Поиск устройств"
        }
        // BTN
    }

}
