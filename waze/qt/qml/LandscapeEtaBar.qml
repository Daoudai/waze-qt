import QtQuick 1.0

Button {
    id: etaBar
    property bool isEta: true

    height: 100

    Text {
        color: "#ffffff"
        anchors.right: etaSeperator.left
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        text: (etaBar.isEta)? navigationData.eta : navigationData.etaTime
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 32
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        id: etaSeperator
        color: "#000000"
        anchors.centerIn: parent

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
        anchors.left: etaSeperator.right
        anchors.verticalCenter: parent.verticalCenter

        text: (etaBar.isEta)? speedometerData.text : navigationData.remainingDistance
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 32
        verticalAlignment: Text.AlignVCenter
    }

    Timer {
        id: eta_message_timer
        repeat: true
        running: etaBar.visible
        interval: 5000
        onTriggered: {
            etaBar.isEta = !etaBar.isEta
        }
    }
}
