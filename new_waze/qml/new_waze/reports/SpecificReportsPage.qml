// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0
import ".."

Page {

    onStatusChanged: {
        if (status === PageStatus.Activating)
        {
            policeReports.clear();
            trafficReports.clear();
            accidentsReports.clear();
            chitChatsReports.clear();
            otherReports.clear();

            for (var j=0; j<reports.count; j++)
            {
                var report = reports.get(j);
                if (report.type === "Police")
                {
                    policeReports.append(report);
                }
                else if (report.type === "Traffic")
                {
                    trafficReports.append(report);
                }
                else if (report.type === "Accidents")
                {
                    accidentsReports.append(report);
                }
                else if (report.type === "Chit Chats")
                {
                    chitChatsReports.append(report);
                }
                else if (report.type === "Other")
                {
                    otherReports.append(report);
                }
            }

            reportTypes.get(0).itemValue = reports.count;
            reportTypes.get(1).itemValue = policeReports.count;
            reportTypes.get(2).itemValue = trafficReports.count;
            reportTypes.get(3).itemValue = accidentsReports.count;
            reportTypes.get(4).itemValue = chitChatsReports.count;
            reportTypes.get(5).itemValue = otherReports.count;
        }
    }

    ListModel {
        id: reportTypes

        ListElement {
            itemImage: "report_list_all.png"
            itemText: "All"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_police.png"
            itemText: "Police"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_loads.png"
            itemText: "Traffic"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_accidents.png"
            itemText: "Accidents"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_chit_chats.png"
            itemText: "Chit Chats"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemImage: "report_list_other.png"
            itemText: "Other"
            itemValue: 0
            hasNext: true
            hasValue: true
        }
    }

    ListModel {
        id: policeReports
    }

    ListModel {
        id: trafficReports
    }

    ListModel {
        id: accidentsReports
    }

    ListModel {
        id: chitChatsReports
    }

    ListModel {
        id: otherReports
    }

    ReportDetailsPage {
        id: details
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
                onClicked: {
                    if (itemText === "All")
                    {
                        details.reportDetails = reports;
                    }
                    else if (itemText === "Police")
                    {
                        details.reportDetails = policeReports;
                    }
                    else if (itemText === "Traffic")
                    {
                        details.reportDetails = trafficReports;
                    }
                    else if (itemText === "Accidents")
                    {
                        details.reportDetails = accidentsReports;
                    }
                    else if (itemText === "Chit Chats")
                    {
                        details.reportDetails = chitChatsReports;
                    }
                    else if (itemText === "Other")
                    {
                        details.reportDetails = otherReports;
                    }
                    appWindow.pageStack.push(details);
                }
            }
        }
    }

}
