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

    BaseReportPage {
        id: reportPolice
    }

    BaseReportPage {
        id: reportTraffic

        option1icon: "alert_icon_traffic_jam_light.png"
        option2icon: "alert_icon_traffic_jam_heavy.png"
        option3icon: "alert_icon_traffic_jam_standstill.png"
    }

    BaseReportPage {
        id: reportAccident

        option1icon: "alert_icon_accident_minor.png"
        option2icon: "alert_icon_accident_major.png"
    }

    BaseReportPage {
        id: reportHazard

        option1icon: "alert_icon_hazard_on_road.png"
        option2icon: "alert_icon_hazard_on_shoulder.png"
        option3icon: "alert_icon_hazard_weather.png"
    }

    BaseReportPage {
        id: reportCamera

        option1icon: "dummy_cam.png"
        option2icon: "speedcam.png"
        option3icon: "redlightcam.png"
    }

    BaseReportPage {
        id: reportChitChat
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
        onItemSelected: {
            if (item.itemText === "Police")
            {
                pageStack.push(reportPolice);
            }
            else if (item.itemText === "Traffic")
            {
                pageStack.push(reportTraffic);
            }
            else if (item.itemText === "Accident")
            {
                pageStack.push(reportAccident);
            }
            else if (item.itemText === "Hazard")
            {
                pageStack.push(reportHazard);
            }
            else if (item.itemText === "Camera")
            {
                pageStack.push(reportCamera);
            }
            else if (item.itemText === "Chit Chat")
            {
                pageStack.push(reportChitChat);
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
            CheckBox {
               id: isGridEnabled
        }
    }
}
