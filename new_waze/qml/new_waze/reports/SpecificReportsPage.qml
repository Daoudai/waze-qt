// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0
import ".."

Page {
    ListModel {
        id: reportTypes

        ListElement {
            itemIndex:0
            itemImage: "report_list_all.png"
            itemText: "All"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemIndex:1
            itemImage: "report_list_police.png"
            itemText: "Police"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemIndex:2
            itemImage: "report_list_loads.png"
            itemText: "Traffic"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemIndex:3
            itemImage: "report_list_accidents.png"
            itemText: "Accidents"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemIndex:4
            itemImage: "report_list_chit_chats.png"
            itemText: "Chit Chats"
            itemValue: 0
            hasNext: true
            hasValue: true
        }

        ListElement {
            itemIndex:5
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

    BaseMenuView {
        id: reportTypeList
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        itemModel: reportTypes
        onItemModelChanged: {
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

        onItemSelected: {
            if (item.itemText === "All")
            {
                details.reportDetails = reports;
            }
            else if (item.itemText === "Police")
            {
                details.reportDetails = policeReports;
            }
            else if (item.itemText === "Traffic")
            {
                details.reportDetails = trafficReports;
            }
            else if (item.itemText === "Accidents")
            {
                details.reportDetails = accidentsReports;
            }
            else if (item.itemText === "Chit Chats")
            {
                details.reportDetails = chitChatsReports;
            }
            else if (item.itemText === "Other")
            {
                details.reportDetails = otherReports;
            }
            appWindow.pageStack.push(details);
        }
    }
}
