// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: rectangle1
    width: 100
    height: 110
    border.width: 3
    border.color: "#000000"

    signal clicked

    function getImage(reportType) {

        if (reportType === "All")
        {
            return "report_list_all.png";
        }
        else if (reportType === "Police")
        {
            return "report_list_police.png";
        }
        else if (reportType === "Traffic")
        {
            return "report_list_loads.png";
        }
        else if (reportType === "Accidents")
        {
            return "report_list_accidents.png"
        }
        else if (reportType === "Chit Chats")
        {
            return "report_list_chit_chats.png";
        }
        else if (reportType === "Other")
        {
            return "report_list_other.png"
        }

        console.log("Unknown report type: " + reportType);
    }

    MouseArea {
        anchors.fill: parent
        onClicked: rectangle1.clicked()
    }

    Image {
        id: image1
        anchors.bottomMargin: 5
        fillMode: Image.PreserveAspectFit
        anchors.leftMargin: 5
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.bottom: speed.top
        anchors.top: parent.top
        source: rectangle1.getImage(type)
    }

    Column {
        id: column1
        anchors.leftMargin: 10
        spacing: 10
        anchors.rightMargin: 5
        anchors.bottomMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: image1.right

        Text {
            id: titleLabel
            text: qsTr(title)
            font.pixelSize: 24
        }

        Text {
            id: locationLabel
            text: qsTr(location)
            font.pixelSize: 18
        }

        Text {
            id: messageLabel
            text: typeof(message) !== 'undefined'? qsTr(message) : ''
            font.pixelSize: 12
        }

        Text {
            id: timestampLabel
            text: qsTr(timestamp)
            font.pixelSize: 18
        }
    }

    Text {
        id: speedLabel
        text: qsTr(speed)
        anchors.bottomMargin: 5
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: image1.horizontalCenter
        font.pixelSize: 18
    }
}
