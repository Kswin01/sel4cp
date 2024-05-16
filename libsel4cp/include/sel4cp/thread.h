#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <sel4/sel4.h>
#include <sel4cp/sel4cp.h>

static inline void
sel4cp_thread_restart(sel4cp_thread thread, uintptr_t entry_point)
{
    seL4_Error err;
    seL4_UserContext ctxt = {0};
    ctxt.pc = entry_point;
    err = seL4_TCB_WriteRegisters(
        THREAD_TCB(thread),
        true,
        0, /* No flags */
        1, /* writing 1 register */
        &ctxt
    );

    if (err != seL4_NoError) {
        sel4cp_dbg_puts("sel4cp_thread_restart: error writing registers\n");
        sel4cp_internal_crash(err);
    }
}

static inline void
sel4cp_thread_resume(sel4cp_thread thread)
{
    seL4_Error err = seL4_TCB_Resume(THREAD_TCB(thread));
    seL4_Assert(!err);
}

static inline void
sel4cp_thread_stop(sel4cp_thread thread)
{
    seL4_Error err;
    err = seL4_TCB_Suspend(THREAD_TCB(thread));
    if (err != seL4_NoError) {
        sel4cp_dbg_puts("sel4cp_thread_stop: error stopping thread\n");
        sel4cp_internal_crash(err);
    }
}

static sel4cp_errno
sel4cp_thread_set_sched_params(sel4cp_thread thread, uint64_t budget, uint64_t period, bool is_periodic)
{
    seL4_Error err;
    err = seL4_SchedControl_ConfigureFlags(
        SCHEDCONTROL_CPTR,
        THREAD_SC(thread),
        budget,
        period,
        0,
        0, /** FIXME: what should the badge be set to? for now we don't have timeout endpoints */
        (is_periodic) ? seL4_SchedContext_NoFlag : seL4_SchedContext_Sporadic
    );
    seL4_Assert(!err);

    return err;
}

static sel4cp_errno
sel4cp_thread_set_address_space(sel4cp_thread thread, sel4cp_pd pd)
{
    seL4_Error err;
    err = seL4_TCB_SetVSpace(
        THREAD_TCB(thread),
        PD_VSPACE(pd),
        0
    );
    seL4_Assert(!err);
    
    return err;
}

static sel4cp_errno
sel4cp_thread_set_priority(sel4cp_thread thread, sel4cp_thread authority, uint64_t priority)
{
    seL4_Error err;

    /* Use the PD's MCP */
    err = seL4_TCB_SetPriority(THREAD_TCB(thread), THREAD_TCB(authority), priority);
    seL4_Assert(!err);

    return err;
}

static sel4cp_errno
sel4cp_thread_set_entry_attr(sel4cp_thread thread, uintptr_t thread_entry, uintptr_t real_entry)
{
    seL4_Error err;
    seL4_UserContext ctxt;

    ctxt.pc = thread_entry;

    // unused, but I'm being verbose so nregisters make sense
    ctxt.sp = 0;
    ctxt.spsr = 0;

    ctxt.x0 = thread;
    ctxt.x1 = real_entry;
    ctxt.x2 = 0; /* Not root thread, set to 0 */
    size_t nregisters = 6;

    err = seL4_TCB_WriteRegisters(
        THREAD_TCB(thread),
        false,
        0,
        nregisters,
        &ctxt
    );
    seL4_Assert(!err);

    return err;
}

/* WARNING: calling this more than once on a thread _before_
 * it's been replied to is considered undefined behaviour. */
static sel4cp_errno
sel4cp_thread_swap_reply(sel4cp_thread thread)
{
    seL4_Error err;
    err = seL4_CNode_Rotate(
        SELF_CNODE_CPTR,
        THREAD_REPLY(thread),
        9,
        0,
        SELF_CNODE_CPTR,
        REPLY_CPTR,
        9,
        0,
        SELF_CNODE_CPTR,
        THREAD_REPLY(thread),
        9
    );
    return err;
}
