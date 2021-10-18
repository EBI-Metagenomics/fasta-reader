#ifndef ERROR_H
#define ERROR_H

#include "far/far.h"
#include "far/rc.h"

struct fasta_target;
struct fasta_tok;

enum fasta_rc error_io(char *dst, int errnum);
enum fasta_rc error_runtime(char *dst, char const *fmt, ...);

#define error_parse(x, ...)                                                    \
    _Generic((x), struct fasta_tok *                                           \
             : __error_parse_tok, struct fasta_target *                        \
             : __error_parse_target)(x, __VA_ARGS__)

enum fasta_rc __error_parse_target(struct fasta_target *tgt, char const *fmt,
                                   ...);
enum fasta_rc __error_parse_tok(struct fasta_tok *tok, char const *fmt, ...);

#endif
