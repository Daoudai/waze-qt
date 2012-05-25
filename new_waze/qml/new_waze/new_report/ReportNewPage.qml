import QtQuick 1.1
import com.nokia.meego 1.0
import ".."

Page {
    id: newReportPage

    ListModel {
        id: newReportMenuOptions

        ListElement {
            itemImage: "reportpolice.png"
            itemText: "Police"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "reporttrafficjam.png"
            itemText: "Traffic"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "reportaccident.png"
            itemText: "Accident"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "reporthazard.png"
            itemText: "Hazard"
            itemValue: ""
            hasNext: true
            hasValue: false
            isSpacer: false
        }

        ListElement {
            itemImage: "add_cam.png"
            itemText: "Camera"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "reportincident.png"
            itemText: "Chit Chat"
            itemValue: ""
            hasNext: true
            hasValue: false
        }

        ListElement {
            itemImage: "map_updates_menu.png"
            itemText: "Edit Map"
            itemValue: ""
            hasNext: true
            hasValue: false
        }
    }

    BaseMenuView {
        isGrid: isGridEnabled.checked
        clip: true
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        desiredRows: appWindow.inPortrait? 4 : 3
        desiredCols: appWindow.inPortrait? 3 : 4

        itemModel: newReportMenuOptions
        onItemSelected: console.log(item.itemText);
    }

    tools: ToolBarLayout {
            ToolIcon {
               iconId: "toolbar-back"
               onClicked: pageStack.pop()
            }
            CheckBox {
               id: isGridEnabled
        }
    }
}
