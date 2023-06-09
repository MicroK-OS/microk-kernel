#include <sys/printk.hpp>
#include <debug/stack.hpp>
#include <stdint.h>

__attribute__((noreturn))
void Panic(const char *message, const char *file, const char *function, unsigned int line) {
        asm volatile ("cli"); // We don't want interrupts while we are panicking

        // Printing the panic message
	PRINTK::PrintK("\r\n\r\n [PANIC] -> KERNEL PANIC!! \r\n"
			" [PANIC] Irrecoverable error in the kernel.\r\n"
			" [PANIC] %s in function %s at line %d\r\n"
			" [PANIC] Cause: %s\r\n\r\n",
			file, function, line, message);

	UnwindStack(32);

	PRINTK::PrintK("\r\n [Hanging now...]\r\n\r\n");

        while (true) {
                // Halting forever
                asm volatile ("cli; hlt");
        }

}

void Oops(const char *message, const char *file, const char *function, unsigned int line) {
        asm volatile ("cli"); // We don't want interrupts while we are panicking

        // Printing the panic message
	PRINTK::PrintK("\r\n\r\n [OOPS] -> KERNEL OOPS!! \r\n"
			" [OOPS] Error in the kernel.\r\n"
			" [OOPS] %s in function %s at line %d\r\n"
			" [OOPS] Cause: %s\r\n\r\n",
			file, function, line, message);

	UnwindStack(32);

	PRINTK::PrintK("\r\n [Continuing execution...]\r\n\r\n");
}
