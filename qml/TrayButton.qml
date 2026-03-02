import QtQuick
import Qt.labs.platform

SystemTrayIcon {
    id: tray_icon
    icon.source: "qrc:/SystemIcon"

    menu: Menu {
        MenuItem {
            text: qsTr("Exit")
            onTriggered: {
                console.log("Exit triggered");
                Qt.quit();
            }
        }
    }

    Component.onCompleted: {
        visible = true;
        show();
    }
}

