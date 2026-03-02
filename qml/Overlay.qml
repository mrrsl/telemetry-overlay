import QtQuick
import QtGraphs
import Qt.labs.platform

Window {
    id: main_window

    property int default_width: 250

    property var mem_used_history: []
    property var mem_proc_history: []
    property var cpu_used_history: []
    property var cpu_proc_history: []

    width: default_width
    height: screen.height

    visible: true
    title: qsTr("Monitor")
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.WindowTransparentForInput

    color: "transparent"

    function turnoverArray(arr, x_negative_threshold) {
        while (arr[0] && arr[0].x < x_negative_threshold) {
            arr.shift();
        }
    }

    function updateArrayXValues(arr) {
        const current_time = Date.now();

        for (let datapoint of arr) {
            datapoint.x = datapoint.timestamp_ms - current_time;
        }
    }

    Component.onCompleted: {
        x = screen.width - width
        y = screen.height - height
    }

    Text {
        id: proc_name
        text: qsTr("Waiting..")
        width: main_window.width
        horizontalAlignment: Text.AlignHCenter
        anchors.top: main_window.top

        color: "#000000"
        style: Text.Outline
        styleColor: "#ffffff"
        font.pointSize: 18

        Connections {
            target: data_manager
            function onnotifyForegroundProc(switched_proc) {
                proc_name.text = switched_proc;
            }
        }
    }

    GraphHeading {
        id: total_mem_title
        text: qsTr("Memory Usage")
        anchors.top: proc_name.bottom
    }

    MemoryUsage {
        id: total_mem_graph
        anchors.top: total_mem_title.bottom

        AreaSeries {
            id: total_mem_series

            borderColor: total_mem_graph.emerald_green
            color: total_mem_graph.mint_green

            upperSeries: LineSeries {
                id: mem_used_line
                Connections {
                    target: data_manager

                    function onnotifyUpdatedMemory(totalAvail, totalMem, procMem) {
                        const mem_used_ref = mem_used_history;
                        updateArrayXValues(mem_used_ref);
                        turnoverArray(mem_used_ref, total_mem_graph.default_window_ms);

                        let mem_usage_percent = (totalMem / totalAvail) * 100;
                        mem_used_ref.push({x: 0, y: mem_usage_percent, timestamp_ms: Date.now()});
                        mem_used_line.replace(mem_used_ref);
                    }
                }
            }
        }
    }

    GraphHeading {
        id: fg_mem_title
        text: qsTr("Foreground Memory Usage");
        anchors.top: total_mem_graph.bottom
    }

    MemoryUsage {
        id: fg_mem
        anchors.top: fg_mem_title.bottom

        AreaSeries {
            id: fg_mem_series

            color: fg_mem.dirty_green

            upperSeries: LineSeries {
                id: fg_mem_line

                Connections {
                    target: data_manager
                    function onnotifyUpdatedMemory(total, total_used, proc_used) {
                        const mem_proc_ref = mem_proc_history;
                        updateArrayXValues(mem_proc_ref);
                        turnoverArray(mem_proc_ref, fg_mem.default_window_ms);

                        let mem_proc_percent = (proc_used / total_used) * 100;
                        mem_proc_ref.push({x: 0, y: mem_proc_percent,timestamp_ms: Date.now()});
                        fg_mem_line.replace(mem_proc_ref);
                    }
                }
            }

        }
    }

    GraphHeading {
        id: cpu_usage_title
        text: qsTr("CPU Usage")
        anchors.top: fg_mem.bottom
    }

    CpuUsage {
        id: cpu_usage
        anchors.top: cpu_usage_title.bottom

        AreaSeries {
            id: cpu_usage_series

            color: cpu_usage.total_usage_color

            upperSeries: LineSeries {
                id: cpu_usage_line

                Connections {
                    target: data_manager
                    function onnotifyUpdatedCpu(total, proc) {

                        const cpu_used_ref = cpu_used_history;
                        updateArrayXValues(cpu_used_ref);
                        turnoverArray(cpu_used_ref, cpu_usage.default_window_ms);

                        let data_entry = total * 100;
                        cpu_used_ref.push({x: 0, y: data_entry, timestamp_ms: Date.now()});
                        cpu_usage_line.replace(cpu_used_ref);
                    }
                }
            }
        }
    }

    GraphHeading {
        id: cpu_proc_title
        anchors.top: cpu_usage.bottom
        text: qsTr("Foreground CPU usage")
    }

    CpuUsage {
        id: cpu_proc
        anchors.top: cpu_proc_title.bottom

        AreaSeries {
            id: cpu_proc_series

            color: cpu_usage.proc_usage_color

            upperSeries: LineSeries {
                id: cpu_proc_line

                Connections {
                    target: data_manager
                    function onnotifyUpdatedCpu(total, proc) {

                        const cpu_proc_ref = cpu_proc_history
                        updateArrayXValues(cpu_proc_ref);
                        turnoverArray(cpu_proc_ref, cpu_proc.default_window_ms);

                        let data_entry = proc * 100;

                        if (data_entry < 0 || data_entry > 100) {
                            data_entry = 0;
                        }

                        cpu_proc_ref.push({x: 0, y: data_entry, timestamp_ms: Date.now()});
                        cpu_proc_line.replace(cpu_proc_ref);
                    }
                }
            }
        }
    }
}
