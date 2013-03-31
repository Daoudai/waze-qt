import QtQuick 1.0

Column {
    id: mapBar

    spacing: 10

    /*
    IconButton {
        id: mapEditButton1
        width: 70
        height: 70
        icon: editor.editState === 0? "update_map_button" : "update_map_button_recording"
        visible: editor.editType === 0

        onClicked: {
            buttonClicked();
            showSideToolbars();
            invokeAction("map_updates_menu")
        }
    }

    IconButton {
        id: mapEditButton2
        width: 70
        height: 70
        icon: editor.editState === 0? "record_new_roads" : "update_map_button_recording"
        visible: editor.editType === 1

        onClicked: {
            buttonClicked();
            showSideToolbars();
            invokeAction("togglenewroads")
        }
    }  */

    Image {
        id: compassButton
        width: 120
        height: 120

        source: imageProvider.getImage(compass.compassState == 0? "controler_day_blank@2x" : "controler_compass_day@2x")

        MouseArea {
            z: 2
            anchors.fill: parent
            onClicked:  {
                buttonClicked();
                showSideToolbars();
                invokeAction("toggleorientation")
            }
        }

        Image {
            rotation: compass.orientation
            visible:   compass.compassState == 0
            anchors.centerIn: parent
            height: parent.height - 10
            source: imageProvider.getImage("Compass")
            z: 1
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
    }
}
