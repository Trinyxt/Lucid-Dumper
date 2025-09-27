#include <iostream>
#include "mem/mem.hpp"
#include "utils/utils.hpp"

Memory mem{};
using namespace Utils;
int main() {
	SetConsoleProperties();
	PrintBanner();
	LogInfo("Initialising Memory");
	if (!mem.Attach()) {

		return -1;
	}else {
		LogSuccess("Successfully attached to process.\n");
	}

	Utils::LogInfo("To Exit Press Enter");
	std::cin.get();
	return 0;
}