#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <thread>
#include <chrono>

#include <QObject>
#include <QString>

#include "hwinfo/hwinfo.h"
#include "procdata.h"

/**
 * Preferred interface for accessing hardware utilization metrics.
 */
class DataManager: public QObject {
    Q_OBJECT

    static constexpr int DEFAULT_INTERVAL_MS = 250;
    static constexpr int MILI_TO_MICROSEC = 1000;

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

    std::vector<hwinfo::CPU> m_cpus;

    /**
     * Thread to update CPU time readings. Note:
     *  - We don't intend for this to do very frequent sampling so no synchronization will be taking place
     */
    std::thread update_thread;

    bool exit_requested;

    /** Most recent measurement of time spent by CPU in kernel and user mode. */
    unsigned long long last_cpu_measurement;

    /**
     *  Most recent measurement of CPU time taken by the foreground process.
     *  This WILL jump around if the user frequently swaps between foreground processes.
     */
    unsigned long long last_proc_measurement;

    /**
     * Effectively the maximum amount of time the CPU can operate for the instance's update interval.
     * (ie. # of logical cores * update interval)
     */
    unsigned long long core_time_interval;

    /**
     * The most recent calculated usage %. Value will be in `[0, 1)`.
     */
    double calculated_use;

    /**
     * Most recent calculated foreground process usgae %. Value will be in `[0, 1)`.
     */
    double calculated_proc_use;

    /** Sample hardware data once. */
    void update();

    /** Contains the loop run by the update thread. */
    void updateLoop();

    /** Utility function to collect and process CPU sampling data. */
    void sampleCpuTimes();

public:
    Q_PROPERTY(unsigned RefreshIntervalMs READ RefreshIntervalMs)
    Q_PROPERTY(unsigned MemTotalKb READ MemTotalKb)
    Q_PROPERTY(unsigned MemUsedKb READ MemUsedKb NOTIFY notifyMemUsedKb)
    Q_PROPERTY(unsigned MemProcKb READ MemProcKb NOTIFY notifyMemProcKb)
    Q_PROPERTY(double CpuTotalUse READ CpuTotal NOTIFY notifyCpuTotal)
    Q_PROPERTY(double CpuProcUse READ CpuProcUse NOTIFY notifyCpuProcUse)

    explicit DataManager(QObject*);
    explicit DataManager();
    ~DataManager();

    unsigned MemTotalKb() const;
    unsigned MemUsedKb() const;
    unsigned MemProcKb() const;

    /** Return total CPU utilization. */
    double CpuTotal();
    /** CPU utilization by the current foreground process. */
    double CpuProcUse();

    unsigned RefreshIntervalMs() const;

signals:
    void notifyMemUsedKb();
    void notifyMemProcKb();
    void notifyCpuTotal();
    void notifyCpuProcUse();
};

#endif // DATAMANAGER_H
