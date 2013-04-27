import QtQuick 1.0
import org.waze 1.0

Item {
    id: base
    width: 800
    height: 480

    signal invokeAction(string action)
    signal buttonClicked()

    property bool isPortrait: deviceOrientation == 3 || deviceOrientation == 4

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
        property string currentTurnType : "big_directions_roundabout"
        property string currentTurnDistance : "12 ק\"מ"
        property int currentExit : 2
        property string nextTurnType : "big_directions_roundabout"
        property string nextTurnDistance : "30 מ'"
        property int nextExit : 3
        property string street : "רח' השפשפת"
    }
    */

	
      Item {
          id: message
          property string message: (typeof(__message) === 'undefined')? "" : __message.text
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

        ZoomBar {
            id: zoomBar

            anchors.left: (isPortrait || !instructionsBar.visible)? parent.left : instructionsBar.right
            anchors.top: isPortrait? instructionsBar.bottom : nextStreet.bottom
        }

        InstructionsBar {
            id: instructionsBar

            visible: navigationData.isNavigation && !wazeCanvas.isDialogActive
            anchors.left: parent.left
            anchors.top: isPortrait? nextStreet.bottom : parent.top
        }
        
        Rectangle {
            id: messageBar
            
            anchors.top: parent.top
            anchors.left: !isPortrait && instructionsBar.visible? instructionsBar.right : parent.left 
            anchors.right: parent.right
            height: 32 
            color: 'black'
            visible: messageText.text !== '' && !wazeCanvas.isDialogActive

            Text {
                id: messageText
                text: message.message
                onTextChanged: {
                    if (text != '')
                    {
                        messageResetTimer.restart();
                    }
                }

                font.pixelSize:28
                font.bold: true
                anchors.centerIn: parent
                color: 'white'
            }

            Timer {
                id: messageResetTimer
                interval: 5000
                repeat: false

                onTriggered: {
                    messageText.text = '';
                }
            }
        }

        Button {
            id: nextStreet
            opacity: 0.9

            radius: 10

            anchors.top: messageBar.visible? messageBar.bottom : parent.top
            anchors.right: parent.right
            anchors.left: !isPortrait && instructionsBar.visible? instructionsBar.right : parent.left 
            visible: navigationData.isNavigation && !wazeCanvas.isDialogActive
            wrapText: true

            text: '<a style="color:lightgreen">' + navigationData.currentTurnDistance + '</a> ' + navigationData.street
            fontSize: 48
            needTranslation: false
        }

        MapBar {
            id: mapBar

            anchors.top: nextStreet.bottom
            anchors.right: parent.right
        }

        IconButton {
            id: rotateLeftButton
            width: 120
            height: 120
            anchors.top: rotateRightButton.top
            anchors.right: isPortrait? parent.right : mapBar.left
            visible: mapBar.visible
            icon: "nav_turn_left"

            onClicked: {
                buttonClicked();
                showSideToolbars();
                invokeAction("counterclockwise")
                wazeCanvas.repaint();
            }

            Image {
                source: imageProvider.getImage("controler_day_blank@2x")
                anchors.fill: parent
                z: -1
            }
        }

        IconButton {
            id: rotateRightButton
            width: 120
            height: 120
            anchors.top: isPortrait? zoomBar.bottom : zoomBar.top
            anchors.left: isPortrait? parent.left : zoomBar.right
            visible: zoomBar.visible
            icon: "nav_turn_right"
            onClicked: {
                buttonClicked();
                showSideToolbars();
                invokeAction("clockwise");
                wazeCanvas.repaint();
            }

            Image {
                source: imageProvider.getImage("controler_day_blank@2x")
                anchors.fill: parent
                z: -1
            }
        }

        IconButton {
            id: showMeButton
            anchors.right: parent.right
            anchors.bottom: bottomBar.top
            anchors.bottomMargin: 5
            width: 120
            height: 120
            icon: "controler_meOnMap_day@2x"
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
            height: 120
            color: 'transparent'
            visible: !wazeCanvas.isDialogActive


            IconButton {
                id: navigateButton
                width: 120
                height: parent.height
                property int originX: 0
                onOriginXChanged: {
                    navigateButton.x = navigateButton.originX;
                }
                property int targetX: bottomBar.width / 2 - navigateButton.width                
                x: originX
                z: 1
                icon: "icon_menu"
                text: "Menu"
                fitImage: false

                onXChanged: {
                    if (x === navigateButton.targetX)
                    {
                        buttonClicked();
                        invokeAction("search_menu");
                        x = navigateButton.originX;
                    }
                }

                MouseArea {
                    id: navigateDragArea

                    anchors.fill: parent
                    anchors.margins: -10


                    drag.target: navigateButton
                    drag.axis: Drag.XAxis
                    drag.minimumX: navigateButton.originX
                    drag.maximumX: navigateButton.targetX

                    onClicked: {
                        buttonClicked();
                        mainMenu.visible = true;
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
                                icon: "icon_menu_press"
                            }
                        },
                        State {
                            name: "dragged"
                            when: navigateDragArea.pressed && navigateButton.originX < navigateButton.x && navigateButton.x < navigateButton.targetX
                            PropertyChanges {
                                target: navigateButton
                                text: !navigationData.isNavigation? "Drive_to" : "Navigate"
                                //icon: "Search";
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
                width: 120
                height: 120
                icon: "icon_report"
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
                    anchors.margins: -10

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
                                icon: "icon_report_press"
                                text: "Events"
                            }
                        },
                        State {
                            name: "dragged"
                            when: reportDragArea.pressed && showAlertsButton.targetX < showAlertsButton.x && showAlertsButton.x < showAlertsButton.originX
                            PropertyChanges {
                                target: showAlertsButton
                                text: "Report"
                                //icon: "Report"
                            }
                        }
                    ]
                }
            }

            LandscapeEtaBar {
                id: landscapeEtaBar

                visible: !isPortrait && navigationData.isNavigation
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                width: parent.width - navigateButton.width - showAlertsButton.width - 10
            }

            PortraitEtaBar {
                id: portraitEtaBar

                visible: isPortrait && navigationData.isNavigation
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                width: parent.width - navigateButton.width - showAlertsButton.width - 10
            }
        }

        MainMenu {
            id: mainMenu

            anchors.fill: parent
            visible: false

            z: 3
        }
    }
}
