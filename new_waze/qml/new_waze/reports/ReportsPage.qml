// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: reportsPage

    // define a blank tab group so we can add the pages of content later
    TabGroup {
        id: tabGroup
        anchors.fill: parent

        currentTab: regional

        // define the content for tab 1
        SpecificReportsPage {
            id: regional
        }

        // define the content for tab 2
        SpecificReportsPage {
            id: course
        }

        // define content for tab 3
        SpecificReportsPage {
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

        CheckBox {
           id: isGridEnabled
        }
    }
}
