#include "mem.hpp"

bool Memory::PatternToBytes(const std::string& pattern, std::vector<BYTE>& bytes, std::string& mask) {
	bytes.clear(); mask.clear();
	size_t i = 0;
	while (i < pattern.size()) {
		if (pattern[i] == ' ') { i++; continue; }
		if (pattern[i] == '?') {
			bytes.push_back(0x00); mask += '?'; i++;
			if (i < pattern.size() && pattern[i] == '?') i++;
		}
		else {
			if (i + 1 >= pattern.size()) return false;
			auto hexCharToInt = [](char c) -> int {
				if (c >= '0' && c <= '9') return c - '0';
				if (c >= 'a' && c <= 'f') return c - 'a' + 10;
				if (c >= 'A' && c <= 'F') return c - 'A' + 10;
				return -1;
				};
			int high = hexCharToInt(pattern[i]);
			int low = hexCharToInt(pattern[i + 1]);
			if (high == -1 || low == -1) return false;
			bytes.push_back((BYTE)((high << 4) | low));
			mask += 'x';
			i += 2;
		}
	}
	return true;
}

bool Memory::DataCompare(const BYTE* data, const BYTE* pattern, const std::string& mask, size_t size) {
	for (size_t i = 0; i < size; i++)
		if (mask[i] == 'x' && data[i] != pattern[i])
			return false;
	return true;
}

uintptr_t Memory::ScanRegion(HANDLE hProcess, uintptr_t base, size_t size, const std::vector<BYTE>& pattern, const std::string& mask) {
	std::vector<BYTE> buffer(size);
	SIZE_T bytesRead;
	if (!ReadProcessMemory(hProcess, (LPCVOID)base, buffer.data(), size, &bytesRead) || bytesRead < pattern.size())
		return 0;
	for (size_t i = 0; i <= bytesRead - pattern.size(); i++)
		if (DataCompare(buffer.data() + i, pattern.data(), mask, pattern.size()))
			return base + i;
	return 0;
}



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