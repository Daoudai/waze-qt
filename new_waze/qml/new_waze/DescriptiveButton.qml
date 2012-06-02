// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

MouseArea {
    id: descriptiveButton
    property string text: qsTr("text")

    width: 70
    height: 70

    property bool isValueVisible: false
    property string value: ""

    property string iconSource

    property color color: "transparent"

    property int valueSize: 28

    Image {
        id: buttonImage
        anchors.right: countText.left
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 5
        source: descriptiveButton.iconSource === ""? descriptiveButton.iconSource : "image://waze/" + descriptiveButton.iconSource
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.bottomMargin: 10
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: text1
        color: "#ffffff"
        text: descriptiveButton.text
        anchors.bottomMargin: 5
        anchors.bottom: parent.bottom
        style: Text.Outline
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 12
        smooth: true
    }

    Text {
        id: countText
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: descriptiveButton.valueSize
        color: "#ffffff"
        text: descriptiveButton.value
        anchors.rightMargin: descriptiveButton.value === ""? 0 : 5
        anchors.bottomMargin: 10
        anchors.topMargin: 5
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        visible: descriptiveButton.isValueVisible
        style: Text.Outline
        verticalAlignment: Text.AlignVCenter
        smooth: true
        wrapMode: Text.WordWrap
    }

    Rectangle {
        id: background
        z: -1
        color: descriptiveButton.color
        anchors.fill: parent
    }
}

