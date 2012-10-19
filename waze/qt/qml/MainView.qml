import QtQuick 1.0
import org.waze 1.0

Item {
    id: base
    width: 800
    height: 480

    signal invokeAction(string action)
    signal buttonClicked()

    property int deviceOrientation: typeof(__orientationSensor) === 'undefined'? -1 : __orientationSensor.orientation
    onDeviceOrientationChanged: {
        if (deviceOrientation == 1)
        {
            mainView.rotation = 0;
            mainView.width = base.width;
            mainView.height = base.height;
        }
        else if (deviceOrientation == 4)
        {
            mainView.rotation = 90;
            mainView.width = base.height;
            mainView.height = base.width;
        }
        else if (deviceOrientation == 2)
        {
            mainView.rotation = 180;
            mainView.width = base.width;
            mainView.height = base.height;
        }
        else if (deviceOrientation == 3)
        {
            mainView.rotation = 270;
            mainView.width = base.height;
            mainView.height = base.width;
        }

        rotation_change_timer.start();
    }

    ///////////////// Translations related ///////////////
    property bool isRTL: typeof(__translator) === 'undefined'? false : __translator.isRTL
    property string t: ""
    function wTr(text) {
        if (typeof(__translator) === 'undefined') return text;

        return __translator.translate(text);
    }
    Connections {
        target: base
        onIsRTLChanged: {
            base.tChanged();
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

    Item {
        id: moods

        property int mood : (typeof(__moods) === 'undefined')? 0 : __moods.mood
    }

    Item {
        id: monitor

        property int gpsState : (typeof(__monitor) === 'undefined')? 0 : __monitor.gpsState
        property int netState : (typeof(__monitor) === 'undefined')? 0 : __monitor.netState
    }

    Item {
        id: compass

        property int orientation : (typeof(__compass) === 'undefined')? 0 : __compass.orientation
        property int compassState : (typeof(__compass) === 'undefined')? 0 : __compass.compassState
    }

    Item {
        id: editor

        property int editState : (typeof(__editor) === 'undefined')? 0 : __editor.editState
        property int editType : (typeof(__editor) === 'undefined')? 0 : __editor.editType
    }

    Item {
        id: navigationData

        property bool isNavigation : (typeof(__navigationData) === 'undefined')? false : __navigationData.isNavigation
        property string eta : (typeof(__navigationData) === 'undefined')? "" : __navigationData.eta
        property string etaTime : (typeof(__navigationData) === 'undefined')? "" : __navigationData.etaTime
        property string remainingDistance : (typeof(__navigationData) === 'undefined')? "" : __navigationData.remainingDistance
        property string currentTurnType : (typeof(__navigationData) === 'undefined')? "" : __navigationData.currentTurnType
        property string currentTurnDistance : (typeof(__navigationData) === 'undefined')? "" : __navigationData.currentTurnDistance
        property int currentExit : (typeof(__navigationData) === 'undefined')? 0 : __navigationData.currentExit
        property string nextTurnType : (typeof(__navigationData) === 'undefined')? "" : __navigationData.nextTurnType
        property string nextTurnDistance : (typeof(__navigationData) === 'undefined')? "" : __navigationData.nextTurnDistance
        property int nextExit : (typeof(__navigationData) === 'undefined')? 0 : __navigationData.nextExit
        property string street : (typeof(__navigationData) === 'undefined')? "" : __navigationData.street
    }

    ////////////// End of Context data binding wrappers //////////////

    Timer {
        id: sidebar_visibility_timer
        repeat: false
        running: true
        interval: 5000
        onTriggered: {
            zoomBar.visible = false;
            mapBar.visible = false;
        }
    }

    Item {
        id: mainView

        Behavior on rotation { RotationAnimation { direction: RotationAnimation.Shortest; duration: 500; easing.type: Easing.OutBounce; } }
        Behavior on width    { NumberAnimation   { duration: 500 } }
        Behavior on height   { NumberAnimation   { duration: 500 } }

        anchors.centerIn: parent
        width: base.width; height: base.height

        Timer {
            id: rotation_change_timer
            repeat: false
            running: false
            interval: 750
            onTriggered: {
                wazeCanvas.repaintAfterResize();
            }
        }

        //////////////////////// Multi-duty Web browser ////////////////////
        WazeBrowser {
            id: wazeBrowser
            width: 0
            height: 0
            objectName: "wazeBrowser"
            visible: false
            url: ""
            z: 1
        }
        ////////////////////End of Multi-duty Web browser ////////////////////

        WazeMap {
            id: wazeCanvas
            width: parent.width
            height: parent.height
            x: 0
            y: 0
            z: -1

            onIsDialogActiveChanged: {
                if (isDialogActive)
                {
                    sidebar_visibility_timer.triggered();
                }
            }

            onClicked: showSideToolbars()
        }

        Rectangle {
            id: topBarBackground
            color: 'black'
            opacity: 0.7

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            height: topBar.height
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

            onClicked: {
                buttonClicked();
                invokeAction("quit");
            }
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
                    buttonClicked();
                    showSideToolbars();
                    invokeAction("zoomin");
                    wazeCanvas.repaint();
                }
            }

            IconButton {
                id: zoomoutButton
                width: 70
                height: 70
                icon: "rm_zoomout"

                onClicked:  {
                    buttonClicked();
                    showSideToolbars();
                    invokeAction("zoomout");
                    wazeCanvas.repaint();
                }
            }
        }

        Row {
            id: instructionsBar

            opacity: 0.9
            spacing: 20

            visible: navigationData.isNavigation && !wazeCanvas.isDialogActive
            anchors.left: parent.left
            anchors.bottom: bottomBar.top

            z: 1

            IconButton {
                id: currentTurn
                width: 120
                height: 120
                icon: navigationData.currentTurnType
                visible: navigationData.currentTurnType != ""
                text: navigationData.currentTurnDistance
                needTranslation: false
                fontSize: 24
                z: 1


                Text {
                    text: navigationData.currentExit
                    style: Text.Outline
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.centerIn: parent
                    color: "white"
                    font.pixelSize: currentTurn.height / 3
                    visible: navigationData.currentExit > 0
                    z: 1
                }

                Rectangle {
                    anchors.fill: parent
                    color: "darkcyan"
                    radius: 20
                    border.color: "black"
                    border.width: 3
                    z: -1
                }

                Rectangle {
                    color: "blue"
                    width: height
                    height: parent.height / 2 - 5
                    anchors.bottom: parent.top
                    anchors.bottomMargin: -radius*2
                    anchors.left: parent.right
                    anchors.leftMargin: -radius
                    radius: height / 2
                    visible: nextTurn.visible
                    z:2

                    Text {
                        text: "1"
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
                id: nextTurn
                width: 120
                height: 120
                icon: navigationData.nextTurnType
                text: navigationData.nextTurnDistance
                needTranslation: false
                visible: navigationData.nextTurnType != ""
                fontSize: 24
                z: 1

                Text {
                    text: navigationData.nextExit
                    style: Text.Outline
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.centerIn: parent
                    color: "white"
                    font.pixelSize: nextTurn.height / 3
                    visible: navigationData.nextExit > 0
                    z: 1
                }

                Rectangle {
                    anchors.fill: parent
                    color: "darkcyan"
                    radius: 20
                    border.color: "black"
                    border.width: 3
                    z: -1
                }

                Rectangle {
                    color: "blue"
                    width: height
                    height: parent.height / 2 - 5
                    anchors.bottom: parent.top
                    anchors.bottomMargin: -radius*2
                    anchors.left: parent.right
                    anchors.leftMargin: -radius
                    radius: height / 2
                    visible: nextTurn.visible
                    z:2

                    Text {
                        text: "2"
                        style: Text.Outline
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        anchors.fill: parent
                        color: "white"
                        font.pixelSize: height / 2
                    }
                }
            }
        }

        Button {
            height: 50
            opacity: 0.9
            width: mainView.width/3

            anchors.right: parent.right
            anchors.left: instructionsBar.right
            anchors.leftMargin: 10
            anchors.bottom: bottomBar.top
            visible: navigationData.isNavigation && !wazeCanvas.isDialogActive

            text: navigationData.street
            fontSize: 24
            needTranslation: false
        }

        Column {
            id: mapBar

            anchors.verticalCenterOffset: -mainView.height / 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter

            spacing: 10

            IconButton {
                id: rotateLeftButton
                width: 70
                height: 70
                icon: "nav_turn_left"

                onClicked: {
                    buttonClicked();
                    showSideToolbars();
                    invokeAction("rotate_left")
                    wazeCanvas.repaint();
                }
            }

            IconButton {
                id: mapEditButton1
                width: 70
                height: 70
                icon: editor.editState === 0? "update_map_button" : "update_map_button_recording"
                visible: editor.editType === 0

                onClicked: {
                    buttonClicked();
                    showSideToolbars();
                    invokeAction("map_updates_menu")
                }
            }

            IconButton {
                id: mapEditButton2
                width: 70
                height: 70
                icon: editor.editState === 0? "record_new_roads" : "update_map_button_recording"
                visible: editor.editType === 1

                onClicked: {
                    buttonClicked();
                    showSideToolbars();
                    invokeAction("togglenewroads")
                }
            }

            Image {
                id: compassButton
                width: 70
                height: 70

                source: imageProvider.getImage(compass.compassState == 0? "north_off" : "north")

                MouseArea {
                    z: 2
                    anchors.fill: parent
                    onClicked:  {
                        buttonClicked();
                        showSideToolbars();
                        invokeAction("toggleorientation")
                    }
                }

                Image {
                    rotation: compass.orientation
                    visible:   compass.compassState == 0
                    anchors.fill: parent
                    source: imageProvider.getImage("compass_needle")
                    z: 1
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                }
            }

            IconButton {
                id: rotateRightButton
                width: 70
                height: 70
                icon: "nav_turn_right"
                rotation: 180
                onClicked: {
                    buttonClicked();
                    showSideToolbars();
                    invokeAction("rotate_right");
                    wazeCanvas.repaint();
                }
            }
        }

        IconButton {
            id: minimizeButton
            width: 40
            height: 50
            icon: "button_sc_3_mid_s"
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.right: exitButton.left
            anchors.rightMargin: 10

            visible: !wazeCanvas.isDialogActive

            onClicked: {
                buttonClicked();
                invokeAction("minimize");
            }
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

                onClicked: {
                    buttonClicked();
                    invokeAction("show_me");
                }
            }

            IconButton {
                id: navigateButton
                width: 140
                height: 70
                icon: "Search_wide"
                pressedIcon: "Search2_wide"
                text: "Drive_to"
                fitImage: false

                visible: !wazeCanvas.isDialogActive && !navigationData.isNavigation

                onClicked: {
                    buttonClicked();
                    invokeAction("search_menu");
                }
            }

            IconButton {
                id: navigationOptionsButton
                width: 140
                height: 70
                icon: "More_wide"
                pressedIcon: "More2_wide"
                text: "Navigate"
                fitImage: false

                visible: !wazeCanvas.isDialogActive && navigationData.isNavigation

                onClicked: {
                    buttonClicked();
                    invokeAction("nav_menu");
                }
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

                onClicked: {
                    buttonClicked();
                    invokeAction("real_time_alerts_list");
                }
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

                onClicked: {
                    buttonClicked();
                    invokeAction("alertsmenu");
                }
            }

            Button {
                id: speedometer
                width: 220
                height: 70
                visible: speedometerData.isVisible && !wazeCanvas.isDialogActive && speedometerData.text != ""
                text: speedometerData.text
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

                onClicked: {
                    buttonClicked();
                    invokeAction("about");
                }
            }

            IconButton {
                id: moodsButton
                width: 70
                height: 50

                icon: moodToIcon(moods.mood)

                visible: !wazeCanvas.isDialogActive

                onClicked: {
                    buttonClicked();
                    invokeAction("mood_dialog");
                }

                function moodToIcon(mood)
                {
                    switch(mood)
                    {
                    case 0 : return "TS_top_mood_disabled";
                    case 1 : return "TS_top_mood_happy";
                    case 2 : return "TS_top_mood_sad";
                    case 3 : return "TS_top_mood_mad";
                    case 4 : return "TS_top_mood_bored";
                    case 5 : return "TS_top_mood_speedy";
                    case 6 : return "TS_top_mood_starving";
                    case 7 : return "TS_top_mood_sleepy";
                    case 8 : return "TS_top_mood_cool";
                    case 9 : return "TS_top_mood_inlove";
                    case 10 : return "TS_top_mood_LOL";
                    case 11 : return "TS_top_mood_peaceful2";
                    case 12 : return "TS_top_mood_singing";
                    case 13 : return "TS_top_mood_wondering";
                    case 14 : return "TS_top_mood_happy";
                    case 15 : return "TS_top_mood_sad";
                    case 16 : return "TS_top_mood_mad";
                    case 17 : return "TS_top_mood_bored";
                    case 18 : return "TS_top_mood_speedy";
                    case 19 : return "TS_top_mood_starving";
                    case 20 : return "TS_top_mood_sleepy";
                    case 21 : return "TS_top_mood_cool";
                    case 22 : return "TS_top_mood_inlove";
                    case 23 : return "TS_top_mood_LOL";
                    case 24 : return "TS_top_mood_peaceful";
                    case 25 : return "TS_top_mood_singing";
                    case 26 : return "TS_top_mood_wondering";
                    case 27 : return "TS_top_mood_bronze";
                    case 28 : return "TS_top_mood_silver";
                    case 29 : return "TS_top_mood_gold";
                    case 30 : return "TS_top_mood_busy";
                    case 31 : return "TS_top_mood_busy";
                    case 32 : return "TS_top_mood_in_a_hurry";
                    case 33 : return "TS_top_mood_in_a_hurry";
                    case 34 : return "TS_top_mood_baby";
                    }
                }
            }

            IconButton {
                id: gpsStatusButton
                width: 70
                height: 50
                icon: gpsToIcon(monitor.gpsState)

                visible: !wazeCanvas.isDialogActive

                onClicked: {
                    buttonClicked();
                    invokeAction("gps_net_stat");
                }

                function gpsToIcon(state) {
                    switch(state)
                    {
                    case 0: return "TS_top_satellite_off";
                    case 1: return "TS_top_satellite_off";
                    case 2: return "TS_top_satellite_poor";
                    case 3: return "TS_top_satellite_on";
                    }
                }
            }

            IconButton {
                id: onlineButton
                width: 70
                height: 50
                icon: netToIcon(monitor.netState)

                visible: !wazeCanvas.isDialogActive

                onClicked: {
                    buttonClicked();
                    invokeAction("gps_net_stat");
                }

                function netToIcon(state) {
                    switch(state)
                    {
                    case 0: return "TS_top_not_connected";
                    case 1: return "TS_top_connected";
                    }
                }
            }

            IconButton {
                id: profileButton
                width: 70
                height: 50
                icon: "general_settings"

                visible: !wazeCanvas.isDialogActive

                onClicked: {
                    buttonClicked();
                    invokeAction("mywaze");
                }
            }

            IconButton {
                id: optionsButton
                width: 70
                height: 50
                icon: "settings_button"

                visible: !wazeCanvas.isDialogActive

                onClicked: {
                    buttonClicked();
                    invokeAction("settingsmenu");
                }
            }
        }

        Flow {
            id: etaBar
            visible: !wazeCanvas.isDialogActive && navigationData.isNavigation
            opacity: 0.9

            anchors.right: parent.right
            anchors.left: parent.left
            anchors.top: topBar.bottom

            Button {
                text: navigationData.eta
                fontSize: 32
                needTranslation: false
                width: mainView.width/3
                height: 50
                visible: navigationData.eta != ""
            }

            Button {
                needTranslation: false
                width: mainView.width/3
                height: 50
                text: navigationData.etaTime
                fontSize: 32
                visible: navigationData.etaTime != ""
            }

            Button {
                needTranslation: false
                width: mainView.width/3
                height: 50
                text: navigationData.remainingDistance
                fontSize: 32
                visible: navigationData.remainingDistance != ""
            }
        }
    }
}
