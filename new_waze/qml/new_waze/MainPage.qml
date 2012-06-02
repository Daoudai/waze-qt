import QtQuick 1.1
import com.nokia.meego 1.0
import "reports"
import "new_report"
import "navigation"
import "settings"

Page {
    id: page1

    Component.onCompleted: {
        sidebar_visibility_timer.start();
    }

    ListModel {
        id: reports

        ListElement{
            distance: "23.6km"
            reporter: "benzi"
            title: "Heavy traffic"
            type: "Traffic"
            location: "Azrieli, Tel-Aviv"
            speed: "5kmh"
            timestamp: ""
        }

        ListElement{
            distance: "42.53km"
            reporter: "bobo"
            title: "Police"
            type: "Police"
            location: "Havazelet, Netanya"
            speed: "15kmh"
            timestamp: ""
        }

        ListElement{
            distance: "60.89km"
            reporter: "baruch"
            title: "Accidents"
            type: "Accidents"
            location: "Jerusalem"
            speed: "7kmh"
            timestamp: ""
        }

        ListElement{
            distance: "90.11km"
            reporter: "Tambal"
            title: "Chit Chats"
            type: "Chit Chats"
            location: "Dado Beach, Haifa"
            message: "Shafot on the beach"
            speed: "0kmh"
            timestamp: ""
        }
    }

    function showSideToolbars()
    {
        right_side_toolbar.visible = true;
        left_side_toolbar.visible = true;
        sidebar_visibility_timer.stop();
        sidebar_visibility_timer.start();
    }

    MouseArea {
        id: mouse_area1
        z: -1
        anchors.fill: parent

        onClicked: showSideToolbars()

        Image {
            id: image1
            anchors.left: parent.left
            anchors.top: parent.top
            fillMode: Image.PreserveAspectCrop
            source: "image://waze/map.png"
        }
    }

    Rectangle {
        id: upper_toolbar
        height: 70
        color: "#00000000"
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0

        visible: appWindow.inPortrait

        Row {
            anchors.fill: parent

            property int __numberOfChildren: 3

            spacing: (mainPage.width - (70*__numberOfChildren))/(__numberOfChildren-1)

            DescriptiveButton {
                text: "About"
                iconSource: "about.png"
                onClicked: aboutDialog.open()
            }

            DescriptiveButton {
                text: "Settings"
                iconSource: "settings_button.png"
                onClicked: pageStack.push(settingsPage)
            }

            DescriptiveButton {
                text: "Quit"
                iconSource: "exit_button.png"
                onClicked: quitDialog.open()
            }
        }

        Rectangle {
            id: upper_background
            color: "#A4000000"
            z: -1
            anchors.fill: parent
        }
    }


    QueryDialog {
        id: quitDialog

        title: DialogTitle {
            image: "waze64.png"
            text: "Exit The New Waze"
        }

        message: "Are you sure you want to exit?"

        acceptButtonText: "Yes"
        rejectButtonText: "No"

        onAccepted: Qt.quit();
    }

    QueryDialog {
        id: aboutDialog

        title: DialogTitle {
            image: "about.png"
            text: "The New Waze"
        }

        message: "This is a mockup of The New Waze (Waze Qt Port)\nDone by Assaf Paz (damagedspline@gmail.com)"

        rejectButtonText: "OK"
    }

    NavigationPage {
        id: navigationPage
    }

    ReportsPage {
        id: reportsPage
    }

    ReportNewPage {
        id: newReportPage

        anchors.fill: parent
    }

    SettingsPage {
        id: settingsPage
    }

    Rectangle {
        id: lower_toolbar
        height: 70
        color: "#00000000"
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0

        Row {
            id: lowerButtonRow
            anchors.fill: parent

            property int __numberOfChildren: appWindow.inPortrait? 4 : 7

            spacing: (mainPage.width - (70*__numberOfChildren))/(__numberOfChildren-1)

            DescriptiveButton {
                text: "Show Me"
                iconSource: "On_map_anonymous.png"
            }

            DescriptiveButton {
                text: "Navigate"
                iconSource: "search_address.png"

                onClicked: pageStack.push(navigationPage)
            }

            DescriptiveButton {
                text: "Reports"
                iconSource: "map_error.png"
                value: reports.count
                isValueVisible: true

                onClicked: pageStack.push(reportsPage)
            }

            DescriptiveButton {
                text: "Report New"
                iconSource: "marked_location.png"
                onClicked: pageStack.push(newReportPage)
            }

            DescriptiveButton {
                text: "About"
                iconSource: "about.png"
                visible: !appWindow.inPortrait
                onClicked: aboutDialog.open()
            }

            DescriptiveButton {
                text: "Settings"
                iconSource: "settings_button.png"
                visible: !appWindow.inPortrait
                onClicked: pageStack.push(settingsPage)
            }

            DescriptiveButton {
                text: "Quit"
                iconSource: "exit_button.png"
                visible: !appWindow.inPortrait
                onClicked: quitDialog.open()
            }
        }

        Rectangle {
            id: lower_background
            color: "#A4000000"
            z: -1
            anchors.fill: parent
        }
    }

    Timer {
        id: sidebar_visibility_timer
        repeat: false
        running: true
        interval: 2000
        onTriggered: {
            left_side_toolbar.visible = false;
            right_side_toolbar.visible = false;
        }
    }

    Rectangle {
        id: left_side_toolbar
        width: 70
        height: page1.height/3
        color: "#00000000"
        anchors.verticalCenterOffset: -page1.height/6
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        Column {
            property int __numberOfChildren: 2

            spacing: (left_side_toolbar.height - (70*__numberOfChildren))/(__numberOfChildren-1)

            DescriptiveButton {
                text: ""
                iconSource: "rm_zoomin.png"
                onClicked: {
                    showSideToolbars();
                    console.log("Zoom in");
                }
            }

            DescriptiveButton {
                text: ""
                iconSource: "north.png"
                onClicked:  {
                    showSideToolbars();
                    console.log("Compass");
                }
            }
        }

        Rectangle {
            id: left_side_background
            color: "#A4000000"
            z: -1
            anchors.fill: parent
        }
    }

    Rectangle {
        id: right_side_toolbar
        width: 70
        height: page1.height/3
        color: "#00000000"
        anchors.verticalCenterOffset: -page1.height/6
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        Column {
            property int __numberOfChildren: 2

            spacing: (right_side_toolbar.height - (70*__numberOfChildren))/(__numberOfChildren-1)

            DescriptiveButton {
                text: ""
                iconSource: "rm_zoomout.png"
                onClicked:  {
                    showSideToolbars();
                    console.log("Zoom out");
                }
            }

            DescriptiveButton {
                text: ""
                iconSource: "record_new_roads.png"
                onClicked:  {
                    showSideToolbars();
                    console.log("Edit Map");
                }
            }
        }

        Rectangle {
            id: right_side_background
            color: "#A4000000"
            z: -1
            anchors.fill: parent
        }
    }
}
