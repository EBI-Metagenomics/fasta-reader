#include "error.h"
#include "aux.h"
#include "far/aux.h"
#include "far/tgt.h"
#include "far/tok.h"
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#define PARSE_ERROR "Parse error: "
#define RUNTIME_ERROR "Runtime error: "
#define LINE ": line"

static int copy_fmt(int dst_size, char *dst, char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(dst, (size_t)dst_size, fmt, ap);
    va_end(ap);
    assert(n >= 0);
    return n;
}

static int copy_ap(int dst_size, char *dst, char const *fmt, va_list ap)
{
    int n = vsnprintf(dst, (size_t)dst_size, fmt, ap);
    assert(n >= 0);
    return n;
}

#define unused(x) ((void)(x))

enum fasta_rc error_io(char *dst, int errnum)
{
    int rc = strerror_r(errnum, dst, FAR_ERROR_SIZE);
    assert(!rc);
    unused(rc);
    return FAR_IOERROR;
}

enum fasta_rc error_runtime(char *dst, char const *fmt, ...)
{
    int n = copy_fmt(FAR_ERROR_SIZE, dst, RUNTIME_ERROR);
    va_list ap;
    va_start(ap, fmt);
    copy_ap(FAR_ERROR_SIZE - n, dst + n, fmt, ap);
    va_end(ap);
    return FAR_RUNTIMEERROR;
}

enum fasta_rc __error_parse_target(struct fasta_target *tgt, char const *fmt,
                                   ...)
{
    int n = copy_fmt(FAR_ERROR_SIZE, tgt->error, PARSE_ERROR);
    va_list ap;
    va_start(ap, fmt);
    copy_fmt(FAR_ERROR_SIZE - n, tgt->error + n, fmt, ap);
    va_end(ap);
    return FAR_PARSEERROR;
}

enum fasta_rc __error_parse_tok(struct fasta_tok *tok, char const *fmt, ...)
{
    int n = copy_fmt(FAR_ERROR_SIZE, tok->error, PARSE_ERROR);
    va_list ap;
    va_start(ap, fmt);
    n += copy_ap(FAR_ERROR_SIZE - n, tok->error + n, fmt, ap);
    va_end(ap);
    copy_fmt(FAR_ERROR_SIZE - n, tok->error + n, "%s %d", LINE,
             tok->line.number);
    return FAR_PARSEERROR;
}
