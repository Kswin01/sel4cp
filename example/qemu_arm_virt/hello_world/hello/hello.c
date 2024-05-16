#include <stdint.h>
#include <sel4cp/sel4cp.h>

#define SYSCALL_THREAD_CREATE 1
#define SYSCALL_THREAD_BLOCK 2
#define SYSCALL_THREAD_RELEASE 3

void
worker_thread(void)
{
    // seL4_DebugSnapshot();
    for (int i = 0; i < 10; i++) {
        sel4cp_dbg_puts("PARTITION 1: IM IN THREAD 2 IM IN THREAD 2 IM IN THREAD 2 IM IN THREAD 2\n");
    }
    sel4cp_dbg_puts("THREAD 2: Going to release\n");
    sel4cp_msginfo msginfo = sel4cp_root_ppcall(sel4cp_msginfo_new(SYSCALL_THREAD_RELEASE, 0));
    seL4_Assert(sel4cp_msginfo_get_label(msginfo) == 0);
    sel4cp_dbg_puts("THREAD 2: We got replied to, we should not be here!\n");
}

void
dummy_thread(void)
{
    sel4cp_dbg_puts("THREAD3: Doing random stuff here\n");
    while(1) {
        asm("nop");
    }
}

void
init(void)
{
    extern void sel4cp_thread_entry(sel4cp_thread thread, bool is_root_thread);

    sel4cp_dbg_puts("PARTITION 1: SPAWN NEW THREAD\n");
    sel4cp_mr_set(0, (uintptr_t)sel4cp_thread_entry);
    sel4cp_mr_set(1, (uintptr_t)worker_thread);
    sel4cp_msginfo msginfo = sel4cp_root_ppcall(sel4cp_msginfo_new(SYSCALL_THREAD_CREATE, 2));
    seL4_Assert(sel4cp_msginfo_get_label(msginfo) == 0);

    sel4cp_dbg_puts("PARTITION 1: SPAWN NEW THREAD\n");
    sel4cp_mr_set(0, (uintptr_t)sel4cp_thread_entry);
    sel4cp_mr_set(1, (uintptr_t)dummy_thread);
    msginfo = sel4cp_root_ppcall(sel4cp_msginfo_new(4, 2));
    seL4_Assert(sel4cp_msginfo_get_label(msginfo) == 0);

    sel4cp_dbg_puts("THREAD 1: blocking myself\n");
    msginfo = sel4cp_root_ppcall(sel4cp_msginfo_new(SYSCALL_THREAD_BLOCK, 0));
    sel4cp_dbg_puts("THREAD 1: IM FREE AMERICA FUCK YEAH\n");
}

void
notified(sel4cp_channel ch)
{
}