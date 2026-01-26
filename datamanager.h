#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <thread>

#include <QObject>
#include <QString>

#include "hwinfo/hwinfo.h"
#include "procdata.h"

/**
 * Preferred interface for accessing hardware utilization metrics.
 */
class DataManager: public QObject {
    Q_OBJECT

    static constexpr unsigned DEFAULT_INTERVAL_MS = 250;
    static constexpr unsigned MAXIMUM_HISTORIC_INTERVAL_MS = 60 * 1000;
    // with 4 billion KB capping out at ~4000 GB we should be okay
    static constexpr long long KB_DIVISOR = 0b10 << 10;

    static const QString PERCENT_POSTFIX;

    /** Interface for OS APIs. */
    ProcData dataSource;

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

    /** Refresh function. */
    void update();

    void update_loop();

public:
    Q_PROPERTY(unsigned RefreshIntervalMs READ RefreshIntervalMs)
    Q_PROPERTY(unsigned MemTotalKb READ MemTotalKb)
    Q_PROPERTY(unsigned MemUsedKb READ MemUsedKb NOTIFY notifyMemUsedKb)
    Q_PROPERTY(unsigned MemProcKb READ MemProcKb NOTIFY notifyMemProcKb)
    Q_PROPERTY(double CpuTotalUse READ CpuTotal NOTIFY notifyCpuTotal)
    Q_PROPERTY(double CpuProcUse READ CpuProcUse NOTIFY notifyCpuProcUse)

    explicit DataManager(QObject*);
    explicit DataManager();

    unsigned MemTotalKb() const;
    unsigned MemUsedKb() const;
    unsigned MemProcKb() const;

    double CpuTotal();
    double CpuProcUse();

    unsigned RefreshIntervalMs() const;

signals:
    void notifyMemUsedKb();
    void notifyMemProcKb();
    void notifyCpuTotal();
    void notifyCpuProcUse();
};

#endif // DATAMANAGER_H
