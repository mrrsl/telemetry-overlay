#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QString>

#include "hwinfo/hwinfo.h"

#include "procdata.h"

class DataManager: public QObject {
    Q_OBJECT

    /** Default refresh interval for telemetry. */
    static constexpr unsigned DEFAULT_INTERVAL_MS = 250;

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

    /** Refresh function. */
    void update();

public:
    Q_PROPERTY(QString MemTotal READ MemTotal_Qt)
    Q_PROPERTY(QString MemUsed READ MemUsed_Qt NOTIFY notifyMemUsed)
    Q_PROPERTY(QString MemProc READ MemProc_Qt NOTIFY notifyMemProc)

    explicit DataManager(QObject*);
    explicit DataManager();

    QString MemTotal_Qt() const;
    QString MemUsed_Qt() const;
    QString MemProc_Qt() const;

signals:
    void notifyMemUsed(int updatedMemUsed);
    void notifyMemProc(int updatedMemProc);
};

#endif // DATAMANAGER_H
