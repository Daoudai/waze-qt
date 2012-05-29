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
        console.log("isGrid Changed to " + isGrid);
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
                var isSpacer = itemModel.get(j).isSpacer;
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

    property int desiredRows: 3
    property int desiredCols: 3

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

    GridView {
        id: menuGrid
        visible: baseMenuView.isGrid

        property int __optimalWidth:menuGrid.width*baseMenuView.desiredCols/model.count
        property int __optimalHeight:menuGrid.height*baseMenuView.desiredRows/model.count

        property int __dim: __optimalWidth
        cellWidth: __optimalWidth
        cellHeight: __optimalHeight
        clip: true
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: baseMenuView.itemModel
        delegate: DescriptiveButton {
            onClicked: baseMenuView.itemSelected(baseMenuView.itemModel.get(index))
            text: qsTr(itemText)
            iconSource: itemImage
            isValueVisible: hasValue
            value: itemValue
            width: Math.min(menuGrid.__optimalHeight, menuGrid.__optimalWidth)
            height: width
            visible: typeof(isSpacer) === 'undefined' || !isSpacer
        }
    }
}
