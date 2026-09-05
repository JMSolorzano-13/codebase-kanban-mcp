/**
 * @sdd-task: Task #1 - spec_board debt parse + additive JSON
 * @sdd-spec: specs/spec-015-s5k-specs-debt-and-path/spec.md
 * @sdd-decision: SDD-ADR-065..066 debt Gherkin: heading/table/unknown/cap/skip/bytes
 * @sdd-why: C owns parse/heading-vs-table/cap; fixtures /tmp only; existing to_json accepts additive debt
 * @human-debug: If existing to_json tests fail → they must strstr additive "debt", not require exact old JSON
 *
 * test_spec_board.c — Parser tests for the spec board (Kanban).
 *
 * Fixtures are synthetic under /tmp. Never touches a real project tree
 * (.grill/ or .sdd-skill/ in the repo). Zero-write: tests only exercise
 * cbm_spec_board_read / to_json / extract_blurb / parse_tech_debt.
 */
#include "../src/foundation/compat.h"
#include "../src/foundation/compat_fs.h"
#include "test_framework.h"
#include "test_helpers.h"
#include "ui/spec_board.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Fixture helpers ──────────────────────────────────────────── */

static char *sb_mkroot(void) {
    char *td = th_mktempdir("cbm_spec_board");
    return td;
}

static int sb_write_active(const char *root, const char *json_body) {
    return th_write_file(TH_PATH(root, ".sdd-skill/specs/active.json"), json_body);
}

static int sb_write_state(const char *root, const char *body) {
    return th_write_file(TH_PATH(root, ".sdd-skill/state.md"), body);
}

static int sb_write_spec_files(const char *root, const char *spec_id, const char *spec_md,
                               const char *tasks_md, const char *checklist_md) {
    char rel[512];
    snprintf(rel, sizeof(rel), ".sdd-skill/specs/%s/spec.md", spec_id);
    if (th_write_file(TH_PATH(root, rel), spec_md) != 0) {
        return -1;
    }
    if (tasks_md) {
        snprintf(rel, sizeof(rel), ".sdd-skill/specs/%s/tasks.md", spec_id);
        if (th_write_file(TH_PATH(root, rel), tasks_md) != 0) {
            return -1;
        }
    }
    if (checklist_md) {
        snprintf(rel, sizeof(rel), ".sdd-skill/specs/%s/checklist.md", spec_id);
        if (th_write_file(TH_PATH(root, rel), checklist_md) != 0) {
            return -1;
        }
    }
    return 0;
}

static int sb_write_test_log(const char *root, const char *body) {
    return th_write_file(TH_PATH(root, ".sdd-skill/history/test_results.log"), body);
}

/* Directory-at-path: fopen rb fails. chmod 0 is ignored on some systems. */
static void sb_spec_md_as_dir(const char *root, const char *spec_id) {
    char rel[512];
    snprintf(rel, sizeof(rel), ".sdd-skill/specs/%s/spec.md", spec_id);
    const char *path = TH_PATH(root, rel);
    (void)cbm_unlink(path);
    (void)th_mkdir_p(path);
}

static const cbm_spec_board_entry_t *sb_find(const cbm_spec_board_t *b, const char *id) {
    for (int i = 0; i < b->spec_count; i++) {
        if (strcmp(b->specs[i].id, id) == 0) {
            return &b->specs[i];
        }
    }
    return NULL;
}

static int sb_count_column(const cbm_spec_board_t *b, const char *column) {
    int n = 0;
    for (int i = 0; i < b->spec_count; i++) {
        if (strcmp(b->specs[i].column, column) == 0) {
            n++;
        }
    }
    return n;
}

static int sb_count_substr(const char *hay, const char *needle) {
    int n = 0;
    size_t step = strlen(needle);
    if (step == 0) {
        return 0;
    }
    for (const char *p = hay; (p = strstr(p, needle)) != NULL; p += step) {
        n++;
    }
    return n;
}

static const cbm_spec_board_epic_t *sb_find_epic(const cbm_spec_board_t *b, const char *id) {
    for (int i = 0; i < b->epic_count; i++) {
        if (strcmp(b->epics[i].id, id) == 0) {
            return &b->epics[i];
        }
    }
    return NULL;
}

static int sb_write_grill_index(const char *root, const char *body) {
    return th_write_file(TH_PATH(root, ".grill/index.md"), body);
}

static int sb_write_epic_md(const char *root, const char *slug, const char *filename,
                            const char *name, const char *summary, const char *status) {
    char rel[512];
    char body[1024];
    snprintf(rel, sizeof(rel), ".grill/plans/%s/epics/%s", slug, filename);
    snprintf(body, sizeof(body), "name: %s\nstatus: %s\n\nsummary: %s\n", name, status, summary);
    return th_write_file(TH_PATH(root, rel), body);
}

static int sb_mkdir_sdd(const char *root) {
    if (th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")) != 0) {
        return -1;
    }
    return th_mkdir_p(TH_PATH(root, ".sdd-skill/history"));
}

static char *sb_slurp(const char *path) {
    FILE *f = cbm_fopen(path, "rb");
    long sz;
    char *buf;
    size_t n;
    if (!f) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    buf = malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[n] = '\0';
    return buf;
}

static void sb_epic_md_as_dir(const char *root, const char *slug, const char *filename) {
    char rel[512];
    snprintf(rel, sizeof(rel), ".grill/plans/%s/epics/%s", slug, filename);
    const char *path = TH_PATH(root, rel);
    (void)cbm_unlink(path);
    (void)th_mkdir_p(path);
}

static int sb_write_tech_debt(const char *root, const char *body) {
    return th_write_file(TH_PATH(root, ".sdd-skill/baseline/TECH_DEBT.md"), body);
}

static void sb_tech_debt_as_dir(const char *root) {
    const char *path = TH_PATH(root, ".sdd-skill/baseline/TECH_DEBT.md");
    (void)cbm_unlink(path);
    (void)th_mkdir_p(path);
}

static const cbm_spec_board_debt_t *sb_find_debt(const cbm_spec_board_t *b, const char *id) {
    for (int i = 0; i < b->debt_count; i++) {
        if (strcmp(b->debt[i].id, id) == 0) {
            return &b->debt[i];
        }
    }
    return NULL;
}

/* ── Tests ────────────────────────────────────────────────────── */

