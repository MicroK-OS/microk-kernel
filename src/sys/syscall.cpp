#include <sys/syscall.hpp>
#include <stdarg.h>
#include <stdint.h>
#include <cdefs.h>
#include <init/kinfo.hpp>
#include <mm/pmm.hpp>
#include <sys/printk.hpp>

// TMP measure: do something better with SMP
__attribute__((__aligned__((16)))) char SyscallStack[64 * 1024];
extern "C" void *StartSyscallStack = &SyscallStack[64 * 1024 - 1];

void HandleSyscallDebugPrintK(char *string);

void HandleSyscallMemoryGetinfo(uintptr_t structbase);
void HandleSyscallMemoryVmalloc(uintptr_t base, size_t length, size_t flags);
void HandleSyscallMemoryVmfree(uintptr_t base, size_t length);
void HandleSyscallMemoryMmap(uintptr_t src, uintptr_t dest, size_t length, size_t flags);
void HandleSyscallMemoryUnmap(uintptr_t base, size_t length);

void HandleSyscallProcExec(size_t TODO);
void HandleSyscallProcFork(size_t TODO);
void HandleSyscallProcReturn(size_t returnCode, uintptr_t stack);
void HandleSyscallProcExit(size_t exitCode, uintptr_t stack);

extern "C" void HandleSyscall(size_t syscallNumber, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5) {
	switch(syscallNumber) {
		case SYSCALL_DEBUG_PRINTK: return HandleSyscallDebugPrintK(arg1);
		case SYSCALL_MEMORY_GETINFO: return HandleSyscallMemoryGetinfo(arg1);
		case SYSCALL_MEMORY_VMALLOC: return HandleSyscallMemoryVmalloc(arg1, arg2, arg3);
		case SYSCALL_MEMORY_VMFREE: return HandleSyscallMemoryVmfree(arg1, arg2);
		case SYSCALL_MEMORY_MMAP: return HandleSyscallMemoryMmap(arg1, arg2, arg3, arg4);
		case SYSCALL_MEMORY_UNMAP: return HandleSyscallMemoryUnmap(arg1, arg2);
		case SYSCALL_PROC_EXEC: return HandleSyscallProcExec(arg1);
		case SYSCALL_PROC_FORK: return HandleSyscallProcFork(arg1);
		case SYSCALL_PROC_RETURN: return HandleSyscallProcReturn(arg1, arg2);
		case SYSCALL_PROC_EXIT: return HandleSyscallProcExit(arg1, arg2);
	}
}

void HandleSyscallDebugPrintK(char *string) {
	PRINTK::PrintK("%s", string);
}

void HandleSyscallMemoryGetinfo(uintptr_t structbase) {
	if (structbase <= 0x1000 || structbase >= 0x00007FFFFFFFFFFF)
		return; /* Make sure it is in valid memory */

	size_t *data = structbase;
	data[0] = PMM::GetFreeMem() + PMM::GetUsedMem();  /* Total */
	data[1] = PMM::GetFreeMem(); /* Free */
	data[2] = PMM::GetUsedMem(); /* Reserved (todo, get correct amount) */
	data[3] = 0; /* Buffers */
}

void HandleSyscallMemoryVmalloc(uintptr_t base, size_t length, size_t flags) {
	/* Process: check if all arguments are valid, allocate and map */
}

void HandleSyscallMemoryVmfree(uintptr_t base, size_t length) {
	/* Process: check if all arguments are valid, just clear those pages and free in the bitmap*/
}

void HandleSyscallMemoryMmap(uintptr_t src, uintptr_t dest, size_t length, size_t flags) {
	/* Process: check if all arguments are valid, switch to kernel address space and map pages */
}

void HandleSyscallMemoryUnmap(uintptr_t base, size_t length) {
	/* Process: check if all arguments are valid, just clear those pages*/
}

void HandleSyscallProcExec(size_t TODO) {
	// TODO
}

void HandleSyscallProcFork(size_t TODO) {
	// TODO
}

void HandleSyscallProcReturn(size_t returnCode, uintptr_t stack) {
	PRINTK::PrintK("Returning: %d form 0x%x\r\n", returnCode, stack); 

	PRINTK::PrintK("Back to the kernel.\r\n");

	while(true);
}

void HandleSyscallProcExit(size_t exitCode, uintptr_t stack) {
	PRINTK::PrintK("Exiting: %d form 0x%x\r\n", exitCode, stack); 
}
