// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: rectangle1
    width: 100
    height: 60
    border.width: 3
    border.color: "#000000"

    signal clicked

    MouseArea {
        anchors.fill: parent
        onClicked: rectangle1.clicked()
    }

    Image {
        id: image1
        width: height
        anchors.bottomMargin: 5
        fillMode: Image.PreserveAspectFit
        anchors.leftMargin: 5
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.bottom: speed.top
        anchors.top: parent.top
        source: type + ".png"
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
            font.pixelSize: 12
        }

        Text {
            id: locationLabel
            text: qsTr(location)
            font.pixelSize: 12
        }

        Text {
            id: messageLabel
            text: qsTr(message)
            font.pixelSize: 12
        }

        Text {
            id: timestampLabel
            text: qsTr(timestamp)
            font.pixelSize: 12
        }
    }

    Text {
        id: speedLabel
        text: qsTr(speed)
        anchors.bottomMargin: 5
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: image1.horizontalCenter
        font.pixelSize: 12
    }
}
