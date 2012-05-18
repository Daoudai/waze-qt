import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: page1

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

        MouseArea {
            id: quitButton
            x: 300
            y: 10
            width: image2.width
            height: image2.height
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10

            onClicked: quitDialog.open();

            Image {
                id: image2
                x: 12
                y: 12
                width: 50
                height: 50
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                source: "exit_button.png"
            }
        }

        MouseArea {
            id: settingsButton
            x: 240
            y: 10
            width: image3.width
            height: image3.height
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.right: quitButton.left
            anchors.rightMargin: 10

            Image {
                id: image3
                x: 17
                y: 15
                width: 50
                height: 50
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                source: "settings_button.png"
            }
        }

        MouseArea {
            id: aboutButton
            x: 10
            y: 10
            width: image4.width
            height: image4.height
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10

            onClicked: aboutDialog.open()

            Image {
                id: image4
                x: 17
                y: 15
                width: 50
                height: 50
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                source: "about.png"
            }
        }

        Rectangle {
            id: rectangle1
            color: "#64000000"
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

    tools: ToolBarLayout {

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
            count: "40"
            isCountVisibile: true

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
}
