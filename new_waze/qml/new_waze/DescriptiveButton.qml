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

    Image {
        source: descriptiveButton.iconSource === ""? descriptiveButton.iconSource : "image://waze/" + descriptiveButton.iconSource
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.bottomMargin: 10
        anchors.topMargin: 5
        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
    }

    Rectangle {
        id: rectangle1
        width: countText.width + (countText.font.pixelSize)
        height: countText.height + 5
        color: "#7d91c0"
        radius: height/2
        opacity: 0.690
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        border.width: 1
        border.color: "#000000"
        z: 1
        visible: descriptiveButton.isValueVisible
        smooth: true
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
        verticalAlignment: Text.AlignTop
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 12
        smooth: true
    }

    Text {
        id: countText
        x: 7
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 14
        color: "#ffffff"
        text: descriptiveButton.value
        visible: descriptiveButton.isValueVisible
        anchors.verticalCenter: rectangle1.verticalCenter
        z: 2
        anchors.horizontalCenter: rectangle1.horizontalCenter
        style: Text.Outline
        verticalAlignment: Text.AlignVCenter
        smooth: true
    }
}

