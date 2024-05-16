#include <stdint.h>
#include <stddef.h>

#include <sel4cp/sel4cp.h>
#include <sel4cp/thread.h>

#define SYSCALL_THREAD_CREATE1 1
#define SYSCALL_THREAD_BLOCK 2
#define SYSCALL_THREAD_RELEASE 3
#define SYSCALL_THREAD_CREATE2 4

void
init(void)
{
}

bool protected(bool is_child, sel4cp_identifier identifier, sel4cp_msginfo *msginfo)
{
    seL4_Assert(is_child);

    sel4cp_thread thread = (sel4cp_thread)identifier;

    uint64_t syscall = sel4cp_msginfo_get_label(*msginfo);
    switch (syscall) {
    case SYSCALL_THREAD_CREATE1: {
                // seL4_DebugSnapshot();
        sel4cp_thread new_thread = 1; // only PD is id 0, so the thread is 1

        uint64_t budget = 200;
        uint64_t period = 1000;

        sel4cp_thread_set_sched_params(new_thread, budget, period, true);

        /* In the beginning, only threads that run are the initial PDs themselves,
         * and so PD ID == thread ID for the initial PDs unless we stop their
         * threads of execution, in which case their ID becomes free.
         * 
         * This is the OS' job to manage. */
        sel4cp_pd pd = thread;

        sel4cp_thread_set_address_space(new_thread, pd);

        uint64_t priority = 199;

        sel4cp_thread_set_priority(thread, pd, priority);

        uintptr_t thread_entry = (uintptr_t)sel4cp_mr_get(0);
        uintptr_t real_entry = (uintptr_t)sel4cp_mr_get(1);

        sel4cp_thread_set_entry_attr(new_thread, thread_entry, real_entry);

        sel4cp_thread_resume(new_thread);
        break;
    }
    case SYSCALL_THREAD_BLOCK: {
        sel4cp_dbg_puts("os: thread requested to block.\n");
        sel4cp_thread_swap_reply(0);
        return true;
    }
    case SYSCALL_THREAD_RELEASE:
        sel4cp_dbg_puts("os: release\n");
        sel4cp_thread_swap_reply(0);
        sel4cp_dbg_puts("os: swapped replies\n");
    default:
        break;
    }

    *msginfo = sel4cp_msginfo_new(0, 0);
    return true;
}

void
notified(sel4cp_channel ch)
{
}

bool fault(sel4cp_thread thread, sel4cp_msginfo *msginfo, bool is_timeout)
{
    if (is_timeout) {
        sel4cp_dbg_puts("timeout fault detected, do nothing\n");
    } else {
        seL4_UserContext regs;
        seL4_TCB_ReadRegisters(THREAD_TCB(0), false, 0, 1, &regs);
        sel4cp_thread_restart(0, regs.pc + 4);
    }
    
    *msginfo = sel4cp_msginfo_new(0, 0);
    return true;
}
