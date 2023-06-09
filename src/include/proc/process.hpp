#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmm.hpp>
#include <sys/vector.hpp>

namespace PROC {
	enum ProcessState {
		P_READY,		/* Process is ready to run */
		P_RUNNING,		/* Process is currently running */
		P_WAITING,		/* Process is currently waiting for I/O */
		P_PAUSED,		/* Process is not currently running */
		P_DONE,			/* Process is no longer alive */
	};

	enum ProcessType {
		PT_KERNEL,		/* Process running in kernel mode */
		PT_USER,		/* Process running in user mode */
		PT_REALTIME,		/* A realtime process running in kernel mode, has the highest priority */
		PT_VM,			/* A virtual machine process, managed by the kernel's VM manager*/
	};

	class Process;
	class Thread;

	class Process {
	public:
		Process(ProcessType type, VMM::VirtualSpace *vms);
		~Process();

		size_t CreateThread(size_t stackSize, uintptr_t entrypoint);
		Thread *GetThread(size_t TID);
		size_t RequestTID();
		void DestroyThread(Thread *thread);

		void SetThreadState(size_t TID, ProcessState state);
		ProcessState GetThreadState(size_t TID);

		void SetMainThread(size_t TID);
		Thread *GetMainThread();

		void SetPriority(uint8_t priority);
		
		ProcessState GetProcessState();
		void SetProcessState(ProcessState state);
		
		size_t GetPID();
		VMM::VirtualSpace *GetVirtualMemorySpace();

		size_t GetHighestFree();
		void SetHighestFree(size_t highestFree);

		ProcessType GetType() { return Type; }
	private:
		size_t PID;
		ProcessState State;
		ProcessType Type;
		uint8_t Priority;
		
		VMM::VirtualSpace *VirtualMemorySpace;
		uintptr_t HighestFree;

		size_t LastTID;
		size_t ThreadNumber;

		Thread *MainThread;
		Vector<Thread*> Threads;
	};

	class Thread {
	public:
		Thread(Process *process, size_t stackSize, uintptr_t entrypoint, size_t *newTID);
		~Thread();
	
		void SetState(ProcessState state);
		void SetInstruction(uintptr_t instruction);
		void SetStack(uintptr_t stack);
		ProcessState GetState();

		size_t GetTID();
		uintptr_t GetStack();
		uintptr_t GetStackBase();
		size_t GetStackSize();
		uintptr_t GetInstruction() { return Instruction; }
	private:
		size_t TID;

		uintptr_t Stack;
		uintptr_t StackBase;
		uintptr_t Instruction;
		size_t StackSize;

		ProcessState State;

		Process *Owner;
	};
}
