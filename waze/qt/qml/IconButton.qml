import QtQuick 1.0

MouseArea {
    id: iconButton
    width: 100
    height: 100

    property string icon: ""
    property string pressedIcon: ""
    property bool fitImage: true

    property string text: ""
    property bool needTranslation: true

    Image {
        id: iconImage
        visible: !iconButton.pressed || pressedIcon  === ""

        anchors.fill: parent
        fillMode: fitImage? Image.PreserveAspectFit : Image.PreserveAspectCrop

        source: imageProvider.getImage(icon)
    }

    Image {
        id: pressedIconImage
        visible: iconButton.pressed

        anchors.fill: parent
        fillMode: fitImage? Image.PreserveAspectFit : Image.PreserveAspectCrop

        source: imageProvider.getImage(pressedIcon)
    }

    Text {
        id: text1
        color: "#ffffff"
        text: needTranslation? t+wTr(iconButton.text) : iconButton.text
        style: Text.Outline
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 14
    }
}
