import QtQuick
import QtGraphs

import li.morris.DataManager 1.0

Window {
    property int default_width: 300
    property int default_height: 700

    property var mem_used_history: []
    property var mem_proc_history: []

    width: default_width
    height: default_height

    visible: true
    title: qsTr("Monitor")
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    color: "transparent"

    Component.onCompleted: {
        x = screen.width - width
        y = screen.height - height
    }

    DataMan {
        id: data_manager
    }

    Text {
        id: fg_mem_title
        text: qsTr("Foreground Memory Usage (Kb)")
        anchors.top: parent.top
        horizontalAlignment: Text.AlignHCenter
        width: parent.width
    }

    MemoryUsage {
        id: fg_mem_graph
        anchors.top: fg_mem_title.bottom

        AreaSeries {
            id: fg_mem_graph_total_usage_series

            borderColor: fg_mem_graph.emerald_green
            color: fg_mem_graph.mint_green

            lowerSeries: LineSeries {
                XYPoint { x: fg_mem_graph.default_window_ms; y: 0 }
                XYPoint {x: 0; y:0 }
            }
            upperSeries: LineSeries {
                id: mem_used_line
                Connections {
                    target: data_manager

                    function onNotifyMemUsedKb() {
                        let now_ms = Date.now();

                        for (let datapoint of mem_used_history) {
                            datapoint.x = (datapoint.timestamp_ms - now_ms);
                        }

                        let oldest = mem_used_history[0];
                        if (oldest && oldest.x < fg_mem_graph.default_window_ms) {
                            mem_used_history.shift();
                        }
                        mem_used_history.push({x: 0, y: data_manager.MemUsedKb, timestamp_ms: now_ms});
                        mem_used_line.replace(mem_used_history);
                    }
                }
            }
        }
    }
}
