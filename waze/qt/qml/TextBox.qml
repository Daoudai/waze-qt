import QtQuick 1.0

Rectangle {
    id: rectangle3
    width: 800
    height: 400
    color: "#000000"

    property bool isRtl: false

    property string title: "title"
    property bool isPassword: false
    property alias text: text_input1.text
    property string actionButtonText: "Action"
    property string cancelButtonText: "Cancel"

    signal mouseAreaPressed()
    signal actionButtonPressed(string text)
    signal cancelButtonPressed()

    onMouseAreaPressed: {
        // auto accept the last word that might not be in a dictionary
        text_input1.focus = false;
        text_input1.focus = true;
    }

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
        height: text_input1.height*1.1
        color: "#ffffff"
        radius: height/3
        anchors.top: buttonRow.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        border.width: 2
        border.color: "#000000"

        TextInput {
            id: text_input1
            echoMode: isPassword? TextInput.PasswordEchoOnEdit : TextInput.Normal
            cursorVisible: true
            horizontalAlignment: isRtl? TextInput.AlignRight : TextInput.AlignLeft
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 64
            focus: true
            selectByMouse: true
            onAccepted: rectangle3.buttonPressed(actionButtonText)
        }
    }

    Row {
        id: buttonRow
        anchors.top: titleText.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        spacing: 10

        Repeater {
            model: 2
            Button {
                width: (buttonRow.width-10)/2
                visible: ((isRtl && index == 1) || (!isRtl && index === 0))? actionButtonText != "" : true
                text: ((isRtl && index == 1) || (!isRtl && index === 0))? actionButtonText : cancelButtonText
                onButtonPressed: rectangle3.buttonPressed(text)
            }
        }
    }

    Text {
        id: titleText
        color: "#ffffff"
        text: rectangle3.title
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 0
        verticalAlignment: Text.AlignVCenter
        visible: true
        font.pixelSize: 28
    }
}
