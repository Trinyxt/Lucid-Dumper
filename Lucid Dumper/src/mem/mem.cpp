#include "mem.hpp"
#include <vector>
#include <string>
#include <TlHelp32.h>
#include <iostream>
#include <sstream>

bool Memory::GetProcID() {
	DWORD procID = 0;
	HWND hwnd = FindWindowA(NULL, "Roblox");
	if (hwnd == NULL) {
		Utils::LogError("Roblox's hwnd was not found");
		return false;
	}
	GetWindowThreadProcessId(hwnd, &procID);
	ProcID = procID;
	bool isValid = ProcID != 0;
	isValid ? Utils::LogDebug("Process ID: " + std::to_string(ProcID)) : Utils::LogError("Process id is not Valid");
	return ProcID != 0;
}

bool Memory::GetHandle() {
	ProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcID);

	bool ProcHandleValid = ProcHandle != nullptr;
	ProcHandleValid ?  Utils::LogDebug("Process Handle is Valid") : Utils::LogError("Process Handle is not Valid");
	return ProcHandleValid;
}

bool Memory::GetBaseAddress() {
	uintptr_t modBase = 0;

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, ProcID);
	MODULEENTRY32W modEntry = { sizeof(modEntry) };

	if (Module32FirstW(hSnap, &modEntry)) {
		do {
			if (!_wcsicmp(modEntry.szModule, ProcName)) {
				modBase = (uintptr_t)modEntry.modBaseAddr;
				break;
			}
		} while (Module32NextW(hSnap, &modEntry));
	}

	CloseHandle(hSnap);
	baseAddress = modBase;

	// Validating
	bool isValid = baseAddress != 0;
	std::stringstream ss;
	ss << "Base Address is 0x" << std::hex << baseAddress;
	isValid ? Utils::LogDebug(ss.str()) : Utils::LogError("Base Address is not Valid");
	return isValid;
}




std::vector<bool> Successfuls;
bool Memory::Attach() {
	Successfuls.push_back(GetProcID());
	Successfuls.push_back(GetHandle());
	Successfuls.push_back(GetBaseAddress());
	for (bool success : Successfuls) {
		if (!success)
		{
			Utils::LogError("Failed to Attach. Make Sure Roblox is Open and you are on Production");
			return false;
		}
			
	}
	return true;
}