// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0
import ".."

Page {
    id: navigationPage

    ListModel {
        id: otherNavigationOptions

        ListElement {
            itemIndex:0
            itemImage: "home.png"
            itemText: "Home (Click to add)"
            itemValue: ""
            hasNext: false
            hasValue: false
        }

        ListElement {
            itemIndex:1
            itemImage: "work.png"
            itemText: "Work (Click to add)"
            itemValue: ""
            hasNext: false
            hasValue: false
        }

        ListElement {
            itemIndex:2
            itemImage: ""
            itemText: ""
            itemValue: ""
            hasNext: false
            hasValue: false
            isSpacer: true
        }


        ListElement {
            itemIndex:3
            itemImage: "search_favorites.png"
            itemText: "My Favorites Places"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:4
            itemImage: "search_history.png"
            itemText: "Last Searches"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:5
            itemImage: "search_marked_locations.png"
            itemText: "Saved Places"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:6
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
            iconSource: "../search_address.png"
            anchors.top: parent.top
            anchors.right: parent.right
        }

        BaseMenuView {
            id: rectangle2
            anchors.topMargin: 10
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.top: search.bottom

            itemModel: otherNavigationOptions

            onItemSelected: {console.log(item.itemText)}

            desiredRows: appWindow.inPortrait? 3 : 2
            desiredCols: appWindow.inPortrait? 2 : 3
        }
    }


    tools: ToolBarLayout {
        ToolIcon {
           iconId: "toolbar-back"
           onClicked: pageStack.pop()
        }
    }
}
