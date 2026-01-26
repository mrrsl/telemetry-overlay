import QtQuick
import QtGraphs

GraphsView {
    property int default_height: 200
    property string main_color: "#000000"
    property string sec_color: "#000020"
    property string mint_green: "#38E4AE"
    property string emerald_green: "#7BD389"
    property string celadon_green: "#AEDCC0"
    property string dirty_green: "#607466"
    property string dark_green: "#343E3D"

    property int default_window_ms: -60 * 1000
    property int plot_area_buffer : 14

    width: parent.width
    height: default_height

    marginLeft: 10
    marginRight: 10

    theme: GraphsTheme {
        colorScheme: GraphsTheme.ColorScheme.Dark
        backgroundVisible: false
        plotAreaBackgroundVisible: false
        labelBackgroundVisible: false

        seriesColors: [
            mint_green,
            emerald_green
        ]

        axisY.mainColor: main_color
        axisY.subColor: sec_color
    }

    axisX: ValueAxis {
        min: default_window_ms
        max: 0
        gridVisible: false
        subGridVisible: false
        tickInterval: Math.abs(default_window_ms) / 4
        labelsVisible: false
    }
    axisY: ValueAxis {
        min: 0
        max: 100
        gridVisible: false
        subGridVisible: false
        tickInterval: 100
    }
}
