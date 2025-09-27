#pragma once
#include <Windows.h>
#include <winternl.h>
#include "utils.hpp"
#include <vector>
#include <string>
#include <TlHelp32.h>
#include <iostream>
#include <sstream>


typedef NTSTATUS(NTAPI* tNtReadVirtualMemory)(
	HANDLE ProcessHandle,
	PVOID BaseAddress,
	PVOID Buffer,
	SIZE_T NumberOfBytesToRead,
	PSIZE_T NumberOfBytesRead
	);

class Memory {
private:
	tNtReadVirtualMemory NtReadVM = nullptr;
public:
	HANDLE ProcHandle = 0;
	DWORD ProcID = 0;
	uintptr_t baseAddress = 0;
	const wchar_t* ProcName = L"RobloxPlayerBeta.exe";

	Memory() {
		HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
		NtReadVM = (tNtReadVirtualMemory)GetProcAddress(ntdll, "NtReadVirtualMemory");
	}

	template<typename T>
	T Read(uintptr_t address) {
		T buffer{};
		if (NtReadVM && ProcHandle) {
			NtReadVM(ProcHandle, (PVOID)address, &buffer, sizeof(T), nullptr);
		}
		else {
			Utils::LogError("NtReadVirtualMemory function  or Process Handle is null.");
		}
		return buffer;
	}
	
	bool GetProcID();
	bool GetHandle();
	bool GetBaseAddress();
	bool Attach();

	bool PatternToBytes(const std::string& pattern, std::vector<BYTE>& bytes, std::string& mask);
	bool DataCompare(const BYTE* data, const BYTE* pattern, const std::string& mask, size_t size);
	uintptr_t ScanRegion(HANDLE hProcess, uintptr_t base, size_t size, const std::vector<BYTE>& pattern, const std::string& mask);

	std::string readstring(uintptr_t addr)
	{
		std::string result;
		result.reserve(204);

		const int length = Read<int>(addr + 0x18);
		const uintptr_t stringAddr = length >= 16 ? Read<uintptr_t>(addr) : addr;

		int offset = 0;
		while (offset < 200)
		{
			char ch = Read<char>(stringAddr + offset);
			if (ch == '\0') break;

			result.push_back(ch);
			offset += sizeof(char);
		}

		return result;
	}
};

inline std::unique_ptr<Memory> Mem = std::make_unique<Memory>();