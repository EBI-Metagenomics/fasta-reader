#ifndef FAR_AUX_H
#define FAR_AUX_H

struct far_aux
{
    union
    {
        struct
        {
            char *begin;
            char *pos;
            char *end;
        } prof;

        struct
        {
            double *begin;
            double *pos;
            double *end;
        } node;
    };
    unsigned idx;
};

#endif
