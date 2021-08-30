#include "node.h"
#include "far/node.h"
#include <math.h>

void far_node_dump(struct far_node const *node, FILE *restrict fd)
{
    if (node->idx == 0)
        fprintf(fd, "COMPO");
    else
        fprintf(fd, "%*u", 5, node->idx);
    for (unsigned i = 0; i < node->symbols_size; ++i)
        fprintf(fd, " %.5f", node->compo[i]);
    fprintf(fd, "\n");

    fprintf(fd, "     ");
    for (unsigned i = 0; i < node->symbols_size; ++i)
        fprintf(fd, " %.5f", node->insert[i]);
    fprintf(fd, "\n");

    fprintf(fd, "     ");
    for (unsigned i = 0; i < FAR_TRANS_SIZE; ++i)
        fprintf(fd, " %.5f", node->trans[i]);
    fprintf(fd, "\n");
}

void node_init(struct far_node *node)
{
    node->symbols_size = 0;
    node->idx = 0;
    for (unsigned i = 0; i < FAR_SYMBOLS_MAX; ++i)
    {
        node->compo[i] = NAN;
        node->insert[i] = NAN;
    }
    for (unsigned i = 0; i < FAR_TRANS_SIZE; ++i)
        node->trans[i] = NAN;
}
