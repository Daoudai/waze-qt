// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

ToolIcon {
    id: descriptiveButton
    property string text: qsTr("text")

    property bool isCountVisibile: false
    property string count: ""

    Rectangle {
        id: rectangle1
        width: countText.font.pixelSize * 2
        height: countText.font.pixelSize * 2
        color: "#7d91c0"
        radius: height/2
        border.width: 1
        border.color: "#000000"
        z: 1
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.verticalCenter: parent.verticalCenter
        visible: descriptiveButton.isCountVisibile
        smooth: true

        Text {
            id: countText
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 14
            color: "#ffffff"
            text: descriptiveButton.count
            style: Text.Outline
            verticalAlignment: Text.AlignVCenter
            anchors.fill: parent
            smooth: true
        }
    }

    Text {
        id: text1
        color: "#ffffff"
        text: descriptiveButton.text
        style: Text.Outline
        font.bold: true
        anchors.top: parent.top
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 12
        smooth: true
    }
}