TEST(spec_board_absent_sdd_skill) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_FALSE(board->sdd_skill_present);
    ASSERT_EQ(board->spec_count, 0);
    ASSERT_FALSE(board->grill_skill_present);
    ASSERT_EQ(board->epic_count, 0);
    ASSERT_FALSE(cbm_spec_board_sdd_skill_present(root));
    ASSERT_FALSE(cbm_spec_board_grill_skill_present(root));

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"sdd_skill_present\":false"));
    ASSERT_NOT_NULL(strstr(json, "\"grill_skill_present\":false"));
    ASSERT_NOT_NULL(strstr(json, "\"specs\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"epics\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_NULL(strstr(json, "gamedev_skill_present"));
    free(json);
    free(board);

    th_rmtree(root);
    PASS();
}

TEST(spec_board_idle_planned_draft_done) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": null,\n"
        "  \"status\": \"idle\",\n"
        "  \"planned_specs\": [\"spec-010-planned\"],\n"
        "  \"draft_specs\": [\"spec-011-draft\"],\n"
        "  \"completed_specs\": [\"spec-008-done\"]\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);
    ASSERT_EQ(sb_write_state(root, "current_role: awaiting_user_command\n"), 0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);
    ASSERT_EQ(board->spec_count, 3);
    ASSERT_EQ(sb_count_column(board, "todo"), 2);
    ASSERT_EQ(sb_count_column(board, "in_progress"), 0);
    ASSERT_EQ(sb_count_column(board, "done"), 1);

    const cbm_spec_board_entry_t *planned = sb_find(board, "spec-010-planned");
    const cbm_spec_board_entry_t *draft = sb_find(board, "spec-011-draft");
    const cbm_spec_board_entry_t *done = sb_find(board, "spec-008-done");
    ASSERT_NOT_NULL(planned);
    ASSERT_NOT_NULL(draft);
    ASSERT_NOT_NULL(done);
    ASSERT_STR_EQ(planned->column, "todo");
    ASSERT_STR_EQ(draft->column, "todo");
    ASSERT_STR_EQ(done->column, "done");
    ASSERT_FALSE(planned->active);
    ASSERT_FALSE(draft->active);
    ASSERT_FALSE(done->active);
    /* Non-active ids without spec.md / tasks.md still leave task_count==0
     * and empty title/blurb (enrich is best-effort; no files → zeros). */
    ASSERT_EQ(planned->task_count, 0);
    ASSERT_FLOAT_EQ(planned->checklist_percent, -1.0, 0.01);
    ASSERT_STR_EQ(planned->blurb, "");
    ASSERT_STR_EQ(draft->blurb, "");
    ASSERT_STR_EQ(done->blurb, "");

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(sb_count_substr(json, "\"blurb\":"), 3);
    ASSERT_NOT_NULL(strstr(json, "\"blurb\":\"\""));
    ASSERT_NOT_NULL(strstr(json, "\"grill_skill_present\":false"));
    ASSERT_NOT_NULL(strstr(json, "\"epics\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_EQ(sb_count_substr(json, "\"kind\""), 0);
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);

    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_active_compact_state) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": \"spec-009-active\",\n"
        "  \"status\": \"in_progress\",\n"
        "  \"planned_specs\": [\"spec-010-next\"],\n"
        "  \"draft_specs\": [\"spec-011-draft\"],\n"
        "  \"completed_specs\": [\"spec-008-done\"]\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);

    /* Compact post-1.4.0 state.md */
    ASSERT_EQ(sb_write_state(root,
                             "role=@implementer task=\"Task #3 — Tests\" spec=spec-009-active "
                             "status=in_progress\n"
                             "notes=\"\"\n"),
              0);

    ASSERT_EQ(sb_write_spec_files(
                  root, "spec-009-active",
                  "# Spec-009: Active Example\n\nBody.\n",
                  "### Task #1 — Setup\n\n"
                  "### Task #2 — Core logic\n\n"
                  "### Task #3 — Tests\n\n"
                  "### Task #4 — Docs\n",
                  "## Feature Status\n\n| Item | % |\n| TOTAL | 40% |\n"),
              0);

    ASSERT_EQ(sb_write_test_log(root,
                                "[2026-01-01T00:00:00Z] Task #1 — @tester — PASS (attempt 1/2) — "
                                "L1 — unit / 3|3|0\n"
                                "[2026-01-01T01:00:00Z] Task #2 — @tester — PASS (attempt 1/2) — "
                                "L1 — unit / 2|2|0\n"
                                "[2026-01-01T02:00:00Z] Task #3 — @tester — FAIL (attempt 1/2) — "
                                "L1 — unit / 4|3|1\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);
    ASSERT_EQ(sb_count_column(board, "todo"), 2); /* planned + draft */
    ASSERT_EQ(sb_count_column(board, "in_progress"), 1);
    ASSERT_EQ(sb_count_column(board, "done"), 1);

    const cbm_spec_board_entry_t *active_e = sb_find(board, "spec-009-active");
    ASSERT_NOT_NULL(active_e);
    ASSERT_TRUE(active_e->active);
    ASSERT_STR_EQ(active_e->column, "in_progress");
    ASSERT_STR_EQ(active_e->title, "Spec-009: Active Example");
    ASSERT_STR_EQ(active_e->current_agent, "@implementer");
    ASSERT_EQ(active_e->task_count, 4);
    ASSERT_EQ(active_e->tasks_done, 2);
    ASSERT_FLOAT_EQ(active_e->checklist_percent, 40.0, 0.01);

    ASSERT_EQ(active_e->tasks[0].number, 1);
    ASSERT_TRUE(active_e->tasks[0].done);
    ASSERT_FALSE(active_e->tasks[0].current);
    ASSERT_STR_EQ(active_e->tasks[0].name, "Setup");

    ASSERT_EQ(active_e->tasks[1].number, 2);
    ASSERT_TRUE(active_e->tasks[1].done);

    ASSERT_EQ(active_e->tasks[2].number, 3);
    ASSERT_FALSE(active_e->tasks[2].done);
    ASSERT_TRUE(active_e->tasks[2].current);
    ASSERT_STR_EQ(active_e->tasks[2].name, "Tests");

    ASSERT_EQ(active_e->tasks[3].number, 4);
    ASSERT_FALSE(active_e->tasks[3].done);
    ASSERT_FALSE(active_e->tasks[3].current);

    /* draft still lands in todo */
    const cbm_spec_board_entry_t *draft = sb_find(board, "spec-011-draft");
    ASSERT_NOT_NULL(draft);
    ASSERT_STR_EQ(draft->column, "todo");

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"sdd_skill_present\":true"));
    ASSERT_NOT_NULL(strstr(json, "\"id\":\"spec-009-active\""));
    ASSERT_NOT_NULL(strstr(json, "\"column\":\"in_progress\""));
    ASSERT_NOT_NULL(strstr(json, "\"current_agent\":\"@implementer\""));
    ASSERT_NOT_NULL(strstr(json, "\"current\":true"));
    ASSERT_EQ(sb_count_substr(json, "\"blurb\":"), board->spec_count);
    ASSERT_STR_EQ(active_e->blurb, "");
    free(json);
    free(board);

    th_rmtree(root);
    PASS();
}

TEST(spec_board_active_legacy_state_and_blocker) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": \"spec-012-legacy\",\n"
        "  \"status\": \"in_progress\",\n"
        "  \"planned_specs\": [],\n"
        "  \"draft_specs\": [],\n"
        "  \"completed_specs\": []\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);

    /* Pre-1.4.0 multi-line state.md with a blocker under ## Notes */
    ASSERT_EQ(sb_write_state(root,
                             "# State\n"
                             "current_role: @tester\n"
                             "current_task: Task #1 — Smoke\n"
                             "\n"
                             "## Notes\n"
                             "- blocked on missing emulator image\n"),
              0);

    ASSERT_EQ(sb_write_spec_files(root, "spec-012-legacy", "# Spec-012: Legacy Format\n",
                                  "### Task #1 — Smoke\n"
                                  "### Task #2 — Polish\n",
                                  NULL /* no checklist → -1 */),
              0);
    ASSERT_EQ(sb_write_test_log(root, ""), 0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);

    const cbm_spec_board_entry_t *e = sb_find(board, "spec-012-legacy");
    ASSERT_NOT_NULL(e);
    ASSERT_TRUE(e->active);
    ASSERT_STR_EQ(e->current_agent, "@tester");
    ASSERT_EQ(e->task_count, 2);
    ASSERT_TRUE(e->tasks[0].current);
    ASSERT_FALSE(e->tasks[1].current);
    ASSERT_FLOAT_EQ(e->checklist_percent, -1.0, 0.01);
    ASSERT_NOT_NULL(strstr(e->blocked_note, "blocked on missing emulator image"));

    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_gamedev_presence) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_FALSE(cbm_spec_board_gamedev_skill_present(root));
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    ASSERT_TRUE(cbm_spec_board_gamedev_skill_present(root));
    th_rmtree(root);
    PASS();
}

