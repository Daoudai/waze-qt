// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Rectangle {
    id: itemDelegate

    height: mouse_area1.height

    signal clicked

    MouseArea {
        id: mouse_area1
        height: 50
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.leftMargin: 0

        onClicked: itemDelegate.clicked()

        Rectangle {
            id: row1
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottomMargin: 5
            anchors.topMargin: 5
            anchors.leftMargin: 5

            Rectangle {
                id: rectangle1
                width: 50
                color: "#00000000"
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                anchors.topMargin: 0

                Image {
                    id: image1
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    source: (itemImage === "")? itemImage : "image://waze/" + itemImage
            }
            }

            Text {
                id: text1
                y: 18
                text: qsTr(itemText)
                anchors.leftMargin: 5
                anchors.left: rectangle1.right
                anchors.verticalCenter: parent.verticalCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 24
            }

            Text {
                id: text2
                x: 230
                y: 0
                text: typeof(itemValue) === 'string'? qsTr(itemValue) : itemValue
                horizontalAlignment: Text.AlignRight
                anchors.rightMargin: 5
                anchors.right: image2.left
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                anchors.left: parent.left
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 20
                visible: hasValue
            }

            ToolIcon {
                id: image2
                x: 255
                y: 0
                width: 35
                height: 40
                smooth: true
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                iconId: "toolbar-next"
                visible: hasNext
            }

        }

    }
}
