import QtQuick 1.0

Rectangle {
    id: contactsDialog
    width: 800
    height: 480
    color: "#000000"
    property string okButtonText: "Ok"
    property string cancelButtonText: "Cancel"
    property string selectedAddress
    property string title: "title"

    property bool isRTL: typeof(__translator) === 'undefined'? false : __translator.isRTL
    property string t: ""
    function wTr(text) {
        if (typeof(__translator) === 'undefined') return text;

        return __translator.translate(text);
    }
    Connections {
        target: __translator
        onTranslationsReloaded: {
            base.tChanged();
        }
    }

    signal okPressed(string address)
    signal cancelPressed
    signal mouseAreaPressed

    function buttonPressed(buttonText)
    {
        mouseAreaPressed();
        if (buttonText == okButtonText)
        {
            okPressed(selectedAddress);
        }
        else
        {
            cancelPressed();
        }
    }

    Row {
        id: button_row
        spacing: 50
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: titleText.bottom
        anchors.topMargin: 10
        Repeater
        {
            model: 2
            Button {
                id: button
                width: (button_row.width-button_row.spacing)/2
                text: ((!isRTL && index === 0) || (isRTL && index === 1))? okButtonText : cancelButtonText
                onButtonPressed: ((!isRTL && index === 0) || (isRTL && index === 1))? okPressed(selectedAddress) : cancelPressed()
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
            clip: true
            anchors.fill: parent
            model: contactModel
            delegate: listdelegate
            currentIndex: -1
            highlightFollowsCurrentItem: false
        }
    }

    Component {
        id: listdelegate

        Rectangle {
            id: wrapper
            border.width: 2
            height: 50;
            width: contacts_list.width;

            property color topColor: "#999999";
            property color bottomColor: "#444444";

            gradient: Gradient {
                 GradientStop { position: 0.0; color: topColor }
                 GradientStop { position: 1.0; color: bottomColor }
            }

            Rectangle {
                id: selectedBackground
                anchors.fill: parent
                visible: contacts_list.currentIndex == index
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "blue" }
                    GradientStop { position: 1.0; color: "darkblue" }
                }
            }

            MouseArea {
                id: mr
                width: wrapper.width;
                height: wrapper.height;
                anchors.centerIn: parent;
                onClicked: {
                    mouseAreaPressed();
                    contacts_list.currentIndex = index;
                    selectedAddress = addressTxt.text;
                }
            }
            Column {
                Row {
                    spacing: 2
                    Item {
                        id: mainAvatar;
                        height: wrapper.height;
                        width: height;

                        Rectangle {
                            border.width: 2;
                            radius: 4;
                            anchors.fill: parent;
                            anchors.margins: 2;

                            Image {
                                id: avatar
                                anchors.fill: parent;
                                anchors.margins: 2;

                                source: contactAvatarUrl;
                                fillMode: Image.PreserveAspectFit
                                smooth:true
                            }
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter;
                                anchors.verticalCenter: parent.verticalCenter;
                                anchors.margins: 2;
                                text: "?";
                                color: "white";
                                font.pixelSize: 28
                                opacity: avatar.status == Image.Ready ? 0 : 1;
                            }
                        }
                    }

                    Column {
                        spacing: 2
                        Text {
                            id: nameTxt
                            width: wrapper.width - mainAvatar.width - 4
                            y: 8;
                            text: contactName
                            font.pixelSize: 18
                            color: "white"
                        }

                        Text {
                            id: addressTxt
                            width: nameTxt.width
                            y: 8;
                            text: contactAddress
                            font.pixelSize: 18
                            color: "white"
                        }
                    }
                }
            }

            states: [
                State {
                    name: "rtl"
                    when: isRTL

                    PropertyChanges {
                        target: nameTxt
                        horizontalAlignment: "AlignRight"
                    }

                    PropertyChanges {
                        target: addressTxt
                        horizontalAlignment: "AlignRight"
                    }
                }
            ]
        }
    }

    Text {
        id: titleText
        color: "#ffffff"
        text: t+wTr(contactsDialog.title)
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 0
        font.pixelSize: 28
    }
}