/* ── extract_blurb (US-002 Then clauses; no HTTP) ───────────────── */

TEST(spec_board_extract_blurb_kpi_not_used) {
    char out[CBM_SPEC_BOARD_BLURB_MAX];
    const char *kpi_only =
        "# Spec-015-fff: KPI Only\n\n"
        "## KPI\n"
        "KPI-MUST-NOT-BLURB\n";
    cbm_spec_board_extract_blurb(kpi_only, out, sizeof(out));
    ASSERT_STR_EQ(out, "");
    ASSERT_NULL(strstr(out, "KPI-MUST-NOT-BLURB"));

    const char *es_then_kpi =
        "# Spec-015-fff: With ES\n\n"
        "## Executive Summary\n"
        "Planned work ships the inbox filter. Operators see unread first. Third is dropped.\n"
        "## KPI\n"
        "KPI-MUST-NOT-BLURB\n";
    cbm_spec_board_extract_blurb(es_then_kpi, out, sizeof(out));
    ASSERT_STR_EQ(out, "Planned work ships the inbox filter. Operators see unread first.");
    ASSERT_NULL(strstr(out, "KPI-MUST-NOT-BLURB"));
    ASSERT_NULL(strstr(out, "Third"));
    PASS();
}

TEST(spec_board_extract_blurb_empty_executive_summary) {
    char out[CBM_SPEC_BOARD_BLURB_MAX];
    memset(out, 'X', sizeof(out));
    const char *empty_then_kpi =
        "## Executive Summary\n"
        "\n"
        "## KPI\n"
        "KPI-MUST-NOT-BLURB\n";
    cbm_spec_board_extract_blurb(empty_then_kpi, out, sizeof(out));
    ASSERT_STR_EQ(out, "");

    const char *ws_only =
        "  ## Executive Summary\n"
        "   \n"
        "## Next\n";
    cbm_spec_board_extract_blurb(ws_only, out, sizeof(out));
    ASSERT_STR_EQ(out, "");

    const char *immediate_h2 =
        "## Executive Summary\n"
        "## KPI\n"
        "KPI-MUST-NOT-BLURB\n";
    cbm_spec_board_extract_blurb(immediate_h2, out, sizeof(out));
    ASSERT_STR_EQ(out, "");
    PASS();
}

TEST(spec_board_extract_blurb_null_or_empty_input) {
    char out[CBM_SPEC_BOARD_BLURB_MAX];
    memset(out, 'X', sizeof(out));
    /* Missing/unreadable md is Task #2 (extract not called). Helper: NULL/empty → "". */
    cbm_spec_board_extract_blurb(NULL, out, sizeof(out));
    ASSERT_STR_EQ(out, "");
    memset(out, 'X', sizeof(out));
    cbm_spec_board_extract_blurb("", out, sizeof(out));
    ASSERT_STR_EQ(out, "");
    cbm_spec_board_extract_blurb("# Title only\nNo heading.\n", out, sizeof(out));
    ASSERT_STR_EQ(out, "");
    PASS();
}

TEST(spec_board_extract_blurb_not_h1_or_h3) {
    char out[CBM_SPEC_BOARD_BLURB_MAX];
    cbm_spec_board_extract_blurb("# Executive Summary\nThis is the H1 body.\n", out, sizeof(out));
    ASSERT_STR_EQ(out, "");
    cbm_spec_board_extract_blurb("### Executive Summary\nShould not count.\n", out, sizeof(out));
    ASSERT_STR_EQ(out, "");
    PASS();
}

TEST(spec_board_extract_blurb_links_and_newlines) {
    char out[CBM_SPEC_BOARD_BLURB_MAX];
    const char *md =
        "## Executive Summary\n"
        "See the [inbox filter](https://ex.com/a.html). Second\n"
        "continues here. Third dropped.\n";
    cbm_spec_board_extract_blurb(md, out, sizeof(out));
    ASSERT_STR_EQ(out, "See the inbox filter. Second continues here.");
    PASS();
}

TEST(spec_board_extract_blurb_truncate_walkback) {
    const char *md =
        "## Executive Summary\n"
        "Alpha beta gamma delta epsilon zeta eta theta. Third dropped.\n";
    char out[24];
    /* 23 usable: hard cut lands mid-epsilon; walk back to last space. */
    cbm_spec_board_extract_blurb(md, out, sizeof(out));
    ASSERT_STR_EQ(out, "Alpha beta gamma delta");
    ASSERT_NULL(strstr(out, "epsilon"));
    ASSERT_NULL(strstr(out, "Third"));
    PASS();
}

TEST(spec_board_to_json_emits_escaped_blurb) {
    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    board->sdd_skill_present = true;
    board->spec_count = 1;
    snprintf(board->specs[0].id, sizeof(board->specs[0].id), "spec-010-aaa-planned");
    snprintf(board->specs[0].column, sizeof(board->specs[0].column), "todo");
    const char *md =
        "## Executive Summary\n"
        "He said \"go\" to the [inbox](http://x). Operators see unread first.\n";
    cbm_spec_board_extract_blurb(md, board->specs[0].blurb, sizeof(board->specs[0].blurb));
    ASSERT_STR_EQ(board->specs[0].blurb, "He said \"go\" to the inbox. Operators see unread first.");

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"blurb\":\"He said \\\"go\\\" to the inbox. Operators see unread first.\""));
    ASSERT_NOT_NULL(strstr(json, "\"grill_skill_present\":false"));
    ASSERT_NOT_NULL(strstr(json, "\"epics\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NULL(strstr(json, "\"kind\""));
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);
    free(board);
    PASS();
}

/* ── Task #2: full cbm_spec_board_read (US-003/005/006 Then) ───── */

