/**
 * @sdd-task: Task #1 - XOR trio select + gamedev relatives
 * @sdd-spec: specs/spec-013-r9w-adr-fill-gamedev-trio/spec.md
 * @sdd-decision: SDD-ADR-058 XOR; SDD-ADR-059 local is-dir; SDD-ADR-060 NULL=neither dir; SDD-ADR-061 no ALWAYS_SKIP
 * @sdd-why: Unit XOR / partial / unreadable / NULL=neither dir; existing sdd fixtures stay green
 * @human-debug: Dual-tree leftover sdd strings in generated means select mixed relatives. NULL
 * on empty .gamedev/ dir means presence treated as missing (file-at-path vs is-dir)
 */
#include "adr/adr_fill.h"
#include "pipeline/pipeline.h"
#include "test_framework.h"
#include "test_helpers.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

static char *af_mkroot(char *out, size_t out_sz) {
    char *td = th_mktempdir("cbm_adr_fill");
    if (!td) {
        return NULL;
    }
    snprintf(out, out_sz, "%s", td);
    return out;
}

static int af_write_trio(const char *root, const char *purpose, const char *stack,
                         const char *decisions) {
    if (purpose &&
        th_write_file(TH_PATH(root, ".sdd-skill/context_ai.md"), purpose) != 0) {
        return -1;
    }
    if (stack &&
        th_write_file(TH_PATH(root, ".sdd-skill/baseline/TECH_STACK.md"), stack) != 0) {
        return -1;
    }
    if (decisions && th_write_file(TH_PATH(root, ".sdd-skill/baseline/ARCHITECTURE_ADR.md"),
                                   decisions) != 0) {
        return -1;
    }
    return 0;
}

static int af_write_gamedev_trio(const char *root, const char *purpose, const char *stack,
                                 const char *decisions) {
    if (purpose &&
        th_write_file(TH_PATH(root, ".gamedev/game_context.md"), purpose) != 0) {
        return -1;
    }
    if (stack &&
        th_write_file(TH_PATH(root, ".gamedev/baseline/TECH_STACK.md"), stack) != 0) {
        return -1;
    }
    if (decisions && th_write_file(TH_PATH(root, ".gamedev/baseline/ARCHITECTURE_ADR.md"),
                                   decisions) != 0) {
        return -1;
    }
    return 0;
}

static char *af_between(const char *doc, const char *start_m, const char *end_m) {
    const char *s;
    const char *e;
    size_t n;
    char *out;

    if (!doc) {
        return NULL;
    }
    s = strstr(doc, start_m);
    if (!s) {
        return NULL;
    }
    s += strlen(start_m);
    e = strstr(s, end_m);
    if (!e) {
        return NULL;
    }
    n = (size_t)(e - s);
    out = malloc(n + 1);
    if (!out) {
        return NULL;
    }
    memcpy(out, s, n);
    out[n] = '\0';
    return out;
}

static bool af_ws_only(const char *s) {
    if (!s) {
        return true;
    }
    while (*s) {
        if (!isspace((unsigned char)*s)) {
            return false;
        }
        s++;
    }
    return true;
}

static void af_make_unreadable(const char *path) {
#ifndef _WIN32
    if (chmod(path, 0) == 0) {
        FILE *f = fopen(path, "rb");
        if (!f) {
            return;
        }
        fclose(f);
        (void)chmod(path, 0644);
    }
#endif
    (void)cbm_unlink(path);
    (void)th_mkdir_p(path);
}

