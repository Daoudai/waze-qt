import QtQuick 1.0

Rectangle {
    id: tile
    width: 150
    height: 150
    color: "#0066ff"

    property string icon: ""
    property string description: ""
    property string value: ""

    property bool needTranslation: true

    signal clicked

    Image {
        id: iconImage
        width: 50
        height: 50
        anchors.left: parent.left
        anchors.leftMargin: 10
        fillMode: Image.PreserveAspectFit
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        source: imageProvider.getImage(icon)
    }

    Text {
        id: descriptionText
        color: "#ffffff"
        text: needTranslation? translator.translate(description) : description
        font.bold: false
        wrapMode: Text.WordWrap
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.bottomMargin: 5
        anchors.bottom: icon.top
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        font.pixelSize: 23
    }

    Text {
        id: valueText
        color: "#ffffff"
        text: needTranslation? translator.translate(value) : value
        font.bold: true
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: iconImage.right
        anchors.leftMargin: 5
        anchors.top: descriptionText.bottom
        anchors.topMargin: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        font.pixelSize: textLength <= 3? 32 : textLength <= 6? 18 : 12

        property int textLength: text.length
    }

    MouseArea {
        id: mouse_area1
        anchors.fill: parent
        onClicked: tile.clicked()
    }
}
