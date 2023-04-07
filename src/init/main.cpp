/*
   File: init/main.cpp
   __  __  _                _  __        ___   ___
  |  \/  |(_) __  _ _  ___ | |/ /       / _ \ / __|
  | |\/| || |/ _|| '_|/ _ \|   <       | (_) |\__ \
  |_|  |_||_|\__||_|  \___/|_|\_\       \___/ |___/

  MicroKernel, a simple futiristic Unix-shattering kernel
  Copyright (C) 2022-2023 Mutta Filippo

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <cdefs.h>
#include <stdint.h>
#include <stddef.h>
#include <mm/pmm.hpp>
#include <mm/heap.hpp>
#include <proc/smp.hpp>
#include <sys/user.hpp>
#include <init/main.hpp>
#include <sys/panic.hpp>
#include <mm/memory.hpp>
#include <mm/bootmem.hpp>
#include <init/kinfo.hpp>
#include <sys/atomic.hpp>
#include <sys/printk.hpp>
#include <init/modules.hpp>
#include <arch/x64/main.hpp>
#include <proc/scheduler.hpp>

/*
   Function that is started by the scheduler once kernel startup is complete.
*/
void RestInit();

int EarlyKernelInit() {
	/* Allocating memory for the info struct */
	InitInfo();

	/* Loading early serial printk */
	PRINTK::EarlyInit();

	return 0;
}

void PrintBanner() {
	/* Getting some useful system info */
	KInfo *info = GetInfo();

	/* Printing banner */
	PRINTK::PrintK(" __  __  _                _  __\r\n"
		       "|  \\/  |(_) __  _ _  ___ | |/ /\r\n"
		       "| |\\/| || |/ _|| '_|/ _ \\|   < \r\n"
		       "|_|  |_||_|\\__||_|  \\___/|_|\\_\\\r\n"
		       "The operating system for the future...at your fingertips.\r\n"
		       "%s %s Started.\r\n"
		       "System stats:\r\n"
		       "  Memory:\r\n"
		       "   Physical: %dkb free out of %dkb (%dkb used).\r\n"
		       "   Virtual: Kernel at 0x%x.\r\n"
		       "   Bootmem: %dkb out of %dkb (%dkb used).\r\n"
		       "   Heap: %dkb free out of %dkb (%dkb used).\r\n"
		       "  CPUs:\r\n"
		       "   SMP Status: %s\r\n"
		       "   CPUs in the system: %d\r\n",
		       CONFIG_KERNEL_CNAME,
		       CONFIG_KERNEL_CVER,
			PMM::GetFreeMem() / 1024,
			(PMM::GetFreeMem() + PMM::GetUsedMem()) / 1024,
			PMM::GetUsedMem() / 1024,
			info->kernelVirtualBase,
			BOOTMEM::GetFree() / 1024,
			BOOTMEM::GetTotal() / 1024,
			(BOOTMEM::GetTotal() - BOOTMEM::GetFree()) / 1024,
			HEAP::GetFree() / 1024,
			HEAP::GetTotal() / 1024,
			(HEAP::GetTotal() - HEAP::GetFree()) / 1024,
			info->SMP.IsEnabled ? "Active" : "Not present",
			info->SMP.IsEnabled ? info->SMP.CpuCount : 1);
}

/*
   Main kernel function.
*/
void KernelStart() {
	PRINTK::PrintK("Kernel started.\r\n");

	/* Starting specific arch-dependent instructions */

#ifdef CONFIG_ARCH_x86_64
	/* Starting x8_64 specific instructions */
	x86_64::Init();
#endif

	/* Starting the memory subsystem */
	MEM::Init();

	/* Initializing the heap */
	HEAP::InitializeHeap(CONFIG_HEAP_BASE, CONFIG_HEAP_SIZE / 0x1000);
	/* With the heap initialize, disable new bootmem allocations */
	BOOTMEM::DeactivateBootmem();

	/* Initializing multiprocessing */
	PROC::InitSMP();

	/* Printing banner to show off */
	PrintBanner();

	/* Initializing the scheduler framework */
	PROC::Scheduler::Initialize();

	/* Starting the modules subsystem */
	PROC::Scheduler::StartKernelThread(MODULE::Init);

	/* Finishing kernel startup */
	PROC::Scheduler::StartKernelThread(RestInit);

	/* Starting the kernel scheduler by adding the root CPU */
	PROC::Scheduler::AddCPU();	

	/* We have finished operating */
	while (true) CPUPause();
}

extern "C" void UserFunction() {
	PRINTK::PrintK("Hello from userspace!\r\n");
	
	uint64_t *test = (uint64_t*)Malloc(128);
	test[0x69] = 0x69;
	PRINTK::PrintK("Number is 0x%x\r\n", test[0x69]);
	Free(test);

	PRINTK::PrintK("Halting system.\r\n");

	ExitUserspace();

	while (true) CPUPause();
}

void RestInit() {
	/* We are done with the boot process */
	PRINTK::PrintK("Kernel is now resting...\r\n");

	void *stack = PMM::RequestPage();
	void *func = &UserFunction;
	PRINTK::PrintK("Switching to userspace.\r\n"
	               " Function Address: 0x%x\r\n"
		       " Stack Address:    0x%x\r\n",
		       (uint64_t)func,
		       (uint64_t)stack);

	EnterUserspace(func, stack);

	PRINTK::PrintK("Returned from userspace.\r\n");

	while (true) {
		CPUPause();
	}
}
