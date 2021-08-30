#include "aux.h"
#include "far/aux.h"
#include <stdlib.h>

void aux_init(struct far_aux *aux)
{
    aux->prof.begin = NULL;
    aux->prof.pos = NULL;
    aux->prof.end = NULL;
    aux->node.begin = NULL;
    aux->node.pos = NULL;
    aux->node.end = NULL;
    aux->idx = 0;
}
