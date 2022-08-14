import QtQuick 2.15

TextInput {
    // серый текст, который позволит MouseArea отработать клик
    property bool fakeDisable: false

    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontText1PixelSize
    font.weight: Theme.fontText1Weight
    verticalAlignment: TextInput.AlignVCenter
    color: enabled && !fakeDisable ? Theme.blackDark : Theme.blackLight
    readOnly: true
    cursorVisible: false
}
