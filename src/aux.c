#include "aux.h"
#include "far/aux.h"
#include <stdlib.h>

void aux_init(struct fasta_aux *aux)
{
    aux->begin = NULL;
    aux->pos = NULL;
    aux->end = NULL;
    aux->id[0] = '\0';
}
