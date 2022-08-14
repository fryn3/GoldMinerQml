import QtQuick 2.15

TextInput {
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontTextH1PixelSize
    font.weight: Theme.fontTextH1Weight
    verticalAlignment: TextInput.AlignVCenter
    color: enabled ? Theme.blackDark : Theme.blackLight
    readOnly: true
    cursorVisible: false
}
