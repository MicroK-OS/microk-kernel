#include <proc/process.hpp>
#include <mm/memory.hpp>
#include <arch/x64/cpu/stack.hpp>

#include <sys/printk.hpp>
#include <sys/panic.hpp>
#include <mm/pmm.hpp>

namespace PROC {
static uint64_t CurrentPID = -1; /* This may seem dumb, but it actually is absolutely fine */

static inline uint64_t RequestPID() {
	return ++CurrentPID;
}

Process::Process(ProcessType type, VMM::VirtualSpace *vms) {
	PID = RequestPID();
	State = P_PAUSED;
	Type = type;
	VirtualMemorySpace = vms;

	HighestFree = 0x0000800000000000;

	LastTID = 0;
	ThreadNumber = 0;
	MainThread = NULL;
	Threads.Init();

}

Process::~Process() {
	if (State != P_DONE) {
		/* The process hasn't been properly dismantled */
		OOPS("Process not properly dismantled");
	}
}

size_t Process::CreateThread(size_t stackSize, uintptr_t entrypoint) {
	size_t newTID;
	
	if(GetProcessState() != P_PAUSED) return 0;

	Thread *newThread = new Thread(this, stackSize, entrypoint, &newTID);

	Threads.Push(newThread);

	return newTID;
}

Thread *Process::GetThread(size_t TID) {
	Thread *thread = Threads.Get(TID - 1);

	return thread;
}

size_t Process::RequestTID() {
	return ++LastTID;
}

void Process::DestroyThread(Thread *thread) {
	if(thread == NULL) return;

}

void Process::SetThreadState(size_t TID, ProcessState state) {
	Thread *thread = Threads.Get(TID - 1);

	if (thread == NULL) return;

	switch(state) {
		case P_DONE: {
			DestroyThread(thread);
			}
			break;
		default:
			thread->SetState(state);
			break;

	}
}

ProcessState Process::GetThreadState(size_t TID) {
	Thread *thread = Threads.Get(TID - 1);
	if (thread == NULL) return P_DONE;

	return thread->GetState();
}

void Process::SetMainThread(size_t TID) {
	Thread *thread = Threads.Get(TID - 1);
	if (thread == NULL) return;

	if (thread->GetState() != P_DONE) MainThread = thread;	
}

Thread *Process::GetMainThread() {
	return MainThread;
}

void Process::SetPriority(uint8_t priority) {
	Priority = priority;
}

void Process::SetProcessState(ProcessState state) {
	switch(state) {
		case P_DONE: {
			delete VirtualMemorySpace;
			State = state;
			}
			break;
		default:
			State = state;
			break;
	}
}

size_t Process::GetPID() {
	return PID;
}

ProcessState Process::GetProcessState() {
	return State;
}

VMM::VirtualSpace *Process::GetVirtualMemorySpace() {
	return VirtualMemorySpace;
}

size_t Process::GetHighestFree() {
	return HighestFree;
}

void Process::SetHighestFree(size_t highestFree) {
	HighestFree = highestFree;
}


Thread::Thread(Process *process, size_t stackSize, uintptr_t entrypoint, size_t *newTID) {
	if (stackSize == 0) stackSize = (512 * 1024);

	TID = process->RequestTID();
	Owner = process;
	Instruction = entrypoint;
	State = P_PAUSED;

	stackSize -= stackSize % 16;

	if (newTID != NULL) *newTID = GetTID();
			

	size_t highestFree = process->GetHighestFree();
	VMM::VirtualSpace *space = process->GetVirtualMemorySpace();

	switch(process->GetType()) {
		case PT_USER: {
			for (uintptr_t i = highestFree - stackSize; i < highestFree; i+= PAGE_SIZE) {
				VMM::MapMemory(space, (void*)PMM::RequestPage(), (void*)i);
			}

			StackBase = Stack = highestFree;

			process->SetHighestFree((highestFree - stackSize) - (highestFree - stackSize) % 16);
			}
			break;
		case PT_KERNEL: {
			}
			break;
	}
}

Thread::~Thread() {
	if (State != P_DONE) {
		OOPS("Thread properly dismantled");
	}
}

void Thread::SetState(ProcessState state) {
	switch(state) {
		default:
			State = state;
			break;
	}
}

ProcessState Thread::GetState() {
	return State;
}

size_t Thread::GetTID() {
	return TID;
}

void Thread::SetInstruction(uintptr_t instruction) {
	Instruction = instruction;
}

void Thread::SetStack(uintptr_t stack) {
	Stack = stack;
}

uintptr_t Thread::GetStackBase() {
	return StackBase;
}

uintptr_t Thread::GetStack() {
	return Stack;
}

size_t Thread::GetStackSize() {
	return StackSize;
}

}
