import QtQuick 1.0

Rectangle {
    id: searchView
    width: 800
    height: 480
    color: "#000000"

    Flow {
        id: flow1
        width: searchView.width - tilebutton2.width - spacing/2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.top: titleText.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 20

        TileButton {
            id: tilebutton1
            width: flow1.width
            icon: ""
            description: "Search place or address"
        }

        TileButton {
            id: tilebutton2
            x: 232
            y: 76
            description: "Home (Touch to add)"
        }

        TileButton {
            id: tilebutton3
            x: 388
            y: 52
            description: "Work (Touch to add)"
        }

        TileButton {
            id: tilebutton4
            x: 538
            y: 24
            description: "Favorites"
        }

        TileButton {
            id: tilebutton5
            x: 686
            y: 22
            description: "Last Searches"
        }

        TileButton {
            id: tilebutton6
            x: 232
            y: 214
            description: "Saved Locations"
        }

        TileButton {
            id: tilebutton7
            x: 416
            y: 198
            description: "Address book"
        }
    }

    Text {
        id: titleText
        x: 216
        color: "#ffffff"
        text: qsTr("Navigate")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.bold: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10
        font.pixelSize: 32
    }
}
