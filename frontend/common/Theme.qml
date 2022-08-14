pragma Singleton

import QtQuick 2.15

QtObject {

    function opacifiedColor(color, opacity) {
        return Qt.rgba(color.r, color.g, color.b, opacity);
    }

    readonly property color bg: "#F4F5F0"
    readonly property color white: "#FFFFFF"
    readonly property color blackDark: "#0E0E0E"
    readonly property color blackLight: "#777775"
    readonly property color greyLight: "#E6E8DF"
    readonly property color greyMedium: "#D0D9BE"
    readonly property color greyDark: "#A0B37D"
    readonly property color greenHover: "#E9F6CF"
    readonly property color greenDown: "#D0E2AB"
    readonly property color greenSelected: "#BDE273"
    readonly property color deepHover: "#096574"
    readonly property color deepNormal: "#00525F"
    readonly property color deepDown: "#034550"
    readonly property int fontWeight400: Font.Normal
    readonly property int fontWeight500: Font.DemiBold

    // for all
    readonly property string fontFamily: "Artifakt Element"
    readonly property color borderNormal: greyDark
    readonly property color borderHover: deepHover
    readonly property color borderFocused: blackLight
    readonly property color borderReadOnly: greyDark
    readonly property color borderDisabled:  greyLight

    // TextH1
    readonly property int fontTextH1PixelSize: 16
    readonly property int fontTextH1Weight: fontWeight500
    readonly property color fontTextH1Color: blackDark
    // Text1
    readonly property int fontText1PixelSize: 14
    readonly property int fontText1Weight: fontWeight400
    readonly property color fontText1Color: blackDark
    // TextBtn
    readonly property int fontTextBtnPixelSize: 14
    readonly property int fontTextBtnWeight: fontWeight500
    readonly property color fontTextBtnColor: white
    // Button
    readonly property color buttonNormalColor: deepNormal
    readonly property color buttonHoverColor: deepHover
    readonly property color buttonDownColor: deepDown
    readonly property color buttonDisabledColor: greyLight
    // progressBar
    readonly property color progressBarBg: opacifiedColor(greyDark, 0.5)
    readonly property color progressBarValue: deepNormal
    // list Delegate
    readonly property color listMouseNormal: white
    readonly property color listMouseHover: greenHover
    readonly property color listMouseDown: greenDown
    readonly property color listRowSelected: greenSelected
    readonly property color listBg: white
    // tab
    readonly property color tabBtnNormal: greyMedium
    readonly property color tabBtnHover: greenHover
    readonly property color tabBtnDown: greenDown
    readonly property color tabBtnSelected: white
    readonly property color tabBtnDisabled: greyLight
    readonly property color tabBg: white
    // LineEdit
    readonly property color leBgNormal: white
    readonly property color leBgReadOnly: greyLight
    readonly property color leBgDisabled: greyLight
    // CheckBox & RadioButton & SpinBox
    readonly property color cbBgNormal: white
    readonly property color cbBgDisabled: greyLight
    readonly property color cbImageNormal: deepNormal
    readonly property color cbImageDown: deepDown // only for SpinBox
    readonly property color cbImageDisabled: blackLight
}
