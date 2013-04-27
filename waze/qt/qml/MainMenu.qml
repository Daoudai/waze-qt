import QtQuick 1.0

Rectangle {
    id: mainMenu

    color: "transparent"

    property int flowWidth: buttonWidth * 3 + 30
    property int buttonWidth: 120

    function iconClicked(action)
    {
     	buttonClicked();
    	mainMenu.visible = false;
    	invokeAction(action);
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            mainMenu.visible = false;
        }
    }

    Rectangle {
        id: menu
        anchors.centerIn: parent
        width: flowWidth
        height: flowWidth

        Rectangle {
            radius: 20

            anchors.fill: parent

            border.color: "black"
            border.width: 10

            color: "lightblue"
        }

        Row {
            id: statusBar
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            height: gpsStatusButton.height
            spacing: 10

            IconButton {
                id: gpsStatusButton
                width: 50
                height: 50
                icon: gpsToIcon(monitor.gpsState)
                text: "Status"

                onClicked: {
                    iconClicked("gps_net_stat");
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
                width: 50
                height: 50
                icon: netToIcon(monitor.netState)
                text: "Status"

                onClicked: {
                    iconClicked("gps_net_stat");
                }

                function netToIcon(state) {
                    switch(state)
                    {
                    case 0: return "TS_top_not_connected";
                    case 1: return "TS_top_connected";
                    }
                }
            }
        }

        Rectangle {
            id: seperator

            anchors.top: statusBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 2

            border.width: 3
            color: "black"
        }

        Flickable {
            id: menuFlow
            anchors.top: seperator.bottom
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            z: 1
            clip: true

            contentWidth: menuItems.width; contentHeight: menuItems.height
            flickableDirection: Flickable.VerticalFlick

            Flow {
                id: menuItems
                spacing: 10
                width: menu.width - 10
                x: 5

                IconButton {
                    id: aboutButton
                    width: buttonWidth
                    height: 100
                    icon: "button_info"
                    text: "About"

                    onClicked: {
                        iconClicked('about');
                    }
                }

                IconButton {
                    id: moodsButton
                    width: buttonWidth
                    height: 100

                    icon: moodToIcon(moods.mood)
                    text: "Mood"

                    onClicked: {
                        iconClicked("mood_dialog");
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
                    id: profileButton
                    width: buttonWidth
                    height: 100
                    icon: "icon_menu_mywaze"
                    text: "Profile"

                    onClicked: {
                        iconClicked("mywaze");
                    }
                }

                IconButton {
                    id: optionsButton
                    width: buttonWidth
                    height: 100
                    icon: "icon_menu_settings"
                    text: "General settings"

                    onClicked: {
                        iconClicked("settingsmenu");
                    }
                }

                IconButton {
                    id: navigateMenuButton
                    width: buttonWidth
                    height: 100
                    icon: "menu_navigate"
                    text: "Drive_to"

                    onClicked: {
                        iconClicked("search_menu");
                    }
                }

                IconButton {
                    id: alternativeRoutesButton
                    width: buttonWidth
                    height: 100
                    icon: "icon_menu_routes"
                    text: "Alternative routes"
                    visible: navigationData.isNavigation

                    onClicked: {
                        iconClicked("recalc_alt_routes");
                    }
                }

                IconButton {
                    id: recalcRoutesButton
                    width: buttonWidth
                    height: 100
                    icon: "icon_small_start_point"
                    text: "Recalculate route"
                    visible: navigationData.isNavigation

                    onClicked: {
                        iconClicked("recalc_route");
                    }
                }

                IconButton {
                    id: stopNavigationButton
                    width: buttonWidth
                    height: 100
                    icon: "icon_menu_stop_nav"
                    text: "Stop navigation"
                    visible: navigationData.isNavigation

                    onClicked: {
                        iconClicked("stop_navigate");
                    }
                }

                IconButton {
                    id: minimizeButton
                    width: buttonWidth
                    height: 100
                    icon: "button_sc_3_mid_s"

                    onClicked: {
                        iconClicked("minimize");
                    }
                }

                IconButton {
                    id: exitButton
                    width: buttonWidth
                    height: 100
                    icon: "icon_menu_switchoff"

                    onClicked: {
                        iconClicked("quit");
                    }
                }
            }
        }

    }
}
