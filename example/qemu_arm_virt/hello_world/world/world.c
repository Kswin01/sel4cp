#include <stdint.h>
#include <sel4cp/sel4cp.h>

#define SYSCALL_THREAD_RELEASE 3

void
init(void)
{
    // for (;;) {
    //     sel4cp_dbg_puts("PARTITION 2: world world world world\n");
    //     // seL4_DebugDumpScheduler();
    // }
    sel4cp_dbg_puts("THREAD 2: Releasing thread 1\n");
    sel4cp_root_ppcall(sel4cp_msginfo_new(SYSCALL_THREAD_RELEASE, 0));
}

void
notified(sel4cp_channel ch)
{
}