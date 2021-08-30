#include "far/far.h"
#include "hope/hope.h"

/* void test_read_empty(void); */
void test_read_mix(void);

int main(void)
{
    /* test_read_empty(); */
    test_read_mix();
    return hope_status();
}

#if 0
void test_read_empty(void)
{
    FILE *fd = fopen(ASSETS "/empty.faa", "r");
    NOTNULL(fd);

    FAR_DECLARE(far, fd);
    FAR_TGT_DECLARE(tgt, &far);

    unsigned i = 0;
    enum far_rc rc = FAR_SUCCESS;
    while (!(rc = far_next_tgt(&far, &tgt)))
    {
        i++;
    }
    EQ(i, 0);
    EQ(rc, FAR_ENDFILE);

    fclose(fd);
}
#endif

void test_read_mix(void)
{
    FILE *fd = fopen(ASSETS "/mix.faa", "r");
    NOTNULL(fd);
    char *id[] = {"LCBO", "MCHU", "gi|5524211|gb|AAD44166.1|",
                  "gi|5524211|gb|AAD44166.1|"};
    char *desc[] = {"- Prolactin precursor - Bovine",
                    "- Calmodulin - Human, rabbit, bovine, rat, and chicken",
                    "cytochrome b [Elephas maximus maximus]",
                    "cytochrome b [Elephas maximus maximus]"};
    char *seq[] = {
        "MDSKGSSQKGSRLLLLLVVSNLLLCQGVVSTPVCPNGPGNCQVSLRDLFDRAVMVSHYI"
        "HDLSSEMFNEFDKRYAQGKGFITMALNSCHTSSLPTPEDKEQAQQTHHEVLMSLILGLL"
        "RSWNDPLYHLVTEVRGMKGAPDAILSRAIEIEEENKRLLEGMEMIFGQVIPGAKETEPY"
        "PVWSGLPSLQTKDEDARYSAFYNLLHCLRRDSSKIDTYLKLLNCRIIYNNNC",
        "MADQLTEEQIAEFKEAFSLFDKDGDGTITTKELGTVMRSLGQNPTEAELQDMINEVDAD"
        "GNGTIDFPEFLTMMARKMKDTDSEEEIREAFRVFDKDGNGYISAAELRHVMTNLGEKLT"
        "DEEVDEMIREADIDGDGQVNYEEFVQMMTAK*",
        "LCLYTHIGRNIYYGSYLYSETWNTGIMLLLITMATAFMGYVLPWGQMSFWGATVITNLFSAIPYIGTNLV"
        "EWIWGGFSVDKATLNRFFAFHFILPFTMVALAGVHLTFLHETGSNNPLGLTSDSDKIPFHPYYTIKDFLG"
        "LLILILLLLLLALLSPDMLGDPDNHMPADPLNTPLHIKPEWYFLFAYAILRSVPNKLGGVLALFLSIVIL"
        "GLMPFLHTSKHRSMMLRPLSQALFWTLTMDLLTLTWIGSQPVEYPYTIIGQMASILYFSIILAFLPIAGX"
        "IENY",
        "LCLYTHIGRNIYYGSYLYSETWNTGIMLLLITMATAFMGYVLPWGQMSFWGATVITNLFSAIPYIGT"};

    FAR_DECLARE(far, fd);
    FAR_TGT_DECLARE(tgt, &far);

    unsigned i = 0;
    enum far_rc rc = FAR_SUCCESS;
    while (!(rc = far_next_tgt(&far, &tgt)))
    {
        EQ(tgt.id, id[i]);
        EQ(tgt.desc, desc[i]);
        EQ(tgt.seq, seq[i]);
        i++;
    }
    EQ(i, 4);

    fclose(fd);
}
