/*
 * Copyright 2021, Breakaway Consulting Pty. Ltd.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <sel4/sel4.h>

#include <sel4cp/sel4cp.h>

const size_t libsel4cp_max_threads;

/* SYSINIT needs to handle this */
#define UNBIND_SC_LABEL 0

#define BADGE_TYPE_BIT      62
#define     GET_BADGE_TYPE(x)       ((x) >> BADGE_TYPE_BIT) 

#define BADGE_TYPE_NTFN     0

#define BADGE_TYPE_FAULT    1
#define     BADGE_FAULT_TIMEOUT_BIT 60
#define     BADGE_FAULT_ROOT_BIT    61
#define     BADGE_FAULT_PD_MASK     0xff
#define     BADGE_FAULT_TID(x)      ((x) & 0xff)

#define BADGE_TYPE_PPC      2
#define     BADGE_PPC_CHANNEL_MASK  0x3f

#define BADGE_TYPE_ROOT_PPC 3
#define     BADGE_ROOT_PPC_TID_MASK 0xff

bool passive;
char sel4cp_name[16];
bool have_signal = false;
seL4_CPtr signal;
seL4_MessageInfo_t signal_msg;

__attribute__((weak)) bool protected(bool is_child, sel4cp_identifier identifier, sel4cp_msginfo *msginfo)
{
    *msginfo = seL4_MessageInfo_new(0, 0, 0, 0);
    return 0;
}

__attribute__((weak)) bool fault(sel4cp_thread thread, sel4cp_msginfo *msginfo, bool is_timeout)
{
    *msginfo = seL4_MessageInfo_new(0, 0, 0, 0);
    return 0;
}

extern void init(void);
extern void notified(sel4cp_channel);

static void
handler_loop(void)
{
    bool have_reply = false;

    for (;;) {
        seL4_Word badge;
        seL4_MessageInfo_t tag;

        if (have_reply) {
            tag = seL4_ReplyRecv(INPUT_CPTR, tag, &badge, REPLY_CPTR);
        } else if (have_signal) {
            tag = seL4_NBSendRecv(signal, signal_msg, INPUT_CPTR, &badge, REPLY_CPTR);
        } else {
            tag = seL4_Recv(INPUT_CPTR, &badge, REPLY_CPTR);
        }

        have_reply = false;

        if (GET_BADGE_TYPE(badge) == BADGE_TYPE_FAULT) {
            if ((badge >> BADGE_FAULT_ROOT_BIT) & 1) {
                /* Fault came from root pd, abort for now */
                seL4_Fail("sel4cp rootpd cannot handle its own fault yet");
            }

            seL4_Fault_t f = seL4_getArchFault(tag);
            bool is_timeout = (seL4_Fault_get_seL4_FaultType(f) == seL4_Fault_Timeout);

            have_reply = fault(BADGE_FAULT_TID(badge), &tag, is_timeout);
        } else if (GET_BADGE_TYPE(badge) == BADGE_TYPE_PPC) {
            have_reply = protected(false, badge & BADGE_PPC_CHANNEL_MASK, &tag);
        } else if (GET_BADGE_TYPE(badge) == BADGE_TYPE_ROOT_PPC) {
            sel4cp_dbg_puts("Doing rootppc\n");
            have_reply = protected(true, badge & BADGE_ROOT_PPC_TID_MASK, &tag);
            if(have_reply) {
                sel4cp_dbg_puts("Done with rootppc, we have a reply\n");
            } else {
                sel4cp_dbg_puts("Done with rootppc, no reply\n");
            }
        } else {
            unsigned int idx = 0;
            do  {
                if (badge & 1) {
                    notified(idx);
                }
                badge >>= 1;
                idx++;
            } while (badge != 0);
        }
    }
}

int
main(void)
{
    init();

    /* 
     If we are passive, now our initialisation is complete we can
     signal the monitor to unbind our scheduling context and bind
     it to our notification object. 
     We delay this signal so we are ready waiting on a recv() syscall
    */
    if (passive) {
        have_signal = true;
        signal_msg = seL4_MessageInfo_new(UNBIND_SC_LABEL, 0, 0, 1);
        seL4_SetMR(0, 0);
        signal = (SYSINIT_CPTR);
    }

    handler_loop();

    return 0;
}
