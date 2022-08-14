import QtQuick 2.15

TextInput {
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontTextBtnPixelSize
    font.weight: Theme.fontTextBtnWeight
    verticalAlignment: TextInput.AlignVCenter
    color: enabled ? Theme.white : Theme.blackLight
    readOnly: true
    cursorVisible: false
}
