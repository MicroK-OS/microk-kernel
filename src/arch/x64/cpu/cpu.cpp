/*
   File: arch/x64/cpu/cpu.cpp
*/

#include <cpuid.h>
#include <sys/printk.hpp>
#include <arch/x64/cpu/cpu.hpp>
#include <sys/panic.hpp>
#include <stdint.h>

/* Enum containing processor features derived from cpuid */
enum {
	CPUID_FEAT_ECX_SSE3         = 1 << 0,
	CPUID_FEAT_ECX_PCLMUL       = 1 << 1,
	CPUID_FEAT_ECX_DTES64       = 1 << 2,
	CPUID_FEAT_ECX_MONITOR      = 1 << 3,
	CPUID_FEAT_ECX_DS_CPL       = 1 << 4,
	CPUID_FEAT_ECX_VMX          = 1 << 5,
	CPUID_FEAT_ECX_SMX          = 1 << 6,
	CPUID_FEAT_ECX_EST          = 1 << 7,
	CPUID_FEAT_ECX_TM2          = 1 << 8,
	CPUID_FEAT_ECX_SSSE3        = 1 << 9,
	CPUID_FEAT_ECX_CID          = 1 << 10,
	CPUID_FEAT_ECX_SDBG         = 1 << 11,
	CPUID_FEAT_ECX_FMA          = 1 << 12,
	CPUID_FEAT_ECX_CX16         = 1 << 13,
	CPUID_FEAT_ECX_XTPR         = 1 << 14,
	CPUID_FEAT_ECX_PDCM         = 1 << 15,
	CPUID_FEAT_ECX_PCID         = 1 << 17,
	CPUID_FEAT_ECX_DCA          = 1 << 18,
	CPUID_FEAT_ECX_SSE4_1       = 1 << 19,
	CPUID_FEAT_ECX_SSE4_2       = 1 << 20,
	CPUID_FEAT_ECX_X2APIC       = 1 << 21,
	CPUID_FEAT_ECX_MOVBE        = 1 << 22,
	CPUID_FEAT_ECX_POPCNT       = 1 << 23,
	CPUID_FEAT_ECX_TSC          = 1 << 24,
	CPUID_FEAT_ECX_AES          = 1 << 25,
	CPUID_FEAT_ECX_XSAVE        = 1 << 26,
	CPUID_FEAT_ECX_OSXSAVE      = 1 << 27,
	CPUID_FEAT_ECX_AVX          = 1 << 28,
	CPUID_FEAT_ECX_F16C         = 1 << 29,
	CPUID_FEAT_ECX_RDRAND       = 1 << 30,
	CPUID_FEAT_ECX_HYPERVISOR   = 1 << 31,

	CPUID_FEAT_EDX_FPU          = 1 << 0,
	CPUID_FEAT_EDX_VME          = 1 << 1,
	CPUID_FEAT_EDX_DE           = 1 << 2,
	CPUID_FEAT_EDX_PSE          = 1 << 3,
	CPUID_FEAT_EDX_TSC          = 1 << 4,
	CPUID_FEAT_EDX_MSR          = 1 << 5,
	CPUID_FEAT_EDX_PAE          = 1 << 6,
	CPUID_FEAT_EDX_MCE          = 1 << 7,
	CPUID_FEAT_EDX_CX8          = 1 << 8,
	CPUID_FEAT_EDX_APIC         = 1 << 9,
	CPUID_FEAT_EDX_SEP          = 1 << 11,
	CPUID_FEAT_EDX_MTRR         = 1 << 12,
	CPUID_FEAT_EDX_PGE          = 1 << 13,
	CPUID_FEAT_EDX_MCA          = 1 << 14,
	CPUID_FEAT_EDX_CMOV         = 1 << 15,
	CPUID_FEAT_EDX_PAT          = 1 << 16,
	CPUID_FEAT_EDX_PSE36        = 1 << 17,
	CPUID_FEAT_EDX_PSN          = 1 << 18,
	CPUID_FEAT_EDX_CLFLUSH      = 1 << 19,
	CPUID_FEAT_EDX_DS           = 1 << 21,
	CPUID_FEAT_EDX_ACPI         = 1 << 22,
	CPUID_FEAT_EDX_MMX          = 1 << 23,
	CPUID_FEAT_EDX_FXSR         = 1 << 24,
	CPUID_FEAT_EDX_SSE          = 1 << 25,
	CPUID_FEAT_EDX_SSE2         = 1 << 26,
	CPUID_FEAT_EDX_SS           = 1 << 27,
	CPUID_FEAT_EDX_HTT          = 1 << 28,
	CPUID_FEAT_EDX_TM           = 1 << 29,
	CPUID_FEAT_EDX_IA64         = 1 << 30,
	CPUID_FEAT_EDX_PBE          = 1 << 31
};