TEST(spec_board_read_kpi_not_used_as_blurb) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": null,\n"
        "  \"planned_specs\": [\"spec-015-fff-kpi\"],\n"
        "  \"draft_specs\": [],\n"
        "  \"completed_specs\": []\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-015-fff-kpi",
                                  "# Spec-015-fff: KPI Only\n\n"
                                  "## KPI\n"
                                  "KPI-MUST-NOT-BLURB\n",
                                  NULL, NULL),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);

    const cbm_spec_board_entry_t *e = sb_find(board, "spec-015-fff-kpi");
    ASSERT_NOT_NULL(e);
    ASSERT_STR_EQ(e->blurb, "");
    ASSERT_NULL(strstr(e->title, "KPI-MUST-NOT-BLURB"));
    ASSERT_NULL(strstr(e->blurb, "KPI-MUST-NOT-BLURB"));

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"sdd_skill_present\":true"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_nonactive_done_requires_spec_id) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": null,\n"
        "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
        "  \"draft_specs\": [],\n"
        "  \"completed_specs\": []\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned",
                                  "# Spec-010-aaa: Planned Work\n\n"
                                  "## Executive Summary\n"
                                  "Planned work ships the inbox filter. Operators see unread first.\n",
                                  "### Task #1 — Write parser\n"
                                  "### Task #2 — Write tests\n",
                                  NULL),
              0);
    ASSERT_EQ(sb_write_test_log(root,
                                "Task #1 — PASS\n"
                                "spec-010-aaa-planned Task #2 — PASS\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);

    const cbm_spec_board_entry_t *e = sb_find(board, "spec-010-aaa-planned");
    ASSERT_NOT_NULL(e);
    ASSERT_FALSE(e->active);
    ASSERT_EQ(e->task_count, 2);
    ASSERT_EQ(e->tasks[0].number, 1);
    ASSERT_FALSE(e->tasks[0].done);
    ASSERT_FALSE(e->tasks[0].current);
    ASSERT_EQ(e->tasks[1].number, 2);
    ASSERT_TRUE(e->tasks[1].done);
    ASSERT_FALSE(e->tasks[1].current);
    ASSERT_STR_EQ(e->current_agent, "");
    ASSERT_FLOAT_EQ(e->checklist_percent, -1.0, 0.01);

    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_missing_spec_md_degrades_one) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": null,\n"
        "  \"planned_specs\": [\"spec-016-ggg-ghost\", \"spec-010-aaa-planned\"],\n"
        "  \"draft_specs\": [],\n"
        "  \"completed_specs\": []\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned",
                                  "# Spec-010-aaa: Planned Work\n\n"
                                  "## Executive Summary\n"
                                  "Planned work ships the inbox filter. Operators see unread first.\n",
                                  "### Task #1 — Write parser\n", NULL),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);

    const cbm_spec_board_entry_t *ghost = sb_find(board, "spec-016-ggg-ghost");
    const cbm_spec_board_entry_t *ok = sb_find(board, "spec-010-aaa-planned");
    ASSERT_NOT_NULL(ghost);
    ASSERT_NOT_NULL(ok);
    ASSERT_STR_EQ(ghost->title, "");
    ASSERT_STR_EQ(ghost->blurb, "");
    ASSERT_STR_EQ(ok->title, "Spec-010-aaa: Planned Work");
    ASSERT_STR_EQ(ok->blurb, "Planned work ships the inbox filter. Operators see unread first.");

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"sdd_skill_present\":true"));
    ASSERT_NOT_NULL(strstr(json, "\"id\":\"spec-016-ggg-ghost\""));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_missing_tasks_md_empty) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": null,\n"
        "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
        "  \"draft_specs\": [],\n"
        "  \"completed_specs\": []\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned",
                                  "# Spec-010-aaa: Planned Work\n\n"
                                  "## Executive Summary\n"
                                  "Planned work ships the inbox filter. Operators see unread first.\n",
                                  NULL, NULL),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);

    const cbm_spec_board_entry_t *e = sb_find(board, "spec-010-aaa-planned");
    ASSERT_NOT_NULL(e);
    ASSERT_EQ(e->task_count, 0);
    ASSERT_EQ(e->tasks_done, 0);
    ASSERT_STR_EQ(e->title, "Spec-010-aaa: Planned Work");

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"task_count\":0"));
    ASSERT_NOT_NULL(strstr(json, "\"tasks\":[]"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_unreadable_spec_md_omits_blurb) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": null,\n"
        "  \"planned_specs\": [\"spec-010-aaa-planned\", \"spec-015-fff-kpi\"],\n"
        "  \"draft_specs\": [],\n"
        "  \"completed_specs\": []\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned",
                                  "# Spec-010-aaa: Planned Work\n\n"
                                  "## Executive Summary\n"
                                  "Should not be read from a directory.\n",
                                  NULL, NULL),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-015-fff-kpi",
                                  "# Spec-015-fff: Sibling\n\n"
                                  "## Executive Summary\n"
                                  "Sibling stays readable.\n",
                                  NULL, NULL),
              0);
    sb_spec_md_as_dir(root, "spec-010-aaa-planned");

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);

    const cbm_spec_board_entry_t *bad = sb_find(board, "spec-010-aaa-planned");
    const cbm_spec_board_entry_t *ok = sb_find(board, "spec-015-fff-kpi");
    ASSERT_NOT_NULL(bad);
    ASSERT_NOT_NULL(ok);
    ASSERT_STR_EQ(bad->title, "");
    ASSERT_STR_EQ(bad->blurb, "");
    ASSERT_STR_EQ(ok->title, "Spec-015-fff: Sibling");
    ASSERT_STR_EQ(ok->blurb, "Sibling stays readable.");

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"sdd_skill_present\":true"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_enrich_every_listed) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": \"spec-011-bbb-active\",\n"
        "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
        "  \"draft_specs\": [],\n"
        "  \"completed_specs\": [\"spec-012-ccc-closed\"]\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);
    ASSERT_EQ(sb_write_state(root,
                             "role=@implementer task=\"Task #2 — Write tests\" spec=spec-011-bbb-active "
                             "status=in_progress\n"
                             "notes=\"\"\n"),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned",
                                  "# Spec-010-aaa: Planned Work\n\n"
                                  "## Executive Summary\n"
                                  "Planned work ships the inbox filter. Operators see unread first.\n",
                                  "### Task #1 — Write parser\n"
                                  "### Task #2 — Write tests\n",
                                  NULL),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-011-bbb-active",
                                  "# Spec-011-bbb: Active Work\n\n"
                                  "## Executive Summary\n"
                                  "Active work fills the card.\n",
                                  "### Task #1 — Write parser\n"
                                  "### Task #2 — Write tests\n",
                                  "## Feature Status\n\n| Item | % |\n| TOTAL | 50% |\n"),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-012-ccc-closed",
                                  "# Spec-012-ccc: Closed Work\n\n"
                                  "## Executive Summary\n"
                                  "Closed work is still readable.\n",
                                  "### Task #1 — Write parser\n"
                                  "### Task #2 — Write tests\n",
                                  NULL),
              0);
    ASSERT_EQ(sb_write_test_log(root,
                                "Task #1 — PASS\n"
                                "spec-010-aaa-planned Task #2 — FAIL\n"
                                "spec-012-ccc-closed Task #1 — PASS\n"
                                "spec-012-ccc-closed Task #2 — PASS\n"
                                "Task #2 — FAIL\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);
    ASSERT_EQ(board->spec_count, 3);

    const cbm_spec_board_entry_t *todo = sb_find(board, "spec-010-aaa-planned");
    const cbm_spec_board_entry_t *ip = sb_find(board, "spec-011-bbb-active");
    const cbm_spec_board_entry_t *done = sb_find(board, "spec-012-ccc-closed");
    ASSERT_NOT_NULL(todo);
    ASSERT_NOT_NULL(ip);
    ASSERT_NOT_NULL(done);

    ASSERT_STR_EQ(todo->title, "Spec-010-aaa: Planned Work");
    ASSERT_STR_EQ(todo->blurb, "Planned work ships the inbox filter. Operators see unread first.");
    ASSERT_EQ(todo->task_count, 2);
    ASSERT_FALSE(todo->tasks[0].done); /* bare Task #1 must not mark non-active */
    ASSERT_FALSE(todo->tasks[1].done);
    ASSERT_FALSE(todo->tasks[0].current);
    ASSERT_STR_EQ(todo->current_agent, "");
    ASSERT_FLOAT_EQ(todo->checklist_percent, -1.0, 0.01);

    ASSERT_TRUE(ip->active);
    ASSERT_STR_EQ(ip->title, "Spec-011-bbb: Active Work");
    ASSERT_STR_EQ(ip->blurb, "Active work fills the card.");
    ASSERT_STR_EQ(ip->current_agent, "@implementer");
    ASSERT_EQ(ip->task_count, 2);
    ASSERT_TRUE(ip->tasks[0].done); /* active bare Task #1 PASS */
    ASSERT_FALSE(ip->tasks[1].done);
    ASSERT_TRUE(ip->tasks[1].current);
    ASSERT_FALSE(ip->tasks[0].current);
    ASSERT_FLOAT_EQ(ip->checklist_percent, 50.0, 0.01);

    ASSERT_STR_EQ(done->title, "Spec-012-ccc: Closed Work");
    ASSERT_STR_EQ(done->blurb, "Closed work is still readable.");
    ASSERT_EQ(done->task_count, 2);
    ASSERT_TRUE(done->tasks[0].done);
    ASSERT_TRUE(done->tasks[1].done);
    ASSERT_FALSE(done->tasks[0].current);
    ASSERT_FALSE(done->tasks[1].current);
    ASSERT_STR_EQ(done->current_agent, "");

    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_to_json_emits_archived_false_and_true) {
    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    board->sdd_skill_present = true;
    board->spec_count = 1;
    snprintf(board->specs[0].id, sizeof(board->specs[0].id), "spec-012-ccc-closed");
    snprintf(board->specs[0].column, sizeof(board->specs[0].column), "done");

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"archived\":false"));
    ASSERT_NULL(strstr(json, "\"archived\":true"));
    ASSERT_NOT_NULL(strstr(json, "\"epics\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);

    board->specs[0].archived = true;
    json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"archived\":true"));
    ASSERT_NULL(strstr(json, "\"archived\":false"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    free(json);
    free(board);
    PASS();
}

TEST(spec_board_read_never_sets_archived) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/specs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".sdd-skill/history")), 0);

    const char *active =
        "{\n"
        "  \"active_spec\": null,\n"
        "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
        "  \"draft_specs\": [],\n"
        "  \"completed_specs\": [\"spec-012-ccc-closed\"]\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active), 0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_EQ(board->spec_count, 2);
    ASSERT_FALSE(board->specs[0].archived);
    ASSERT_FALSE(board->specs[1].archived);

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(sb_count_substr(json, "\"archived\":false"), 2);
    ASSERT_NULL(strstr(json, "\"archived\":true"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

/* ── Task #1: grill read + additive JSON (US-001/002/004/005/006 Then) ─ */

static const char *k_inbox_index =
    "# .grill/\n\n"
    "| slug | title | status |\n"
    "|------|-------|--------|\n"
    "| inbox-plan | Inbox Plan | draft |\n";

static const char *k_inbox_epic_id = ".grill/plans/inbox-plan/epics/epic-001-inbox.md";

TEST(spec_board_grill_mixed_todo_json) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\n"
                              "  \"active_spec\": null,\n"
                              "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
                              "  \"draft_specs\": [],\n"
                              "  \"completed_specs\": []\n"
                              "}\n"),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned",
                                  "# Spec-010-aaa: Planned Work\n\n"
                                  "## Executive Summary\n"
                                  "Planned work ships the inbox filter.\n",
                                  NULL, NULL),
              0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox",
                              "Filter unread first.", "pending"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);
    ASSERT_TRUE(board->grill_skill_present);
    ASSERT_TRUE(cbm_spec_board_grill_skill_present(root));
    ASSERT_EQ(board->epic_count, 1);
    ASSERT_NOT_NULL(sb_find(board, "spec-010-aaa-planned"));

    const cbm_spec_board_epic_t *ep = sb_find_epic(board, k_inbox_epic_id);
    ASSERT_NOT_NULL(ep);
    ASSERT_STR_EQ(ep->title, "inbox");
    ASSERT_STR_EQ(ep->summary, "Filter unread first.");
    ASSERT_STR_EQ(ep->plan_title, "Inbox Plan");
    ASSERT_STR_EQ(ep->column, "todo");

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"grill_skill_present\":true"));
    ASSERT_NOT_NULL(strstr(json, "\"id\":\".grill/plans/inbox-plan/epics/epic-001-inbox.md\""));
    ASSERT_NOT_NULL(strstr(json, "\"kind\":\"epic\""));
    ASSERT_NOT_NULL(strstr(json, "\"title\":\"inbox\""));
    ASSERT_NOT_NULL(strstr(json, "\"summary\":\"Filter unread first.\""));
    ASSERT_NOT_NULL(strstr(json, "\"plan_title\":\"Inbox Plan\""));
    ASSERT_NOT_NULL(strstr(json, "\"column\":\"todo\""));
    ASSERT_NOT_NULL(strstr(json, "\"id\":\"spec-010-aaa-planned\""));
    ASSERT_EQ(sb_count_substr(json, "\"kind\""), 1);
    ASSERT_NULL(strstr(json, "\"kind\":\"spec\""));
    ASSERT_NULL(strstr(json, "companion_grill"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_NULL(strstr(json, "gamedev_skill_present"));
    {
        const char *epic_at = strstr(json, "\"id\":\".grill/plans/inbox-plan/epics/epic-001-inbox.md\"");
        const char *spec_at = strstr(json, "\"id\":\"spec-010-aaa-planned\"");
        ASSERT_NOT_NULL(epic_at);
        ASSERT_NOT_NULL(spec_at);
        /* specs array is serialized before epics; Mixed Todo UI concatenates
         * epics then specs — C order of arrays is independent. Both present. */
        (void)epic_at;
        (void)spec_at;
    }
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_companion_to_exact_omits) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\n"
                              "  \"active_spec\": null,\n"
                              "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
                              "  \"draft_specs\": [],\n"
                              "  \"completed_specs\": []\n"
                              "}\n"),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned",
                                  "# Spec-010-aaa: Planned\n\n"
                                  "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md\n",
                                  NULL, NULL),
              0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox",
                              "Filter unread first.", "pending"),
              0);

    char epic_path[1024];
    snprintf(epic_path, sizeof(epic_path), "%s",
             TH_PATH(root, ".grill/plans/inbox-plan/epics/epic-001-inbox.md"));
    char *before = sb_slurp(epic_path);
    ASSERT_NOT_NULL(before);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NULL(sb_find_epic(board, k_inbox_epic_id));
    ASSERT_EQ(board->epic_count, 0);
    ASSERT_NOT_NULL(sb_find(board, "spec-010-aaa-planned"));
    ASSERT_STR_EQ(board->specs[0].companion_grill, k_inbox_epic_id);

    char *after = sb_slurp(epic_path);
    ASSERT_NOT_NULL(after);
    ASSERT_STR_EQ(before, after);
    free(before);
    free(after);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_active_json_source_omits) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\n"
                              "  \"active_spec\": null,\n"
                              "  \"planned_specs\": [],\n"
                              "  \"draft_specs\": [],\n"
                              "  \"completed_specs\": [],\n"
                              "  \"source\": {\n"
                              "    \"grill_epic\": \".grill/plans/inbox-plan/epics/epic-002-later.md\"\n"
                              "  }\n"
                              "}\n"),
              0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-002-later.md", "later", "Later work.",
                              "pending"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NULL(sb_find_epic(board, ".grill/plans/inbox-plan/epics/epic-002-later.md"));
    ASSERT_EQ(board->epic_count, 0);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_companion_trailing_notes_match) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\n"
                              "  \"active_spec\": null,\n"
                              "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
                              "  \"draft_specs\": [],\n"
                              "  \"completed_specs\": []\n"
                              "}\n"),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned",
                                  "# Spec-010-aaa: Planned\n\n"
                                  "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md (ADR-001)\n",
                                  NULL, NULL),
              0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NULL(sb_find_epic(board, k_inbox_epic_id));
    ASSERT_STR_EQ(board->specs[0].companion_grill, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_done_spec_claim_omits) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\n"
                              "  \"active_spec\": null,\n"
                              "  \"planned_specs\": [],\n"
                              "  \"draft_specs\": [],\n"
                              "  \"completed_specs\": [\"spec-012-ccc-closed\"]\n"
                              "}\n"),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-012-ccc-closed",
                                  "# Spec-012-ccc: Closed\n\n"
                                  "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md\n",
                                  NULL, NULL),
              0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NULL(sb_find_epic(board, k_inbox_epic_id));
    const cbm_spec_board_entry_t *done = sb_find(board, "spec-012-ccc-closed");
    ASSERT_NOT_NULL(done);
    ASSERT_STR_EQ(done->column, "done");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_two_plans_index_then_nnn) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\n"
                              "  \"active_spec\": null,\n"
                              "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
                              "  \"draft_specs\": [],\n"
                              "  \"completed_specs\": []\n"
                              "}\n"),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned", "# Spec-010\n", NULL, NULL), 0);
    ASSERT_EQ(sb_write_grill_index(root,
                                   "# .grill/\n\n"
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| plan-a | Plan A | draft |\n"
                                   "| plan-b | Plan B | draft |\n"),
              0);
    /* Disk order reversed vs NNN to prove sort is numeric, not readdir. */
    ASSERT_EQ(sb_write_epic_md(root, "plan-a", "epic-002-second.md", "second", "s2", "pending"), 0);
    ASSERT_EQ(sb_write_epic_md(root, "plan-a", "epic-001-first.md", "first", "s1", "pending"), 0);
    ASSERT_EQ(sb_write_epic_md(root, "plan-b", "epic-001-other.md", "other", "s3", "pending"), 0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_EQ(board->epic_count, 3);
    ASSERT_STR_EQ(board->epics[0].id, ".grill/plans/plan-a/epics/epic-001-first.md");
    ASSERT_STR_EQ(board->epics[1].id, ".grill/plans/plan-a/epics/epic-002-second.md");
    ASSERT_STR_EQ(board->epics[2].id, ".grill/plans/plan-b/epics/epic-001-other.md");
    ASSERT_STR_EQ(board->epics[0].plan_title, "Plan A");
    ASSERT_STR_EQ(board->epics[2].plan_title, "Plan B");
    ASSERT_NOT_NULL(sb_find(board, "spec-010-aaa-planned"));
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_closed_plan_leftover_stays) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_grill_index(root,
                                   "# .grill/\n\n"
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| old-plan | Old Plan | closed |\n"),
              0);
    ASSERT_EQ(sb_write_epic_md(root, "old-plan", "epic-009-leftover.md", "leftover", "Left.",
                              "pending"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NOT_NULL(sb_find_epic(board, ".grill/plans/old-plan/epics/epic-009-leftover.md"));
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_pending_and_detailed_listed) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "a", "pending"), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-003-ready.md", "ready", "b", "detailed"), 0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NOT_NULL(sb_find_epic(board, k_inbox_epic_id));
    ASSERT_NOT_NULL(sb_find_epic(board, ".grill/plans/inbox-plan/epics/epic-003-ready.md"));
    ASSERT_EQ(board->epic_count, 2);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_missing_companion_keeps_epic) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\n"
                              "  \"active_spec\": null,\n"
                              "  \"planned_specs\": [\"spec-010-aaa-inbox\"],\n"
                              "  \"draft_specs\": [],\n"
                              "  \"completed_specs\": []\n"
                              "}\n"),
              0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-inbox",
                                  "# Spec-010-aaa: Inbox\n\nNo Companion-to grill path.\n", NULL,
                                  NULL),
              0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NOT_NULL(sb_find_epic(board, k_inbox_epic_id));
    ASSERT_NOT_NULL(sb_find(board, "spec-010-aaa-inbox"));
    ASSERT_STR_EQ(board->specs[0].companion_grill, "");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_65th_omitted_specs_stay_64) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);

    char active[16384];
    int pos = 0;
    pos += snprintf(active + pos, sizeof(active) - (size_t)pos,
                    "{\"active_spec\":null,\"planned_specs\":[");
    for (int i = 0; i < 64; i++) {
        pos += snprintf(active + pos, sizeof(active) - (size_t)pos, "%s\"spec-%03d-slot\"",
                        i ? "," : "", i + 1);
    }
    pos += snprintf(active + pos, sizeof(active) - (size_t)pos,
                    "],\"draft_specs\":[],\"completed_specs\":[]}");
    ASSERT_EQ(sb_write_active(root, active), 0);

    ASSERT_EQ(sb_write_grill_index(root,
                                   "# .grill/\n\n"
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| cap-plan | Cap | draft |\n"),
              0);
    for (int i = 1; i <= 65; i++) {
        char fname[64];
        char name[32];
        snprintf(fname, sizeof(fname), "epic-%03d-item.md", i);
        snprintf(name, sizeof(name), "item-%03d", i);
        ASSERT_EQ(sb_write_epic_md(root, "cap-plan", fname, name, "s", "pending"), 0);
    }

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_EQ(board->epic_count, 64);
    ASSERT_EQ(board->spec_count, 64);
    ASSERT_NULL(sb_find_epic(board, ".grill/plans/cap-plan/epics/epic-065-item.md"));
    ASSERT_NOT_NULL(sb_find_epic(board, ".grill/plans/cap-plan/epics/epic-064-item.md"));

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_NULL(strstr(json, "Has more"));
    ASSERT_EQ(board->epic_count, CBM_SPEC_BOARD_MAX_EPICS);
    ASSERT_EQ(board->spec_count, CBM_SPEC_BOARD_MAX_SPECS);
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_no_grill_dir) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\n"
                              "  \"active_spec\": null,\n"
                              "  \"planned_specs\": [\"spec-010-planned\"],\n"
                              "  \"draft_specs\": [\"spec-011-draft\"],\n"
                              "  \"completed_specs\": [\"spec-008-done\"]\n"
                              "}\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);
    ASSERT_FALSE(board->grill_skill_present);
    ASSERT_EQ(board->epic_count, 0);
    ASSERT_EQ(board->spec_count, 3);

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"grill_skill_present\":false"));
    ASSERT_NOT_NULL(strstr(json, "\"epics\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_unreadable_epic_skipped) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "ok", "pending"),
              0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-002-broken.md", "broken", "no", "pending"),
              0);
    sb_epic_md_as_dir(root, "inbox-plan", "epic-002-broken.md");

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NOT_NULL(sb_find_epic(board, k_inbox_epic_id));
    ASSERT_NULL(sb_find_epic(board, ".grill/plans/inbox-plan/epics/epic-002-broken.md"));
    ASSERT_EQ(board->epic_count, 1);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_read_leaves_bytes_identical) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    const char *active_body =
        "{\n"
        "  \"active_spec\": null,\n"
        "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
        "  \"draft_specs\": [],\n"
        "  \"completed_specs\": [],\n"
        "  \"source\": { \"grill_epic\": \"\" }\n"
        "}\n";
    ASSERT_EQ(sb_write_active(root, active_body), 0);
    ASSERT_EQ(sb_write_spec_files(root, "spec-010-aaa-planned", "# Spec\n", NULL, NULL), 0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"),
              0);

    char p_index[1024], p_epic[1024], p_active[1024];
    snprintf(p_index, sizeof(p_index), "%s", TH_PATH(root, ".grill/index.md"));
    snprintf(p_epic, sizeof(p_epic), "%s",
             TH_PATH(root, ".grill/plans/inbox-plan/epics/epic-001-inbox.md"));
    snprintf(p_active, sizeof(p_active), "%s", TH_PATH(root, ".sdd-skill/specs/active.json"));
    char *b_index = sb_slurp(p_index);
    char *b_epic = sb_slurp(p_epic);
    char *b_active = sb_slurp(p_active);
    ASSERT_NOT_NULL(b_index);
    ASSERT_NOT_NULL(b_epic);
    ASSERT_NOT_NULL(b_active);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->grill_skill_present);

    char *a_index = sb_slurp(p_index);
    char *a_epic = sb_slurp(p_epic);
    char *a_active = sb_slurp(p_active);
    ASSERT_STR_EQ(b_index, a_index);
    ASSERT_STR_EQ(b_epic, a_epic);
    ASSERT_STR_EQ(b_active, a_active);
    free(b_index);
    free(b_epic);
    free(b_active);
    free(a_index);
    free(a_epic);
    free(a_active);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_fills_without_sdd) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox",
                              "Filter unread first.", "pending"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_FALSE(board->sdd_skill_present);
    ASSERT_TRUE(board->grill_skill_present);
    ASSERT_EQ(board->spec_count, 0);
    ASSERT_NOT_NULL(sb_find_epic(board, k_inbox_epic_id));
    ASSERT_STR_EQ(board->epics[0].plan_title, "Inbox Plan");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_grill_plan_title_fallback_and_unlisted) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_write_grill_index(root,
                                   "# .grill/\n\n"
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| indexed | Indexed Title | draft |\n"),
              0);
    ASSERT_EQ(sb_write_epic_md(root, "indexed", "epic-001-a.md", "a", "sa", "pending"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".grill/plans/zeta-unlisted/plan.md"),
                            "---\nslug: zeta-unlisted\ntitle: From Plan Md\n---\n# Z\n"),
              0);
    ASSERT_EQ(sb_write_epic_md(root, "zeta-unlisted", "epic-001-z.md", "z", "sz", "pending"), 0);
    ASSERT_EQ(sb_write_epic_md(root, "alpha-unlisted", "epic-001-x.md", "x", "sx", "pending"), 0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_EQ(board->epic_count, 3);
    ASSERT_STR_EQ(board->epics[0].id, ".grill/plans/indexed/epics/epic-001-a.md");
    ASSERT_STR_EQ(board->epics[0].plan_title, "Indexed Title");
    /* unlisted after index, slug asc: alpha-unlisted then zeta-unlisted */
    ASSERT_STR_EQ(board->epics[1].id, ".grill/plans/alpha-unlisted/epics/epic-001-x.md");
    ASSERT_STR_EQ(board->epics[1].plan_title, "alpha-unlisted");
    ASSERT_STR_EQ(board->epics[2].id, ".grill/plans/zeta-unlisted/epics/epic-001-z.md");
    ASSERT_STR_EQ(board->epics[2].plan_title, "From Plan Md");
    free(board);
    th_rmtree(root);
    PASS();
}

