#include "far/far.h"
#include "hope/hope.h"

#if 0
void test_hmm_3profs(void);
void test_hmm_empty(void);
void test_hmm_corrupted1(void);
void test_hmm_corrupted2(void);
void test_hmm_corrupted3(void);
void test_hmm_corrupted4(void);
void test_hmm_corrupted5(void);
void test_hmm_corrupted6(void);
void test_hmm_corrupted7(void);
void test_hmm_corrupted8(void);

void check_3profs0(struct far_prof *prof);
void check_3profs1(struct far_prof *prof);
void check_3profs2(struct far_prof *prof);

void (*check_prof[3])(struct far_prof *prof) = {
    check_3profs0,
    check_3profs1,
    check_3profs2,
};
#endif

int main(void)
{
#if 0
    test_hmm_3profs();
    test_hmm_empty();
    test_hmm_corrupted1();
    test_hmm_corrupted2();
    test_hmm_corrupted3();
    test_hmm_corrupted4();
    test_hmm_corrupted5();
    test_hmm_corrupted6();
    test_hmm_corrupted7();
    test_hmm_corrupted8();
#endif
    return hope_status();
}

#if 0
void test_hmm_3profs(void)
{
    FILE *fd = fopen(ASSETS "/three-profs.hmm", "r");
    NOTNULL(fd);
    unsigned symbol_size = 20;
    unsigned prof_size[] = {40, 235, 449};

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    unsigned prof_idx = 0;
    enum far_rc rc = FAR_SUCCESS;
    while (!(rc = far_next_prof(&far, &prof)))
    {
        EQ(prof.symbols_size, symbol_size);
        unsigned node_idx = 0;
        while (!(rc = far_next_node(&far, &prof)))
        {
            EQ(prof.node.idx, node_idx);
            check_prof[prof_idx](&prof);
            node_idx++;
        }
        EQ(prof.node.idx, prof_size[prof_idx]);
        EQ(far_prof_length(&prof), prof_size[prof_idx]);
        prof_idx++;
    }
    EQ(prof_idx, 3);

    fclose(fd);
}

void test_hmm_empty(void)
{
    FILE *fd = fopen(ASSETS "/empty.hmm", "r");
    NOTNULL(fd);

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    EQ(far_next_prof(&far, &prof), FAR_ENDFILE);
    EQ(far_next_prof(&far, &prof), FAR_RUNTIMEERROR);
    EQ(far.error, "Runtime error: unexpected prof_next_prof call");

    fclose(fd);
}

void test_hmm_corrupted1(void)
{
    FILE *fd = fopen(ASSETS "/corrupted1.hmm", "r");
    NOTNULL(fd);

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    unsigned prof_idx = 0;
    enum far_rc rc = FAR_SUCCESS;
    while (!(rc = far_next_prof(&far, &prof)))
    {
        unsigned node_idx = 0;
        while (!(rc = far_next_node(&far, &prof)))
        {
            node_idx++;
        }
        if (prof_idx == 2 && node_idx == 6)
        {
            EQ(rc, FAR_PARSEERROR);
            EQ(far.error, "Parse error: unexpected end-of-file: line 563");
        }
        if (prof_idx == 2)
            EQ(node_idx, 6);
        prof_idx++;
    }
    EQ(prof_idx, 3);
    EQ(rc, FAR_RUNTIMEERROR);

    fclose(fd);
}

void test_hmm_corrupted2(void)
{
    FILE *fd = fopen(ASSETS "/corrupted2.hmm", "r");
    NOTNULL(fd);
    unsigned prof_size[] = {40};

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    unsigned prof_idx = 0;
    enum far_rc rc = FAR_SUCCESS;
    while (!(rc = far_next_prof(&far, &prof)))
    {
        unsigned node_idx = 0;
        while (!(rc = far_next_node(&far, &prof)))
        {
            node_idx++;
        }
        EQ(prof.node.idx, prof_size[prof_idx]);
        EQ(far_prof_length(&prof), prof_size[prof_idx]);
        prof_idx++;
        EQ(rc, FAR_ENDNODE);
    }
    EQ(prof_idx, 1);
    EQ(rc, FAR_PARSEERROR);
    EQ(far.error, "Parse error: expected content before end-of-line: line 153");

    fclose(fd);
}

void test_hmm_corrupted3(void)
{
    FILE *fd = fopen(ASSETS "/corrupted3.hmm", "r");
    NOTNULL(fd);
    unsigned prof_size[] = {40};

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    unsigned prof_idx = 0;
    enum far_rc rc = FAR_SUCCESS;
    while (!(rc = far_next_prof(&far, &prof)))
    {
        unsigned node_idx = 0;
        while (!(rc = far_next_node(&far, &prof)))
        {
            node_idx++;
        }
        EQ(prof.node.idx, prof_size[prof_idx]);
        EQ(far_prof_length(&prof), 0);
        prof_idx++;
    }
    EQ(prof_idx, 0);
    EQ(rc, FAR_PARSEERROR);
    EQ(far.error, "Parse error: missing LENG field");

    fclose(fd);
}

