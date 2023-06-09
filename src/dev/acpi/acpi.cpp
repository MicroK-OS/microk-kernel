#include <dev/acpi/acpi.hpp>
#include <init/kinfo.hpp>
#include <sys/printk.hpp>
#include <mm/vmm.hpp>

#include <arch/x64/dev/acpi.hpp>

ACPI::SDTHeader *xsdt;

namespace ACPI {
SDTHeader *GetXSDT() {
	return xsdt;
}
/* ACPI Initialization function */
void Init() {
	KInfo *info = GetInfo();
	PRINTK::PrintK("RSDP at: 0x%x\r\n", info->RSDP);

	RSDP2 *rsdp = info->RSDP - info->higherHalfMapping;

	PRINTK::PrintK("Loading the XSDT table from 0x%x.\r\n", rsdp->XSDTAddress);
	xsdt = (SDTHeader*)(rsdp->XSDTAddress);

	//x86_64::LoadMADT();
}

/* Function that finds a table given the XSDT and the signature */
void *FindTable(SDTHeader *sdtHeader, char *signature) {
	/* Finding the number of entries */
        int entries = (sdtHeader->Length - sizeof(ACPI::SDTHeader) ) / 8;
        for (int i = 0; i < entries; i++) {
		/* Getting the table header */
                ACPI::SDTHeader *newSDTHeader = (ACPI::SDTHeader*)*(uint64_t*)((uint64_t)sdtHeader + sizeof(ACPI::SDTHeader) + (i * 8));

		/* Checking if the signatures match */
                for (int j = 0; j < 4; j++) {
                        if(newSDTHeader->Signature[j] != signature[j]) break;
			/* If so, then return the table header */
                        if(j == 3) return newSDTHeader;
                }
        }

	/* The request table hasn't been found */
        return NULL;
}

}
