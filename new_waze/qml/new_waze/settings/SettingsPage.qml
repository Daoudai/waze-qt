import QtQuick 1.1
import com.nokia.meego 1.0
import ".."

Page {
    id: settingsPage

    ListModel {
        id: settingsMenuOptions

        ListElement {
            itemIndex:0
            itemImage: "traffic.png"
            itemText: "Voice Navigation"
            itemValue: "None"
            hasNext: false
            hasValue: true
            values: [ListElement{text:"None"}, ListElement{text:"Basic"}, ListElement{text:"Full"}]
            configType: "user"
            configName: "VoiceNavigation"
        }

        ListElement {
            itemIndex:1
            itemImage: "home.png"
            itemText: "Map 2D/3D"
            itemValue: "2D"
            hasNext: false
            hasValue: true
            values: [ListElement{text:"2D"}, ListElement{text:"3D"}]
            configType: "user"
            configName: "Map"
        }

        ListElement {
            itemIndex:2
            itemImage: "home.png"
            itemText: "Day/Night View"
            itemValue: "Day"
            hasNext: false
            hasValue: true
            values: [ListElement{text:"Day"}, ListElement{text:"Night"}]
            configType: "user"
            configName: "NightMode"
        }

        ListElement {
            itemIndex:3
            itemImage: "home.png"
            itemText: "Grid Mode"
            itemValue: "Off"
            hasNext: false
            hasValue: true
            values: [ListElement{text:"Off"}, ListElement{text:"On"}]
            configType: "user"
            configName: "GridMode"
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
            itemImage: "general_settings.png"
            itemText: "General Settings"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:6
            itemImage: "map_settings.png"
            itemText: "Map"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:7
            itemImage: "group_settings.png"
            itemText: "Groups"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:8
            itemImage: "search_address.png"
            itemText: "Navigation Options"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:9
            itemImage: "download_settings.png"
            itemText: "Data Usage"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemIndex:10
            itemImage: "about.png"
            itemText: "Help/Support"
            itemValue: ""
            hasNext: true
            hasValue: false
        }
    }

    ListModel {
        id: selectionDialogModel
    }

    SelectionDialog {
        id: selectionDialog

        titleText: ""

        property string image
        property string text

        title: DialogTitle {
            image: selectionDialog.image
            text: selectionDialog.text
        }

        property variant selectedMenuItem

        model: selectionDialogModel

        onAccepted: {
            selectedMenuItem.itemValue = selectionDialogModel.get(selectionDialog.selectedIndex).text;
            config.setValue(selectedMenuItem.configType, selectedMenuItem.configName, selectedMenuItem.itemValue);
        }
    }

    BaseMenuView {
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        itemModel: settingsMenuOptions

        onItemSelected: {
            if (typeof(item.values) !== 'undefined')
            {
                selectionDialogModel.clear();
                for (var i = 0; i < item.values.count; i++)
                {
                    var value = item.values.get(i);
                    selectionDialogModel.append(value);

                    if (value.text === item.itemValue)
                    {
                        selectionDialog.selectedIndex = i;
                    }
                }
                selectionDialog.text = item.itemText;
                selectionDialog.image = item.itemImage;
                selectionDialog.selectedMenuItem = item;
                selectionDialog.open();
            }
            else
            {
                console.log(item.itemText);
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
