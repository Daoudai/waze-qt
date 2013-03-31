import QtQuick 1.0

Flow {
    id: instructionsBar

    opacity: 0.9
    spacing: 20

    states: [
        State {
            name: "landscape"
            when: !isPortrait
            PropertyChanges {
                target: instructionsBar
                width: 120
            }
        },
        State {
            name: "portrait"
            when: isPortrait
            PropertyChanges {
                target: instructionsBar
                width: 120
            }
        }
    ]

    z: 1

    IconButton {
        id: currentTurn
        width: 120
        height: 120
        icon: navigationData.currentTurnType
        visible: navigationData.currentTurnType != ""
        text: navigationData.currentTurnDistance
        needTranslation: false
        fontSize: 24
        z: 1


        Text {
            text: navigationData.currentExit
            style: Text.Outline
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: currentTurn.height / 3
            visible: navigationData.currentExit > 0
            z: 1
        }

        Rectangle {
            anchors.fill: parent
            color: "darkcyan"
            radius: 20
            border.color: "black"
            border.width: 3
            z: -1
        }

        Rectangle {
            color: "blue"
            width: height
            height: parent.height / 2 - 5
            anchors.bottom: parent.top
            anchors.bottomMargin: -radius*2
            anchors.left: parent.right
            anchors.leftMargin: -radius
            radius: height / 2
            visible: nextTurn.visible
            z:2

            Text {
                text: "1"
                style: Text.Outline
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill: parent
                color: "white"
                font.pixelSize: height / 2
            }
        }
    }

    IconButton {
        id: nextTurn
        width: 120
        height: 120
        icon: navigationData.nextTurnType
        text: navigationData.nextTurnDistance
        needTranslation: false
        visible: navigationData.nextTurnType != ""
        fontSize: 24
        z: 1

        Text {
            text: navigationData.nextExit
            style: Text.Outline
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: nextTurn.height / 3
            visible: navigationData.nextExit > 0
            z: 1
        }

        Rectangle {
            anchors.fill: parent
            color: "darkcyan"
            radius: 20
            border.color: "black"
            border.width: 3
            z: -1
        }

        Rectangle {
            color: "blue"
            width: height
            height: parent.height / 2 - 5
            anchors.bottom: parent.top
            anchors.bottomMargin: -radius*2
            anchors.left: parent.right
            anchors.leftMargin: -radius
            radius: height / 2
            visible: nextTurn.visible
            z:2

            Text {
                text: "2"
                style: Text.Outline
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill: parent
                color: "white"
                font.pixelSize: height / 2
            }
        }
    }
}
