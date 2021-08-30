#ifndef BUG_H
#define BUG_H

#ifdef NDEBUG
#define BUG(cond)
#else
#define BUG(cond)                                                              \
    do                                                                         \
    {                                                                          \
        if (!(cond)) break;                                                    \
        __bug(__FILE__, __LINE__, #cond);                                      \
    } while (0)
#endif

void __bug(char const *file, int line, char const *cond)
    __attribute__((noreturn));

#endif
