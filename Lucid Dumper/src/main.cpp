#include <iostream>
#include "mem/mem.hpp"
#include "utils/utils.hpp"
#include "Offsets.h"

using namespace Utils;

void EnableVTMode() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) return;

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode)) return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
}

void GetFakeDatamodelPtr() {
    std::string Pattern = "48 8D 1D ? ? ? ? 8B 07 39 05 ? ? ? ? 7F";

    std::vector<BYTE> bytes;
    std::string mask;
    if (!Mem->PatternToBytes(Pattern, bytes, mask)) return;

    uintptr_t base = Mem->baseAddress;

    MEMORY_BASIC_INFORMATION mbi;
    while (true) {
        if (VirtualQueryEx(Mem->ProcHandle, (LPCVOID)base, &mbi, sizeof(mbi)) != sizeof(mbi))
            break;

        if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) {
            uintptr_t found = Mem->ScanRegion(Mem->ProcHandle, (uintptr_t)mbi.BaseAddress, mbi.RegionSize, bytes, mask);
            if (found) {
                // READ RELATIVE OFFSET
                int32_t rel = 0;
                ReadProcessMemory(Mem->ProcHandle, (LPCVOID)(found + 3), &rel, sizeof(rel), nullptr);
                
                uintptr_t fakeDatamodelAddr = found + 7 + rel; // lea instruction length = 7 bytes

                uintptr_t offset = fakeDatamodelAddr - Mem->baseAddress;
                offset += 0x8;
                Offsets::FakeDatamodelDataModelOffset = offset;
                std::ostringstream ss;
                ss << "Datamodel found at address: 0x"
                    << std::hex << std::uppercase << fakeDatamodelAddr
                    << " (offset: 0x" << offset << ")";
                LogOffset(ss.str());
                break;
            }
        }
        base += mbi.RegionSize;
    }
}

void GetPlaceId() {
    auto DM = Offsets::Datamodel;
    uintptr_t placeId = 0;
    int i = 0;
    const int maxOffset = 0x1000; 

    while (placeId != Offsets::PlaceId && i < maxOffset) {
        placeId = Mem->Read<uintptr_t>(DM + i * sizeof(uintptr_t));
        i++;
    }
    uintptr_t placeIdOffset;
    if (placeId == Offsets::PlaceId) {
        placeIdOffset = (i - 1) * sizeof(uintptr_t);
        std::stringstream ss;
        ss << std::hex << placeIdOffset;
        LogOffset("Place Id found at offset: 0x" + ss.str());
    }
    else {
        LogError("GameID Not Found within" + std::to_string(maxOffset * sizeof(uintptr_t)) + " bytes");
    }
    Offsets::PlaceIDOffset = placeIdOffset;
}

void GetGameId() {
    auto DM = Offsets::Datamodel;
    uintptr_t GameId = 0;
    int i = 0;
    const int maxOffset = 0x1000;

    while (GameId != Offsets::GameId && i < maxOffset) {
        GameId = Mem->Read<uintptr_t>(DM + i * sizeof(uintptr_t));
        i++;
    }
    uintptr_t gameIdOffset = 0;
    if (GameId == Offsets::GameId) {
        gameIdOffset = (i - 1) * sizeof(uintptr_t);
        std::stringstream ss;
        ss << std::hex << gameIdOffset;
        LogOffset("Game Id found at offset: 0x" + ss.str());
		Offsets::GameLoadedOffset = gameIdOffset;
    }
    else
		LogError("GameID Not Found within" + std::to_string(maxOffset * sizeof(uintptr_t)) + " bytes");
    Offsets::PlaceIDOffset = gameIdOffset;
}

void GetNameOffset() {
    auto DM = Offsets::Datamodel;
    uintptr_t nameOffset = 0;
    while (true)
    {
        const uintptr_t namePtr = Mem->Read<uintptr_t>(DM + nameOffset);
        const std::string name = Mem->readstring(namePtr);

        if (name == "Ugc" || name == "LuaApp")
        {
            std::stringstream ss;
            ss << std::hex << nameOffset;
			LogOffset("Name offset found at: 0x" + ss.str());
			Offsets::NameOffset = nameOffset;
            break;
        } 
        nameOffset++;
    }
}

