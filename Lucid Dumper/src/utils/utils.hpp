#pragma once
#include <string>
#include <iostream>
#include <Windows.h>
#include "ColorDef.h"
#include <sstream>
#include <thread>
#include <vector>


namespace Utils {
    // Console
    void ClearConsole();
    void PrintBanner();
    void SetConsoleProperties();
	void BounceConsole();

    // Logging
    void LogError(const std::string& message);
    void LogInfo(const std::string& message);
    void LogSuccess(const std::string& message);
    void LogWarning(const std::string& message);
    void LogDebug(const std::string& message);
    void LogOffset(const std::string& message);
}
