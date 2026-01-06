/*
API wrapper for retrieving process telemetry from Windows.
The goal is to simplify retrieval of data on resources used by the foreground process.
Ideally, avoid export of any OS-specific types and only return standard types to keep things portable.
*/

#ifndef PROC_DATA_SOURCE_HH
#define PROC_DATA_SOURCE_HH

#define _WIN32_DCOM

#include <windows.h>
#include <wbemidl.h>
#include <Psapi.h>
#include <WinBase.h>

#include <QString>

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

    /** Returns the sum of two `FILETIME` structures as a 128 bit integer. */
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
     * @return Returns 0 on any unsuccessful `win32` call.
     */
    unsigned long long getFgProcessMemory();
    
    /**
     * Get the path of the foreground process.
     * @return Empty string if the call to `QueryFullProcessImageName` fails.
     */
    QString getFgProcessName();
};

#endif