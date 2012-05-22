import QtQuick 1.1
import com.nokia.meego 1.0
import ".."

Page {
    id: settingsPage

    ListModel {
        id: settingsMenuOptions

        ListElement {
            itemImage: "traffic.png"
            itemText: "Voice Navigation"
            itemValue: "None"
            hasNext: false
            hasValue: true
            values: [ListElement{text:"None"}, ListElement{text:"Basic"}, ListElement{text:"Full"}]
        }

        ListElement {
            itemImage: "home.png"
            itemText: "Map 2D/3D"
            itemValue: "2D"
            hasNext: false
            hasValue: true
            values: [ListElement{text:"2D"}, ListElement{text:"3D"}]
        }

        ListElement {
            itemImage: "home.png"
            itemText: "Day/Night View"
            itemValue: "Night"
            hasNext: false
            hasValue: true
            values: [ListElement{text:"Day"}, ListElement{text:"Night"}]
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
            itemImage: "general_settings.png"
            itemText: "General Settings"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "map_settings.png"
            itemText: "Map"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "group_settings.png"
            itemText: "Groups"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "search_address.png"
            itemText: "Navigation Options"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "download_settings.png"
            itemText: "Data Usage"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "about.png"
            itemText: "Help/Support"
            itemValue: ""
            hasNext: true
            hasValue: false
        }
    }

    ListModel {
        id: settingsMenuOptionsNoSpacers
    }

    onStatusChanged: {
        if (status === PageStatus.Activating)
        {
            settingsMenuOptionsNoSpacers.clear();

            for (var i = 0; i < settingsMenuOptions.count; i++)
            {
                var element = settingsMenuOptions.get(i);
                if (typeof(element.isSpacer) === 'undefined' || !element.isSpacer)
                {
                    settingsMenuOptionsNoSpacers.append(element);
                }
            }
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

        property int settingsMenuIndex

        model: selectionDialogModel

        onAccepted: {
            settingsMenuOptions.get(settingsMenuIndex).itemValue = selectionDialogModel.get(selectionDialog.selectedIndex).text;
        }
    }

    ListView {
        id: settingsMenuList
        visible: !isGrid.checked
        clip: true
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: settingsMenuOptions
        delegate: WazeListItem {
            width: settingsMenuList.width
            onClicked: {
                if (typeof(values) !== 'undefined')
                {
                    selectionDialogModel.clear();
                    for (var i = 0; i < values.count; i++)
                    {
                        var value = values.get(i);
                        selectionDialogModel.append(value);

                        if (value.text === itemValue)
                        {
                            selectionDialog.selectedIndex = i;
                        }
                    }
                    selectionDialog.text = itemText;
                    selectionDialog.image = itemImage;
                    selectionDialog.settingsMenuIndex = index;
                    selectionDialog.open();
                }
                else
                {
                    console.log(itemText);
                }
            }
            visible: typeof(isSpacer) === 'undefined' || !isSpacer
        }
    }

    GridView {
        id: settingsMenuGrid
        visible: isGrid.checked

        property int __desiredRows: appWindow.inPortrait? 3 : 2
        property int __desiredCols: appWindow.inPortrait? 3 : 4
        property int __optimalWidth:settingsMenuGrid.width*__desiredRows/model.count
        property int __optimalHeight:settingsMenuGrid.height*__desiredCols/model.count

        property int __dim: __optimalWidth
        cellWidth: __optimalWidth
        cellHeight: __optimalHeight
        clip: true
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: settingsMenuOptionsNoSpacers
        delegate: DescriptiveButton {
            onClicked: {
                if (typeof(values) !== 'undefined')
                {
                    selectionDialogModel.clear();
                    for (var i = 0; i < values.count; i++)
                    {
                        var value = values.get(i);
                        selectionDialogModel.append(value);

                        if (value.text === itemValue)
                        {
                            selectionDialog.selectedIndex = i;
                        }
                    }
                    selectionDialog.text = itemText;
                    selectionDialog.image = itemImage;
                    selectionDialog.settingsMenuIndex = index;
                    selectionDialog.open();
                }
                else
                {
                    console.log(itemText);
                }
            }
            text: qsTr(itemText)
            iconSource: itemImage
            isValueVisible: hasValue
            value: itemValue
            width: Math.min(settingsMenuGrid.__optimalHeight, settingsMenuGrid.__optimalWidth)
            height: width
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
