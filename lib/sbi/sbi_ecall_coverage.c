#include <sbi/sbi_coverage.h>
#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_version.h>
#include <sbi/riscv_asm.h>

#define SBI_EXT_COV_TEST        0x0

// static unsigned long trace_log[4096] __attribute__((section(".data")));
// static unsigned int trace_index __attribute__((section(".data")));
// static unsigned int trace_enabled __attribute__((section(".data")));

#define MAX_TRACE_LOG 8192 * (sizeof(unsigned long)) // 64kb

static unsigned long trace_log[MAX_TRACE_LOG];
static unsigned int trace_index = 0;
volatile int trace_enabled;

static void __attribute__((no_instrument_function)) trace_pc(unsigned long pc) 
{
    if (trace_index < MAX_TRACE_LOG) {
        trace_log[trace_index++] = pc;
    }
}

void __attribute__((no_instrument_function, noinline))
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
    if (!trace_enabled) {
        return ;
    }
    trace_pc((unsigned long)this_fn);
}

void __attribute__((no_instrument_function))
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
}

struct sbi_ecall_extension ecall_coverage;

static int __attribute__((no_instrument_function)) sbi_ecall_coverage_handler(unsigned long extid, unsigned long funcid,
    struct sbi_trap_regs *regs,
    struct sbi_ecall_return *out)
{
    int ret = 0;

    switch (funcid) {
        case SBI_EXT_COV_TEST:
            out->value = 0xdeadbeef;
            break;
        default:
        ret = SBI_ENOTSUPP;
    }

    return ret;
}

static int __attribute__((no_instrument_function)) sbi_ecall_coverage_register_extensions(void)
{
	return sbi_ecall_register_extension(&ecall_coverage);
}

struct sbi_ecall_extension ecall_coverage = {
	.name			= "cov",
	.extid_start		= SBI_EXT_COV,
	.extid_end		= SBI_EXT_COV,
	.register_extensions	= sbi_ecall_coverage_register_extensions,
	.handle			= sbi_ecall_coverage_handler,
};


