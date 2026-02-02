#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <thread>
#include <chrono>
#include <cstdint>

#include <QObject>
#include <QString>

#include "hwinfo/hwinfo.h"
#include "procdata.h"

// Word size check macro courtesy of alex tingle @ https://stackoverflow.com/questions/1505582/determining-32-vs-64-bit-in-c
#if (INTPTR_MAX == INT32_MAX)
    #define HANDLE_INT_T int32_t
#else
    #define HANDLE_INT_T int64_t
#endif

/**
 * Preferred interface for accessing hardware utilization metrics.
 * The class will store the last measurements recorded due to how CPU utilization needs to be calculated.
 */
class DataManager: public QObject {
    Q_OBJECT

    /* Legacy Win32 restricts process paths to 256 wide chars.*/
    static constexpr unsigned PROC_NAME_BUFFER_SIZE = 256;
    static constexpr unsigned DEFAULT_INTERVAL_MS = 250;
    static constexpr unsigned MILI_TO_MICROSEC = 1000;
    // with 4 billion KB capping out at ~4000 GB we should be okay
    static constexpr long long KB_DIVISOR = 0b10 << 10;
    static const QString PERCENT_POSTFIX;

    /** Interface for OS APIs. */
    ProcData data_source;

    /** Refresh interval. */
    unsigned m_interval;

    /** Bytes of available system memory. */
    int64_t m_MemTotal;

    /** Bytes of allocated system memory. */
    int64_t m_MemUsed;

    /** Bytes of memory used by current process. */
    int64_t m_MemProc;

    /** Thread-UNSAFE container of CPU state. */
    std::vector<hwinfo::CPU> m_cpus;

    /** Separate thread that updates measurements according to `m_interval`. */
    std::thread update_thread;

    /** Last measurement of total kernal and user time spent by the CPU. */
    unsigned long long last_cpu_measurement;

    /**
     *  Least measurement of total time scheduled by the foreground process.
     *  This measurement will jump around if the user tabs through applications in between updates.
     */
    unsigned long long last_proc_measurement;

    /** Total time available to the CPU, for purposes of calculating utilization. */
    unsigned long long core_time_interval;

    /** Total CPU utiliztion. */
    double calculated_use;

    /** Foreground CPU utilization. */
    double calculated_proc_use;

    /** Last foreground process recorded. */
    HANDLE_INT_T last_proc_handle;

    /** Refresh function. */
    void update();

    /** Loop executed by the update thread. */
    void updateLoop();

    /** Helper function to update CPU measurements. */
    void sampleCpuTimes();

    /** Checks the underlying datasource for the current handle to the foreground application. Notify if it's different from the last one. */
    void sampleProcHandle();

public:
    Q_PROPERTY(unsigned RefreshIntervalMs READ RefreshIntervalMs)
    Q_PROPERTY(unsigned MemTotalKb READ MemTotalKb)
    Q_PROPERTY(unsigned MemUsedKb READ MemUsedKb NOTIFY notifyMemUsedKb)
    Q_PROPERTY(unsigned MemProcKb READ MemProcKb NOTIFY notifyMemProcKb)
    Q_PROPERTY(double CpuTotalUse READ CpuTotal NOTIFY notifyCpuTotal)
    Q_PROPERTY(double CpuProcUse READ CpuProcUse NOTIFY notifyCpuProcUse)
    Q_PROPERTY(QString ForegroundProc READ ForegroundProc NOTIFY notifyForegroundProc)

    explicit DataManager(QObject*);
    explicit DataManager();
    ~DataManager();

    /** Return total memory available. */
    unsigned MemTotalKb() const;

    /** Return total memory used. */
    unsigned MemUsedKb() const;

    /** Return memory used by current foreground process. */
    unsigned MemProcKb() const;


    /** Return total CPU utilization. */
    double CpuTotal();

    /** CPU utilization by the current foreground process. */
    double CpuProcUse();

    /** Returns the name of the foreground process. **/
    QString ForegroundProc();

    /** Get refresh intervale of the the update loop. */
    unsigned RefreshIntervalMs() const;

signals:
    void notifyMemUsedKb();
    void notifyMemProcKb();
    void notifyCpuTotal();
    void notifyCpuProcUse();
    void notifyForegroundProc(QString);
};

#endif // DATAMANAGER_H
