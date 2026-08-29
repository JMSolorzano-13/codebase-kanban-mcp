/*
 * test_spec_board.c — Parser tests for the sdd-skill Spec Board (Kanban).
 *
 * Fixtures are synthetic under /tmp. Never touches a real project tree
 * (e.g. bevy-tetris). Zero-write contract of the production reader is
 * preserved: tests only exercise cbm_spec_board_read / to_json.
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

/* ── Tests ────────────────────────────────────────────────────── */

TEST(spec_board_absent_sdd_skill) {
    char *root = sb_mkroot();
    ASSERT_NOT_NULL(root);

    cbm_spec_board_t *board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_spec_board_read(root, board);
    ASSERT_FALSE(board->sdd_skill_present);
    ASSERT_EQ(board->spec_count, 0);
    ASSERT_FALSE(cbm_spec_board_sdd_skill_present(root));

    char *json = cbm_spec_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"sdd_skill_present\":false"));
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
    /* Non-active specs intentionally skip deep parse. */
    ASSERT_EQ(planned->task_count, 0);
    ASSERT_FLOAT_EQ(planned->checklist_percent, -1.0, 0.01);

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

/* ── Suite ────────────────────────────────────────────────────── */

SUITE(spec_board) {
    RUN_TEST(spec_board_absent_sdd_skill);
    RUN_TEST(spec_board_idle_planned_draft_done);
    RUN_TEST(spec_board_active_compact_state);
    RUN_TEST(spec_board_active_legacy_state_and_blocker);
    RUN_TEST(spec_board_gamedev_presence);
}
