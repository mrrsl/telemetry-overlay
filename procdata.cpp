#include "procdata.h"

#ifdef GTEST_PRESENT
#include <iostream>
#endif

HANDLE ProcData::process_heap = NULL;
PDH_COUNTER_PATH_ELEMENTS ProcData::counter_parts;
bool ProcData::initSuccess = false;

ProcData::ProcData() {
    pLocate = NULL;
    pServ = NULL;
    lastProc = 0;
    lastProcHandle = NULL;
    init_pdh_counters();

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
    process_heap = GetProcessHeap();
    if (process_heap == NULL) {
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

    if (initSuccess) {
        HeapFree(process_heap, 0, counter_parts.szObjectName);
        HeapFree(process_heap, 0, counter_parts.szInstanceName);
        HeapFree(process_heap, 0, counter_parts.szCounterName);
    }
}
// TODO: replace the HeapAlloc'd strings with data pointers to instantiated wstrings
void ProcData::init_pdh_counters() {

    if (initSuccess)
        return;

    /* Need to maintain const qualifiers so we're just dynamically allocating strings here
     * free() checklist
     * - counter_parts.szObjectName
     * - counter_parts.szInstanceName
     * - counter_parts.szCounterName
     * - gpu_counter_full_path
     */
    const size_t gpu_obj_len = (wcslen(PDH_GPU_OBJ) + 1 )  * sizeof(WCHAR);
    const size_t gpu_inst_len = (wcslen(PDH_INSTANCE_FILTER) + 1) * sizeof(WCHAR);
    const size_t gpu_count_len = (wcslen(PDH_COUNTER_NAME) + 1) * sizeof(WCHAR);

    counter_parts.szMachineName = NULL;
    counter_parts.szObjectName = (LPWSTR) HeapAlloc(process_heap, HEAP_ZERO_MEMORY, gpu_obj_len);
    counter_parts.szInstanceName = (LPWSTR) HeapAlloc(process_heap, HEAP_ZERO_MEMORY, gpu_inst_len);
    counter_parts.szParentInstance = NULL;
    counter_parts.dwInstanceIndex = 0;
    counter_parts.szCounterName = (LPWSTR) HeapAlloc(process_heap, HEAP_ZERO_MEMORY, gpu_count_len);

    std::memcpy((void*) PDH_GPU_OBJ, counter_parts.szObjectName, gpu_obj_len);
    std::memcpy((void*) PDH_INSTANCE_FILTER, counter_parts.szInstanceName, gpu_inst_len);
    std::memcpy((void*) PDH_COUNTER_NAME, counter_parts.szCounterName, gpu_count_len);

    DWORD full_name_size = 0;
    PdhMakeCounterPath(&counter_parts, NULL, &full_name_size, 0);
    pdh_wildcard_path = std::vector<WCHAR>(full_name_size);
    PdhMakeCounterPath(&counter_parts, pdh_wildcard_path.data(), &full_name_size, 0);
}


bool ProcData::initSuccessful() const {
    return initSuccess;
}

HANDLE ProcData::getFgProcHandle() {
    HWND hForeground = GetForegroundWindow();
    DWORD procId;
    GetWindowThreadProcessId(hForeground, &procId);

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

std::string ProcData::getLastPathItem(LPWSTR path, DWORD size) {
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
    DWORD wchar_conversion_status = WideCharToMultiByte(
        CP_UTF8,
        0,
        (LPCWCH) end_sentinel,
        -1,
        NULL,
        0,
        NULL,
        NULL
    );

    auto last_item = std::string(static_cast<size_t>(wchar_conversion_status), '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        (LPCWCH) end_sentinel,
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

    DWORD written_size = max_buffer_size;
    WCHAR titleBuffer[max_buffer_size];
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
bool check_path_pid(DWORD pid, LPWSTR begin, LPWSTR end) {
    // we're relying on WCHAR being 2 bytes in size so the full 4 WCHAR prefix should fit into a 64 bit type
    const auto pid_marker = L"pid_";
    const int64_t equality_mask = (int64_t) *pid_marker;
    const size_t end_margin = sizeof(int64_t) / sizeof(WCHAR) - 1;

    std::wstring pid_string = std::to_wstring(pid);
    std::wstring pid_match;
    LPWSTR number_end;

    while (begin + end_margin < end) {
        int64_t chunk = (int64_t) *begin;
        if (chunk == equality_mask) {
            begin = begin + end_margin + 1;
            break;
        }
        begin++;
    }

    number_end = begin + 1;

    while (number_end < end && *number_end >= L'0' && *number_end <= L'9') {
        number_end++;
    }

    pid_match = std::wstring(begin, number_end);

    return pid_string == pid_match;
}

std::vector<WCHAR> ProcData::parseGpuCounterPaths(DWORD pid, LPWSTR path_list, DWORD list_size) {
    
    LPWSTR start, end;
    std::vector<WCHAR> result;

    start = path_list;
    end = path_list + 1;

    while (end - start < list_size) {

        while (*end != L'\0')
            end++;

        // Check that we can safely look ahead for double null
        if (end - path_list + 1 >= list_size)
            return result;

        if (check_path_pid(pid, start, end)) {
            size_t num_wchars = (end - start) * sizeof(WCHAR);
            result = std::vector<WCHAR>(num_wchars);
            std::copy(start, end, result.begin());
            break;
        }
    }

    return result;
}

std::vector<WCHAR> ProcData::getFgGpuPath(HANDLE p_handle, LPWSTR path) {
    using std::vector;
    if (p_handle == NULL)
        return std::vector<WCHAR>();

    DWORD pid = GetProcessId(p_handle);
    if (pid == 0)
        return std::vector<WCHAR>();

    DWORD list_length = 0;
    PdhExpandWildCardPath(
        NULL,
        path,
        NULL,
        &list_length,
        0
    );
    LPWSTR path_list = (LPWSTR) HeapAlloc(process_heap, HEAP_ZERO_MEMORY, list_length * sizeof(WCHAR));
    PdhExpandWildCardPath(
        NULL,
        path,
        path_list,
        &list_length,
        0
    );

    return parseGpuCounterPaths(pid, path_list, list_length);

}

double ProcData::getFgProcessGpuUsage() {

    typedef std::vector<WCHAR> wch_vec;

    getFgProcHandle();
    wch_vec gpu_fg_path = getFgGpuPath(lastProcHandle, pdh_wildcard_path.data());

#ifdef GTEST_PRESENT
    std::wcerr << std::wstring(gpuPath.begin(), gpuPath.end()) << std::endl;
#endif


    return 0.0;
}
