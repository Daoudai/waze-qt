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

    WazeImageProvider {
        id: imageProvider
    }

    Translator {
        id: translator
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

    IconButton {
        id: exitButton
        width: 70
        height: 50
        icon: "exit_button"
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0

        visible: !wazeCanvas.isDialogActive

        onClicked: mainView.invokeAction("quit")
    }

    IconButton {
        id: zoominButton
        y: 86
        width: 70
        height: 70
        icon: "rm_zoomin"
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.bottom: zoomoutButton.top
        anchors.bottomMargin: 0

        onClicked: {
            showSideToolbars();
            mainView.invokeAction("zoomin")
        }
    }

    IconButton {
        id: zoomoutButton
        y: 156
        width: 70
        height: 70
        icon: "rm_zoomout"
        anchors.verticalCenterOffset: -mainView.height / 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter

        onClicked:  {
            showSideToolbars();
            mainView.invokeAction("zoomout")
        }
    }

    IconButton {
        id: optionsButton
        x: 0
        width: 70
        height: 50
        icon: "settings_button"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 0

        visible: !wazeCanvas.isDialogActive

        onClicked: mainView.invokeAction("settingsmenu")
    }

    IconButton {
        id: minimizeButton
        x: 504
        width: 70
        height: 50
        icon: "menu_page_down"
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.right: exitButton.left
        anchors.rightMargin: 5

        visible: !wazeCanvas.isDialogActive

        onClicked: mainView.invokeAction("minimize")
    }

    Flow {
        id: flow1
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        IconButton {
            id: showMeButton
            width: 120
            height: 70
            icon: "me_on_map_wide"
            pressedIcon: "me_on_map2_wide"
            fitImage: false

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("show_me")
        }

        IconButton {
            id: navigateButton
            width: 120
            height: 70
            icon: "Search_wide"
            pressedIcon: "Search2_wide"
            fitImage: false

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("search_menu")
        }

        IconButton {
            id: showAlertsButton
            width: 120
            height: 70
            icon: "Live_event_wide"
            pressedIcon: " Live_event2_wide"
            fitImage: false

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("real_time_alerts_list")
        }

        IconButton {
            id: reportAlertButton
            width: 120
            height: 70
            icon: "Report_wide"
            pressedIcon: "Report2_wide"
            fitImage: false

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("alertsmenu")
        }

        Button {
            id: speedometer
            width: 220
            height: 70
            visible: typeof(speedometerData) !== 'undefined' && speedometerData.isVisible && !wazeCanvas.isDialogActive
//            anchors.right: parent.right
//            anchors.rightMargin: 10
//            anchors.bottom: parent.bottom
//            anchors.bottomMargin: 10
            text: typeof(speedometerData) !== 'undefined'? speedometerData.text : ""
            needTranslation: false
        }
    }

}
