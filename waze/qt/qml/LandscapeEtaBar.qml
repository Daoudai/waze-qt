import QtQuick 1.0

Button {
    id: etaBar

    height: 60

    Text {
        color: "#ffffff"
        anchors.right: etaSeperator1.left
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        text: navigationData.etaTime
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 22
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        id: etaSeperator1
        color: "#000000"
        x: parent.width / 4

        text: "|"
        font.bold: true
        style: Text.Outline
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 30
    }

    Text {
        color: "#ffffff"
        anchors.right: etaSeperator2.left
        anchors.left: etaSeperator1.right
        anchors.verticalCenter: parent.verticalCenter

        text: navigationData.eta
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 22
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        id: etaSeperator2
        color: "#000000"
        x: parent.width * 2 / 4

        text: "|"
        font.bold: true
        style: Text.Outline
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 30
    }

    Text {
        color: "#ffffff"
        anchors.right: etaSeperator3.left
        anchors.left: etaSeperator2.right
        anchors.verticalCenter: parent.verticalCenter

        text: speedometerData.text
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 22
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        id: etaSeperator3
        color: "#000000"
        x: parent.width * 3 / 4

        text: "|"
        font.bold: true
        style: Text.Outline
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 30
    }

    Text {
        color: "#ffffff"
        anchors.right: parent.right
        anchors.left: etaSeperator3.right
        anchors.verticalCenter: parent.verticalCenter

        text: navigationData.remainingDistance
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 22
        verticalAlignment: Text.AlignVCenter
    }
}
