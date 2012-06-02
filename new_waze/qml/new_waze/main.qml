import QtQuick 1.1
import com.nokia.meego 1.0
import org.waze 1.0

PageStackWindow {
    id: appWindow

    initialPage: mainPage

    showStatusBar: false

    MainPage {
        id: mainPage
    }

    Config {
        id: config
    }

    WazeImageProvider {
        id: imageProvider
    }

    function getImage(imageName)
    {
        return imageProvider.getImage(imageName);
    }
}
