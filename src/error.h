#ifndef ERROR_H
#define ERROR_H

#include "far/far.h"
#include "far/rc.h"

struct far_tgt;
struct far_tok;

enum far_rc error_io(char *dst, int errnum);
enum far_rc error_runtime(char *dst, char const *fmt, ...);

#define error_parse(x, ...)                                                    \
    _Generic((x), struct far_tok *                                             \
             : __error_parse_tok, struct far_prof *                            \
             : __error_parse_prof)(x, __VA_ARGS__)

enum far_rc __error_parse_prof(struct far_tgt *prof, char const *fmt, ...);
enum far_rc __error_parse_tok(struct far_tok *tok, char const *fmt, ...);

#endif
