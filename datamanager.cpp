#include "datamanager.h"

const QString DataManager::PERCENT_POSTFIX = QString::fromUtf8(" %");

DataManager::DataManager(QObject *parent):
    QObject{parent},
    data_source{}
{
    m_interval = DEFAULT_INTERVAL_MS;
    m_cpus = hwinfo::getAllCPUs();
    m_MemTotal = hwinfo::Memory().total_Bytes();

    last_cpu_measurement = -1;
    last_proc_measurement = -1;
    calculated_use = 0.0;
    calculated_proc_use = 0.0;

#ifdef WIN32
    last_pid = 0;
#else
    last_pid = -1;
#endif

    core_time_interval = m_interval *
        MILI_TO_MICROSEC *
        m_cpus[0].numLogicalCores();


    update();
    update_thread = std::thread(&DataManager::updateLoop, this);
}

DataManager::DataManager(): DataManager(nullptr) {}

void DataManager::update() {

    const hwinfo::Memory mem;
    m_MemUsed = m_MemTotal - mem.available_Bytes();
    m_MemProc = data_source.getFgProcessMemory();

    sampleCpuTimes();
    sampleProcHandle();
    
    quint64 total = m_MemTotal,
            total_used = m_MemUsed,
            proc_used = m_MemProc;

    emit notifyUpdatedMemory(total, total_used, proc_used);
    emit notifyUpdatedCpu(CpuTotal(), CpuProcUse());

}

DataManager::~DataManager() {
    update_thread.detach();
    update_thread.~thread();
    data_source.~ProcData();
}

void DataManager::updateLoop() {

    while (true) {
        update();
        std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
    }
}

unsigned DataManager::MemTotalKb() const {
    return m_MemTotal / DataManager::KB_DIVISOR;
}

unsigned DataManager::MemUsedKb() const {
    return m_MemUsed / DataManager::KB_DIVISOR;
}

unsigned DataManager::MemProcKb() const {
    return m_MemProc / DataManager::KB_DIVISOR;
}

void DataManager::sampleCpuTimes() {
    typedef unsigned long long ull;

    double core_time_div = static_cast<double>(core_time_interval);

    // Snapshots to minimize the effect of a potential race condition
    ull last_cpu_measurement_snapshot = last_cpu_measurement;
    ull last_cpu_proc_snapshot = last_proc_measurement;

    ull total_cpu_time = data_source.getTotalCpuTime();
    ull total_proc_time = data_source.getTotalProcessTime();

    ull cpu_diff = 0;
    ull proc_diff = 0;

    if (last_cpu_measurement_snapshot < 0) {
        calculated_use = 0.0;
    } else {
        cpu_diff = total_cpu_time - last_cpu_measurement_snapshot;
        calculated_use = cpu_diff / core_time_div;
    }

    if (last_cpu_proc_snapshot < 0) {
        calculated_proc_use = 0.0;
    } else {
        proc_diff = total_proc_time - last_cpu_proc_snapshot;
        // Account for changing foreground process;
        if (proc_diff < 0)
            proc_diff = 0;
        calculated_proc_use = proc_diff / static_cast<double>(cpu_diff);

        if (calculated_proc_use > 1.0)
            calculated_proc_use = 1.0;
    }

    last_cpu_measurement = total_cpu_time;
    last_proc_measurement = total_proc_time;
}

qreal DataManager::CpuProcUse() {
    return calculated_proc_use;
}

qreal DataManager::CpuTotal() {
    return calculated_use;
}

unsigned DataManager::RefreshIntervalMs() const {
    return DataManager::m_interval;
}

bool ProcData::procHandleValid(HANDLE procHandle) {
    DWORD handleStatus = WaitForSingleObject(procHandle, 0);
    return handleStatus == WAIT_TIMEOUT;
}

QString DataManager::ForegroundProc() {
    std::wstring foreground_name_st_string = data_source.getFgProcessName();
    return QString::fromStdWString(foreground_name_st_string);
}

void DataManager::sampleProcHandle() {

    // Sidestepping the need to use Win32 types by doing this scuffed comparison
    auto pid = data_source.getFgProcId();

    if (pid != last_pid) {

        last_pid = pid;
        emit notifyForegroundProc(this->ForegroundProc());
    }
}
