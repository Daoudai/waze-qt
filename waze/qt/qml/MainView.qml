import QtQuick 1.0
import org.waze 1.0

Item {
    id: mainView
    width: 800
    height: 480

    signal invokeAction(string action)

    Component.onCompleted: {
        sidebar_visibility_timer.start();
    }

    function showSideToolbars()
    {
        if (!wazeCanvas.isDialogActive)
        {
            zoominButton.visible = true;
            zoomoutButton.visible = true;
            sidebar_visibility_timer.stop();
            sidebar_visibility_timer.start();
        }
    }

    Timer {
        id: sidebar_visibility_timer
        repeat: false
        running: true
        interval: 4000
        onTriggered: {
            zoominButton.visible = false;
            zoomoutButton.visible = false;
        }
    }

    WazeMap {
        id: wazeCanvas
        anchors.fill: parent
        z: -1

        onIsDialogActiveChanged: {
            if (isDialogActive)
            {
                sidebar_visibility_timer.triggered();
            }
        }

        onClicked: showSideToolbars()
    }

    Button {
        id: exitButton
        width: 70
        height: 50
        text: "X"
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0

        visible: !wazeCanvas.isDialogActive

        onButtonPressed: mainView.invokeAction("quit")
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

        visible: !wazeCanvas.isDialogActive

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

        onButtonPressed: {
            showSideToolbars();
            mainView.invokeAction("zoomin")
        }
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

        onButtonPressed:  {
            showSideToolbars();
            mainView.invokeAction("zoomout")
        }
    }

    Button {
        id: optionsButton
        x: 0
        width: 70
        height: 50
        text: "O"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 0

        visible: !wazeCanvas.isDialogActive

        onButtonPressed: mainView.invokeAction("settingsmenu")
    }

    Button {
        id: minimizeButton
        x: 504
        width: 70
        height: 50
        text: "-"
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.right: exitButton.left
        anchors.rightMargin: 5

        visible: !wazeCanvas.isDialogActive

        onButtonPressed: mainView.invokeAction("minimize")
    }

    Button {
        id: navigateButton
        width: 70
        height: 70
        text: "%"
        anchors.left: showMeButton.right
        anchors.leftMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        visible: !wazeCanvas.isDialogActive

        onButtonPressed: mainView.invokeAction("search_menu")
    }

}
