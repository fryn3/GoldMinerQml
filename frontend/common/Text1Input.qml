import QtQuick 2.15

TextInput {
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontText1PixelSize
    font.weight: Theme.fontText1Weight
    color: enabled ? Theme.blackDark : Theme.blackLight
    focus: true
    cursorVisible: activeFocus
}
