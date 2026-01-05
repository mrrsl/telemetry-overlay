#include "procdatasource.hpp"

#include <array>

ProcData::ProcData() {
    pLocate = NULL;
    pServ = NULL;
    lastProc = 0;
    lastProcHandle = NULL;

    HRESULT hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,                   
        NULL
    );
    if (FAILED(hres)) {
        initSuccess = false;
        return;
    }

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLocate
    );
    if (FAILED(hres)) {
        initSuccess = false;
        return;
    }
    hres = pLocate->ConnectServer(
        BSTR(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pServ
    );
    if (FAILED(hres)) {
        initSuccess = false;
        return;
    }
    initSuccess = true;
}

ProcData::~ProcData() {
    if (pServ != NULL)
        pServ->Release();
    if (pLocate != NULL)
        pLocate->Release();
    if (lastProcHandle != NULL)
        CloseHandle(lastProcHandle);
}

bool ProcData::initSuccessful() const {
    return initSuccess;
}

HANDLE ProcData::getFgProcHandle() {
    HWND hForeground = GetForegroundWindow();
    DWORD procId = GetWindowThreadProcessId(hForeground, NULL);
    if (procId == 0)
        return NULL;
    if (procId == lastProc)
        return lastProcHandle;
    if (lastProcHandle != NULL)
        CloseHandle(lastProcHandle);
    lastProc = procId;
    lastProcHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, procId);
    return lastProcHandle;
}

unsigned long long ProcData::filetimeSum(FILETIME ft0, FILETIME ft1) {
    ULARGE_INTEGER a0 {0}, a1 {0};
    a0.HighPart = ft0.dwHighDateTime;
    a0.LowPart = ft0.dwLowDateTime;
    a1.HighPart = ft1.dwHighDateTime;
    a1.LowPart = ft1.dwLowDateTime;

    return a0.QuadPart + a1.QuadPart;
}

unsigned long long ProcData::getTotalProcessTime() {
    HANDLE hProc = getFgProcHandle();
    if (hProc == NULL)
        return ~0x0u;

    FILETIME scratchFt;
    FILETIME kTime;
    FILETIME uTime;
    GetProcessTimes(hProc, &scratchFt, &scratchFt, &kTime, &uTime);

    return ProcData::filetimeSum(kTime, uTime);
}

unsigned long long ProcData::getTotalCpuTime() {
    FILETIME iTime, kTime, uTime;
    GetSystemTimes(&iTime, &kTime, &uTime);

    return ProcData::filetimeSum(kTime, uTime);
}

unsigned long ProcData::getFgProcessMemory() {
    HANDLE hProc = getFgProcHandle();
    PROCESS_MEMORY_COUNTERS pc;
    if (lastProcHandle == NULL)
        return 0;
}

QString ProcData::getFgProcessName() {
    using std::array;
    constexpr unsigned long nameBufferSize = 256;

    getFgProcHandle();
    auto titleBuffer = array<WCHAR, nameBufferSize>();
    HWND fgWindow = GetForegroundWindow();
    GetWindowText(fgWindow, titleBuffer.data(), nameBufferSize);

    return QString::fromWCharArray(titleBuffer.data());
}