import QtQuick 1.0

Column {
    id: zoomBar

    spacing: 10

    IconButton {
        id: zoominButton
        width: 120
        height: 120
        icon: "controler_zoomIn_day@2x"

        onClicked: {
            buttonClicked();
            showSideToolbars();
            invokeAction("zoomin");
            wazeCanvas.repaint();
        }
    }

    IconButton {
        id: zoomoutButton
        width: 120
        height: 120
        icon: "controler_zoomOut_day@2x"

        onClicked:  {
            buttonClicked();
            showSideToolbars();
            invokeAction("zoomout");
            wazeCanvas.repaint();
        }
    }
}
