#include <arch/x64/dev/acpi.hpp>
#include <sys/panic.hpp>
#include <sys/printk.hpp>
#include <stddef.h>
#include <arch/x64/dev/apic.hpp>
#include <arch/x64/dev/ioapic.hpp>

namespace x86_64 {
void LoadMADT() {
	MADTHeader *madt = FindTable(ACPI::GetXSDT(), (char*)"APIC");
	if (madt == 0) PANIC("MADT not fount");

	PRINTK::PrintK("MADT\r\n"
			" -> LAPIC: 0x%x\r\n"
			" -> Flags: 0x%x\r\n",
			madt->LAPICAddress,
			madt->Flags);

	x86_64::EnableAPIC();

	for (MADTEntryHeader *ptr = (uintptr_t)madt->EntriesBegin; (uintptr_t)ptr < (uintptr_t)madt + madt->Header.Length; ptr += ptr->Length) {
		switch (ptr->Type) {
			case 0: {
				PLAPICEntry *lapic = (uintptr_t)ptr + sizeof(MADTEntryHeader);
				PRINTK::PrintK("-> PL APIC\r\n"
						"     Processor ID: %d\r\n"
						"     APIC ID:      %d\r\n"
						"     Flags:        %d\r\n",
						lapic->ACPIProcessorID, lapic->APICID, lapic->Flags);
				/* IPI Startup code TODO
				if(lapic->ACPIProcessorID != 0) {
					void *base = GetAPICBase();
					*((volatile uint32_t*)(base + 0x280)) = 0;
					*((volatile uint32_t*)(base + 0x310)) = 0;
					*((volatile uint32_t*)(base + 0x300)) = 0;

					do {
						__asm__ __volatile__ ("pause" : : : "memory");
					} while(*((volatile uint32_t*)(base + 0x300)) & (1 << 12));

					*((volatile uint32_t*)(base + 0x310)) = 0;
					*((volatile uint32_t*)(base + 0x300)) = 0;

					do {
						__asm__ __volatile__ ("pause" : : : "memory");
					} while(*((volatile uint32_t*)(base + 0x300)) & (1 << 12));

					for(int j = 0; j < 2; j++) {
						*((volatile uint32_t*)(base + 0x280)) = 0;
						*((volatile uint32_t*)(base + 0x310)) = (*((volatile uint32_t*)(base + 0x310)) & 0x00ffffff) | (lapic->ACPIProcessorID << 24);
						*((volatile uint32_t*)(base + 0x300)) = (*((volatile uint32_t*)(base + 0x300)) & 0xfff0f800) | 0x000608;

						do {
							__asm__ __volatile__ ("pause" : : : "memory");
						} while(*((volatile uint32_t*)(base + 0x300)) & (1 << 12));
					}	
				}*/
				}
				break;
			case 1: {
				IOAPICEntry *ioapic = (uintptr_t)ptr + sizeof(MADTEntryHeader);
				PRINTK::PrintK("-> IO APIC\r\n"
						"     IOAPIC ID:                    %d\r\n"
						"     IOAPIC Address:               0x%x\r\n"
						"     Global System Interrupt Base: %d\r\n",
						ioapic->IOAPICID, ioapic->IOAPICAddress, ioapic->GlobalSystemInterruptBase);
				EnableIOAPIC(ioapic->IOAPICAddress);
				}
				break;
			case 2:
				PRINTK::PrintK("-> IOAPIC Interrupt Source Override\r\n");
				break;
			case 4:
				PRINTK::PrintK("-> LAPIC Interrupts\r\n");
				break;
			case 5:
				PRINTK::PrintK("-> LAPIC Address Override\r\n");
				break;
			case 9:
				PRINTK::PrintK("-> x2APIC\r\n");
				break;
			default:
				PRINTK::PrintK("-> Unknown\r\n");
				break;
		}
	}
}
}
