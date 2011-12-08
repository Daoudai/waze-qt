import QtQuick 1.0

Rectangle {
    id: rectangle3
    width: 400
    height: 125
    color: "#00000000"

    property bool isRtl: false

    property bool isPassword: false
    property string text: "text"
    property string actionButtonText: "Action"
    property string cancelButtonText: "Cancel"

    signal mouseAreaPressed()
    signal actionButtonPressed(string text)
    signal cancelButtonPressed()

    function buttonPressed(buttonText)
    {
        mouseAreaPressed();
        if (buttonText == actionButtonText)
        {
            actionButtonPressed(text_input1.text);
        }
        else
        {
            cancelButtonPressed();
        }
    }

    Rectangle {
        id: rectangle1
        x: 10
        y: 9
        color: "#ffffff"
        height: 50
        radius: height/3
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        border.width: 2
        border.color: "#000000"

        TextInput {
            id: text_input1
            height: 20
            text: rectangle3.text
            echoMode: isPassword? TextInput.PasswordEchoOnEdit : TextInput.Normal
            cursorVisible: true
            horizontalAlignment: isRtl? TextInput.AlignRight : TextInput.AlignLeft
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 18
            focus: true
        }
    }

    Row {
        id: buttonRow
        height: 50
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        spacing: 10
        Repeater {
            model: 2
            Button {
                width: (buttonRow.width-10)/2
                visible: ((isRtl && index == 1) || (!isRtl && index === 0))? actionButtonText != "" : true
                text: ((isRtl && index == 1) || (!isRtl && index === 0))? actionButtonText : cancelButtonText
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                onButtonPressed: rectangle3.buttonPressed(text)
            }
        }
    }
}
