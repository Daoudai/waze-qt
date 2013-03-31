import QtQuick 1.0

Button {
    id: etaBar
    property bool isEta: true

    height: 120

    Text {
        color: "#ffffff"
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top

        text: (etaBar.isEta)? navigationData.eta : navigationData.etaTime
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 32
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        color: "#ffffff"
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom

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
