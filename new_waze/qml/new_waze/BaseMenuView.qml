// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: baseMenuView

    width: 800
    height: 400
    color: "#00000000"
    smooth: true

    property ListModel itemModel: ListModel {}

    property ListModel __noSpacersModel: ListModel {}

    property bool isGrid: false

    property int desiredRows: 3
    property int desiredCols: 3

    signal itemSelected(variant item)

    onItemModelChanged: {
        __noSpacersModel.clear();

        for (var i = 0; i < itemModel.count; i++)
        {
            var element = itemModel.get(i);
            if (typeof(element.isSpacer) === 'undefined' || !element.isSpacer)
            {
                __noSpacersModel.append(element);
            }
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
            value: itemValue
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
        model: baseMenuView.__noSpacersModel
        delegate: DescriptiveButton {
            onClicked: baseMenuView.itemSelected(baseMenuView.__noSpacersModel.get(index))
            text: qsTr(itemText)
            iconSource: itemImage
            isValueVisible: hasValue
            value: itemValue
            width: Math.min(menuGrid.__optimalHeight, menuGrid.__optimalWidth)
            height: width
        }
    }
}
