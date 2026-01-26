import QtQuick
import QtGraphs

GraphsView {
    property int default_height: 150
    property string total_usage_color: "#A1CDF1"
    property string proc_usage_color: "#8EEDF7"

    property int default_window_ms: -60 * 1000

    width: parent.width
    marginLeft: 10
    marginRight: 10

    theme: GraphsTheme {
        colorScheme: GraphsTheme.ColorSchemes.Automatic
        backgroundVisible: false
        plotAreaBackgroundVisible: false
        labelBackgroundVisible: false
    }

    axisX: ValueAxis {
        min: default_window_ms
        max: 0
        tickInterval: (default_window_ms * -1) / 4
        gridVisible: false
        subGridVisible: false
        labelsVisible: false
    }

    axisY: ValueAxis {
        min: 0
        max: 100
        tickInterval: 100
        gridVisible: false
        subGridVisible: false
    }
}
