#include <libdummycrt/thread.h>
#include <libdummycrt/string.h>
#include <stdint.h>
#include <stddef.h>
#include <sel4/sel4.h>

#include "tls.h"

/* From linker script. */
extern char __tdata_start;
extern char __tdata_end;
extern char __tbss_end;

/* TLS variables */
__thread seL4_IPCBuffer *__sel4_ipc_buffer;
uint64_t global_tid;

void sel4cp_thread_setup(uint64_t tid, int (*entry)(void))
{
    const uintptr_t tdata_start = (uintptr_t)&__tdata_start;
    const uintptr_t tdata_end = (uintptr_t)&__tdata_end;
    const uintptr_t tbss_end = (uintptr_t)&__tbss_end;

    /* Determine our TLS environment. */
    const struct tls_info tls_env = {
        .tls_template = (void *)tdata_start,
        .tls_template_size = tbss_end - tdata_start,
        .tls_tdata_size = tdata_end - tdata_start,
        .align = sizeof(seL4_Word), // @andyb: this should be p_align in the ELF Phdr
    };

    const uintptr_t thread_tls_memory =  VSPACE_TLS_START + tid * tls_env.tls_template_size;

    /* Copy the TLS template to this thread's TLS memory area. */
    const uintptr_t tp = copy_tls(&tls_env, thread_tls_memory);
    if (!tp) {
        return;
    }

    /* Now that we have a valid tp, set in the arch-defined TLS register. */
    arch_set_tp(tp);

    /* Set any TLS variables we need. */
    const uintptr_t ipc_buffer = VSPACE_IPCBUFF_START + tid * VSPACE_IPCBUFF_SIZE;
    __sel4_ipc_buffer = (seL4_IPCBuffer *)ipc_buffer;
    global_tid = tid;
    /* Call the actual thread entry.
     * NOTE: Do nothing with the return value for now.
     */
    (void) entry();
}
