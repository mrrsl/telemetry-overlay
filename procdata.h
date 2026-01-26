#ifndef PROCDATA_H
#define PROCDATA_H

#define _WIN32_DCOM

#include <windows.h>
#include <wbemidl.h>
#include <Psapi.h>
#include <WinBase.h>
#include <Pdh.h>

#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <vector>

#pragma comment(lib, "wbemuuid.lib")

class ProcData {

    /**  */
    static constexpr int MICROSEC_TO_FILETIME =  10;

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

    
public:
    /** Sets up program to make necessary WMI calls. Avoid creating more than one COM Object */
    ProcData();

    /** Release COM resources here. */
    ~ProcData();

    /** Gets the handle for the process that created the current foreground window. */
    HANDLE getFgProcHandle();

    /** Sum two 'FILETIME's into a single integral type. */
    static unsigned long long filetimeSum(FILETIME, FILETIME);

    /** Gets the relative difference between two `FILETIME`s */
    static long long filetimeDiff(FILETIME, FILETIME);

    /** Retrieves the engtype_3D instances from a large double-null separated string. */
    std::vector<WCHAR> parseGpuCounterPaths(std::vector<WCHAR> &instanceList);

    /** Check if a GPU instance path has the given PID. */
    static bool instanceHasPid(DWORD pid, std::vector<WCHAR>::iterator beg, std::vector<WCHAR>::iterator end);

    /** Check if an instance path has the correct one for GPU 3D utilization. */
    static bool correctInstanceType(std::vector<WCHAR>::iterator beg, std::vector<WCHAR>::iterator end);

    /** Get the instance path for the foreground process. */
    std::vector<WCHAR> getFgGpuPath();

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
    QString getFgProcessName();

    /**
     * Get the percent utilization of the GPU's 3D rendering engine by the current foreground process.
     * @return Number in the range `[0, 1)`.
     */
    double getFgProcessGpuUsage();
};

#endif // PROCDATA_H
