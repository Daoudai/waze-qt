import QtQuick 1.0

Rectangle {
    id: button
    property string text: "text"

    signal buttonPressed

    width: buttonText.width*2
    radius: 18
    border.color: "#696969"
    gradient: Gradient {
        GradientStop {
            position: 0.070
            color: "#585858"
        }

        GradientStop {
            position: 1
            color: "#000000"
        }
    }
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 0
    anchors.top: parent.top
    anchors.topMargin: 0

    MouseArea {
        id: button1ClickArea
        anchors.fill: parent
        onClicked: buttonPressed()
        Text {
            id: buttonText
            x: -26
            y: -58
            color: "#ffffff"
            text: button.text
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 18
        }
    }
}
