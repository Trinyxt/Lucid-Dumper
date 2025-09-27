#include "Utils.hpp"
#include <iostream>
#include <Windows.h>
#include "ColorDef.h"

namespace Utils {
    void SetConsoleProperties() {
		SetConsoleTitle(L"Lucid Dumper | github.com/trinyxt");
    }

    void ClearConsole() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void PrintBanner() {
        std::cout << CYAN;
        std::cout << R"( _      _     _     _
| |    (_)   | |   | |
| |     _ ___| |__ | | ___  ___
| |    | / __| '_ \| |/ _ \/ __|
| |____| \__ \ | | | |  __/\__ \
|______|_|___/_| |_|_|\___||___/
        )" << std::endl;
        std::cout << RESET;
    }

    void LogError(const std::string& message) {
        std::cout << RED << "[ERROR] " << message << RESET << std::endl;
    }

    void LogInfo(const std::string& message) {
        std::cout << CYAN << "[INFO] " << message << RESET << std::endl;
    }

    void LogSuccess(const std::string& message) {
        std::cout << GREEN << "[SUCCESS] " << message << RESET << std::endl;
    }

    void LogWarning(const std::string& message) {
        std::cout << YELLOW << "[WARNING] " << message << RESET << std::endl;
    }

    void LogDebug(const std::string& message) {
        std::cout << MAGENTA << "[DEBUG] " << message << RESET << std::endl;
    }

    void LogOffset(const std::string& message) {
        std::cout << WHITE << "[OFFSET] " << message << RESET << std::endl;
    }
}
