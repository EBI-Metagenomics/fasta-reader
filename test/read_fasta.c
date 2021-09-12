#include "far/far.h"
#include "hope/hope.h"

void test_read_empty(void);
void test_read_mix(void);
void test_read_damaged1(void);
void test_read_damaged2(void);
void test_read_damaged3(void);
void test_write_mix(void);

int main(void)
{
    test_read_empty();
    test_read_mix();
    test_read_damaged1();
    test_read_damaged2();
    test_read_damaged3();
    test_write_mix();
    return hope_status();
}

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

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static char *mix_id[] = {"LCBO", "MCHU", "gi|5524211|gb|AAD44166.1|",
                         "gi|5524211|gb|AAD44166.1|"};

static char *mix_desc[] = {
    "- Prolactin precursor - Bovine",
    "- Calmodulin - Human, rabbit, bovine, rat, and chicken",
    "cytochrome b [Elephas maximus maximus]",
    "cytochrome b [Elephas maximus maximus]"};

static char *mix_seq[] = {
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

void test_read_mix(void)
{
    FILE *fd = fopen(ASSETS "/mix.faa", "r");
    NOTNULL(fd);

    FAR_DECLARE(far, fd);
    FAR_TGT_DECLARE(tgt, &far);

    unsigned i = 0;
    enum far_rc rc = FAR_SUCCESS;
    while (!(rc = far_next_tgt(&far, &tgt)))
    {
        EQ(tgt.id, mix_id[i]);
        EQ(tgt.desc, mix_desc[i]);
        EQ(tgt.seq, mix_seq[i]);
        i++;
    }
    EQ(i, 4);
    EQ(rc, FAR_ENDFILE);

    fclose(fd);
}

void test_read_damaged1(void)
{
    FILE *fd = fopen(ASSETS "/damaged1.faa", "r");
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
    EQ(rc, FAR_PARSEERROR);
    EQ(far.error, "Parse error: unexpected token: line 1");
    far_clear_error(&far);
    EQ(far.error, "");

    fclose(fd);
}

void test_read_damaged2(void)
{
    FILE *fd = fopen(ASSETS "/damaged2.faa", "r");
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
    EQ(rc, FAR_PARSEERROR);
    EQ(far.error, "Parse error: unexpected id: line 2");
    far_clear_error(&far);
    EQ(far.error, "");

    fclose(fd);
}

void test_read_damaged3(void)
{
    FILE *fd = fopen(ASSETS "/damaged3.faa", "r");
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
    EQ(rc, FAR_PARSEERROR);
    EQ(far.error, "Parse error: unexpected token: line 4");
    far_clear_error(&far);
    EQ(far.error, "");

    fclose(fd);
}

void test_write_mix(void)
{
    FAR_DECLARE(far, NULL);
    FAR_TGT_DECLARE(tgt, &far);

    for (unsigned i = 0; i < ARRAY_SIZE(mix_id); ++i)
    {
        far_tgt_write(mix_id[i], mix_desc[i], mix_seq[i], 60, stdout);
    }
}
