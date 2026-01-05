import QtQuick

Window {
    id: mainWindow

    width: 640
    height: 480
    visible: true
    title: qsTr("Resources")
    color: "blue"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    Component.onCompleted: {
        x = Screen.width - width
        y = 0
    }

    Text {
        text: "System Memory: " + DataManager.MemTotal
        color: "white"
        font.pointSize: 16

        Component.onCompleted: {
            x = parent.width - this.width
            y = 0
        }
    }
}
