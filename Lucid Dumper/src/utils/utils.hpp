#pragma once
#include <string>

namespace Utils {
    // Console
    void ClearConsole();
    void PrintBanner();
    void SetConsoleProperties();

    // Logging
    void LogError(const std::string& message);
    void LogInfo(const std::string& message);
    void LogSuccess(const std::string& message);
    void LogWarning(const std::string& message);
    void LogDebug(const std::string& message);
    void LogOffset(const std::string& message);
}
