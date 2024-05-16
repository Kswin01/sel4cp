/*
 * Copyright 2021, Breakaway Consulting Pty. Ltd.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
/* seL4 Core Platform interface */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <sel4/sel4.h>

typedef unsigned int sel4cp_identifier;

typedef sel4cp_identifier sel4cp_channel;
typedef sel4cp_identifier sel4cp_pd;
typedef sel4cp_identifier sel4cp_thread;

typedef seL4_MessageInfo_t sel4cp_msginfo;

typedef unsigned int sel4cp_errno;

#define SEL4CP_MAX_CHILD_PDS (1)

enum pd_cspace_caps {
    /* Common between PD and its threads */
    ROOT_PD_EP_CPTR = 1, // Used for syscalls

    /* PD caps */
    REPLY_CPTR = 4,
    INPUT_CPTR,
    SELF_CNODE_CPTR,

    /* SchedControl cap to configure SchedContexts */
    SCHEDCONTROL_CPTR,

    /* For unbinding SCs, revisit... */
    SYSINIT_CPTR,

    BASE_OUTPUT_NTFN_CPTR = 10,
    BASE_OUTPUT_EP_CPTR = BASE_OUTPUT_NTFN_CPTR + 64,
    BASE_IRQ_CPTR = BASE_OUTPUT_EP_CPTR + 64,
    BASE_TCB_CPTR = BASE_IRQ_CPTR + 64
};

/* As configured by the user in the SDF. These macros help find a thread's object using
 * offsets into the CSpace. For more info, refer to BASE_TCB_CAP in tool/sel4coreplat/__main__.py */
extern const size_t libsel4cp_max_threads;
#define THREAD_TCB(thread)      (BASE_TCB_CPTR + (thread))
#define THREAD_SC(thread)       (BASE_TCB_CPTR + libsel4cp_max_threads + (thread))
#define THREAD_CSPACE(thread)   (BASE_TCB_CPTR + libsel4cp_max_threads * 2 + (thread))
#define THREAD_REPLY(thread)    (BASE_TCB_CPTR + libsel4cp_max_threads * 3 + (thread))
#define THREAD_PIVOT_REPLY      (BASE_TCB_CPTR + libsel4cp_max_threads * 3 + 4)
#define PD_VSPACE(pd)           (BASE_TCB_CPTR + libsel4cp_max_threads * 4 + (pd))

#define SEL4CP_MAX_CHANNELS 63

/* User provided functions */
void init(void);
void notified(sel4cp_channel ch);
bool protected(bool is_child, sel4cp_identifier identifier, sel4cp_msginfo *msginfo);
bool fault(sel4cp_thread thread, sel4cp_msginfo *msginfo, bool is_timeout);

extern char sel4cp_name[16];
/* These next three variables are so our PDs can combine a signal with the next Recv syscall */
extern bool have_signal;
extern seL4_CPtr signal;
extern seL4_MessageInfo_t signal_msg;

/*
 * Output a single character on the debug console.
 */
void sel4cp_dbg_putc(int c);

/*
 * Output a NUL terminated string to the debug console.
 */
void sel4cp_dbg_puts(const char *s);

static inline void
sel4cp_internal_crash(seL4_Error err)
{
    /*
     * Currently crash be dereferencing NULL page
     *
     * Actually derference 'err' which means the crash reporting will have
     * `err` as the fault address. A bit of a cute hack. Not a good long term
     * solution but good for now.
     */
    int *x = (int *)(uintptr_t) err;
    *x = 0;
}

static inline void
sel4cp_notify(sel4cp_channel ch)
{
    seL4_Signal(BASE_OUTPUT_NTFN_CPTR + ch);
}

static inline void
sel4cp_irq_ack(sel4cp_channel ch)
{
    seL4_IRQHandler_Ack(BASE_IRQ_CPTR + ch);
}

static inline sel4cp_msginfo
sel4cp_ppcall(sel4cp_channel ch, sel4cp_msginfo msginfo)
{
    return seL4_Call(BASE_OUTPUT_EP_CPTR + ch, msginfo);
}

static inline sel4cp_msginfo
sel4cp_root_ppcall(sel4cp_msginfo msginfo)
{
    return seL4_Call(ROOT_PD_EP_CPTR, msginfo);
}

static inline sel4cp_msginfo
sel4cp_msginfo_new(uint64_t label, uint16_t count)
{
    return seL4_MessageInfo_new(label, 0, 0, count);
}

static inline uint64_t
sel4cp_msginfo_get_label(sel4cp_msginfo msginfo)
{
    return seL4_MessageInfo_get_label(msginfo);
}

static inline uint64_t
sel4cp_msginfo_get_length(sel4cp_msginfo msginfo)
{
    return seL4_MessageInfo_get_length(msginfo);
}

static inline sel4cp_msginfo
sel4cp_msginfo_set_length(sel4cp_msginfo msginfo, uint64_t length)
{
    return seL4_MessageInfo_set_length(msginfo, length);
}

static void
sel4cp_mr_set(uint8_t mr, uint64_t value)
{
    seL4_SetMR(mr, value);
}

static uint64_t
sel4cp_mr_get(uint8_t mr)
{
    return seL4_GetMR(mr);
}
