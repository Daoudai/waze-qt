import QtQuick 1.0

Rectangle {
    id: contacts
    width: 400
    height: 400
    color: "#000000"
    radius: 26
    clip: false
    opacity: 0.850
    border.color: "#000000"

    property string okButtonText: "OK"
    property string cancelButtonText: "Cancel"

    signal okPressed(string address)
    signal cancelPressed


    Row {
        id: button_row
        height: 50
        opacity: 0.850
        spacing: 50
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20

        Rectangle {
            id: okButton
            x: 13
            width: (button_row.width-button_row.spacing)/2
            radius: 18
            gradient: Gradient {
                GradientStop {
                    position: 0.070
                    color: "#585858"
                }

                GradientStop {
                    position: 1
                    color: "#000000"
                }
            }
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 0

            MouseArea {
                id: okClickArea
                anchors.fill: parent
                onClicked: okClicked("")
                Text {
                    id: okText
                    x: -26
                    y: -58
                    color: "#ffffff"
                    text: okButtonText
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 18
                }
            }
        }

        Rectangle {
            id: cancelButton
            x: 7
            y: 8
            width: (button_row.width-button_row.spacing)/2
            radius: 18
            anchors.top: parent.top
            anchors.topMargin: 0
            MouseArea {
                id: cancelClickArea
                anchors.fill: parent
                onClicked: cancelPressed()
                Text {
                    id: cancelText
                    x: -26
                    y: -58
                    color: "#ffffff"
                    text: cancelButtonText
                    font.pixelSize: 18
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.bold: true
                }
            }
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            gradient: Gradient {
                GradientStop {
                    position: 0.070
                    color: "#585858"
                }

                GradientStop {
                    position: 1
                    color: "#000000"
                }
            }
        }
    }

    Rectangle {
        id: rectangle1
        height: 200
        color: "#00000000"
        border.color: "#ffffff"
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.top: button_row.bottom
        anchors.topMargin: 20

        ListView {
            id: contacts_list
            anchors.fill: parent
            delegate: Item {
                x: 5
                height: 40
                Row {
                    id: row1
                    spacing: 10
                    Rectangle {
                        width: 40
                        height: 40
                        color: colorCode
                    }

                    Text {
                        text: name
                        anchors.verticalCenter: parent.verticalCenter
                        font.bold: true
                    }
                }
            }
            model: ListModel {
                ListElement {
                    name: "Grey"
                    colorCode: "grey"
                }

                ListElement {
                    name: "Red"
                    colorCode: "red"
                }

                ListElement {
                    name: "Blue"
                    colorCode: "blue"
                }

                ListElement {
                    name: "Green"
                    colorCode: "green"
            }
        }
    }
    }
}