TEST(adr_fill_migrate_unmarked_to_manual) {
    char root[256];
    char *out;
    char *manual;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_trio(root, "PURPOSE-ALPHA-GRAPH\n", "STACK-ALPHA-C11\n",
                            "DECISION-ALPHA-PATH\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/baseline/DEV_LOG.md"),
                            "SECRET-DEVLOG-ALPHA\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/baseline/TECH_DEBT.md"),
                            "SECRET-TECHDEBT\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/docs/constitution.md"),
                            "SECRET-CONSTITUTION\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/human/notes.md"), "SECRET-HUMAN\n"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/specs/x.md"), "SECRET-SPECS\n"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/history/decisions.log"),
                            "SECRET-HISTORY\n"),
              0);

    out = cbm_adr_fill_document(root, "# Existing ADR\n");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_END));
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_MANUAL_START));
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_MANUAL_END));
    ASSERT(strstr(out, CBM_ADR_GENERATED_START) < strstr(out, CBM_ADR_MANUAL_START));
    ASSERT_NOT_NULL(strstr(out, "PURPOSE-ALPHA-GRAPH"));
    ASSERT_NOT_NULL(strstr(out, "STACK-ALPHA-C11"));
    ASSERT_NOT_NULL(strstr(out, "DECISION-ALPHA-PATH"));
    ASSERT_NOT_NULL(strstr(out, "# Purpose"));
    ASSERT_NOT_NULL(strstr(out, "# Stack"));
    ASSERT_NOT_NULL(strstr(out, "# Decisions"));
    ASSERT_NULL(strstr(out, "SECRET-DEVLOG-ALPHA"));
    ASSERT_NULL(strstr(out, "SECRET-TECHDEBT"));
    ASSERT_NULL(strstr(out, "SECRET-CONSTITUTION"));
    ASSERT_NULL(strstr(out, "SECRET-HUMAN"));
    ASSERT_NULL(strstr(out, "SECRET-SPECS"));
    ASSERT_NULL(strstr(out, "SECRET-HISTORY"));

    manual = af_between(out, CBM_ADR_MANUAL_START, CBM_ADR_MANUAL_END);
    ASSERT_NOT_NULL(manual);
    ASSERT_NOT_NULL(strstr(manual, "# Existing ADR"));
    free(manual);
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_first_create_empty_manual) {
    char root[256];
    char *out;
    char *manual;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_trio(root, "PURPOSE-BETA-NEW\n", "STACK-BETA-NEW\n",
                            "DECISION-BETA-NEW\n"),
              0);

    out = cbm_adr_fill_document(root, NULL);
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(out, "PURPOSE-BETA-NEW"));
    ASSERT_NOT_NULL(strstr(out, "STACK-BETA-NEW"));
    ASSERT_NOT_NULL(strstr(out, "DECISION-BETA-NEW"));
    manual = af_between(out, CBM_ADR_MANUAL_START, CBM_ADR_MANUAL_END);
    ASSERT_NOT_NULL(manual);
    ASSERT_TRUE(af_ws_only(manual));
    free(manual);
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_partial_context_ai_only) {
    char root[256];
    char *out;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/baseline")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/context_ai.md"), "PURPOSE-PARTIAL-ONLY\n"),
              0);

    out = cbm_adr_fill_document(root, "");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(out, "PURPOSE-PARTIAL-ONLY"));
    ASSERT_NOT_NULL(strstr(out, "# Purpose"));
    ASSERT_NULL(strstr(out, "# Stack"));
    ASSERT_NULL(strstr(out, "# Decisions"));
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_unreadable_architecture_adr_omits) {
    char root[256];
    char *out;
    const char *arch;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_trio(root, "PURPOSE-READABLE\n", "STACK-READABLE\n",
                            "DECISION-UNREADABLE-FULL-COPY\n"),
              0);
    arch = TH_PATH(root, ".sdd-skill/baseline/ARCHITECTURE_ADR.md");
    af_make_unreadable(arch);

    out = cbm_adr_fill_document(root, "");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, "PURPOSE-READABLE"));
    ASSERT_NOT_NULL(strstr(out, "STACK-READABLE"));
    ASSERT_NULL(strstr(out, "DECISION-UNREADABLE-FULL-COPY"));
    ASSERT_NULL(strstr(out, "# Decisions"));
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_no_sdd_skill_leaves_unmarked) {
    char root[256];
    char *out;
    const char *existing = "# Hand only\n";

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    out = cbm_adr_fill_document(root, existing);
    ASSERT_NULL(out);
    ASSERT_NULL(strstr(existing, "CBM-GENERATED"));
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_skill_present_trio_all_missing) {
    char root[256];
    char *out;
    char *generated;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill")), 0);

    out = cbm_adr_fill_document(root, "# Keep me\n");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_END));
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_MANUAL_START));
    ASSERT_NULL(strstr(out, "# Purpose"));
    ASSERT_NULL(strstr(out, "# Stack"));
    ASSERT_NULL(strstr(out, "# Decisions"));
    generated = af_between(out, CBM_ADR_GENERATED_START, CBM_ADR_GENERATED_END);
    ASSERT_NOT_NULL(generated);
    ASSERT_TRUE(af_ws_only(generated));
    ASSERT_NOT_NULL(strstr(out, "# Keep me"));
    free(generated);
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_preserves_manual_on_refill) {
    char root[256];
    char *first;
    char *second;
    char *manual;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_trio(root, "PURPOSE-V1\n", "STACK-V1\n", "DECISION-V1\n"), 0);
    first = cbm_adr_fill_document(root, "# Old notes\n");
    ASSERT_NOT_NULL(first);

    ASSERT_EQ(af_write_trio(root, "PURPOSE-V2\n", "STACK-V2\n", "DECISION-V2\n"), 0);
    second = cbm_adr_fill_document(root, first);
    ASSERT_NOT_NULL(second);
    ASSERT_NOT_NULL(strstr(second, "PURPOSE-V2"));
    ASSERT_NULL(strstr(second, "PURPOSE-V1"));
    manual = af_between(second, CBM_ADR_MANUAL_START, CBM_ADR_MANUAL_END);
    ASSERT_NOT_NULL(manual);
    ASSERT_NOT_NULL(strstr(manual, "# Old notes"));
    ASSERT_NULL(strstr(manual, "PURPOSE-V2"));
    free(manual);
    free(first);
    free(second);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_empty_file_omits_h1) {
    char root[256];
    char *out;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/context_ai.md"), "PURPOSE-NONEMPTY\n"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/baseline/TECH_STACK.md"), ""), 0);
    ASSERT_EQ(
        th_write_file(TH_PATH(root, ".sdd-skill/baseline/ARCHITECTURE_ADR.md"), "DECISION-OK\n"),
        0);

    out = cbm_adr_fill_document(root, "");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, "PURPOSE-NONEMPTY"));
    ASSERT_NOT_NULL(strstr(out, "DECISION-OK"));
    ASSERT_NULL(strstr(out, "# Stack"));
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(pipeline_new_leaves_adr_fill_false) {
    char root[CBM_SZ_512];
    cbm_pipeline_t *p;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    p = cbm_pipeline_new(root, NULL, CBM_MODE_FAST);
    ASSERT_NOT_NULL(p);
    ASSERT_FALSE(cbm_pipeline_get_adr_fill(p));
    cbm_pipeline_free(p);
    th_cleanup(root);
    PASS();
}

