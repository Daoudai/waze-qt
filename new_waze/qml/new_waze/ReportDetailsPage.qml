// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: reportsPage

    property ListModel reportDetails

    // define a blank tab group so we can add the pages of content later
    TabGroup {
        id: tabGroup
        anchors.fill: parent

        currentTab: regional

        // define the content for tab 1
        SpecificReportsDetailsPage {
            id: regional
        }

        // define the content for tab 2
        SpecificReportsDetailsPage {
            id: course
        }

        // define content for tab 3
        SpecificReportsDetailsPage {
            id: groups
        }
    }


    tools: ToolBarLayout {
        ToolIcon {
           iconId: "toolbar-back"
           onClicked: pageStack.pop()
        }

        ButtonRow {
            TabButton { tab: regional; text: "Region" }
            TabButton { tab: course; text: "Course" }
            TabButton { tab: groups; text: "Group" }
        }
    }
}
