import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import "common" as Common

import cpp.DeviceModel 43.21

GridLayout {
    id: root

    property int devModelCurrentIndex: core.devModelCurrentIndex
    property DeviceModel deviceModel: core.deviceModel
    property var currentDeviceCam: core.currentDeviceCam

    onCurrentDeviceCamChanged: {
        console.info("ZAQAZ currentDeviceCam =", JSON.stringify(currentDeviceCam));
    }

    enabled: devModelCurrentIndex !== -1
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

        text: currentDeviceCam.mac
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "IP"
    }
    Common.LineEdit {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        readOnly: true

        text: currentDeviceCam.ip
    }

    Common.Button {
        Layout.columnSpan: 4
        Layout.fillWidth: true
        text: "Прочитать конфигурацию камеры"
        onClicked: {
            core.readDevConfig();
        }
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Unique ID"
    }
    Common.LineEdit {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        text: currentDeviceCam.uniqueId

        textInput.onEditingFinished: {
            console.assert(deviceModel.set(devModelCurrentIndex
                                           , text
                                           , DeviceModel.DmUniqueIdRole)
                           , "ERROR: setData returned false")
        }
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Status"
    }
    Common.LineEdit {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        text: currentDeviceCam.statusString

        textInput.onEditingFinished: {
            console.assert(deviceModel.set(devModelCurrentIndex
                                           , text
                                           , DeviceModel.DmStatusStringRole)
                           , "ERROR: setData returned false")
        }
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Video duration"
    }
    Common.SpinBox {
        Layout.fillWidth: true

        Binding on text {
            value: currentDeviceCam.videoDuration
        }

        textInput.onEditingFinished: {
            console.info("ZAQAZ textInput.onEndingFinished");
            console.assert(deviceModel.set(devModelCurrentIndex
                                           , text
                                           , DeviceModel.DmVideoDurationRole)
                           , "ERROR: setData returned false")
        }
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Charge detect delay"
    }
    Common.SpinBox {
        Layout.fillWidth: true
        Binding on text {
            value: currentDeviceCam.chargeDetectDelay
        }

        textInput.onEditingFinished: {
            console.assert(deviceModel.set(devModelCurrentIndex
                                           , text
                                           , DeviceModel.DmChargeDetectDelayRole)
                           , "ERROR: setData returned false")
        }
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Mode"
    }
    Common.SpinBox {
        Layout.fillWidth: true
        Binding on text {
            value: currentDeviceCam.mode
        }

        textInput.onEditingFinished: {
            console.assert(deviceModel.set(devModelCurrentIndex
                                           , text
                                           , DeviceModel.DmModeRole)
                           , "ERROR: setData returned false")
        }
    }

    Common.Text1 {
        Layout.alignment: Qt.AlignVCenter
        text: "Video rotation"
    }
    Column {
        id: rotationColumn

        readonly property bool videoRotation: currentDeviceCam.videoRotation

        Layout.fillWidth: true
        spacing: 8
        Common.RadioButton {
            id: rotation0
            width: parent.width
            text: "0"
            checked: rotationColumn.videoRotation === false
            clickedBehavior: function() {
                if (rotationColumn.videoRotation === false) {
                    return;
                }
                console.assert(deviceModel.set(devModelCurrentIndex
                                               , false
                                               , DeviceModel.DmVideoRotationRole)
                               , "ERROR: setData returned false");
            }
        }
        Common.RadioButton {
            id: rotation180
            width: parent.width
            text: "180"
            checked: rotationColumn.videoRotation === true
            clickedBehavior: function() {
                if (rotationColumn.videoRotation === true) {
                    return;
                }
                console.assert(deviceModel.set(devModelCurrentIndex
                                               , true
                                               , DeviceModel.DmVideoRotationRole)
                               , "ERROR: setData returned false");
            }
        }
    }

    Common.CheckBox {
        Layout.columnSpan: 4
        text: "Log write"
        checked: currentDeviceCam.logWrite
        clickedBehavior: function() {
            console.info("ZAQAZ logWrite", checked);
            console.assert(deviceModel.set(devModelCurrentIndex
                                           , !checked
                                           , DeviceModel.DmLogWriteRole)
                           , "ERROR: setData returned false");
        }
    }

    Common.Button {
        Layout.columnSpan: 4
        Layout.fillWidth: true
        text: "Отправить"

        onClicked: {
            core.writeDevConfig();
        }
    }

    Item {
        Layout.columnSpan: 4
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
