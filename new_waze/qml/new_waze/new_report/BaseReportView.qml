import QtQuick 1.1
import com.nokia.meego 1.0

Rectangle {
    id: baseReport

    width:800
    height: 400
    color: "#00000000"

    property bool isInMySide: button1.checked
    property bool isPortrait: false
    property string option1icon: ""
    property string option2icon: ""
    property string option3icon: ""

    ButtonRow {
        id: _buttonrow1
        width: isPortrait? baseReport.width: baseReport.width/2
        height: 100
        anchors.left: parent.left
        anchors.top: _buttonrow2.bottom

        Button {
            id: button1
            text: qsTr("My Side")
            checked: true
        }

        Button {
            id: button2
            text: qsTr("Other Side")
        }
    }

    ButtonRow {
        id: _buttonrow2
        width: isPortrait? baseReport.width: baseReport.width/2
        height: 100
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: parent.top

        Button {
            iconSource: (option1icon === "")? option1icon : "image://waze/" + option1icon
            checked: true
            visible: option1icon !== ""
        }
        Button {
            iconSource: (option2icon === "")? option2icon : "image://waze/" + option2icon
            visible: option2icon !== ""
        }
        Button {
            iconSource: (option3icon === "")? option3icon : "image://waze/" + option3icon
            visible: option3icon !== ""
        }
    }

    Rectangle {
        id: rectangle1
        x: 0
        y: 100
        width: isPortrait? baseReport.width: baseReport.width/2
        height: 100
        color: "#00000000"
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: _buttonrow1.bottom

        TextField {
            id: textfield1
            y: 36
            width: rectangle1.width-button4.width-10
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
        }

        Button {
            id: button4
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.verticalCenter: parent.verticalCenter
            width: 50
            iconSource: "image://waze/recorder_record.png"
        }
    }

    Button {
        id: button3
        text: qsTr("Send Report")
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Rectangle {
        id: rectangle2
        width: baseReport.width/2
        height: 300
        color: "#00000000"
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0

        Button {
            id: button5
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.horizontalCenter: parent.horizontalCenter
            iconSource: "image://waze/add_image_box.png"
        }

        Rectangle {
            anchors.topMargin: 10
            anchors.top: button5.bottom
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: parent.left

        }
    }
}
