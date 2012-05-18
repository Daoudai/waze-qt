// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Page {

    property alias reportModel: reportTypes

    ListModel {
        id: reportTypes

        ListElement {
            itemImage: "report_list_all.png"
            itemText: "All"
            itemValue: "0"
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_police.png"
            itemText: "Police"
            itemValue: "0"
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_loads.png"
            itemText: "Traffic"
            itemValue: "0"
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_accidents.png"
            itemText: "Accidents"
            itemValue: "0"
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_chit_chats.png"
            itemText: "Chit Chats"
            itemValue: "0"
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_other.png"
            itemText: "Other"
            itemValue: "0"
            hasNext: true
            hasValue: true
        }
    }

    Rectangle {
        id: rectangle2
        color: "#00000000"
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.top: parent.top

        ListView {
            id: reportTypeList
            clip: true
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            model: reportTypes
            delegate: WazeListItem {
                width: reportTypeList.width
                onClicked: console.log(itemText)
            }
        }
    }

}