void GetGameLoaded() {
    auto DM = Offsets::Datamodel;
    uintptr_t GameLoadedOffset = 0;
    while (true)
    {
        const int GameLoaded = Mem->Read<int>(DM + GameLoadedOffset);

        if (GameLoaded == 31)
        {
            std::stringstream ss;
            ss << std::hex << GameLoadedOffset;
            LogOffset("Game Loaded offset found at: 0x" + ss.str());
            break;
        }
        GameLoadedOffset++;
    }
}

void GetWorkspace() {
    auto DM = Offsets::Datamodel;
    uintptr_t WorkspaceOffset = 0;
    while (true)
    {
        const uintptr_t Workspace = Mem->Read<uintptr_t>(DM + WorkspaceOffset);
        uintptr_t WorkSpaceNameAddr = Mem->Read<uintptr_t>(Workspace + Offsets::NameOffset);

        std::string WorkspaceName = Mem->readstring(WorkSpaceNameAddr);
        if (WorkspaceName == "Workspace")
        {
            std::stringstream ss;
            ss << std::hex << WorkspaceOffset;
            LogOffset("Workspace offset found at: 0x" + ss.str());
            Offsets::WorkspaceOffset = WorkspaceOffset;
            break;
        }
        WorkspaceOffset++;
    }
}

void GetParentOffset() {
    uintptr_t Workspace = Mem->Read<uintptr_t>(Offsets::WorkspaceOffset + Offsets::Datamodel);
    uintptr_t ParentOffset = 0;
    const size_t maxScan = 0x1000; // prevent infinite loop

    while (true) {
        uintptr_t Parent = Mem->Read<uintptr_t>(Workspace + ParentOffset);
        uintptr_t ParentNameAddr = Mem->Read<uintptr_t>(Parent + Offsets::NameOffset);
        std::string ParentName = Mem->readstring(ParentNameAddr);

        if (ParentName == "Ugc" || ParentName == "LuaApp") {
            std::stringstream ss;
            ss << std::hex << ParentOffset;
            LogOffset("Parent offset found at: 0x" + ss.str());
            Offsets::ParentOffset = ParentOffset;
            return;
        }
        ParentOffset += 1;
    }
    LogError("Parent Offset Wasn't Found");

    
}

void GetClassDescriptor() {
    uintptr_t classDescOffset = 0;
    while (true)
    {
        const uintptr_t classNamePtr = Mem->Read<uintptr_t>(
            Mem->Read<uintptr_t>(Offsets::Datamodel + classDescOffset) + 0x8);
        const std::string className = Mem->readstring(classNamePtr);

        if (className == "DataModel")
        {
			LogOffset("Class Descriptor offset found at: 0x" + std::to_string(classDescOffset));
            break;
        }
        classDescOffset++;
    }
}

void GetOffsets() {
    GetFakeDatamodelPtr();
    uintptr_t FakeDM= Mem->Read<uintptr_t>(Offsets::FakeDatamodelDataModelOffset + Mem->baseAddress);
	Offsets::Datamodel = Mem->Read<uintptr_t>(FakeDM + Offsets::FakeDmToReal);
    GetPlaceId();
    GetGameId();
    GetNameOffset();
    GetGameLoaded();
    GetWorkspace();
    GetParentOffset();
    GetClassDescriptor();
}

#include <thread>
int main() {
	EnableVTMode();
	SetConsoleProperties();
	PrintBanner();
	LogInfo("Initialising Memory");
	if (!Mem->Attach()) {

		return -1;
	}else {
		LogSuccess("Successfully attached to process.\n\n");
	}
    GetOffsets();
    std::cout << std::endl;
	Utils::LogInfo("To Exit Press Enter");
  
    //std::thread([]() {BounceConsole();}).detach();
	
    std::cin.get();
	return 0;
}