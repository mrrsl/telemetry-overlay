#ifndef PROCDATA_H
#define PROCDATA_H

#define _WIN32_DCOM

#include <windows.h>
#include <wbemidl.h>
#include <Psapi.h>
#include <WinBase.h>
#include <Pdh.h>
#include <winnt.h>

#include <string>
#include <vector>

#pragma comment(lib, "wbemuuid.lib")

/**
 * Interface for interacting with the OS API to fetch process-specific information. Where possible, any implementation code should avoid
 * using anything not defined in the Win32 API or the standard library.
 */
class ProcData {

    /** Reminder: the full quad-word stored in FILETIME represents the number of 100-nanosecond units. */
    static constexpr int MICROSEC_TO_FILETIME =  10;

    /** Size of the buffer that stores the PDH path to the GPU instance for the foreground application. */
    static constexpr size_t PATH_BUFFER_SIZE = 512;

    /* Plan to use the Win32 HeapAlloc. */
    static HANDLE process_heap;

    /**
     *  Resource name to use for `IWebmLocator::ConnectServer`.
     *  TODO: give this a proper type name eventually and figure out how to maintain the proper const qualifier in a `BSTR` call.
     */
    static constexpr auto WMI_RESOURCE_NAME = L"ROOT\\CIMV2";

    static constexpr auto PDH_GPU_OBJ = L"GPU Engine";
    static constexpr auto PDH_COUNTER_NAME = L"Utilization";
    static constexpr auto PDH_INSTANCE_FILTER = L"*_engtype_3D";

    //static constexpr auto GPU_ENGINE_FORMAT = L"\\GPU Engine\\(%s)\\Utilization Percentage";

    static PDH_COUNTER_PATH_ELEMENTS counter_parts;

    /** Handle for calling pdh functions. */
    static PDH_HCOUNTER gpu_counter_handle;

    /** Set to true if there were no errors during initialization. */
    static bool initSuccess;

    /* Microsoft documentation isn't entirely clear what the dwUserData argument for Pdh functions actaully is for but most examples seem to use 0. */
    DWORD_PTR PDH_USER_PTR = 0;

    /** WMI namespace locator. */
    IWbemLocator *pLocate;

    /** WMI service interface. */
    IWbemServices *pServ;

    /** ID of the last process that got queried. */
    DWORD lastProc;

    /** Handle to the process identified by `lastProc`. */
    HANDLE lastProcHandle;

    /** PDH Handle to gather GPU data. */
    PDH_HQUERY gpu_query_handle;

    /** Wildcard path to query pdh for the correct counter. */
    std::vector<WCHAR> pdh_wildcard_path;

    /* Utility function */
    void init_pdh_counters();

public:
    /** Sets up program to make necessary WMI calls. Avoid creating more than one COM Object */
    ProcData();

    /** Releasing COM resources here. */
    ~ProcData();

    /** Gets the handle for the process that created the current foreground window. */
    HANDLE getFgProcHandle();

    /** Sum two 'FILETIME's into a single integral type. */
    static unsigned long long filetimeSum(FILETIME, FILETIME);

    /** Gets the relative difference between two `FILETIME`s */
    static long long filetimeDiff(FILETIME, FILETIME);

    /** Get the last item of a \-delimited path. */
    static std::string getLastPathItem(LPWSTR path, DWORD size);

    /**
     * Retrieves the engtype_3D instances from a large double-null separated string.
     *
     * @param pid Process ID to look for.
     * @param list Null separated, double-null terminated list of strings.
     * @param list_length Size of the list.
     *
     * @return Vector array containing the path with the given PID.
     */
    static std::vector<WCHAR> parseGpuCounterPaths(DWORD pid, LPWSTR list, DWORD list_length);

    /** Check if a GPU instance path has the given PID. */
    static bool instanceHasPid(DWORD pid, std::vector<WCHAR>::iterator beg, std::vector<WCHAR>::iterator end);

    /** Check if an instance path has the correct one for GPU 3D utilization. */
    static bool correctInstanceType(std::vector<WCHAR>::iterator beg, std::vector<WCHAR>::iterator end);

    /**
     * Get the instance path for the foreground process.
     *
     * @param proc_handle Process handle.
     * @param wildcard_path PDH counter path containing a wildcard.
     * @param path_len length of `wildcard_path`
     */
    static std::vector<WCHAR> getFgGpuPath(HANDLE proc_handle, LPWSTR wildcard_path);

    /** Check that the process handle hasn't timed out. */
    static bool procHandleValid(HANDLE);
    
    /**
     * Checks whether the instance was initialized successfully.
     * @return Returns true if no error codes were returned in the constructor.
     */
    bool initSuccessful() const;

    /**
     * Gets the total amount of user and kernel time (micro-seconds) spent by the foreground process.
     */
    unsigned long long getTotalProcessTime();

    /**
     * Get the total amount of time (microseconds) spent by the CPU in kernel and user mode.
     */
    unsigned long long getTotalCpuTime();

    /**
     * Gets the amount of memory in bytes allocated by the current foreground process.
     * @return Returns 0 on any unsuccessful `win32` call.
     */
    unsigned long long getFgProcessMemory();

    /**
     * Get the path of the foreground process.
     * @return Empty string if the call to `QueryFullProcessImageName` fails.
     */
    std::string getFgProcessName();

    /**
     * Get the percent utilization of the GPU's 3D rendering engine by the current foreground process.
     * @return Number in the range `[0, 1)`.
     */
    double getFgProcessGpuUsage();
};

#endif // PROCDATA_H
