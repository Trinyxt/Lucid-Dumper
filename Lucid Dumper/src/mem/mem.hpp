#pragma once
#include <Windows.h>
#include <winternl.h>
#include "utils.hpp"

typedef NTSTATUS(NTAPI* tNtReadVirtualMemory)(
	HANDLE ProcessHandle,
	PVOID BaseAddress,
	PVOID Buffer,
	SIZE_T NumberOfBytesToRead,
	PSIZE_T NumberOfBytesRead
	);

class Memory {
private:
	HANDLE ProcHandle = 0;
	DWORD ProcID = 0;
	tNtReadVirtualMemory NtReadVM = nullptr;
public:
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
};