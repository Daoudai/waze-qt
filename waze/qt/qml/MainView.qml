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

    ////// Below is mock data to test the navigation view
    /*
    Item {
        id: navigationData

        property bool isNavigation : true
        property string eta : "120 דק'"
        property string etaTime : "23:13"
        property string remainingDistance : "138 ק\"מ"
        property string currentTurnType : "nav_turn_right"
        property string currentTurnDistance : "12 ק\"מ"
        property int currentExit : 0
        property string nextTurnType : ""
        property string nextTurnDistance : ""
        property int nextExit : 0
        property string street : "רח' השפשפת"
    }
    */


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

            anchors.left: instructionsBar.visible? instructionsBar.right : parent.left
            anchors.leftMargin: 10
            anchors.top: nextStreet.bottom

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

        Flow {
            id: instructionsBar

            opacity: 0.9
            spacing: 20

            visible: navigationData.isNavigation && !wazeCanvas.isDialogActive
            anchors.left: parent.left
            anchors.top: nextStreet.bottom

            states: [
                State {
                    name: "landscape"
                    when: mainView.width >= mainView.height
                    PropertyChanges {
                        target: instructionsBar
                        width: 120
                    }
                },
                State {
                    name: "portrait"
                    when: mainView.width < mainView.height
                    PropertyChanges {
                        target: instructionsBar
                        width: 120
                    }
                }
            ]

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
            id: nextStreet
            height: 50
            opacity: 0.9

            anchors.top: parent.top
            anchors.right: exitButton.left
            anchors.rightMargin: 0
            anchors.left: minimizeButton.right
            anchors.leftMargin: 10
            visible: navigationData.isNavigation && !wazeCanvas.isDialogActive

            text: navigationData.street
            fontSize: 24
            needTranslation: false
        }

        Column {
            id: mapBar

            anchors.top: zoomBar.top
            anchors.right: parent.right
            anchors.rightMargin: 10

            spacing: 10

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
        }

        IconButton {
            id: rotateLeftButton
            width: 70
            height: 70
            anchors.top: mapBar.top
            anchors.right: mapBar.left
            anchors.rightMargin: 10
            visible: mapBar.visible
            icon: "nav_turn_left"

            onClicked: {
                buttonClicked();
                showSideToolbars();
                invokeAction("counterclockwise")
                wazeCanvas.repaint();
            }

            Image {
                source: imageProvider.getImage("north_off")
                anchors.fill: parent
                z: -1
            }
        }

        IconButton {
            id: rotateRightButton
            width: 70
            height: 70
            anchors.top: zoomBar.top
            anchors.left: zoomBar.right
            anchors.leftMargin: 10
            visible: zoomBar.visible
            icon: "nav_turn_right"
            onClicked: {
                buttonClicked();
                showSideToolbars();
                invokeAction("clockwise");
                wazeCanvas.repaint();
            }

            Image {
                source: imageProvider.getImage("north_off")
                anchors.fill: parent
                z: -1
            }
        }

        IconButton {
            id: minimizeButton
            width: 40
            height: 50
            icon: "button_sc_3_mid_s"
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 5

            visible: !wazeCanvas.isDialogActive

            onClicked: {
                buttonClicked();
                invokeAction("minimize");
            }
        }

        IconButton {
            id: showMeButton
            anchors.right: parent.right
            anchors.bottom: bottomBar.top
            anchors.bottomMargin: 70
            width: 70
            height: 70
            icon: "location"
            text: "Me_on_map"
            fitImage: true

            visible: !wazeCanvas.isDialogActive

            onClicked: {
                buttonClicked();
                invokeAction("show_me");
            }
        }

        Rectangle {
            id: bottomBar
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.bottom: parent.bottom
            height: 70
            color: 'transparent'
            visible: !wazeCanvas.isDialogActive


            IconButton {
                id: navigateButton
                width: 70
                height: parent.height
                property int originX: 0
                onOriginXChanged: {
                    navigateButton.x = navigateButton.originX;
                }
                property int targetX: bottomBar.width / 2 - navigateButton.width                
                x: originX
                z: 1
                icon: "More"
                text: "Menu"
                fitImage: false

                onXChanged: {
                    if (x === navigateButton.targetX)
                    {
                        buttonClicked();
                        if (!navigationData.isNavigation)
                        {
                            invokeAction("search_menu");
                        }
                        else
                        {
                            invokeAction("nav_menu");
                        }
                        x = navigateButton.originX;
                    }
                }

                MouseArea {
                    id: navigateDragArea

                    anchors.fill: parent
                    anchors.margins: -60


                    drag.target: navigateButton
                    drag.axis: Drag.XAxis
                    drag.minimumX: navigateButton.originX
                    drag.maximumX: navigateButton.targetX

                    onClicked: {
                        buttonClicked();
                        topBar.visible = true;
                    }

                    onReleased: {
                        navigateButton.x = navigateButton.originX;
                    }

                    states: [
                        State {
                            name: "pressed"
                            when: navigateDragArea.pressed && navigateButton.x == navigateButton.originX
                            PropertyChanges {
                                target: navigateButton
                                text: "Menu"
                                icon: "More2"
                            }
                        },
                        State {
                            name: "dragged"
                            when: navigateDragArea.pressed && navigateButton.originX < navigateButton.x && navigateButton.x < navigateButton.targetX
                            PropertyChanges {
                                target: navigateButton
                                text: !navigationData.isNavigation? "Drive_to" : "Navigate"
                                icon: "Search";
                            }
                        }
                    ]
                }
            }

            IconButton {
                id: showAlertsButton
                property int originX: bottomBar.width - showAlertsButton.width
                onOriginXChanged: {
                    showAlertsButton.x = showAlertsButton.originX;
                }
                property int targetX: bottomBar.width / 2                
                x: originX
                z: 1
                width: 70
                height: 70
                icon: "Live_event"
                text: "Events"
                fitImage: false

                Rectangle {
                    color: "red"
                    width: height
                    height: parent.height / 2 - 5
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: -20
                    anchors.right: parent.right
                    anchors.rightMargin: -10
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

                onXChanged: {
                    if (x === showAlertsButton.targetX)
                    {
                        buttonClicked();
                        invokeAction("alertsmenu");

                        x = showAlertsButton.originX;
                    }
                }

                MouseArea {
                    id: reportDragArea

                    anchors.fill: parent
                    anchors.margins: -60

                    drag.target: showAlertsButton
                    drag.axis: Drag.XAxis
                    drag.minimumX: showAlertsButton.targetX
                    drag.maximumX: showAlertsButton.originX

                    onClicked: {
                        buttonClicked();
                        invokeAction("real_time_alerts_list");
                    }

                    onReleased: {
                        showAlertsButton.x = showAlertsButton.originX;
                    }

                    states: [
                        State {
                            name: "pressed"
                            when: reportDragArea.pressed && showAlertsButton.x === showAlertsButton.originX
                            PropertyChanges {
                                target: showAlertsButton
                                icon: "Live_event2"
                                text: "Events"
                            }
                        },
                        State {
                            name: "dragged"
                            when: reportDragArea.pressed && showAlertsButton.targetX < showAlertsButton.x && showAlertsButton.x < showAlertsButton.originX
                            PropertyChanges {
                                target: showAlertsButton
                                text: "Report"
                                icon: "Report"
                            }
                        }
                    ]
                }
            }

            Button {
                id: etaBar
                property bool isEta: true

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                width: 270
                height: 60
                visible: navigationData.isNavigation

                Text {
                    color: "#ffffff"
                    anchors.right: etaSeperator.left
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    text: (etaBar.isEta)? navigationData.eta : navigationData.etaTime
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 20
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
                    font.pointSize: 20
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
        }

        Rectangle {
            id: topBar

            anchors.fill: parent

            visible: false

            Rectangle {
                color: "#000000"
                opacity: 0.700
                anchors.fill: parent

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        topBar.visible = false;
                    }
                }
            }

            Flow {
                anchors.centerIn: parent
                width: 340
                z: 1

                spacing: 10

                IconButton {
                    id: aboutButton
                    width: 140
                    height: 100
                    icon: "about"
                    text: "About"

                    onClicked: {
                        buttonClicked();
                        topBar.visible = false;
                        invokeAction("about");
                    }
                }

                IconButton {
                    id: moodsButton
                    width: 140
                    height: 100

                    icon: moodToIcon(moods.mood)
                    text: "Mood"

                    onClicked: {
                        buttonClicked();
                        topBar.visible = false;
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
                    width: 140
                    height: 100
                    icon: gpsToIcon(monitor.gpsState)
                    text: "Status"

                    onClicked: {
                        buttonClicked();
                        topBar.visible = false;
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
                    width: 140
                    height: 100
                    icon: netToIcon(monitor.netState)
                    text: "Status"

                    onClicked: {
                        buttonClicked();
                        topBar.visible = false;
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
                    width: 140
                    height: 100
                    icon: "general_settings"
                    text: "Profile"

                    onClicked: {
                        buttonClicked();
                        topBar.visible = false;
                        invokeAction("mywaze");
                    }
                }

                IconButton {
                    id: optionsButton
                    width: 140
                    height: 100
                    icon: "settings_button"
                    text: "General settings"

                    onClicked: {
                        buttonClicked();
                        topBar.visible = false;
                        invokeAction("settingsmenu");
                    }
                }

                IconButton {
                    id: navigateMenuButton
                    width: 140
                    height: 100
                    icon: "Search"
                    text: "Drive_to"

                    onClicked: {
                        buttonClicked();
                        topBar.visible = false;
                        invokeAction("search_menu");
                    }
                }
            }
        }
    }
}
