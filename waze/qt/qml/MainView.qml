import QtQuick 1.0
import org.waze 1.0

Item {
    id: mainView
    width: 800
    height: 480

    signal invokeAction(string action)

    WazeMap {
        id: wazeCanvas
        anchors.fill: parent
        z: -1
    }

    Button {
        id: exitButton
        text: "X"
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0

        onButtonPressed: mainView.invokeAction("exit")
    }

    Button {
        id: showMeButton
        width: 70
        height: 70
        text: "@"
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        onButtonPressed: mainView.invokeAction("show_me")
    }

    Button {
        id: zoominButton
        y: 86
        width: 70
        height: 70
        text: "+"
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.bottom: zoomoutButton.top
        anchors.bottomMargin: 0

        onButtonPressed: mainView.invokeAction("zoomin")
    }

    Button {
        id: zoomoutButton
        y: 156
        width: 70
        height: 70
        text: "-"
        anchors.verticalCenterOffset: -mainView.height / 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter

        onButtonPressed: mainView.invokeAction("zoomout")
    }

}
