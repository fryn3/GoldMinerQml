import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4


import "common" as Common

GridLayout {
    id: grid

    width: parent.width
    columns: 4
    rowSpacing: 8
    columnSpacing: 8

    Common.TextH1 {
        Layout.columnSpan: 4
        Layout.fillWidth: true
        text: "Настройки для выбранной камеры"
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "MAC"
    }
    Common.LineEdit {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        readOnly: true
        text: "88:4D..."
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "IP"
    }
    Common.LineEdit {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        readOnly: true
        text: "192.168.28.43"
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Device name"
    }
    Common.LineEdit {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        text: "testCam"
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Unique ID"
    }
    Common.LineEdit {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        text: "uniq"
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Status"
    }
    Common.LineEdit {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        text: "st"
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Video duration"
    }
    Common.SpinBox {
        Layout.fillWidth: true
        text: "60"
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Charge detect\n delay"
    }
    Common.SpinBox {
        Layout.fillWidth: true
        text: "5"
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Mode"
    }
    Common.SpinBox {
        Layout.fillWidth: true
        text: "1"
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Video rotation"
    }
    Column {
        spacing: 8
        Common.RadioGroup { id: rg }
        Common.RadioButton {
            radioGroup: rg
            text: "0"
        }
        Common.RadioButton {
            radioGroup: rg
            text: "180"
        }
    }

    Common.CheckBox {
        Layout.columnSpan: 4
        checked: true
        text: "Log write"
    }

    Common.Button {
        Layout.columnSpan: 4
        Layout.fillWidth: true
        text: "Отправить"
    }

    Item {
        Layout.columnSpan: 4
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
