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
    
    emit notifyMemUsedKb();
    emit notifyMemProcKb();
    emit notifyCpuTotal();
    emit notifyCpuProcUse();
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

    double core_time_div = static_cast<double>(core_time_interval);
    unsigned long long total_cpu_time = data_source.getTotalCpuTime();
    unsigned long long total_proc_time = data_source.getTotalProcessTime();
    unsigned long long cpu_diff = 0;
    unsigned long long proc_diff = 0;

    if (last_cpu_measurement < 0) {
        calculated_use = 0.0;
    } else {
        cpu_diff = total_cpu_time - last_cpu_measurement;
        calculated_use = cpu_diff / core_time_div;
    }

    if (last_proc_measurement < 0) {
        calculated_proc_use = 0.0;
    } else {
        proc_diff = total_proc_time - last_proc_measurement;
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

double DataManager::CpuProcUse() {
    return calculated_proc_use;
}

double DataManager::CpuTotal() {
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
    std::string foreground_name_st_string = data_source.getFgProcessName();
    return QString::fromStdString(foreground_name_st_string);
}

void DataManager::sampleProcHandle() {

    auto handle = data_source.getFgProcHandle();
    HANDLE_INT_T handle_int = reinterpret_cast<HANDLE_INT_T>(handle);

    if (handle_int != last_proc_handle) {
        emit notifyForegroundProc(ForegroundProc());
        last_proc_handle = handle_int;
    }
}
