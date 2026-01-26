import QtQuick
import QtGraphs

GraphsView {
    property int default_height: 150
    property int default_window_ms: -60 * 1000

    width: parent.width
    height: default_height

    marginLeft: 0
    marginRight: 5

    theme: GraphsTheme {
        colorScheme: GraphsTheme.ColorScheme.Automatic
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
