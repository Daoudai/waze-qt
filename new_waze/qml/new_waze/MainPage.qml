import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: page1

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
            title: "Accident"
            type: "Accident"
            location: "Jerusalem"
            speed: "7kmh"
            timestamp: ""
        }

        ListElement{
            distance: "90.11km"
            reporter: "Tambal"
            title: "Chit Chat"
            type: "Chit Chat"
            location: "Dado Beach, Haifa"
            message: "Shafot on the beach"
            speed: "0kmh"
            timestamp: ""
        }
    }

    MouseArea {
        id: mouse_area1
        z: -1
        anchors.fill: parent

        Image {
            id: image1
            anchors.left: parent.left
            anchors.top: parent.top
            fillMode: Image.PreserveAspectCrop
            source: "map.png"
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

        titleText: "Exit The New Waze"
        message: "Are you sure you want to exit?"

        acceptButtonText: "Yes"
        rejectButtonText: "No"

        onAccepted: Qt.quit();
    }

    QueryDialog {
        id: aboutDialog

        titleText: "The New Waze"

        message: "This is a mockup of The New Waze (Waze Qt Port)\nDone by Assaf Paz (damagedspline@gmail.com)"

        rejectButtonText: "OK"
    }

    NavigationPage {
        id: navigationPage
    }

    ReportsPage {
        id: reportsPage
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
                iconSource: "location.png"
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
}
