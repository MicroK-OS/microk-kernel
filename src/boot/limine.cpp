#include <limine.h>
#include <sys/panic.hpp>
#include <init/main.hpp>
#include <init/kinfo.hpp>
#include <boot/boot.hpp>
#include <mm/bootmem.hpp>
#include <sys/printk.hpp>

/* Function called by Limine */
extern "C" void LimineEntry(void);

/*
   Setting the correct entry point for Limine
*/
static volatile limine_entry_point_request entryPointRequest {
	.id = LIMINE_ENTRY_POINT_REQUEST,
	.revision = 0,
	.entry = &LimineEntry
};

/*
   System time request for Limine
*/
static volatile limine_boot_time_request timeRequest {
	.id = LIMINE_BOOT_TIME_REQUEST,
	.revision = 0
};

/*
   Bootloader info request for Limine
*/
static volatile limine_bootloader_info_request bootloaderRequest {
	.id = LIMINE_BOOTLOADER_INFO_REQUEST,
	.revision = 0
};

/*
   Stack request for Limine
   Stack size configured in autoconf.h
*/
static volatile limine_stack_size_request stackRequest {
	.id = LIMINE_STACK_SIZE_REQUEST,
	.revision = 0,
	.stack_size = CONFIG_STACK_SIZE
};

/*
   Module request for Limine
*/
static volatile limine_module_request moduleRequest {
	.id = LIMINE_MODULE_REQUEST,
	.revision = 0
};

static volatile limine_hhdm_request hhdmRequest {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0
};

static volatile limine_memmap_request mMapRequest {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0
};

static volatile limine_kernel_address_request kaddrRequest {
	.id = LIMINE_KERNEL_ADDRESS_REQUEST,
	.revision = 0
};

static volatile limine_framebuffer_request fbRequest {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0
};

extern "C" void LimineEntry() {
	/* Startup basic kernel runtime services */
	EarlyKernelInit();

	KInfo *info = GetInfo();

	/* Checking requests have been answered by Limine */
	/* If not, give up and shut down */
	if(timeRequest.response == NULL ||
	   stackRequest.response == NULL ||
	   bootloaderRequest.response == NULL ||
	   moduleRequest.response == NULL ||
	   mMapRequest.response == NULL ||
	   hhdmRequest.response == NULL ||
	   kaddrRequest.response == NULL
	   )
		PANIC("Requests not answered by Limine");

	/* Transporting the MMAP */
	PRINTK::PrintK("Copying the memory map from Limine...\r\n");
	uint64_t mMapEntryCount = mMapRequest.response->entry_count;
	info->mMapEntryCount = mMapEntryCount;
	PRINTK::PrintK("Allocating for %d memory map entries.\r\n", mMapEntryCount);
	for (int i = 0; i < mMapEntryCount; i++) {
		info->mMap[i] = (MMapEntry*)BOOTMEM::Malloc(sizeof(MMapEntry) + 1);
		info->mMap[i]->base = mMapRequest.response->entries[i]->base;
		info->mMap[i]->length = mMapRequest.response->entries[i]->length;
		info->mMap[i]->type = mMapRequest.response->entries[i]->type;
	}

	/* Transporting modules */
	PRINTK::PrintK("Copying modules from Limine...\r\n");
	uint64_t moduleCount = moduleRequest.response->module_count;
	info->moduleCount = moduleCount;
	PRINTK::PrintK("Allocating for %d modules.\r\n", moduleCount);
	for (int i = 0; i < moduleCount; i++) {
		info->modules[i] = (BootFile*)BOOTMEM::Malloc(sizeof(BootFile) + 1);
		info->modules[i]->address = moduleRequest.response->modules[i]->address;
		info->modules[i]->size = moduleRequest.response->modules[i]->size;
		info->modules[i]->path = moduleRequest.response->modules[i]->path;
		info->modules[i]->cmdline = moduleRequest.response->modules[i]->cmdline;
	}

	info->higherHalfMapping = hhdmRequest.response->offset;
	info->kernelPhysicalBase = kaddrRequest.response->physical_base;
	info->kernelVirtualBase = kaddrRequest.response->virtual_base;

	PRINTK::PrintK("Welcome from MicroK.\r\n"
		       "The kernel is booted by %s %s at %d\r\n",
		       bootloaderRequest.response->name,
		       bootloaderRequest.response->version,
		       timeRequest.response->boot_time);

	if(fbRequest.response == NULL) {
		info->fbPresent = false;
	} else {
		info->fbPresent = true;
		info->framebuffer = (Framebuffer*)BOOTMEM::Malloc(sizeof(MMapEntry) + 1);
		info->framebuffer->Address = fbRequest.response->framebuffers[0]->address;
		info->framebuffer->Width = fbRequest.response->framebuffers[0]->width;
		info->framebuffer->Height = fbRequest.response->framebuffers[0]->height;
		info->framebuffer->Bpp = fbRequest.response->framebuffers[0]->bpp;
		info->framebuffer->RedShift = fbRequest.response->framebuffers[0]->red_mask_shift;
		info->framebuffer->GreenShift = fbRequest.response->framebuffers[0]->green_mask_shift;
		info->framebuffer->BlueShift = fbRequest.response->framebuffers[0]->blue_mask_shift;
	}

	/* Launch the kernel proper */
	KernelStart();
}
