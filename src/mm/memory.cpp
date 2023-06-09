#include <mm/memory.hpp>
#include <sys/printk.hpp>
#include <sys/panic.hpp>
#include <mm/pmm.hpp>
#include <mm/vmm.hpp>
#include <mm/bootmem.hpp>
#include <mm/heap.hpp>
#include <init/kinfo.hpp>

static char *memTypeStrings[] = {
	"Usable",
	"Reserved",
	"ACPI Reclaimable",
	"ACPI NVS",
	"Bad",
	"Bootloader reclaimable",
	"Kernel and modules",
	"Framebuffer"
};

void *Malloc(size_t size) {
	if(HEAP::IsHeapActive()) return HEAP::Malloc(size);
	if(BOOTMEM::BootmemIsActive()) return BOOTMEM::Malloc(size);
	else return NULL;
}

void Free(void *p) {
	HEAP::Free(p);
}

void *operator new(size_t size) {
	if(HEAP::IsHeapActive()) return HEAP::Malloc(size);
	if(BOOTMEM::BootmemIsActive()) return BOOTMEM::Malloc(size);
	else return NULL;
}

void *operator new[](size_t size) {
	if(HEAP::IsHeapActive()) return HEAP::Malloc(size);
	if(BOOTMEM::BootmemIsActive()) return BOOTMEM::Malloc(size);
	else return NULL;
}

void operator delete(void* p) {
	// Now, here comes the problem in deciding who allocated this block
	// We should assume that someone that allocs on BOOTMEM
	// will not call free

	HEAP::Free(p);
}

void operator delete(void* p, size_t size) {
	// Now, here comes the problem in deciding who allocated this block
	// We should assume that someone that allocs on BOOTMEM
	// will not call free

	HEAP::Free(p);
}

void operator delete[](void* p) {
	// Now, here comes the problem in deciding who allocated this block
	// We should assume that someone that allocs on BOOTMEM
	// will not call free

	HEAP::Free(p);
}

void operator delete[](void* p, size_t size) {
	// Now, here comes the problem in deciding who allocated this block
	// We should assume that someone that allocs on BOOTMEM
	// will not call free

	HEAP::Free(p);
}

namespace MEM {
void Init() {
	KInfo *info = GetInfo();

	PRINTK::PrintK("Memory map:\r\n");

	for (int i = 0; i < info->mMapEntryCount; i++) {
		PRINTK::PrintK("[0x%x - 0x%x] -> %s\r\n",
				info->mMap[i].base,
				info->mMap[i].base + info->mMap[i].length,
				memTypeStrings[info->mMap[i].type]);
	}

	VMM::InitVMM();

	PRINTK::PrintK("Done.\r\n");
}
}
