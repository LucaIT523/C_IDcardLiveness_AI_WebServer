#pragma once

#include <windows.h>
#include <iostream>



void CheckServiceStatus(SC_HANDLE service);


bool IsServiceRunning(TCHAR* serviceName);

void StartServiceByName(TCHAR* serviceName);

void StopServiceByName(TCHAR* serviceName);

bool IsServiceRegistered(TCHAR* serviceName);