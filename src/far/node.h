#ifndef FAR_NODE_H
#define FAR_NODE_H

#include "far/export.h"
#include <stdio.h>

#define FAR_SYMBOLS_MAX 32
#define FAR_TRANS_SIZE 7

struct far_node
{
    unsigned symbols_size;
    unsigned idx;
    union
    {
        double compo[FAR_SYMBOLS_MAX];
        double match[FAR_SYMBOLS_MAX];
    };
    double insert[FAR_SYMBOLS_MAX];
    double trans[FAR_TRANS_SIZE];
};

FAR_API void far_node_dump(struct far_node const *node, FILE *restrict fd);

#endif
