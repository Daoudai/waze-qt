import QtQuick 1.0

Rectangle {
    id: button
    height: buttonText.height
    property string text: ""
    property bool needTranslation: true

    signal buttonPressed

    width: buttonText.width*2
    radius: height/3
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

    MouseArea {
        id: button1ClickArea
        anchors.fill: parent
        onClicked: buttonPressed()
    }

    Text {
        id: buttonText
        color: "#ffffff"
        text: needTranslation? translator.translate(button.text) : button.text
        font.bold: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 48
    }

    Gradient {
        id: pressedGradient
        GradientStop {
            position: 0.070
            color: "#000000"
        }

        GradientStop {
            position: 1
            color: "#585858"
        }
    }

    states: [
        State {
            name: "Pressed"
            when: button1ClickArea.pressed
            PropertyChanges {
                target: button
                gradient:pressedGradient
            }
        }
    ]
}
