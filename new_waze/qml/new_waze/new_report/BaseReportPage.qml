import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: policeReportPage

    property alias option1icon: view.option1icon
    property alias option2icon: view.option2icon
    property alias option3icon: view.option3icon

    BaseReportView {
        id: view
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.top: parent.top
    }

    tools: ToolBarLayout {
        ToolIcon {
           iconId: "toolbar-back"
           onClicked: pageStack.pop()
        }
    }
}

