import QtQuick 2.15
import QtQuick.Layouts 1.15

import "common" as Common

ColumnLayout {
    id: col
    anchors.fill: parent
    spacing: 16
    Common.Button {
        Layout.fillWidth: true
        text: "Показать картинку с камеры"
    }
    Rectangle {
        id: vlcVideoPlayer
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "black"
    }
}
