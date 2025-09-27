#include "Utils.hpp"

void PrintCenteredText(const std::string& text, int extraTopPadding = 0) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    std::vector<std::string> lines;
    std::string line;

    for (char c : text) {
        if (c == '\n') {
            lines.push_back(line);
            line.clear();
        }
        else {
            line.push_back(c);
        }
    }
    if (!line.empty()) lines.push_back(line);

    size_t maxWidth = 0;
    for (const auto& l : lines) {
        if (l.size() > maxWidth) maxWidth = l.size();
    }

    for (int i = 0; i < extraTopPadding; i++) {
        std::cout << "\n";
    }

    for (const auto& l : lines) {
        int padding = (consoleWidth - static_cast<int>(l.size())) / 2;
        if (padding < 0) padding = 0;
        std::cout << std::string(padding, ' ') << l << "\n";
    }
}

 void Utils::BounceConsole() {
    HWND hwnd = GetConsoleWindow();
    if (!hwnd) return;

    RECT rc;
    GetWindowRect(hwnd, &rc);

    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Initial position (center)
    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;

    // Velocity
    int vx = 15; // pixels per step
    int vy = 15;

    while (true) {
        // Move
        x += vx;
        y += vy;

        // Bounce on edges
        if (x <= 0 || x + width >= screenWidth) vx = -vx;
        if (y <= 0 || y + height >= screenHeight) vy = -vy;

        SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        // Slow down movement
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

namespace Utils {
    void SetConsoleProperties() {
        SetConsoleTitle(L"Lucid Dumper | github.com/trinyxt");
        HWND hwnd = GetConsoleWindow();
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hwnd, 0, 220, LWA_ALPHA);


        RECT rc;
        GetWindowRect(hwnd, &rc);

        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        // Get screen size
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // Calculate centered position
        int x = (screenWidth - width) / 2;
        int y = (screenHeight - height) / 2;

        SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
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
        std::string logo =
            "   _                   __                        \n"
            " _//             /    /  )                       \n"
            " /   . . _. o __/    /  / . . ______  _   _  __ \n"
            "/___(_/_(__<_(_/_   /__/_(_/_/ / / <_/_)_</_/ (_\n"
            "                                    /           \n"
            "                                    '             \n";

        PrintCenteredText(logo, 3);
        std::cout << RESET;
    }

    void LogError(const std::string& message) {
        std::cout << RED;
        PrintCenteredText("[ERROR] " + message);
        std::cout << RESET;
    }

    void LogInfo(const std::string& message) {
        std::cout << CYAN;
        PrintCenteredText("[INFO] " + message);
        std::cout << RESET;
    }

    void LogSuccess(const std::string& message) {
        std::cout << GREEN;
        PrintCenteredText("[SUCCESS] " + message);
        std::cout << RESET;
    }

    void LogWarning(const std::string& message) {
        std::cout << YELLOW;
        PrintCenteredText("[WARNING] " + message);
        std::cout << RESET;
    }

    void LogDebug(const std::string& message) {
        std::cout << MAGENTA;
        PrintCenteredText("[DEBUG] " + message);
        std::cout << RESET;
    }

    void LogOffset(const std::string& message) {
        std::cout << BRIGHT_BLUE;
        PrintCenteredText("[OFFSET] " + message);
        std::cout << RESET;
    }
}

