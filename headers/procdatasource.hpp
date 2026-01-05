#ifndef PROC_DATA_SOURCE_HH
#define PROC_DATA_SOURCE_HH

#define _WIN32_DCOM

#include <windows.h>
#include <wbemidl.h>
#include <Psapi.h>

#pragma comment(lib, "wbemuuid.lib")

class ProcData {
    /** Set to true if there were no errors during initialization. */
    bool initSuccess;

    /** WMI namespace locator. */
    IWbemLocator *pLocate;

    /** WMI service interface. */
    IWbemServices *pServ;

    /** ID of the last process that got queried. */
    DWORD lastProc;

    /** Handle to the process identified by `lastProc`. */
    HANDLE lastProcHandle;

    /** Gets the handle for the process that created the current foreground window. */
    HANDLE getFgProcHandle();

    static unsigned long long filetimeSum(FILETIME, FILETIME);
public:
    /** Sets up program to make necessary WMI calls. Check */
    ProcData();

    /** Release COM resources here. */
    ~ProcData();

    /**
     * Checks whether the instance was initialized successfully.
     * @return Returns true if no error codes were returned in the constructor.
     */
    bool initSuccessful() const;

    /**
     * Gets the total amount of user and kernel time spent by the foreground process.
     */
    unsigned long long getTotalProcessTime();

    /**
     * Get the total amount of time spent by the CPU in kernel and user mode.
     */
    unsigned long long getTotalCpuTime();

    /**
     * Gets the amount of memory in bytes allocated by the current foreground process.
     * @return Returns 0 if the last cached process has a `NULL` handle.
     */
    unsigned long getFgProcessMemory();
    
};

#endif