/* Call to asm function that activate SSE
   (should only execuded after checking if SSE is actually present) */
extern "C" void ActivateSSE();
extern "C" void ActivateAVX();
extern "C" void EnableSCE(void *syscallEntrypoint);
extern "C" void SyscallEntry();

namespace x86_64 {
/*
   Function that initializes the x86CPU class
*/
void x86CPU::Init() {
	/* We check for SSE first of all */
        if(CheckSSE(1)) {
		PRINTK::PrintK("SSE status: Present but not active\r\n");
		/* If it is present, we activate it */
		ActivateSSE();
		sseStatus = true;
		PRINTK::PrintK("SSE status: Active\r\n");

		if(CheckSSE(2)) PRINTK::PrintK("SSE status: SSE 2 present\r\n");

		if(CheckSSE(3) == 1) PRINTK::PrintK("SSE status: SSE 3 present\r\n");
		else if (CheckSSE(3) == 2) PRINTK::PrintK("SSE status SSE and SSSE 3 present\r\n");

		if(CheckSSE(4)) PRINTK::PrintK("SSE status: SSE 4 present\r\n");

		if(CheckAVX()) {
			PRINTK::PrintK("AVX status: Present but not active\r\n");
			ActivateAVX();

			PRINTK::PrintK("AVX status: Active\r\n");
		}
        } else {
		PRINTK::PrintK("SSE status: Not present\r\n");

		/* Otherwise, we disable SSE */
		sseStatus = false;
	}

	if(CheckMSR()) {
		PRINTK::PrintK("MSRs available.\r\n");
		PRINTK::PrintK("Syscall entry at 0x%x\r\n", &SyscallEntry);
		EnableSCE(&SyscallEntry);
	} else {
		PANIC("MSRs are not available");
	}
}

/*
   Function that tests the presence of SSE via CPUID;
*/
int x86CPU::CheckSSE(int version) {
	unsigned int eax, ebx, ecx, edx;
	__cpuid(1, eax, ebx, ecx, edx);

	int result = 0;

	switch (version) {
		case 1:
			if((bool)(edx & CPUID_FEAT_EDX_SSE)) result = 1;
			break;
		case 2:
			if((bool)(edx & CPUID_FEAT_EDX_SSE2)) result = 1;
			break;
		case 3:
			if((bool)(edx & CPUID_FEAT_ECX_SSE3)) {
				result = 1;
				if((bool)(edx & CPUID_FEAT_ECX_SSSE3))
					result = 2;
			}
			break;
		case 4:
			if((bool)(ecx & CPUID_FEAT_ECX_SSE4_1)) {
				result = 1;
				if((bool)(ecx & CPUID_FEAT_ECX_SSE4_2))
					result = 2;
			}
			break;
		default:
			break;
	}

	return result;
}

bool x86CPU::CheckAVX() {
	unsigned int eax, unused, ecx;
	__cpuid(1, eax, unused, ecx, unused);
	return ecx & CPUID_FEAT_ECX_AVX;
}

bool x86CPU::CheckMSR() {
	unsigned int eax, unused, edx;
	__cpuid(1, eax, unused, unused, edx);
	return edx & CPUID_FEAT_EDX_MSR;
}

/*
   Function that gets the CPU vendor string from CPUID
*/
void x86CPU::GetVendor(char *string) {
	uint32_t ebx, edx, ecx, unused;
	__cpuid(0, unused, ebx, ecx, edx);

	/* This bitshift logic is used to get the correct 8-bit characters
	   from the 32 bit registers. The vendor name is 12 characters + \0 */
	string[0] = (uint8_t)ebx;
	string[1] = (uint8_t)(ebx >> 8);
	string[2] = (uint8_t)(ebx >> 16);
	string[3] = (uint8_t)(ebx >> 24);
	string[4] = (uint8_t)edx;
	string[5] = (uint8_t)(edx >> 8);
	string[6] = (uint8_t)(edx >> 16);
	string[7] = (uint8_t)(edx >> 24);
	string[8] = (uint8_t)ecx;
	string[9] = (uint8_t)(ecx >> 8);
	string[10] = (uint8_t)(ecx >> 16);
	string[11] = (uint8_t)(ecx >> 24);
	string[12] = '\0';
}
}
