#include "procdata.h"

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
        BSTR(WMI_RESOURCE_NAME),
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

long long ProcData::filetimeDiff(FILETIME ft0, FILETIME ft1) {
    ULARGE_INTEGER a0, a1;
    a0.HighPart = ft0.dwHighDateTime;
    a0.LowPart = ft0.dwLowDateTime;
    a1.HighPart = ft1.dwHighDateTime;
    a1.LowPart = ft1.dwLowDateTime;

    return a0.QuadPart - a1.QuadPart;
}

std::wstring ProcData::getLastPathItem(LPWSTR path, DWORD size) {
    /*
     * This function makes the following assumptions:
     * - path is null-terminated
     * - path uses the win32 format
     * - LPWSTR and CHAR resolve to wchar_t* and wchar_t and thus can be used with std::wstring
     */
    LPWSTR end_sentinel = path + size;
    LPWSTR start_sentinel = end_sentinel - 1;
    bool found_item = false;


    while (start_sentinel > path && !found_item) {
        WCHAR ch = *start_sentinel;
        if (ch == L'\\') {
            found_item = true;
        }
    }
    int wchar_conversion_status = WideCharToMultiByte(
        CP_UTF8,
        0,
        end_sentinel,
        -1,
        NULL,
        0,
        NULL,
        NULl
    );

    std::string last_item{wchar_conversion_status, '\0'};

    WideCharToMultiByte(
        CP_UTF8,
        0,
        end_sentinel,
        -1,
        last_item.data(),
        wchar_conversion_status,
        NULL,
        NULL
    );

    return last_item;
}

unsigned long long ProcData::getTotalProcessTime() {
    HANDLE hProc = getFgProcHandle();
    if (hProc == NULL)
        return ~0x0u;

    FILETIME scratch_time;
    FILETIME kernel_time;
    FILETIME user_time;
    GetProcessTimes(hProc, &scratch_time, &scratch_time, &kernel_time, &user_time);

    unsigned long long time_spent = ProcData::filetimeSum(kernel_time, user_time);

    return time_spent / MICROSEC_TO_FILETIME;
}

unsigned long long ProcData::getTotalCpuTime() {
    FILETIME i_file_time, k_file_time, u_file_time;
    GetSystemTimes(&i_file_time, &k_file_time, &u_file_time);

    ULARGE_INTEGER itime_int;
    itime_int.HighPart = i_file_time.dwHighDateTime;
    itime_int.LowPart = i_file_time.dwLowDateTime;

    // Remember this is number of 100ns intervals
    unsigned long long idle_in_kernel = itime_int.QuadPart;
    unsigned long long time_spent = ProcData::filetimeSum(k_file_time, u_file_time);

    return (time_spent - idle_in_kernel) / MICROSEC_TO_FILETIME;
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

std::string ProcData::getFgProcessName() {
    // QueryFullProcessImageName does not let us peek at the length of the process name before hand
    constexpr unsigned max_buffer_size = 256;
    HANDLE fgHandle = getFgProcHandle();

    if (fgHandle == NULL)
        return std::string("");

    DWORD written_size = nameBufferSize;
    WCHAR titleBuffer[nameBufferSize];
    QueryFullProcessImageName(fgHandle, 0, titleBuffer, &written_size);
    std::string process_name = getLastPathItem(titleBuffer, written_size);

    return process_name;
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
    std::vector<WCHAR> gpuPath = getFgGpuPath();
    return 0.0;
}
