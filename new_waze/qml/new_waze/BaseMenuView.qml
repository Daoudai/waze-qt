// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: baseMenuView

    width: 800
    height: 400
    color: "#00000000"
    smooth: true

    property ListModel itemModel: ListModel {}
    onItemModelChanged: {
        isGridChanged();
    }

    property bool isGrid: config.getValue("user", "GridMode", "Off") === "On"
    onIsGridChanged: {
        if (isGrid) {
            privateFunctions.filterForGrid();
        } else {
            privateFunctions.unfilterForList();
        }
    }

    Component.onCompleted: {
        if (isGrid) {
            privateFunctions.filterForGrid();
        }
    }

    Item {
        id: privateFunctions

        function filterForGrid()
        {
            for (var j=0,i=0; j<itemModel.count; i++, j++)
            {
                var isSpacer = itemModel.get(i).isSpacer;
                if (typeof(isSpacer) !== 'undefined' && isSpacer)
                {
                    itemModel.move(i--,itemModel.count - 1,1);
                }
            }
        }

        function unfilterForList()
        {
            for (var i=0; i<itemModel.count; i++)
            {
                itemModel.move(i,itemModel.get(i).itemIndex,1);
            }
        }
    }

    Connections {
        target: config
        onConfigChanged: {
            if (type === "user" && name === "GridMode")
            {
                baseMenuView.isGrid = (newValue === "On");
            }
        }
    }

    signal itemSelected(variant item)

    onItemSelected: {
        if (typeof(item.configName) !== 'undefined' && typeof(item.configType) !== 'undefined')
        {
            config.setValue(item.configType, item.configName, item.itemValue);
        }
    }

    ListView {
        id: menuList
        visible: !baseMenuView.isGrid
        clip: true
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        model: baseMenuView.itemModel
        delegate: WazeListItem {
            text: itemText
            value: (typeof(configName) === 'undefined' || typeof(configType) === 'undefined')? itemValue : config.getValue(configType, configName, itemValue)
            iconSource: itemImage
            isNextIndicatorVisible: hasNext
            isValueVisible: hasValue
            width: menuList.width
            onClicked: baseMenuView.itemSelected(baseMenuView.itemModel.get(index))
            visible: typeof(isSpacer) === 'undefined' || !isSpacer
        }
    }

    DescriptiveButton {
        id: prevButton

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        visible: baseMenuView.isGrid && menuGrid.currentIndex >= 4

        onClicked: {
            if (menuGrid.currentIndex > 8)
            {
                menuGrid.currentIndex -= 8;
            }
            else
            {
                menuGrid.currentIndex = 0;
            }
        }

        text: qsTr("Prev")
        iconSource: "left_side.png"
    }

    GridView {
        id: menuGrid
        visible: baseMenuView.isGrid
        cellWidth: -5 + (menuGrid.width > menuGrid.height? menuGrid.width / 4 : menuGrid.width / 2)
        cellHeight: -5 + (menuGrid.width > menuGrid.height? menuGrid.height / 2 : menuGrid.height / 4)
        clip: true
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: prevButton.right
        anchors.right: nextButton.left
        anchors.topMargin: 15
        anchors.bottomMargin: 15
        model: baseMenuView.itemModel
        snapMode: GridView.SnapOneRow
        flickDeceleration: 500
        highlightFollowsCurrentItem: true
        highlightRangeMode: GridView.StrictlyEnforceRange
        preferredHighlightBegin: 0; preferredHighlightEnd: 0
        cacheBuffer: width;
        currentIndex: 0
        delegate: DescriptiveButton {
            onClicked: baseMenuView.itemSelected(baseMenuView.itemModel.get(index))
            text: qsTr(itemText)
            iconSource: itemImage
            isValueVisible: hasValue
            value: itemValue
            width: menuGrid.cellWidth - 5
            height: menuGrid.cellHeight - 5
            color: "blue"
            visible: typeof(isSpacer) === 'undefined' || !isSpacer
        }
    }

    DescriptiveButton {
        id: nextButton

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        visible: baseMenuView.isGrid && menuGrid.currentIndex < menuGrid.count - 8

        onClicked: {
            if (menuGrid.currentIndex < menuGrid.count - 8)
            {
                menuGrid.currentIndex += 8;
            }
            else
            {
                menuGrid.currentIndex += (menuGrid.count - menuGrid.currentIndex) % 8;
            }
        }

        text: qsTr("Next")
        iconSource: "right_side.png"
    }
}
