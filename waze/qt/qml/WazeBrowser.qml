import QtQuick 1.0
import QtWebKit 1.0

Rectangle {
    id: rectangle1
    width: 800
    height: 480

    property alias url: webView.url
    property alias html: webView.html

    property int titleHeight

    Flickable{
        id: flick
        flickableDirection: Flickable.VerticalFlick
        anchors.fill: parent
        anchors.topMargin: titleHeight

        contentWidth: Math.min(parent.width,webView.width)
        contentHeight: Math.max(parent.height,webView.height)

        clip: true

        WebView {
            id: webView

            preferredWidth: flick.width
            preferredHeight: flick.height

            onLoadStarted: {
                progressIndicator.visible = true;
            }

            onLoadFinished: {
                progressIndicator.visible = false;
            }
        }
    }

    Rectangle {
        id: progressbar
        visible: webView.progress < 1
        y: 20
        height: 50
        anchors.right: parent.right
        anchors.rightMargin: 50
        anchors.left: parent.left
        anchors.leftMargin: 50
        anchors.verticalCenter: parent.verticalCenter
        border.width: 5
        border.color: "#000000"

        Rectangle {
            id: progressIndicator
            width: webView.progress * parent.width
            height: parent.height
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#1a666d"
                }

                GradientStop {
                    position: 1
                    color: "#91ebff"
                }
            }

            border.width: 2
            border.color: "#000000"

            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 0
        }
    }
}
