#include "procdata.h"
#include <winnt.h>

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
    DWORD procId;
    DWORD procThreadId = GetWindowThreadProcessId(hForeground, &procId);

    if (procId == 0)
        return NULL;

    if (procId == lastProc && lastProcHandle != NULL)
        return lastProcHandle;

    if (lastProcHandle != NULL)
        CloseHandle(lastProcHandle);

    lastProc = procId;
    lastProcHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, TRUE, procId);
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

unsigned long long ProcData::getFgProcessMemory() {
    HANDLE hProc = getFgProcHandle();
    PROCESS_MEMORY_COUNTERS pc;
    if (hProc == NULL)
        return 0;

    BOOL queryRes = GetProcessMemoryInfo(hProc, &pc, sizeof(pc));
    if (queryRes == TRUE)
        return pc.WorkingSetSize;
    else return 0;
}

QString ProcData::getFgProcessName() {
    using std::array;
    constexpr unsigned long nameBufferSize = 256;
    const QRegularExpression pathEndRegexp(R"([\\^].+?$)");

    getFgProcHandle();
    auto titleBuffer = array<WCHAR, nameBufferSize>();
    HWND fgWindow = GetForegroundWindow();
    GetWindowText(fgWindow, titleBuffer.data(), nameBufferSize);

    QString title = QString::fromWCharArray(titleBuffer.data());
    QRegularExpressionMatch matches = pathEndRegexp.match(title);

    if (matches.hasMatch()) {
        return matches.captured(0);
    } else {
        return QString("");
    }
}

bool ProcData::correctInstanceType(std::vector<WCHAR>::iterator beg, std::vector<WCHAR>::iterator end) {
    const std::wstring expectedEnding = std::wstring(L"engtype_3D");

    if (end - beg < 0 || end - beg < expectedEnding.size()) return false;
    auto sentinel = end - expectedEnding.size();

    std::wstring parsedEnding = std::wstring(sentinel, end);

    return expectedEnding == parsedEnding;
}

bool ProcData::instanceHasPid(DWORD pid, std::vector<WCHAR>::iterator beg, std::vector<WCHAR>::iterator end) {
    std::wstring pidString = std::to_wstring(pid);
    const std::wstring expected_prefix = std::wstring(L"pid");

    bool parsedPidPrefix = false;
    auto sentinel = beg;

    // Keep it for DRY
    auto advanceSentinel = [&sentinel]() {
        while (*sentinel != L'_') sentinel++;
    };

    // parse for "pid" first, then attempt to parse the number
    advanceSentinel();
    std::wstring parsed_prefix = std::wstring(beg, sentinel);
    if (expected_prefix != parsed_prefix) return false;
    
    beg = sentinel + 1;
    sentinel += 2;
    if (sentinel >= end) return false;
    advanceSentinel();
    std::wstring parsedPid = std::wstring(beg, sentinel);

    return pidString == parsedPid;
}

std::vector<WCHAR> ProcData::parseGpuCounterPaths(std::vector<WCHAR> &instances) {
    
    // First isolate the individual instances, then check for PID and the correct ending
    for (auto left_outer = instances.begin(); left_outer < instances.end(); left_outer++) {
        if (*left_outer == L'\0') continue;

        auto right_inner = left_outer + 1;
        while (*right_inner != L'\0') right_inner++;
        
        if ( instanceHasPid(lastProc, left_outer, right_inner) && correctInstanceType(left_outer, right_inner)) {
            return std::vector<WCHAR>(left_outer, right_inner);
        }
    }
    return std::vector<WCHAR>();
}
std::vector<WCHAR> ProcData::getFgGpuPath() {
    using std::vector;
    constexpr auto GPU_ENGINE_PREFIX = L"GPU Engine";
    constexpr auto GPU_ENGINE_FORMAT = L"\\GPU Engine\\(%s)\\Utilization Percentage";

    // Trying to avoid excessive foreground process queries
    if (lastProc == NULL) {
        return vector<WCHAR>();
    }
    // Enumeration through GPU instances first
    // PdhEnumObjectItems called twice, first to get size of string, then to allocate a string with 
    DWORD counterSz, instanceSz;
    PdhEnumObjectItemsW(
        NULL,
        NULL,
        GPU_ENGINE_PREFIX,
        NULL,
        &counterSz,
        NULL,
        &instanceSz,
        PERF_DETAIL_WIZARD,
        0
    );
    auto instanceString = vector<WCHAR>(instanceSz);
    PdhEnumObjectItemsW(
        NULL,
        NULL,
        GPU_ENGINE_PREFIX,
        NULL,
        &counterSz,
        instanceString.data(),
        &instanceSz,
        PERF_DETAIL_WIZARD,
        0
    );
    // We assume each instance will at least begin with pid_#
    return parseGpuCounterPaths(instanceString);
}

double ProcData::getFgProcessGpuUsage() {
    getFgProcHandle();
    return 0.0;
}
