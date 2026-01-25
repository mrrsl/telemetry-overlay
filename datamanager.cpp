#include "datamanager.h"

DataManager::DataManager(QObject *parent): QObject{parent} {
    update();
    m_MemTotal = hwinfo::Memory().total_Bytes();
}

DataManager::DataManager(): DataManager(nullptr) {}

void DataManager::update() {
    m_cpus = hwinfo::getAllCPUs();

    const hwinfo::Memory mem;
    m_MemUsed = m_MemTotal - mem.available_Bytes();
    m_MemProc = dataSource.getFgProcessMemory();
}

QString DataManager::MemTotal_Qt() const {
    return QString::number(m_MemTotal);
}

QString DataManager::MemUsed_Qt() const {
    return QString::number(m_MemUsed);
}

QString DataManager::MemProc_Qt() const {
    return QString::number(m_MemProc);
}
