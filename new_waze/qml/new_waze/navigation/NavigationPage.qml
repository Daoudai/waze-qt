// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0
import ".."

Page {
    id: navigationPage

    ListModel {
        id: otherNavigationOptions

        ListElement {
            itemImage: "home.png"
            itemText: "Home (Click to add)"
            itemValue: ""
            hasNext: false
            hasValue: false
        }

        ListElement {
            itemImage: "work.png"
            itemText: "Work (Click to add)"
            itemValue: ""
            hasNext: false
            hasValue: false
        }

        ListElement {
            itemImage: ""
            itemText: ""
            itemValue: ""
            hasNext: false
            hasValue: false
            isSpacer: true
        }


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

    ListModel {
        id: otherNavigationOptionsNoSpacers
    }

    onStatusChanged: {
        if (status === PageStatus.Activating)
        {
            otherNavigationOptionsNoSpacers.clear();

            for (var i = 0; i < otherNavigationOptions.count; i++)
            {
                var element = otherNavigationOptions.get(i);
                if (typeof(element.isSpacer) === 'undefined' || !element.isSpacer)
                {
                    otherNavigationOptionsNoSpacers.append(element);
                }
            }
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
                visible: !isGrid.checked
                clip: true
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                model: otherNavigationOptions
                delegate: WazeListItem {
                    width: otherOptionsList.width
                    onClicked: console.log(itemText)
                    visible: typeof(isSpacer) === 'undefined' || !isSpacer
                }
            }

            GridView {
                id: otherOptionsGrid
                visible: isGrid.checked

                property int __desiredRows: appWindow.inPortrait? 3 : 2
                property int __desiredCols: appWindow.inPortrait? 2 : 3
                property int __optimalWidth:otherOptionsGrid.width*__desiredRows/model.count
                property int __optimalHeight:otherOptionsGrid.height*__desiredCols/model.count

                property int __dim: __optimalWidth
                cellWidth: __optimalWidth
                cellHeight: __optimalHeight
                clip: true
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                model: otherNavigationOptionsNoSpacers
                delegate: DescriptiveButton {
                    onClicked: console.log(itemText)
                    text: qsTr(itemText)
                    iconSource: itemImage
                    isValueVisible: hasValue
                    value: itemValue
                    width: Math.min(otherOptionsGrid.__optimalHeight, otherOptionsGrid.__optimalWidth)
                    height: width
                }
            }
        }
    }


    tools: ToolBarLayout {
        ToolIcon {
           iconId: "toolbar-back"
           onClicked: pageStack.pop()
        }
        CheckBox {
           id: isGrid
        }
    }
}
