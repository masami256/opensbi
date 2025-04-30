#include <sbi/sbi_coverage.h>
#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_version.h>
#include <sbi/sbi_string.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_console.h>
#define SBI_EXT_COV_TEST                0x0

// ecall func ids
#define SBI_FUZZ_CMD_INIT_TRACE_BUF         0x1
#define SBI_FUZZ_CMD_TRACE_START            0x2
#define SBI_FUZZ_CMD_TRACE_STOP             0x3
#define SBI_FUZZ_CMD_TRACE_LOG_COUNT        0x4
#define SBI_FUZZ_CMD_RESET_TRACE_LOG        0x5
#define SBI_FUZZ_CMD_COPY_TRACE_LOG         0x6
#define SBI_FUZZ_CMD_UNMAP_TRACE_LOG        0x7
#define SBI_FUZZ_CMD_TEST_WRITE             0x80
#define SBI_FUZZ_CMD_TEST_ERROR             0x90

static unsigned long *trace_log;
static unsigned long trace_log_size;

static unsigned int trace_index = 0;
volatile int trace_enabled;

struct sbi_ecall_extension ecall_coverage;

static void __attribute__((no_instrument_function)) sbi_fuzz_trace_enabled(int enabled)
{
    trace_enabled = enabled;
}

static void __attribute__((no_instrument_function)) trace_pc(unsigned long pc) 
{
    if (trace_index < trace_log_size) {
        trace_log[trace_index++] = pc;
    }
}

static void __attribute__((no_instrument_function)) sbi_fuzz_test_write(void)
{
    trace_log[trace_index++] = 0xcafebabe;
}

static int __attribute__((no_instrument_function)) sbi_fuzz_cov_copy_data(struct sbi_trap_regs *regs, struct sbi_ecall_return *out) 
{
    return SBI_OK;
}

static void __attribute__((no_instrument_function)) sbi_fuzz_init_buffer(struct sbi_trap_regs *regs) 
{
    trace_log = (void *) regs->a0;
    trace_log_size = regs->a1;
    trace_index = 0;
    trace_enabled = 0;
}

static void __attribute__((no_instrument_function))  sbi_fuzz_reset_trace_log(void)
{
    sbi_memset(trace_log, 0x0, trace_log_size);
    trace_index = 0;
}

static void __attribute__((no_instrument_function)) sbi_fuzz_unmap_trace_log(void)
{
    trace_log = NULL;
    trace_log_size = 0;
    trace_index = 0;
    trace_enabled = 0;
}

static void __attribute__((no_instrument_function)) sbi_fuzz_test_error()
{
    char *ptr = NULL;
    sbi_printf("Run sbi_fuzz_test_error\n");

    sbi_strcpy(ptr, "hello world, this is a test\n");
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

static int __attribute__((no_instrument_function)) sbi_ecall_coverage_handler(unsigned long extid, unsigned long funcid,
    struct sbi_trap_regs *regs,
    struct sbi_ecall_return *out)
{
    int ret = SBI_OK;
    switch (funcid) {
        case SBI_EXT_COV_TEST:
            out->value = 0xdeadbeef;
            break;
        case SBI_FUZZ_CMD_INIT_TRACE_BUF:
            sbi_fuzz_init_buffer(regs);
            break;
        case SBI_FUZZ_CMD_TRACE_START:
            sbi_fuzz_trace_enabled(1);
            break;
        case SBI_FUZZ_CMD_TRACE_STOP:
            sbi_fuzz_trace_enabled(0);
            break;
        case SBI_FUZZ_CMD_TRACE_LOG_COUNT:
            out->value = trace_index;
            break;
        case SBI_FUZZ_CMD_RESET_TRACE_LOG:
            sbi_fuzz_reset_trace_log();
        case SBI_FUZZ_CMD_COPY_TRACE_LOG:
            ret = sbi_fuzz_cov_copy_data(regs, out);
            break;
        case SBI_FUZZ_CMD_UNMAP_TRACE_LOG:
            sbi_fuzz_unmap_trace_log();
            break;
        case SBI_FUZZ_CMD_TEST_WRITE:
            sbi_fuzz_test_write();
            break;
        case SBI_FUZZ_CMD_TEST_ERROR:
            sbi_fuzz_test_error();
        default:
            ret = SBI_ENOTSUPP;
            break;
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