TEST(pipeline_apply_adr_fill_false_leaves_prior) {
    char root[CBM_SZ_512];
    cbm_pipeline_t *p;
    char *saved;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_trio(root, "PURPOSE-WATCHER-SHOULD-NOT-FILL\n", "STACK\n", "DEC\n"), 0);
    p = cbm_pipeline_new(root, NULL, CBM_MODE_FAST);
    ASSERT_NOT_NULL(p);
    ASSERT_FALSE(cbm_pipeline_get_adr_fill(p));
    saved = strdup("# Before watch\n");
    ASSERT_NOT_NULL(saved);
    cbm_pipeline_apply_adr_fill(p, &saved);
    ASSERT_STR_EQ(saved, "# Before watch\n");
    ASSERT_NULL(strstr(saved, "PURPOSE-WATCHER-SHOULD-NOT-FILL"));
    ASSERT_NULL(strstr(saved, "CBM-GENERATED"));
    free(saved);
    cbm_pipeline_free(p);
    th_cleanup(root);
    PASS();
}

TEST(pipeline_apply_adr_fill_true_splices) {
    char root[CBM_SZ_512];
    cbm_pipeline_t *p;
    char *saved;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_trio(root, "PURPOSE-USER-PERSIST\n", "STACK-INCR\n", "DEC-INCR\n"), 0);
    p = cbm_pipeline_new(root, NULL, CBM_MODE_FAST);
    ASSERT_NOT_NULL(p);
    cbm_pipeline_set_adr_fill(p, true);
    ASSERT_TRUE(cbm_pipeline_get_adr_fill(p));
    saved = strdup("# Before persist\n");
    ASSERT_NOT_NULL(saved);
    cbm_pipeline_apply_adr_fill(p, &saved);
    ASSERT_NOT_NULL(saved);
    ASSERT_NOT_NULL(strstr(saved, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(saved, "PURPOSE-USER-PERSIST"));
    ASSERT_NOT_NULL(strstr(saved, "STACK-INCR"));
    ASSERT_NOT_NULL(strstr(saved, "# Before persist"));
    free(saved);
    cbm_pipeline_free(p);
    th_cleanup(root);
    PASS();
}

TEST(pipeline_adr_fill_would_change_true_vs_false) {
    char root[CBM_SZ_512];
    cbm_pipeline_t *p;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_trio(root, "PURPOSE-DELTA\n", "STACK-DELTA\n", "DEC-DELTA\n"), 0);
    p = cbm_pipeline_new(root, NULL, CBM_MODE_FAST);
    ASSERT_NOT_NULL(p);
    ASSERT_FALSE(cbm_pipeline_adr_fill_would_change(p, "# Before watch\n"));
    cbm_pipeline_set_adr_fill(p, true);
    ASSERT_TRUE(cbm_pipeline_adr_fill_would_change(p, "# Before persist\n"));
    cbm_pipeline_free(p);
    th_cleanup(root);
    PASS();
}

