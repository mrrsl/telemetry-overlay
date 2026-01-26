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

    /** Thread-UNSAFE container of CPU state. */
    std::vector<hwinfo::CPU> m_cpus;

    std::thread update_thread;

    bool exit_requested;

    unsigned long long last_cpu_measurement;

    unsigned long long last_proc_measurement;

    unsigned long long core_time_interval;

    double calculated_use;

    double calculated_proc_use;

    /** Refresh function. */
    void update();

    void updateLoop();

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
