#include "error.h"
#include "aux.h"
#include "bug.h"
#include "far/aux.h"
#include "far/prof.h"
#include "far/tok.h"
#include <stdarg.h>
#include <string.h>

#define PARSE_ERROR "Parse error: "
#define RUNTIME_ERROR "Runtime error: "
#define LINE ": line"

static int copy_fmt(int dst_size, char *dst, char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(dst, dst_size, fmt, ap);
    va_end(ap);
    BUG(n < 0);
    return n;
}

static int copy_ap(int dst_size, char *dst, char const *fmt, va_list ap)
{
    int n = vsnprintf(dst, dst_size, fmt, ap);
    BUG(n < 0);
    return n;
}

enum far_rc error_io(char *dst, int errnum)
{
    int rc = strerror_r(errnum, dst, FAR_ERROR_SIZE);
    BUG(rc);
    return FAR_IOERROR;
}

enum far_rc error_runtime(char *dst, char const *fmt, ...)
{
    int n = copy_fmt(FAR_ERROR_SIZE, dst, RUNTIME_ERROR);
    va_list ap;
    va_start(ap, fmt);
    copy_ap(FAR_ERROR_SIZE - n, dst + n, fmt, ap);
    va_end(ap);
    return FAR_RUNTIMEERROR;
}

enum far_rc __error_parse_prof(struct far_prof *prof, char const *fmt, ...)
{
    int n = copy_fmt(FAR_ERROR_SIZE, prof->error, PARSE_ERROR);
    va_list ap;
    va_start(ap, fmt);
    copy_fmt(FAR_ERROR_SIZE - n, prof->error + n, fmt, ap);
    va_end(ap);
    return FAR_PARSEERROR;
}

enum far_rc __error_parse_tok(struct far_tok *tok, char const *fmt, ...)
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
