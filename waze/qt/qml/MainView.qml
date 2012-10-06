import QtQuick 1.0
import org.waze 1.0

Item {
    id: mainView
    width: 800
    height: 480

    signal invokeAction(string action)

    ///////////////// Translations related ///////////////
    property bool isRTL: typeof(__translator) === 'undefined'? false : __translator.isRTL
    property string t: ""
    function wTr(text) {
        if (typeof(__translator) === 'undefined') return text;

        return __translator.translate(text);
    }
    Connections {
        target: __translator
        onTranslationsReloaded: {
            mainView.tChanged();
        }
    }
    ////////////// End Of Translations related ///////////

    Component.onCompleted: {
        sidebar_visibility_timer.start();
    }

    function showSideToolbars()
    {
        if (!wazeCanvas.isDialogActive)
        {
            zoomBar.visible = true;
            mapBar.visible = true;
            sidebar_visibility_timer.stop();
            sidebar_visibility_timer.start();
        }
    }

    ////////////// Context data binding wrappers ////////////
    Item {
        id: imageProvider

        function getImage(imageName)
        {
            if (typeof(__imageProvider) === 'undefined') return "";

            return __imageProvider.getImage(imageName);
        }
    }

    Item {
        id: speedometerData

        property bool isVisible : (typeof(__speedometerData) === 'undefined')? false : __speedometerData.isVisible
        property string text : (typeof(__speedometerData) === 'undefined')? "" : __speedometerData.text
    }

    Item {
        id: alerts

        property string alertsCount : (typeof(__alerts) === 'undefined')? "" : __alerts.alertsCount
    }
    ////////////// End of Context data binding wrappers //////////////

    Timer {
        id: sidebar_visibility_timer
        repeat: false
        running: true
        interval: 4000
        onTriggered: {
            zoomBar.visible = false;
            mapBar.visible = false;
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

    Column {
        id: zoomBar

        anchors.verticalCenterOffset: -mainView.height / 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter

        spacing: 10

        IconButton {
            id: zoominButton
            width: 70
            height: 70
            icon: "rm_zoomin"

            onClicked: {
                showSideToolbars();
                mainView.invokeAction("zoomin")
            }
        }

        IconButton {
            id: zoomoutButton
            width: 70
            height: 70
            icon: "rm_zoomout"

            onClicked:  {
                showSideToolbars();
                mainView.invokeAction("zoomout")
            }
        }
    }


    Column {
        id: mapBar

        anchors.verticalCenterOffset: -mainView.height / 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter

        spacing: 10

        IconButton {
            id: mapEditButton
            width: 70
            height: 70
            icon: "update_map_button"

            onClicked: {
                showSideToolbars();
                mainView.invokeAction("map_updates_menu")
            }
        }

        IconButton {
            id: compassButton
            width: 70
            height: 70
            icon: "north"

            onClicked:  {
                showSideToolbars();
                mainView.invokeAction("toggleorientation")
            }
        }
    }

    IconButton {
        id: minimizeButton
        width: 70
        height: 50
        icon: "menu_page_down"
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.right: exitButton.left
        anchors.rightMargin: 10

        visible: !wazeCanvas.isDialogActive

        onClicked: mainView.invokeAction("minimize")
    }

    Flow {
        id: bottomBar
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.bottom: parent.bottom

        IconButton {
            id: showMeButton
            width: 140
            height: 70
            icon: "me_on_map_wide"
            pressedIcon: "me_on_map2_wide"
            text: "Me_on_map"
            fitImage: false

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("show_me")
        }

        IconButton {
            id: navigateButton
            width: 140
            height: 70
            icon: "Search_wide"
            pressedIcon: "Search2_wide"
            text: "Drive_to"
            fitImage: false

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("search_menu")
        }

        IconButton {
            id: showAlertsButton
            width: 140
            height: 70
            icon: "Live_event_wide"
            pressedIcon: "Live_event2_wide"
            text: "Events"
            fitImage: false

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("real_time_alerts_list")
            z: 1

            Rectangle {
                color: "red"
                width: height
                height: parent.height / 2 - 5
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 10
                radius: height / 2
                visible: typeof(alertsCountText.text) !== 'undefined' && alertsCountText.text !== ""

                Text {
                    id: alertsCountText
                    text: alerts.alertsCount
                    style: Text.Outline
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.fill: parent
                    color: "white"
                    font.pixelSize: height / 2
                }
            }
        }

        IconButton {
            id: reportAlertButton
            width: 140
            height: 70
            icon: "Report_wide"
            pressedIcon: "Report2_wide"
            text: "Report"
            fitImage: false

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("alertsmenu")
        }

        Button {
            id: speedometer
            width: 220
            height: 70
            visible: typeof(speedometerData) !== 'undefined' && speedometerData.isVisible && !wazeCanvas.isDialogActive
            text: typeof(speedometerData) !== 'undefined'? speedometerData.text : ""
            needTranslation: false
        }
    }

    Flow {
        id: topBar
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.right: minimizeButton.left
        anchors.left: parent.left

        spacing: 10

        IconButton {
            id: aboutButton
            width: 70
            height: 50
            icon: "about"

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("about")
        }

        IconButton {
            id: moodsButton
            width: 70
            height: 50
            icon: "TS_top_mood_happy"

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("mood_dialog")
        }

        IconButton {
            id: gpsStatusButton
            width: 70
            height: 50
            icon: "TS_top_satellite_on"

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("gps_net_stat")
        }

        IconButton {
            id: onlineButton
            width: 70
            height: 50
            icon: "TS_top_connected"

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("gps_net_stat")
        }

        IconButton {
            id: profileButton
            width: 70
            height: 50
            icon: "general_settings"

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("mywaze")
        }

        IconButton {
            id: optionsButton
            width: 70
            height: 50
            icon: "settings_button"

            visible: !wazeCanvas.isDialogActive

            onClicked: mainView.invokeAction("settingsmenu")
        }
    }

}