TEST(pipeline_apply_adr_fill_error_leaves_prior) {
    char root[CBM_SZ_512];
    cbm_pipeline_t *p;
    char *saved;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    /* No .sdd-skill/ → fill returns NULL; prior blob stays (same as OOM). */
    p = cbm_pipeline_new(root, NULL, CBM_MODE_FAST);
    ASSERT_NOT_NULL(p);
    cbm_pipeline_set_adr_fill(p, true);
    saved = strdup("# Prior ADR\n");
    ASSERT_NOT_NULL(saved);
    cbm_pipeline_apply_adr_fill(p, &saved);
    ASSERT_STR_EQ(saved, "# Prior ADR\n");
    ASSERT_NULL(strstr(saved, "CBM-GENERATED"));
    free(saved);
    cbm_pipeline_free(p);
    th_cleanup(root);
    PASS();
}

TEST(adr_fill_extract_cap_drops_tail) {
    char root[256];
    char body[CBM_SZ_2K];
    char *out;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    memset(body, 'x', sizeof(body) - 1);
    body[sizeof(body) - 1] = '\0';
    memcpy(body, "BEFORE-CAP-UNIQUE\n", 17);
    memcpy(body + (size_t)CBM_ADR_EXTRACT_MAX, "AFTER-CAP-UNIQUE\n", 17);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/context_ai.md"), body), 0);

    out = cbm_adr_fill_document(root, "");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, "BEFORE-CAP-UNIQUE"));
    ASSERT_NULL(strstr(out, "AFTER-CAP-UNIQUE"));
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_gamedev_dual_tree_xor) {
    char root[256];
    char *out;
    char *manual;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_gamedev_trio(root, "PURPOSE-GAME-BEVY\n", "STACK-GAME-BEVY\n",
                                    "DECISION-GAME-BEVY\n"),
              0);
    ASSERT_EQ(af_write_trio(root, "PURPOSE-SDD-MVP1\n", "STACK-SDD-MVP1\n",
                            "DECISION-SDD-MVP1\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/01-preproduction/gdd.md"),
                            "SECRET-GDD-BEVY\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".sdd-skill/baseline/DEV_LOG.md"),
                            "SECRET-DEVLOG-BEVY\n"),
              0);

    out = cbm_adr_fill_document(root, "# Existing ADR\n");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_END));
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_MANUAL_START));
    ASSERT_NOT_NULL(strstr(out, "PURPOSE-GAME-BEVY"));
    ASSERT_NOT_NULL(strstr(out, "STACK-GAME-BEVY"));
    ASSERT_NOT_NULL(strstr(out, "DECISION-GAME-BEVY"));
    ASSERT_NULL(strstr(out, "PURPOSE-SDD-MVP1"));
    ASSERT_NULL(strstr(out, "STACK-SDD-MVP1"));
    ASSERT_NULL(strstr(out, "DECISION-SDD-MVP1"));
    ASSERT_NULL(strstr(out, "SECRET-GDD-BEVY"));
    ASSERT_NULL(strstr(out, "SECRET-DEVLOG-BEVY"));
    manual = af_between(out, CBM_ADR_MANUAL_START, CBM_ADR_MANUAL_END);
    ASSERT_NOT_NULL(manual);
    ASSERT_NOT_NULL(strstr(manual, "# Existing ADR"));
    free(manual);
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_gamedev_only_tech_stack) {
    char root[256];
    char *out;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_gamedev_trio(root, NULL, "STACK-PARTIAL-ONLY\n", NULL), 0);
    ASSERT_EQ(af_write_trio(root, "PURPOSE-SDD-FALLBACK\n", NULL, NULL), 0);

    out = cbm_adr_fill_document(root, "");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, "STACK-PARTIAL-ONLY"));
    ASSERT_NOT_NULL(strstr(out, "# Stack"));
    ASSERT_NULL(strstr(out, "# Purpose"));
    ASSERT_NULL(strstr(out, "# Decisions"));
    ASSERT_NULL(strstr(out, "PURPOSE-SDD-FALLBACK"));
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_gamedev_empty_dir_no_sdd_fallback) {
    char root[256];
    char *out;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    ASSERT_EQ(af_write_trio(root, "PURPOSE-SDD-EMPTYDIR\n", NULL, NULL), 0);

    out = cbm_adr_fill_document(root, "");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(out, CBM_ADR_GENERATED_END));
    ASSERT_NULL(strstr(out, "PURPOSE-SDD-EMPTYDIR"));
    ASSERT_NULL(strstr(out, "# Purpose"));
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_file_at_path_gamedev_uses_sdd) {
    char root[256];
    char *out;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev"), "not-a-directory\n"), 0);
    ASSERT_EQ(af_write_trio(root, "PURPOSE-FILE-AT-PATH-SDD\n", "STACK-FILE-AT-PATH-SDD\n",
                            "DECISION-FILE-AT-PATH-SDD\n"),
              0);

    out = cbm_adr_fill_document(root, "");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, "PURPOSE-FILE-AT-PATH-SDD"));
    ASSERT_NOT_NULL(strstr(out, "STACK-FILE-AT-PATH-SDD"));
    ASSERT_NOT_NULL(strstr(out, "DECISION-FILE-AT-PATH-SDD"));
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_neither_skill_dir_null) {
    char root[256];
    char *out;
    const char *existing = "# Last leftover\n";

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    out = cbm_adr_fill_document(root, existing);
    ASSERT_NULL(out);
    ASSERT_NULL(strstr(existing, "CBM-GENERATED"));
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_gamedev_extract_cap_drops_tail) {
    char root[256];
    char body[CBM_SZ_4K];
    char *out;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    memset(body, 'x', sizeof(body) - 1);
    body[sizeof(body) - 1] = '\0';
    memcpy(body, "DECISION-HEAD-BEVY\n", 19);
    memcpy(body + 2000, "DECISION-TAIL-BEVY\n", 19);
    ASSERT_EQ(af_write_gamedev_trio(root, NULL, NULL, body), 0);

    out = cbm_adr_fill_document(root, "");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, "DECISION-HEAD-BEVY"));
    ASSERT_NULL(strstr(out, "DECISION-TAIL-BEVY"));
    free(out);
    th_rmtree(root);
    PASS();
}

