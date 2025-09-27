#include <cstdint>
namespace Offsets {
	//offsets
	uintptr_t FakeDatamodelDataModelOffset;
	uintptr_t PlaceIDOffset;
	uintptr_t GameIdOffset;
	uintptr_t NameOffset;
	uintptr_t GameLoadedOffset;
	uintptr_t ParentOffset;
	uintptr_t WorkspaceOffset;


	// Manually Update
	uintptr_t FakeDmToReal = 0x1C0;

	// Instances
	uintptr_t Datamodel;
	uint64_t PlaceId = 119113468072163;
	uint64_t GameId = 8816831536;
}