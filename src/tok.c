#include "far/tok.h"
#include "error.h"
#include "far/error.h"
#include "tok.h"
#include <string.h>

#define DELIM " \t\r"

static void add_space_before_newline(char line[FAR_TOK_LINE_MAX]);
static enum far_rc next_line(FILE *restrict fd, char error[FAR_ERROR_SIZE],
                             char line[FAR_TOK_LINE_MAX]);

void tok_init(struct far_tok *tok, char *error)
{
    tok->id = FAR_TOK_NEWLINE;
    tok->value = tok->line.data;
    memset(tok->line.data, '\0', FAR_TOK_LINE_MAX);
    tok->line.number = 0;
    tok->line.consumed = true;
    tok->line.ctx = NULL;
    tok->error = error;
}

enum far_rc tok_next(struct far_tok *tok, FILE *restrict fd)
{
    enum far_rc rc = FAR_SUCCESS;

    if (tok->line.consumed)
    {
        if ((rc = next_line(fd, tok->error, tok->line.data)))
        {
            if (rc == FAR_ENDFILE)
            {
                tok->value = NULL;
                tok->id = FAR_TOK_EOF;
                tok->line.data[0] = '\0';
                return FAR_SUCCESS;
            }
            return rc;
        }
        tok->value = strtok_r(tok->line.data, DELIM, &tok->line.ctx);
        tok->line.number++;

        if (!tok->value) return FAR_PARSEERROR;
    }
    else
        tok->value = strtok_r(NULL, DELIM, &tok->line.ctx);

    if (!strcmp(tok->value, "\n"))
        tok->id = FAR_TOK_NEWLINE;
    else if (!strcmp(tok->value, "//"))
        tok->id = FAR_TOK_SLASH;
    else if (!strcmp(tok->value, "HMM"))
        tok->id = FAR_TOK_HMM;
    else if (!strcmp(tok->value, "COMPO"))
        tok->id = FAR_TOK_COMPO;
    else
        tok->id = FAR_TOK_WORD;

    tok->line.consumed = tok->id == FAR_TOK_NEWLINE;

    return FAR_SUCCESS;
}

static enum far_rc next_line(FILE *restrict fd, char error[FAR_ERROR_SIZE],
                             char line[FAR_TOK_LINE_MAX])
{
    if (!fgets(line, FAR_TOK_LINE_MAX - 1, fd))
    {
        if (feof(fd)) return FAR_ENDFILE;

        return error_io(error, ferror(fd));
    }

    add_space_before_newline(line);
    return FAR_SUCCESS;
}

static void add_space_before_newline(char line[FAR_TOK_LINE_MAX])
{
    unsigned n = (unsigned)strlen(line);
    if (n > 0)
    {
        if (line[n - 1] == '\n')
        {
            line[n - 1] = ' ';
            line[n] = '\n';
            line[n + 1] = '\0';
        }
        else
        {
            line[n - 1] = '\n';
            line[n] = '\0';
        }
    }
}