/* ── Task #1: TECH_DEBT.md parse + additive debt JSON (spec-015) ─ */

TEST(spec_board_parse_tech_debt_null_or_empty) {
    cbm_spec_board_debt_t items[CBM_SPEC_BOARD_MAX_DEBT];
    int count = 99;

    memset(items, 0, sizeof(items));
    cbm_spec_board_parse_tech_debt(NULL, items, &count);
    ASSERT_EQ(count, 0);
    count = 99;
    cbm_spec_board_parse_tech_debt("", items, &count);
    ASSERT_EQ(count, 0);
    PASS();
}

TEST(spec_board_debt_open_heading_item) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_tech_debt(root,
                                 "# Tech Debt\n\n"
                                 "## TD-005: leftover cache\n"
                                 "Title: wrong title from field\n"
                                 "Status: identified\n"
                                 "Description: leftover cache body.\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].id, "TD-005");
    ASSERT_STR_EQ(board->debt[0].title, "leftover cache");

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[{\"id\":\"TD-005\",\"title\":\"leftover cache\"}]"));
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_NULL(strstr(json, "\"severity\""));
    ASSERT_NULL(strstr(json, "\"category\""));
    ASSERT_NULL(strstr(json, "\"status\""));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_debt_all_resolved_empty) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_tech_debt(root,
                                 "## TD-001: one\nStatus: resolved\n\n"
                                 "## TD-002: two\nStatus: resolved\n\n"
                                 "## TD-003: three\nStatus: resolved\n\n"
                                 "## TD-004: four\nStatus: resolved\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_EQ(board->debt_count, 0);

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_debt_missing_file_empty) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_TRUE(board->sdd_skill_present);
    ASSERT_EQ(board->debt_count, 0);

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_debt_heading_status_wins_over_table) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_tech_debt(root,
                                 "## TD-005: leftover cache\n"
                                 "ID: TD-005 | Category: leftover | Status: identified | Identified: 2026-09-01\n\n"
                                 "## Debt Summary\n"
                                 "| ID | Title | Status |\n"
                                 "|----|-------|--------|\n"
                                 "| TD-005 | leftover cache | resolved |\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NOT_NULL(sb_find_debt(board, "TD-005"));
    ASSERT_STR_EQ(sb_find_debt(board, "TD-005")->title, "leftover cache");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_debt_table_status_when_heading_has_none) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_tech_debt(root,
                                 "## TD-006: no status line\n"
                                 "Title: ignore me\n"
                                 "Description: no Status token here.\n\n"
                                 "## Debt Summary\n"
                                 "| ID | Title | Status |\n"
                                 "|----|-------|--------|\n"
                                 "| TD-006 | no status line | in_progress |\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NOT_NULL(sb_find_debt(board, "TD-006"));
    ASSERT_STR_EQ(sb_find_debt(board, "TD-006")->title, "no status line");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_debt_unknown_status_is_open) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_tech_debt(root, "## TD-007: typo status\nStatus: resolvd\n"), 0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NOT_NULL(sb_find_debt(board, "TD-007"));
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_debt_17th_open_omitted) {
    char *root = sb_mkroot();
    char body[4096];
    int n = 0;
    int i;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    for (i = 1; i <= 17; i++) {
        n += snprintf(body + n, sizeof(body) - (size_t)n, "## TD-%03d: item %d\nStatus: identified\n\n",
                      i, i);
    }
    ASSERT_EQ(sb_write_tech_debt(root, body), 0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_EQ(board->debt_count, 16);
    ASSERT_NULL(sb_find_debt(board, "TD-017"));
    ASSERT_NOT_NULL(sb_find_debt(board, "TD-016"));

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, "TD-017"));
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_debt_unreadable_file_empty) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_tech_debt(root, "## TD-005: leftover cache\nStatus: identified\n"), 0);
    sb_tech_debt_as_dir(root);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_EQ(board->debt_count, 0);

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_debt_heading_without_id_skipped) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_tech_debt(root,
                                 "## leftover with no id\n"
                                 "Status: identified\n\n"
                                 "## TD-005: leftover cache\n"
                                 "Status: identified\n"),
              0);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_NOT_NULL(sb_find_debt(board, "TD-005"));
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].title, "leftover cache");
    {
        int i;
        for (i = 0; i < board->debt_count; i++) {
            ASSERT_TRUE(strcmp(board->debt[i].title, "leftover with no id") != 0);
        }
    }

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, "leftover with no id"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(spec_board_debt_read_leaves_bytes_identical) {
    char *root = sb_mkroot();
    char p_debt[1024], p_active[1024], p_epic[1024];
    char *b_debt, *b_active, *b_epic;
    char *a_debt, *a_active, *a_epic;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(sb_mkdir_sdd(root), 0);
    ASSERT_EQ(sb_write_active(root,
                              "{\"active_spec\":null,\"planned_specs\":[],\"draft_specs\":[],"
                              "\"completed_specs\":[]}\n"),
              0);
    ASSERT_EQ(sb_write_tech_debt(root, "## TD-005: leftover cache\nStatus: identified\n"), 0);
    ASSERT_EQ(sb_write_grill_index(root, k_inbox_index), 0);
    ASSERT_EQ(sb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"), 0);

    snprintf(p_debt, sizeof(p_debt), "%s", TH_PATH(root, ".sdd-skill/baseline/TECH_DEBT.md"));
    snprintf(p_active, sizeof(p_active), "%s", TH_PATH(root, ".sdd-skill/specs/active.json"));
    snprintf(p_epic, sizeof(p_epic), "%s",
             TH_PATH(root, ".grill/plans/inbox-plan/epics/epic-001-inbox.md"));
    b_debt = sb_slurp(p_debt);
    b_active = sb_slurp(p_active);
    b_epic = sb_slurp(p_epic);
    ASSERT_NOT_NULL(b_debt);
    ASSERT_NOT_NULL(b_active);
    ASSERT_NOT_NULL(b_epic);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);

    a_debt = sb_slurp(p_debt);
    a_active = sb_slurp(p_active);
    a_epic = sb_slurp(p_epic);
    ASSERT_STR_EQ(b_debt, a_debt);
    ASSERT_STR_EQ(b_active, a_active);
    ASSERT_STR_EQ(b_epic, a_epic);
    free(b_debt);
    free(b_active);
    free(b_epic);
    free(a_debt);
    free(a_active);
    free(a_epic);
    free(board);
    th_rmtree(root);
    PASS();
}