TEST(adr_fill_gamedev_unreadable_game_context) {
    char root[256];
    char *out;
    const char *purpose;

    ASSERT_NOT_NULL(af_mkroot(root, sizeof(root)));
    ASSERT_EQ(af_write_gamedev_trio(root, "PURPOSE-GAME-UNREADABLE\n", "STACK-READABLE-GAME\n",
                                    "DECISION-READABLE-GAME\n"),
              0);
    ASSERT_EQ(af_write_trio(root, "PURPOSE-SDD-UNREADABLE\n", NULL, NULL), 0);
    purpose = TH_PATH(root, ".gamedev/game_context.md");
    af_make_unreadable(purpose);

    out = cbm_adr_fill_document(root, "");
    ASSERT_NOT_NULL(out);
    ASSERT_NOT_NULL(strstr(out, "STACK-READABLE-GAME"));
    ASSERT_NOT_NULL(strstr(out, "DECISION-READABLE-GAME"));
    ASSERT_NULL(strstr(out, "# Purpose"));
    ASSERT_NULL(strstr(out, "PURPOSE-SDD-UNREADABLE"));
    ASSERT_NULL(strstr(out, "PURPOSE-GAME-UNREADABLE"));
    free(out);
    th_rmtree(root);
    PASS();
}

SUITE(adr_fill) {
    RUN_TEST(adr_fill_migrate_unmarked_to_manual);
    RUN_TEST(adr_fill_first_create_empty_manual);
    RUN_TEST(adr_fill_partial_context_ai_only);
    RUN_TEST(adr_fill_unreadable_architecture_adr_omits);
    RUN_TEST(adr_fill_no_sdd_skill_leaves_unmarked);
    RUN_TEST(adr_fill_skill_present_trio_all_missing);
    RUN_TEST(adr_fill_preserves_manual_on_refill);
    RUN_TEST(adr_fill_empty_file_omits_h1);
    RUN_TEST(adr_fill_extract_cap_drops_tail);
    RUN_TEST(adr_fill_gamedev_dual_tree_xor);
    RUN_TEST(adr_fill_gamedev_only_tech_stack);
    RUN_TEST(adr_fill_gamedev_empty_dir_no_sdd_fallback);
    RUN_TEST(adr_fill_file_at_path_gamedev_uses_sdd);
    RUN_TEST(adr_fill_neither_skill_dir_null);
    RUN_TEST(adr_fill_gamedev_extract_cap_drops_tail);
    RUN_TEST(adr_fill_gamedev_unreadable_game_context);
    RUN_TEST(pipeline_new_leaves_adr_fill_false);
    RUN_TEST(pipeline_apply_adr_fill_false_leaves_prior);
    RUN_TEST(pipeline_apply_adr_fill_true_splices);
    RUN_TEST(pipeline_adr_fill_would_change_true_vs_false);
    RUN_TEST(pipeline_apply_adr_fill_error_leaves_prior);
}
