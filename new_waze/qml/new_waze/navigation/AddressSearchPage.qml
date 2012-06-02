import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: addressSearchPage

    Column {
        anchors.fill: parent
        anchors.topMargin: 30

        spacing: 30

        TextField {
            id: search
            x: 10
            width: addressSearchPage.width - x*2
        }

        Button {
            text: qsTr("Search")
            x: 10
            width: addressSearchPage.width - x*2
            onClicked: console.log("Searching for <" + search.text + "> ...")
        }
    }

    tools: ToolBarLayout {
        ToolIcon {
           iconId: "toolbar-back"
           onClicked: pageStack.pop()
        }
    }
}