void test_hmm_corrupted4(void)
{
    FILE *fd = fopen(ASSETS "/corrupted4.hmm", "r");
    NOTNULL(fd);

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    unsigned prof_idx = 0;
    enum far_rc rc = FAR_SUCCESS;
    while (!(rc = far_next_prof(&far, &prof)))
    {
        unsigned node_idx = 0;
        while (!(rc = far_next_node(&far, &prof)))
        {
            node_idx++;
        }
        EQ(rc, FAR_PARSEERROR);
        EQ(far.error, "Parse error: profile length mismatch: line 33");
        EQ(node_idx, 3);
        EQ(prof.node.idx, 2);
        EQ(far_prof_length(&prof), 40);
        prof_idx++;
    }
    EQ(prof_idx, 1);
    EQ(rc, FAR_ENDFILE);

    fclose(fd);
}

void test_hmm_corrupted5(void)
{
    FILE *fd = fopen(ASSETS "/corrupted5.hmm", "r");
    NOTNULL(fd);

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    enum far_rc rc = FAR_SUCCESS;
    rc = far_next_prof(&far, &prof);
    EQ(rc, FAR_PARSEERROR);
    EQ(far.error, "Parse error: unexpected token: line 4");
    rc = far_next_node(&far, &prof);
    EQ(rc, FAR_RUNTIMEERROR);
    EQ(far.error, "Runtime error: unexpected prof_next_node call");

    fclose(fd);
}

void test_hmm_corrupted6(void)
{
    FILE *fd = fopen(ASSETS "/corrupted6.hmm", "r");
    NOTNULL(fd);

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    enum far_rc rc = FAR_SUCCESS;
    rc = far_next_prof(&far, &prof);
    EQ(rc, FAR_SUCCESS);
    rc = far_next_node(&far, &prof);
    EQ(rc, FAR_PARSEERROR);
    EQ(far.error, "Parse error: failed to parse decimal number: line 25");

    fclose(fd);
}

void test_hmm_corrupted7(void)
{
    FILE *fd = fopen(ASSETS "/corrupted7.hmm", "r");
    NOTNULL(fd);

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    enum far_rc rc = FAR_SUCCESS;
    rc = far_next_prof(&far, &prof);
    EQ(rc, FAR_PARSEERROR);
    EQ(far.error, "Parse error: invalid header: line 1");
    rc = far_next_node(&far, &prof);
    EQ(rc, FAR_RUNTIMEERROR);
    EQ(far.error, "Runtime error: unexpected prof_next_node call");

    fclose(fd);
}

void test_hmm_corrupted8(void)
{
    FILE *fd = fopen(ASSETS "/corrupted8.hmm", "r");
    NOTNULL(fd);

    FAR_DECLARE(far, fd);

    FAR_PROF_DECLARE(prof, &far);

    enum far_rc rc = FAR_SUCCESS;
    rc = far_next_prof(&far, &prof);
    EQ(rc, FAR_PARSEERROR);
    EQ(far.error, "Parse error: expected i->i: line 9");
    rc = far_next_node(&far, &prof);
    EQ(rc, FAR_RUNTIMEERROR);
    EQ(far.error, "Runtime error: unexpected prof_next_node call");

    fclose(fd);
}
void check_3profs0(struct far_prof *prof)
{
    if (prof->node.idx == 0)
    {
        CLOSE(prof->node.compo[0], -2.29746);
        CLOSE(prof->node.compo[prof->symbols_size - 1], -3.82158);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.00201);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 2], -0.0);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 1], -INFINITY);
    }
    if (prof->node.idx == 1)
    {
        CLOSE(prof->node.match[0], -0.34643);
        CLOSE(prof->node.match[prof->symbols_size - 1], -7.58384);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.00201);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 1], -0.95510);
    }
    if (prof->node.idx == 40)
    {
        CLOSE(prof->node.match[0], -3.29199);
        CLOSE(prof->node.match[prof->symbols_size - 1], -3.78781);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.00135);
        CLOSE(prof->node.trans[2], -INFINITY);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 1], -INFINITY);
    }
}

void check_3profs1(struct far_prof *prof)
{
    if (prof->node.idx == 0)
    {
        CLOSE(prof->node.compo[0], -2.47491);
        CLOSE(prof->node.compo[prof->symbols_size - 1], -3.46896);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.02633);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 1], -INFINITY);
    }
    if (prof->node.idx == 1)
    {
        CLOSE(prof->node.match[0], -3.26601);
        CLOSE(prof->node.match[prof->symbols_size - 1], -4.14252);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.02633);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 1], -0.95510);
    }
    if (prof->node.idx == 235)
    {
        CLOSE(prof->node.match[0], -2.75686);
        CLOSE(prof->node.match[prof->symbols_size - 1], -3.85418);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.01780);
        CLOSE(prof->node.trans[2], -INFINITY);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 1], -INFINITY);
    }
}

void check_3profs2(struct far_prof *prof)
{
    if (prof->node.idx == 0)
    {
        CLOSE(prof->node.compo[0], -2.55148);
        CLOSE(prof->node.compo[prof->symbols_size - 1], -3.24305);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.01335);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 1], -INFINITY);
    }
    if (prof->node.idx == 1)
    {
        CLOSE(prof->node.match[0], -2.77993);
        CLOSE(prof->node.match[prof->symbols_size - 1], -2.88211);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.01335);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 1], -0.95510);
    }
    if (prof->node.idx == 449)
    {
        CLOSE(prof->node.match[0], -3.39753);
        CLOSE(prof->node.match[prof->symbols_size - 1], -4.58563);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.00900);
        CLOSE(prof->node.trans[2], -INFINITY);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[FAR_TRANS_SIZE - 1], -INFINITY);
    }
}
#endif
