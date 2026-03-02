import QtQuick
import QtGraphs

/**
Rolling sample history.
- current time is t = 0
- t is the negation of the time elapsed since the sample was recorded
*/
GraphsView {
    property int default_height: 150

    // maximum time to keep historical data
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
        alignment: Qt.AlignRight
    }
}
