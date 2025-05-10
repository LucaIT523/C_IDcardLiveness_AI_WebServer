
#include "SvcMng.h"



void CheckServiceStatus(SC_HANDLE service) {
    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;

    if (!QueryServiceStatusEx(
        service,                     // handle to service
        SC_STATUS_PROCESS_INFO,      // information level
        (LPBYTE)&ssp,                // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &bytesNeeded))               // size needed if buffer is too small
    {
        //        std::cerr << "QueryServiceStatusEx failed (" << GetLastError() << ")\n";
        return;
    }

    if (ssp.dwCurrentState == SERVICE_RUNNING) {
        //        std::cout << "Service is running\n";
    }
    else {
        //        std::cout << "Service is not running\n";
    }
}
bool IsServiceRunning(TCHAR* serviceName) {
    // Open a handle to the service control manager
    SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scManager == NULL) {
        //        std::cerr << "OpenSCManager failed (" << GetLastError() << ")\n";
        return false;
    }

    // Open a handle to the specified service
    SC_HANDLE service = OpenService(scManager, serviceName, SERVICE_QUERY_STATUS);
    if (service == NULL) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
            //            std::cout << "Service does not exist\n";
        }
        else {
            //            std::cerr << "OpenService failed (" << error << ")\n";
        }
        CloseServiceHandle(scManager);
        return false;
    }

    // Query the service status
    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;
    if (!QueryServiceStatusEx(
        service,
        SC_STATUS_PROCESS_INFO,
        (LPBYTE)&ssp,
        sizeof(SERVICE_STATUS_PROCESS),
        &bytesNeeded)) {
        //        std::cerr << "QueryServiceStatusEx failed (" << GetLastError() << ")\n";
        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return false;
    }

    // Check if the service is running
    bool isRunning = (ssp.dwCurrentState == SERVICE_RUNNING);

    // Close the service handle
    CloseServiceHandle(service);
    // Close the service control manager handle
    CloseServiceHandle(scManager);

    return isRunning;
}

void StartServiceByName(TCHAR* serviceName) {
    SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scManager == NULL) {
        //       std::cerr << "OpenSCManager failed (" << GetLastError() << ")\n";
        return;
    }

    SC_HANDLE service = OpenService(scManager, serviceName, SERVICE_START | SERVICE_QUERY_STATUS);
    if (service == NULL) {
        //        std::cerr << "OpenService failed (" << GetLastError() << ")\n";
        CloseServiceHandle(scManager);
        return;
    }

    CheckServiceStatus(service);

    if (!StartService(service, 0, NULL)) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_ALREADY_RUNNING) {
            //            std::cout << "Service is already running\n";
        }
        else {
            //            std::cerr << "StartService failed (" << error << ")\n";
        }
    }
    else {
        //        std::cout << "Service start pending...\n";
        CheckServiceStatus(service);
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scManager);
}

void StopServiceByName(TCHAR* serviceName) {
    SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scManager == NULL) {
        //       std::cerr << "OpenSCManager failed (" << GetLastError() << ")\n";
        return;
    }

    SC_HANDLE service = OpenService(scManager, serviceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (service == NULL) {
        //        std::cerr << "OpenService failed (" << GetLastError() << ")\n";
        CloseServiceHandle(scManager);
        return;
    }

    CheckServiceStatus(service);

    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;

    if (!QueryServiceStatusEx(
        service,                     // handle to service
        SC_STATUS_PROCESS_INFO,      // information level
        (LPBYTE)&ssp,                // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &bytesNeeded))               // size needed if buffer is too small
    {
        //        std::cerr << "QueryServiceStatusEx failed (" << GetLastError() << ")\n";
        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return;
    }

    if (ssp.dwCurrentState == SERVICE_STOPPED) {
        //        std::cout << "Service is already stopped\n";
    }
    else {
        SERVICE_STATUS status;
        if (!ControlService(service, SERVICE_CONTROL_STOP, &status)) {
            //            std::cerr << "ControlService failed (" << GetLastError() << ")\n";
        }
        else {
            //            std::cout << "Service stop pending...\n";
            CheckServiceStatus(service);
        }
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scManager);
}
bool IsServiceRegistered(TCHAR* serviceName) {
    // Open a handle to the service control manager
    SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scManager == NULL) {
        //        std::cerr << "OpenSCManager failed (" << GetLastError() << ")\n";
        return false;
    }

    // Open a handle to the specified service
    SC_HANDLE service = OpenService(scManager, serviceName, SERVICE_QUERY_STATUS);
    if (service == NULL) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
            //          std::cout << "Service is not registered\n";
        }
        else {
            //          std::cerr << "OpenService failed (" << error << ")\n";
        }
        CloseServiceHandle(scManager);
        return false;
    }

    // If we successfully opened the service, it is registered
 //   std::cout << "Service is registered\n";

    // Close the service handle
    CloseServiceHandle(service);
    // Close the service control manager handle
    CloseServiceHandle(scManager);

    return true;
}