/* ── Suite ────────────────────────────────────────────────────── */

SUITE(spec_board) {
    RUN_TEST(spec_board_absent_sdd_skill);
    RUN_TEST(spec_board_idle_planned_draft_done);
    RUN_TEST(spec_board_active_compact_state);
    RUN_TEST(spec_board_active_legacy_state_and_blocker);
    RUN_TEST(spec_board_gamedev_presence);
    RUN_TEST(spec_board_extract_blurb_kpi_not_used);
    RUN_TEST(spec_board_extract_blurb_empty_executive_summary);
    RUN_TEST(spec_board_extract_blurb_null_or_empty_input);
    RUN_TEST(spec_board_extract_blurb_not_h1_or_h3);
    RUN_TEST(spec_board_extract_blurb_links_and_newlines);
    RUN_TEST(spec_board_extract_blurb_truncate_walkback);
    RUN_TEST(spec_board_to_json_emits_escaped_blurb);
    RUN_TEST(spec_board_read_kpi_not_used_as_blurb);
    RUN_TEST(spec_board_nonactive_done_requires_spec_id);
    RUN_TEST(spec_board_missing_spec_md_degrades_one);
    RUN_TEST(spec_board_missing_tasks_md_empty);
    RUN_TEST(spec_board_unreadable_spec_md_omits_blurb);
    RUN_TEST(spec_board_enrich_every_listed);
    RUN_TEST(spec_board_to_json_emits_archived_false_and_true);
    RUN_TEST(spec_board_read_never_sets_archived);
    RUN_TEST(spec_board_grill_mixed_todo_json);
    RUN_TEST(spec_board_grill_companion_to_exact_omits);
    RUN_TEST(spec_board_grill_active_json_source_omits);
    RUN_TEST(spec_board_grill_companion_trailing_notes_match);
    RUN_TEST(spec_board_grill_done_spec_claim_omits);
    RUN_TEST(spec_board_grill_two_plans_index_then_nnn);
    RUN_TEST(spec_board_grill_closed_plan_leftover_stays);
    RUN_TEST(spec_board_grill_pending_and_detailed_listed);
    RUN_TEST(spec_board_grill_missing_companion_keeps_epic);
    RUN_TEST(spec_board_grill_65th_omitted_specs_stay_64);
    RUN_TEST(spec_board_grill_no_grill_dir);
    RUN_TEST(spec_board_grill_unreadable_epic_skipped);
    RUN_TEST(spec_board_grill_read_leaves_bytes_identical);
    RUN_TEST(spec_board_grill_fills_without_sdd);
    RUN_TEST(spec_board_grill_plan_title_fallback_and_unlisted);
    RUN_TEST(spec_board_parse_tech_debt_null_or_empty);
    RUN_TEST(spec_board_debt_open_heading_item);
    RUN_TEST(spec_board_debt_all_resolved_empty);
    RUN_TEST(spec_board_debt_missing_file_empty);
    RUN_TEST(spec_board_debt_heading_status_wins_over_table);
    RUN_TEST(spec_board_debt_table_status_when_heading_has_none);
    RUN_TEST(spec_board_debt_unknown_status_is_open);
    RUN_TEST(spec_board_debt_17th_open_omitted);
    RUN_TEST(spec_board_debt_unreadable_file_empty);
    RUN_TEST(spec_board_debt_heading_without_id_skipped);
    RUN_TEST(spec_board_debt_read_leaves_bytes_identical);
}
