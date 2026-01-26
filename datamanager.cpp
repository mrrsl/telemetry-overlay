#include "datamanager.h"
#include <chrono>

const QString DataManager::PERCENT_POSTFIX = QString::fromUtf8(" %");

DataManager::DataManager(QObject *parent):
    QObject{parent},
    dataSource{},
    update_thread{&DataManager::update_loop, this} {

    update();

    exit_requested = false;
    m_interval = DEFAULT_INTERVAL_MS;
    m_MemTotal = hwinfo::Memory().total_Bytes();
}

DataManager::DataManager(): DataManager(nullptr) {}

void DataManager::update() {

    m_cpus = hwinfo::getAllCPUs();

    const hwinfo::Memory mem;
    m_MemUsed = m_MemTotal - mem.available_Bytes();
    m_MemProc = dataSource.getFgProcessMemory();
    

    emit notifyMemUsedKb();
    emit notifyMemProcKb();
    emit notifyCpuTotal();
    emit notifyCpuProcUse();
}

void DataManager::update_loop() {

    while (!exit_requested) {
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

double DataManager::CpuProcUse() {
    unsigned long long total_time = dataSource.getTotalCpuTime();
    unsigned long long fg_proc_time = dataSource.getTotalProcessTime();
    
    double time_fraction = static_cast<double>(fg_proc_time) / total_time;


    return time_fraction;
}

double DataManager::CpuTotal() {
    auto total_utilization = m_cpus[0].currentUtilisation();
    total_utilization *= 100;
    
    return total_utilization;
}

unsigned DataManager::RefreshIntervalMs() const {
    return DataManager::m_interval;
}
