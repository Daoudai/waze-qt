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
            itemImage: "search_address.png"
            itemText: "Search"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:1
            itemImage: ""
            itemText: ""
            itemValue: ""
            hasNext: false
            hasValue: false
            isSpacer: true
        }

        ListElement {
            itemIndex:2
            itemImage: "home.png"
            itemText: "Home (Click to add)"
            itemValue: ""
            hasNext: false
            hasValue: false
        }

        ListElement {
            itemIndex:3
            itemImage: "work.png"
            itemText: "Work (Click to add)"
            itemValue: ""
            hasNext: false
            hasValue: false
        }

        ListElement {
            itemIndex:4
            itemImage: ""
            itemText: ""
            itemValue: ""
            hasNext: false
            hasValue: false
            isSpacer: true
        }


        ListElement {
            itemIndex:5
            itemImage: "search_favorites.png"
            itemText: "My Favorites Places"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:6
            itemImage: "search_history.png"
            itemText: "Last Searches"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:7
            itemImage: "search_marked_locations.png"
            itemText: "Saved Places"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:8
            itemImage: "search_ab.png"
            itemText: "Phonebook"
            itemValue: ""
            hasNext: true
            hasValue: false
        }
    }

    BaseMenuView {
        id: rectangle2
        anchors.fill: parent

        itemModel: otherNavigationOptions

        onItemSelected: {
            if (item.itemText === "Search")
            {
                pageStack.push(addressSearchPage);
            }
            else
            {
                console.log(item.itemText);
            }
        }
    }

    AddressSearchPage {
        id: addressSearchPage
    }

    tools: ToolBarLayout {
        ToolIcon {
           iconId: "toolbar-back"
           onClicked: pageStack.pop()
        }
    }
}
