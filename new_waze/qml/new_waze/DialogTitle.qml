// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: dialogTitle
    property string image
    property string text
    width: label.width+image1.width+10
    height: image1.height
    color: "#00000000"

    Image {
        id: image1
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 0
        fillMode: Image.PreserveAspectFit
        source: (dialogTitle.image === "")? dialogTitle.image : "image://waze/" + dialogTitle.image
    }

    Text {
        id: label
        color: "#ffffff"
        text: dialogTitle.text
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pointSize: 20
    }
}
