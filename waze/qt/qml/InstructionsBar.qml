import QtQuick 1.0

Flow {
    id: instructionsBar

    opacity: 0.9

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
            border.color: "black"
            border.width: 3
            z: -1
        }
    }

    IconButton {
        id: nextTurn
        width: 80
        height: 80
        icon: navigationData.nextTurnType
        text: navigationData.nextTurnDistance
        needTranslation: false
        visible: navigationData.nextTurnType != ""
        fontSize: 18
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
            border.color: "black"
            border.width: 3
            z: -1
        }
    }
}
