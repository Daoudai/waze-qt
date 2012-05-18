// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: navigationPage

    ListModel {
        id: otherNavigationOptions

        ListElement {
            itemImage: "search_favorites.png"
            itemText: "My Favorites Places"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "search_history.png"
            itemText: "Last Searches"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "search_marked_locations.png"
            itemText: "Saved Places"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "search_ab.png"
            itemText: "Phonebook"
            itemValue: ""
            hasNext: true
            hasValue: false
        }
    }

    Rectangle {
        id: rectangle1
        color: "#00000000"
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        anchors.fill: parent

        TextField {
            id: search
            anchors.rightMargin: 10
            anchors.top: parent.top
            anchors.right: button1.left
            anchors.left: parent.left
        }

        Button {
            id: button1
            width: 70
            iconSource: "search_address.png"
            anchors.top: parent.top
            anchors.right: parent.right
        }

        Rectangle {
            id: rectangle2
            color: "#00000000"
            anchors.topMargin: 10
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.top: search.bottom

            ListView {
                id: otherOptionsList
                clip: true
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                model: otherNavigationOptions
                delegate: WazeListItem {
                    width: otherOptionsList.width
                    onClicked: console.log(itemText)
                }
            }
        }

    }


    tools: ToolBarLayout {
        ToolIcon {
           iconId: "toolbar-back"
           onClicked: pageStack.pop()
       }
    }
}
