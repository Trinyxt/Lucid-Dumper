#include <iostream>
#include "mem/mem.hpp"
#include "utils/utils.hpp"
#include "Offsets.h"

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
                Utils::LogOffset(ss.str());
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
        Utils::LogOffset("Place Id found at offset: 0x" + ss.str());
    }
    else {
        Utils::LogError("GameID Not Found within" + std::to_string(maxOffset * sizeof(uintptr_t)) + " bytes");
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
        Utils::LogOffset("Game Id found at offset: 0x" + ss.str());
		Offsets::GameLoadedOffset = gameIdOffset;
    }
    else
		Utils::LogError("GameID Not Found within" + std::to_string(maxOffset * sizeof(uintptr_t)) + " bytes");
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
			Utils::LogOffset("Name offset found at: 0x" + ss.str());
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
            Utils::LogOffset("Game Loaded offset found at: 0x" + ss.str());
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
            Utils::LogOffset("Workspace offset found at: 0x" + ss.str());
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
            Utils::LogOffset("Parent offset found at: 0x" + ss.str());
            Offsets::ParentOffset = ParentOffset;
            return;
        }
        ParentOffset += 1;
    }
    Utils::LogError("Parent Offset Wasn't Found");

    
}

std::string GetName(uintptr_t Address) {
    uintptr_t WorkSpaceNameAddr = Mem->Read<uintptr_t>(Address + Offsets::NameOffset);
    return Mem->readstring(WorkSpaceNameAddr);

}

void DumpChildrenOffsets() {
    uintptr_t ChildrenOffset = 0;
    const uintptr_t maxScan = 0x100000;

    while (ChildrenOffset < maxScan) {
        uintptr_t childrenPtr = Mem->Read<uintptr_t>(Offsets::Datamodel + ChildrenOffset);
        uintptr_t childrenEnd = Mem->Read<uintptr_t>(childrenPtr + 0x8);

        // Debug: show each candidate offset
        {
            std::stringstream ss;
            ss << "[SCAN] Offset=0x" << std::hex << ChildrenOffset
                << " Ptr=0x" << childrenPtr
                << " End=0x" << childrenEnd;
            Utils::LogDebug(ss.str());
        }

        // Iterate possible children
        for (auto child = Mem->Read<uintptr_t>(childrenPtr);
            child < childrenEnd;
            child += 0x10)
        {
            std::string ChildName = GetName(child);
            if (ChildName == "")
                continue;

            // Debug: show each name read
            {
                std::stringstream ss;
                ss << "   [CHILD] Addr=0x" << std::hex << child
                  
                    << " Name=\"" << ChildName << "\"";
                Utils::LogDebug(ss.str());
            }

            if (ChildName == "Workspace") {
                std::stringstream ss;
                ss << std::hex << ChildrenOffset;
                Utils::LogOffset("Children Start offset found at: 0x" + ss.str());
                return;
            }
        }

        ChildrenOffset++;
    }

    Utils::LogError("Children offset not found within scan limit");
}


std::vector<uintptr_t> GetChildren(uintptr_t Address)
{
    std::vector<uintptr_t> Instancechildren;

    uintptr_t childrenptr = Mem->Read<uintptr_t>(Address + Offsets::ChildrenOffset);
    uintptr_t childrenend = Mem->Read<uintptr_t>(childrenptr + 0x8);

    for (auto child = Mem->Read<uintptr_t>(childrenptr); child < childrenend; child += 0x10) {
        Instancechildren.push_back(Mem->Read<uintptr_t>(child));
    }

    return Instancechildren;
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
			Utils::LogOffset("Class Descriptor offset found at: 0x" + std::to_string(classDescOffset));
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
    DumpChildrenOffsets();
}

#include <thread>
int main() {
	EnableVTMode();
    Utils::SetConsoleProperties();
    Utils::PrintBanner();
	Utils::LogInfo("Initialising Memory");
	if (!Mem->Attach()) {

		return -1;
	}else {
		Utils::LogSuccess("Successfully attached to process.\n\n");
	}
    GetOffsets();
    std::cout << std::endl;
	Utils::LogInfo("To Exit Press Enter");
  
    //std::thread([]() {BounceConsole();}).detach();
	
    std::cin.get();
	return 0;
}