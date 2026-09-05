/**
 * @sdd-task: Task #1 - game_board debt parse + additive JSON
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-072 always-emit debt[{id,title}]; SDD-ADR-073 parse backlog.md in game_board.c
 * @sdd-why: Open debt:* only; cap 16; fixtures under /tmp; never parse a live backlog.md
 * @human-debug: If Game debt stays [] with an open comment → tag charset / resolved-by in body / path not regular; if 17th shows → cap not applied
 *
 * test_game_board.c — Parse / present / artifact walk / inbox / expand / overlay.
 * Fixtures under /tmp only. Never touches a real .gamedev tree.
 */
#include "../src/foundation/compat.h"
#include "../src/foundation/compat_fs.h"
#include "../src/foundation/platform.h"
#include "test_framework.h"
#include "test_helpers.h"
#include "ui/game_board.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *gb_mkroot(void) {
    return th_mktempdir("cbm_game_board");
}

static char *gb_read_alloc(const char *path) {
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

static int gb_assert_empty_counts(const cbm_game_board_t *b) {
    ASSERT_EQ(b->inbox_count, 0);
    ASSERT_EQ(b->preproduction_count, 0);
    ASSERT_EQ(b->production_count, 0);
    ASSERT_EQ(b->postproduction_count, 0);
    return 0;
}

static int gb_assert_empty_arrays_json(const char *json) {
    ASSERT_NOT_NULL(strstr(json, "\"blocked\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"inbox\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"preproduction\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"production\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"postproduction\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NULL(strstr(json, "Pre-production"));
    ASSERT_NULL(strstr(json, "Post-production"));
    return 0;
}

static const char *k_inbox_epic_id = ".grill/plans/inbox-plan/epics/epic-001-inbox.md";

static int gb_write_grill_index(const char *root, const char *body) {
    return th_write_file(TH_PATH(root, ".grill/index.md"), body);
}

static int gb_write_epic_md(const char *root, const char *slug, const char *filename,
                            const char *name, const char *summary, const char *status) {
    char rel[512];
    char body[1024];
    snprintf(rel, sizeof(rel), ".grill/plans/%s/epics/%s", slug, filename);
    snprintf(body, sizeof(body), "name: %s\nstatus: %s\n\nsummary: %s\n", name, status, summary);
    return th_write_file(TH_PATH(root, rel), body);
}

static int gb_seed_gamedev(const char *root) {
    return th_mkdir_p(TH_PATH(root, ".gamedev"));
}

static int gb_write_backlog(const char *root, const char *body) {
    return th_write_file(TH_PATH(root, ".gamedev/backlog.md"), body);
}

static int gb_write_registry(const char *root, const char *rows) {
    char body[8192];
    snprintf(body, sizeof(body),
             "| Epic | Plan | Name | Origin | Status |\n"
             "|------|------|------|--------|--------|\n"
             "%s",
             rows ? rows : "");
    return th_write_file(TH_PATH(root, ".gamedev/epics_registry.md"), body);
}

static int gb_seed_inbox_plan_epic(const char *root) {
    if (gb_seed_gamedev(root) != 0) {
        return -1;
    }
    if (gb_write_grill_index(root,
                             "| slug | title | status |\n"
                             "|------|-------|--------|\n"
                             "| inbox-plan | Inbox Plan | draft |\n") != 0) {
        return -1;
    }
    return gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending");
}

static int gb_write_gdd_companion(const char *root) {
    return th_write_file(TH_PATH(root, ".gamedev/phases/01-preproduction/gdd.md"),
                         "status: draft\n"
                         "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md\n");
}

static int gb_inbox_has_id(const cbm_game_board_t *b, const char *id) {
    int i;
    for (i = 0; i < b->inbox_count; i++) {
        if (strcmp(b->inbox[i].id, id) == 0) {
            return 1;
        }
    }
    return 0;
}

TEST(game_board_absent_gamedev) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    ASSERT_NOT_NULL(root);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_FALSE(board->gamedev_skill_present);
    ASSERT_STR_EQ(board->phase, "");
    ASSERT_STR_EQ(board->focus, "");
    ASSERT_STR_EQ(board->continue_cmd, "");
    if (gb_assert_empty_counts(board) != 0) {
        return 1;
    }
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"gamedev_skill_present\":false"));
    ASSERT_NOT_NULL(strstr(json, "\"phase\":null"));
    ASSERT_NOT_NULL(strstr(json, "\"focus\":null"));
    ASSERT_NOT_NULL(strstr(json, "\"continue\":\"\""));
    if (gb_assert_empty_arrays_json(json) != 0) {
        return 1;
    }
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_empty_dir_present_true) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    ASSERT_STR_EQ(board->phase, "");
    ASSERT_STR_EQ(board->focus, "");
    ASSERT_STR_EQ(board->continue_cmd, "/gamedev-skill continue");
    if (gb_assert_empty_counts(board) != 0) {
        return 1;
    }
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"gamedev_skill_present\":true"));
    ASSERT_NOT_NULL(strstr(json, "\"phase\":null"));
    ASSERT_NOT_NULL(strstr(json, "\"focus\":null"));
    ASSERT_NOT_NULL(strstr(json, "\"continue\":\"/gamedev-skill continue\""));
    if (gb_assert_empty_arrays_json(json) != 0) {
        return 1;
    }
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_compact_phase_focus) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"),
                            "phase=02-production focus=\"Triaging playtest round 3\"\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    ASSERT_STR_EQ(board->phase, "02-production");
    ASSERT_STR_EQ(board->focus, "Triaging playtest round 3");
    ASSERT_STR_EQ(board->continue_cmd, "/gamedev-skill continue");
    if (gb_assert_empty_counts(board) != 0) {
        return 1;
    }
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"phase\":\"02-production\""));
    ASSERT_NOT_NULL(strstr(json, "\"focus\":\"Triaging playtest round 3\""));
    if (gb_assert_empty_arrays_json(json) != 0) {
        return 1;
    }
    ASSERT_NULL(strstr(json, "\"Production\""));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_prefers_compact_over_alias) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"),
                            "active_phase=01-preproduction\n"
                            "director_focus=\"legacy focus\"\n"
                            "phase=03-postproduction focus=\"compact focus\"\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_STR_EQ(board->phase, "03-postproduction");
    ASSERT_STR_EQ(board->focus, "compact focus");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_legacy_equals_alias) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"),
                            "active_phase=01-preproduction director_focus=\"legacy eq\"\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_STR_EQ(board->phase, "01-preproduction");
    ASSERT_STR_EQ(board->focus, "legacy eq");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_legacy_colon_alias) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"),
                            "# leftover\n"
                            "active_phase: 02-production\n"
                            "director_focus: colon focus notes\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_STR_EQ(board->phase, "02-production");
    ASSERT_STR_EQ(board->focus, "colon focus notes");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_unknown_phase_token_is_null) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"),
                            "phase=99-unknown focus=\"still here\"\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_STR_EQ(board->phase, "");
    ASSERT_STR_EQ(board->focus, "still here");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"phase\":null"));
    ASSERT_NOT_NULL(strstr(json, "\"focus\":\"still here\""));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_unreadable_state_md) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/state.md")), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    ASSERT_STR_EQ(board->phase, "");
    ASSERT_STR_EQ(board->focus, "");
    ASSERT_STR_EQ(board->continue_cmd, "/gamedev-skill continue");
    free(board);
    th_rmtree(root);
    PASS();
}

static int gb_assert_card_json(const char *json, const char *id, const char *kind, const char *title,
                               const char *track, const char *work_state, const char *owner,
                               const char *cont) {
    char key[320];
    const char *p;
    const char *next;
    size_t span;
    char buf[4096];

    snprintf(key, sizeof(key), "\"id\":\"%s\"", id);
    p = strstr(json, key);
    ASSERT_NOT_NULL(p);
    next = strstr(p + 1, "\"id\":");
    while (p > json && *p != '{') {
        p--;
    }
    span = next ? (size_t)(next - p) : strlen(p);
    if (span >= sizeof(buf)) {
        span = sizeof(buf) - 1;
    }
    memcpy(buf, p, span);
    buf[span] = '\0';
    snprintf(key, sizeof(key), "\"kind\":\"%s\"", kind);
    ASSERT_NOT_NULL(strstr(buf, key));
    snprintf(key, sizeof(key), "\"title\":\"%s\"", title);
    ASSERT_NOT_NULL(strstr(buf, key));
    snprintf(key, sizeof(key), "\"track\":\"%s\"", track);
    ASSERT_NOT_NULL(strstr(buf, key));
    snprintf(key, sizeof(key), "\"work_state\":\"%s\"", work_state);
    ASSERT_NOT_NULL(strstr(buf, key));
    snprintf(key, sizeof(key), "\"owner\":\"%s\"", owner);
    ASSERT_NOT_NULL(strstr(buf, key));
    snprintf(key, sizeof(key), "\"continue\":\"%s\"", cont);
    ASSERT_NOT_NULL(strstr(buf, key));
    ASSERT_NOT_NULL(strstr(buf, "\"summary\":\"\""));
    ASSERT_NOT_NULL(strstr(buf, "\"plan_title\":\"\""));
    ASSERT_NOT_NULL(strstr(buf, "\"blurb\":"));
    ASSERT_NOT_NULL(strstr(buf, "\"tasks\":"));
    ASSERT_NOT_NULL(strstr(buf, "\"inputs\":"));
    ASSERT_NOT_NULL(strstr(buf, "\"last_decision\":"));
    ASSERT_NOT_NULL(strstr(buf, "\"open\":"));
    ASSERT_NOT_NULL(strstr(buf, "\"recent\":"));
    ASSERT_NOT_NULL(strstr(buf, "\"blocked_by\":"));
    ASSERT_NOT_NULL(strstr(buf, "\"archived\":"));
    ASSERT_NULL(strstr(buf, "\"column\""));
    ASSERT_NULL(strstr(buf, "has_more"));
    return 0;
}

static int gb_array_len(const char *json, const char *key) {
    char open[80];
    const char *p;
    int depth = 1;
    int n = 0;
    bool in_str = false;

    snprintf(open, sizeof(open), "\"%s\":[", key);
    p = strstr(json, open);
    if (!p) {
        return -1;
    }
    p += strlen(open);
    if (*p == ']') {
        return 0;
    }
    while (*p && depth > 0) {
        if (in_str) {
            if (*p == '\\' && p[1]) {
                p += 2;
                continue;
            }
            if (*p == '"') {
                in_str = false;
            }
            p++;
            continue;
        }
        if (*p == '"') {
            in_str = true;
        } else if (*p == '{') {
            if (depth == 1) {
                n++;
            }
            depth++;
        } else if (*p == '[') {
            depth++;
        } else if (*p == '}' || *p == ']') {
            depth--;
        }
        p++;
    }
    return n;
}

TEST(game_board_gdd_artifact_json) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/01-preproduction/gdd.md"),
                            "status: draft\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/docs/agents.md"),
                            "| gdd.md | @analyst | H |\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    ASSERT_EQ(board->preproduction_count, 1);
    ASSERT_STR_EQ(board->preproduction[0].id, ".gamedev/phases/01-preproduction/gdd.md");
    ASSERT_STR_EQ(board->preproduction[0].kind, "artifact");
    ASSERT_STR_EQ(board->preproduction[0].title, "gdd.md");
    ASSERT_STR_EQ(board->preproduction[0].track, "B");
    ASSERT_STR_EQ(board->preproduction[0].work_state, "pending");
    ASSERT_STR_EQ(board->preproduction[0].owner, "@game-designer");
    ASSERT_STR_EQ(board->preproduction[0].continue_cmd, "/gamedev-skill continue @game-designer");
    ASSERT_STR_EQ(board->preproduction[0].summary, "");
    ASSERT_STR_EQ(board->preproduction[0].plan_title, "");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    if (gb_assert_card_json(json, ".gamedev/phases/01-preproduction/gdd.md", "artifact", "gdd.md",
                            "B", "pending", "@game-designer",
                            "/gamedev-skill continue @game-designer") != 0) {
        return 1;
    }
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_NULL(strstr(json, "\"column\""));
    ASSERT_NOT_NULL(strstr(json, "\"continue\":\"/gamedev-skill continue\""));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_missing_narrative_bible_not_placeholder) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/01-preproduction/gdd.md"),
                            "status: draft\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->preproduction_count, 1);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, "narrative-bible.md"));
    ASSERT_NOT_NULL(strstr(json, ".gamedev/phases/01-preproduction/gdd.md"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_sys_dir_without_spec_md) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-001-movement")),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/02-production/systems/loose.md"),
                            "status: wip\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 1);
    ASSERT_STR_EQ(board->production[0].id, ".gamedev/phases/02-production/systems/SYS-001-movement");
    ASSERT_STR_EQ(board->production[0].title, "SYS-001-movement");
    ASSERT_STR_EQ(board->production[0].track, "A");
    ASSERT_STR_EQ(board->production[0].work_state, "pending");
    ASSERT_STR_EQ(board->production[0].owner, "@gameplay-engineer");
    ASSERT_STR_EQ(board->production[0].continue_cmd, "/gamedev-skill continue @gameplay-engineer");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    if (gb_assert_card_json(json, ".gamedev/phases/02-production/systems/SYS-001-movement",
                            "artifact", "SYS-001-movement", "A", "pending", "@gameplay-engineer",
                            "/gamedev-skill continue @gameplay-engineer") != 0) {
        return 1;
    }
    ASSERT_NULL(strstr(json, "loose.md"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_two_in_progress_stay_in_production) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-001-movement/spec.md"),
                            "status: in_review\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-002-score/spec.md"),
                            "status: wip\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 2);
    ASSERT_STR_EQ(board->production[0].id, ".gamedev/phases/02-production/systems/SYS-001-movement");
    ASSERT_STR_EQ(board->production[0].work_state, "in_progress");
    ASSERT_STR_EQ(board->production[1].id, ".gamedev/phases/02-production/systems/SYS-002-score");
    ASSERT_STR_EQ(board->production[1].work_state, "in_progress");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    if (gb_assert_card_json(json, ".gamedev/phases/02-production/systems/SYS-001-movement",
                            "artifact", "SYS-001-movement", "A", "in_progress",
                            "@gameplay-engineer",
                            "/gamedev-skill continue @gameplay-engineer") != 0) {
        return 1;
    }
    if (gb_assert_card_json(json, ".gamedev/phases/02-production/systems/SYS-002-score", "artifact",
                            "SYS-002-score", "A", "in_progress", "@gameplay-engineer",
                            "/gamedev-skill continue @gameplay-engineer") != 0) {
        return 1;
    }
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_status_ready_is_in_progress) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/02-production/qa/playtest-log.md"),
                            "status: ready\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 1);
    ASSERT_STR_EQ(board->production[0].id, ".gamedev/phases/02-production/qa/playtest-log.md");
    ASSERT_STR_EQ(board->production[0].work_state, "in_progress");
    ASSERT_STR_EQ(board->production[0].track, "H");
    ASSERT_STR_EQ(board->production[0].owner, "@qa-lead");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    if (gb_assert_card_json(json, ".gamedev/phases/02-production/qa/playtest-log.md", "artifact",
                            "playtest-log.md", "H", "in_progress", "@qa-lead",
                            "/gamedev-skill continue @qa-lead") != 0) {
        return 1;
    }
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_65th_production_omitted_no_has_more) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    int i;
    char rel[160];

    ASSERT_NOT_NULL(root);
    for (i = 1; i <= 65; i++) {
        snprintf(rel, sizeof(rel), ".gamedev/phases/02-production/systems/SYS-%03d-sys", i);
        ASSERT_EQ(th_mkdir_p(TH_PATH(root, rel)), 0);
    }
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 64);
    ASSERT_STR_EQ(board->production[0].title, "SYS-001-sys");
    ASSERT_STR_EQ(board->production[63].title, "SYS-064-sys");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(gb_array_len(json, "production"), 64);
    ASSERT_NULL(strstr(json, "SYS-065-sys"));
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_non_cards_and_postprod_exist_only) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"), "phase=02-production focus=\"x\"\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/roadmap.md"), "not a card\n"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/game_context.md"), "not a card\n"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/backlog.md"), "not a card\n"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/assets_registry.md"), "not a card\n"), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/docs")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/baseline")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/history")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/prompts")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/03-postproduction/optimization.md"),
                            "status: approved\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/03-postproduction/postmortem.md"),
                            "status: blocked\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_STR_EQ(board->phase, "02-production");
    ASSERT_EQ(board->postproduction_count, 2);
    ASSERT_STR_EQ(board->postproduction[0].id, ".gamedev/phases/03-postproduction/optimization.md");
    ASSERT_STR_EQ(board->postproduction[0].work_state, "done");
    ASSERT_STR_EQ(board->postproduction[0].owner, "@performance-engineer");
    ASSERT_STR_EQ(board->postproduction[1].id, ".gamedev/phases/03-postproduction/postmortem.md");
    ASSERT_STR_EQ(board->postproduction[1].work_state, "blocked");
    ASSERT_STR_EQ(board->postproduction[1].owner, "@analyst");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, "\"id\":\".gamedev/state.md\""));
    ASSERT_NULL(strstr(json, "roadmap.md"));
    ASSERT_NULL(strstr(json, "game_context.md"));
    ASSERT_NULL(strstr(json, "backlog.md"));
    ASSERT_NULL(strstr(json, "assets_registry.md"));
    ASSERT_NULL(strstr(json, "platform-integration.md"));
    ASSERT_EQ(board->inbox_count, 0);
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_production_order_sys_lvl_art_qa) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-010-late")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-002-early")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/phases/02-production/levels/LVL-001-hub")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/phases/02-production/art/z-last")), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/phases/02-production/art/a-first")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/02-production/qa/playtest-log.md"),
                            "status: draft\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 6);
    ASSERT_STR_EQ(board->production[0].title, "SYS-002-early");
    ASSERT_STR_EQ(board->production[1].title, "SYS-010-late");
    ASSERT_STR_EQ(board->production[2].title, "LVL-001-hub");
    ASSERT_STR_EQ(board->production[2].track, "H");
    ASSERT_STR_EQ(board->production[3].title, "a-first");
    ASSERT_STR_EQ(board->production[3].owner, "@content-artist");
    ASSERT_STR_EQ(board->production[4].title, "z-last");
    ASSERT_STR_EQ(board->production[5].title, "playtest-log.md");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_present_false_skips_phase_walk) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, "gdd.md"), "status: draft\n"), 0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_FALSE(board->gamedev_skill_present);
    if (gb_assert_empty_counts(board) != 0) {
        return 1;
    }
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    if (gb_assert_empty_arrays_json(json) != 0) {
        return 1;
    }
    ASSERT_NOT_NULL(strstr(json, "\"gamedev_skill_present\":false"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

static int gb_assert_epic_json(const char *json, const char *id, const char *title,
                               const char *summary, const char *plan_title) {
    char key[320];
    const char *p;
    const char *next;
    size_t span;
    char buf[4096];

    snprintf(key, sizeof(key), "\"id\":\"%s\"", id);
    p = strstr(json, key);
    ASSERT_NOT_NULL(p);
    next = strstr(p + 1, "\"id\":");
    while (p > json && *p != '{') {
        p--;
    }
    span = next ? (size_t)(next - p) : strlen(p);
    if (span >= sizeof(buf)) {
        span = sizeof(buf) - 1;
    }
    memcpy(buf, p, span);
    buf[span] = '\0';
    ASSERT_NOT_NULL(strstr(buf, "\"kind\":\"epic\""));
    snprintf(key, sizeof(key), "\"title\":\"%s\"", title);
    ASSERT_NOT_NULL(strstr(buf, key));
    snprintf(key, sizeof(key), "\"summary\":\"%s\"", summary);
    ASSERT_NOT_NULL(strstr(buf, key));
    snprintf(key, sizeof(key), "\"plan_title\":\"%s\"", plan_title);
    ASSERT_NOT_NULL(strstr(buf, key));
    ASSERT_NOT_NULL(strstr(buf, "\"track\":null"));
    ASSERT_NOT_NULL(strstr(buf, "\"work_state\":null"));
    ASSERT_NOT_NULL(strstr(buf, "\"owner\":\"\""));
    ASSERT_NOT_NULL(strstr(buf, "\"continue\":\"/gamedev-skill continue\""));
    ASSERT_NOT_NULL(strstr(buf, "\"blurb\":\"\""));
    ASSERT_NOT_NULL(strstr(buf, "\"tasks\":[]"));
    ASSERT_NOT_NULL(strstr(buf, "\"inputs\":\"\""));
    ASSERT_NOT_NULL(strstr(buf, "\"last_decision\":\"\""));
    ASSERT_NOT_NULL(strstr(buf, "\"open\":\"\""));
    ASSERT_NOT_NULL(strstr(buf, "\"recent\":\"\""));
    ASSERT_NOT_NULL(strstr(buf, "\"blocked_by\":null"));
    ASSERT_NOT_NULL(strstr(buf, "\"archived\":false"));
    ASSERT_NULL(strstr(buf, "\"column\""));
    return 0;
}

TEST(game_board_unconverted_grill_epic_inbox) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "# .grill/\n\n"
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| inbox-plan | Inbox Plan | draft |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox",
                              "Filter unread first.", "pending"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/roadmap.md"), "no table cell for this plan\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    ASSERT_STR_EQ(board->inbox[0].kind, "epic");
    ASSERT_STR_EQ(board->inbox[0].title, "inbox");
    ASSERT_STR_EQ(board->inbox[0].summary, "Filter unread first.");
    ASSERT_STR_EQ(board->inbox[0].plan_title, "Inbox Plan");
    ASSERT_STR_EQ(board->inbox[0].track, "");
    ASSERT_STR_EQ(board->inbox[0].work_state, "");
    ASSERT_STR_EQ(board->inbox[0].owner, "");
    ASSERT_STR_EQ(board->inbox[0].continue_cmd, "/gamedev-skill continue");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    if (gb_assert_epic_json(json, k_inbox_epic_id, "inbox", "Filter unread first.", "Inbox Plan") !=
        0) {
        return 1;
    }
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_companion_to_exact_path_omits_epic) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    char epic_path[1024];
    char *before;
    char *after;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| inbox-plan | Inbox Plan | draft |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox",
                              "Filter unread first.", "pending"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/01-preproduction/gdd.md"),
                            "status: draft\n"
                            "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md "
                            "(see notes)\n"),
              0);
    snprintf(epic_path, sizeof(epic_path), "%s/%s", root, k_inbox_epic_id);
    before = gb_read_alloc(epic_path);
    ASSERT_NOT_NULL(before);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, k_inbox_epic_id));
    after = gb_read_alloc(epic_path);
    ASSERT_NOT_NULL(after);
    ASSERT_STR_EQ(before, after);
    free(before);
    free(after);
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_roadmap_slug_plus_table_nnn_omits) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| inbox-plan | Inbox Plan | draft |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/roadmap.md"),
                            "Token inbox-plan is claimed.\n\n"
                            "| Epic | Status |\n"
                            "|------|--------|\n"
                            "| 001 | mapped |\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, k_inbox_epic_id));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_closed_grill_plan_leftover_stays) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| old-plan | Old Plan | closed |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "old-plan", "epic-009-leftover.md", "leftover", "Left.",
                              "pending"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, ".grill/plans/old-plan/epics/epic-009-leftover.md");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_missing_conversion_sits_beside_sys) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| inbox-plan | Inbox Plan | draft |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-001-inbox")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/roadmap.md"), "no 001 cell here\n"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    ASSERT_EQ(board->production_count, 1);
    ASSERT_STR_EQ(board->production[0].id, ".gamedev/phases/02-production/systems/SYS-001-inbox");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_no_grill_directory_empty_inbox) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    ASSERT_EQ(board->inbox_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"inbox\":[]"));
    ASSERT_FALSE(cbm_is_dir(TH_PATH(root, ".grill")));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_inbox_order_index_then_nnn) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| plan-a | Plan A | draft |\n"
                                   "| plan-b | Plan B | draft |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "plan-a", "epic-002-second.md", "second", "s2", "pending"), 0);
    ASSERT_EQ(gb_write_epic_md(root, "plan-a", "epic-001-first.md", "first", "s1", "pending"), 0);
    ASSERT_EQ(gb_write_epic_md(root, "plan-b", "epic-001-other.md", "other", "s3", "pending"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 3);
    ASSERT_STR_EQ(board->inbox[0].id, ".grill/plans/plan-a/epics/epic-001-first.md");
    ASSERT_STR_EQ(board->inbox[1].id, ".grill/plans/plan-a/epics/epic-002-second.md");
    ASSERT_STR_EQ(board->inbox[2].id, ".grill/plans/plan-b/epics/epic-001-other.md");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_plan_folder_cite_without_nnn_stays) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| inbox-plan | Inbox Plan | draft |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/roadmap.md"),
                            "See .grill/plans/inbox-plan/ for leftovers.\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_kebab_name_does_not_convert) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| inbox-plan | Inbox Plan | draft |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-001-inbox")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/roadmap.md"), "no table cell\n"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_table_nnn_without_slug_does_not_convert) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| inbox-plan | Inbox Plan | draft |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "s", "pending"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/roadmap.md"),
                            "| Epic | Status |\n"
                            "|------|--------|\n"
                            "| 001 | mapped |\n"),
              0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/game_context.md"), "no matching token here\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_read_leaves_skill_trees) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char state_path[1024];
    char epic_path[1024];
    char index_path[1024];
    char *state_before;
    char *epic_before;
    char *index_before;
    char *state_after;
    char *epic_after;
    char *index_after;
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev")), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"), "phase=02-production focus=\"keep\"\n"),
              0);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| inbox-plan | Inbox Plan | draft |\n"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-001-inbox.md", "inbox", "keep", "pending"),
              0);
    snprintf(state_path, sizeof(state_path), "%s/.gamedev/state.md", root);
    snprintf(index_path, sizeof(index_path), "%s/.grill/index.md", root);
    snprintf(epic_path, sizeof(epic_path), "%s/%s", root, k_inbox_epic_id);
    state_before = gb_read_alloc(state_path);
    index_before = gb_read_alloc(index_path);
    epic_before = gb_read_alloc(epic_path);
    ASSERT_NOT_NULL(state_before);
    ASSERT_NOT_NULL(index_before);
    ASSERT_NOT_NULL(epic_before);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    state_after = gb_read_alloc(state_path);
    index_after = gb_read_alloc(index_path);
    epic_after = gb_read_alloc(epic_path);
    ASSERT_NOT_NULL(state_after);
    ASSERT_NOT_NULL(index_after);
    ASSERT_NOT_NULL(epic_after);
    ASSERT_STR_EQ(state_before, state_after);
    ASSERT_STR_EQ(index_before, index_after);
    ASSERT_STR_EQ(epic_before, epic_after);
    ASSERT_FALSE(cbm_is_dir(TH_PATH(root, ".sdd-skill")));
    ASSERT_FALSE(cbm_file_exists(TH_PATH(root, ".sdd-skill")));
    free(state_before);
    free(epic_before);
    free(index_before);
    free(state_after);
    free(epic_after);
    free(index_after);
    free(board);
    th_rmtree(root);
    PASS();
}

static const char *k_sys_id = ".gamedev/phases/02-production/systems/SYS-001-movement";
static const char *k_gdd_id = ".gamedev/phases/01-preproduction/gdd.md";
static const char *k_playtest_id = ".gamedev/phases/02-production/qa/playtest-log.md";
static const char *k_lvl_id = ".gamedev/phases/02-production/levels/LVL-001-well";

static int gb_write_sys_expand(const char *root) {
    if (th_write_file(TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-001-movement/spec.md"),
                      "status: draft\n"
                      "open: Ignore this open line.\n"
                      "last_decision: —\n\n"
                      "## What it does\n\n"
                      "Moves the tetromino left and right. DAS applies after the first tap.\n\n"
                      "## Inputs / Outputs\n\n"
                      "Grid occupancy from collision. Outputs a new piece position.\n\n"
                      "## Acceptance\n\n"
                      "Do not dump this section.\n") != 0) {
        return -1;
    }
    return th_write_file(
        TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-001-movement/tasks.md"),
        "- [x] Parse input\n"
        "- [ ] Apply DAS\n"
        "\n"
        "<!-- not a task -->\n"
        "- [ ] Subagent: skip me\n"
        "- [ ] Path: skip me too\n");
}

TEST(game_board_track_a_sys_blurb_tasks_inputs) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    const cbm_game_board_card_t *c;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_write_sys_expand(root), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 1);
    c = &board->production[0];
    ASSERT_STR_EQ(c->id, k_sys_id);
    ASSERT_STR_EQ(c->track, "A");
    ASSERT_STR_EQ(c->blurb, "Moves the tetromino left and right. DAS applies after the first tap.");
    ASSERT_STR_EQ(c->inputs, "Grid occupancy from collision. Outputs a new piece position.");
    ASSERT_STR_EQ(c->open, "Ignore this open line.");
    ASSERT_EQ(c->task_count, 2);
    ASSERT_EQ(c->tasks[0].number, 1);
    ASSERT_STR_EQ(c->tasks[0].name, "Parse input");
    ASSERT_TRUE(c->tasks[0].done);
    ASSERT_EQ(c->tasks[1].number, 2);
    ASSERT_STR_EQ(c->tasks[1].name, "Apply DAS");
    ASSERT_FALSE(c->tasks[1].done);
    ASSERT_FALSE(c->archived);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"blurb\":\"Moves the tetromino left and right. DAS applies after the first tap.\""));
    ASSERT_NOT_NULL(strstr(json, "\"inputs\":\"Grid occupancy from collision. Outputs a new piece position.\""));
    ASSERT_NOT_NULL(strstr(json, "{\"number\":1,\"name\":\"Parse input\",\"done\":true}"));
    ASSERT_NOT_NULL(strstr(json, "{\"number\":2,\"name\":\"Apply DAS\",\"done\":false}"));
    ASSERT_NOT_NULL(strstr(json, "\"archived\":false"));
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_NULL(strstr(json, "Do not dump this section"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_track_b_gdd_header_only) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    const cbm_game_board_card_t *c;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/01-preproduction/gdd.md"),
                            "status: draft\n"
                            "last_decision: Lock the loop as rotating tetrominoes.\n"
                            "open: Need player fantasy one-liner.\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->preproduction_count, 1);
    c = &board->preproduction[0];
    ASSERT_STR_EQ(c->id, k_gdd_id);
    ASSERT_STR_EQ(c->track, "B");
    ASSERT_STR_EQ(c->last_decision, "Lock the loop as rotating tetrominoes.");
    ASSERT_STR_EQ(c->open, "Need player fantasy one-liner.");
    ASSERT_STR_EQ(c->blurb, "");
    ASSERT_STR_EQ(c->inputs, "");
    ASSERT_EQ(c->task_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"last_decision\":\"Lock the loop as rotating tetrominoes.\""));
    ASSERT_NOT_NULL(strstr(json, "\"open\":\"Need player fantasy one-liner.\""));
    ASSERT_NOT_NULL(strstr(json, "\"blurb\":\"\""));
    ASSERT_NOT_NULL(strstr(json, "\"tasks\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"inputs\":\"\""));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_what_it_does_wins_over_open) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    const cbm_game_board_card_t *c;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_write_sys_expand(root), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 1);
    c = &board->production[0];
    ASSERT_STR_EQ(c->blurb, "Moves the tetromino left and right. DAS applies after the first tap.");
    ASSERT_STR_EQ(c->open, "Ignore this open line.");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_playtest_recent_last_round_only) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    const cbm_game_board_card_t *c;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/02-production/qa/playtest-log.md"),
                            "status: ready\n\n"
                            "## Round 1 — 2026-08-01 — build 0.1\n"
                            "old finding\n"
                            "## Round 2 — 2026-08-20 — build 0.4\n"
                            "DAS feels sticky\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 1);
    c = &board->production[0];
    ASSERT_STR_EQ(c->id, k_playtest_id);
    ASSERT_NOT_NULL(strstr(c->recent, "Round 2"));
    ASSERT_NOT_NULL(strstr(c->recent, "DAS feels sticky"));
    ASSERT_NULL(strstr(c->recent, "old finding"));
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "Round 2"));
    ASSERT_NOT_NULL(strstr(json, "DAS feels sticky"));
    ASSERT_NULL(strstr(json, "old finding"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_level_recent_last_8_changelog) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    const cbm_game_board_card_t *c;
    int i;
    char body[1024];
    int pos = 0;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/02-production/levels/LVL-001-well/level.md"),
                            "status: draft\nlast_decision: Keep the well.\nopen: Tune gravity.\n"),
              0);
    body[0] = '\0';
    for (i = 1; i <= 10; i++) {
        static const char *names[] = {"one", "two", "three", "four", "five",
                                      "six", "seven", "eight", "nine", "ten"};
        int n = snprintf(body + pos, sizeof(body) - (size_t)pos, "line-%s\n", names[i - 1]);
        ASSERT_GT(n, 0);
        pos += n;
    }
    ASSERT_EQ(th_write_file(
                  TH_PATH(root, ".gamedev/phases/02-production/levels/LVL-001-well/changelog.md"),
                  body),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 1);
    c = &board->production[0];
    ASSERT_STR_EQ(c->id, k_lvl_id);
    ASSERT_NOT_NULL(strstr(c->recent, "line-ten"));
    ASSERT_NULL(strstr(c->recent, "line-one"));
    ASSERT_STR_EQ(c->last_decision, "Keep the well.");
    ASSERT_STR_EQ(c->open, "Tune gravity.");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_48_task_cap_omits_49th) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    char tasks[4096];
    int i;
    int pos = 0;
    const cbm_game_board_card_t *c;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-001-movement/spec.md"),
                            "status: draft\n\n## What it does\n\nCap test.\n"),
              0);
    tasks[0] = '\0';
    for (i = 1; i <= 49; i++) {
        int n = snprintf(tasks + pos, sizeof(tasks) - (size_t)pos, "- [ ] Cap task %02d\n", i);
        ASSERT_GT(n, 0);
        pos += n;
    }
    ASSERT_EQ(th_write_file(
                  TH_PATH(root, ".gamedev/phases/02-production/systems/SYS-001-movement/tasks.md"),
                  tasks),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->production_count, 1);
    c = &board->production[0];
    ASSERT_EQ(c->task_count, 48);
    ASSERT_EQ(c->tasks[47].number, 48);
    ASSERT_STR_EQ(c->tasks[47].name, "Cap task 48");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_NULL(strstr(json, "Cap task 49"));
    ASSERT_NOT_NULL(strstr(json, "Cap task 48"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_needs_review_is_not_strip_row) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"),
                            "phase=02-production focus=\"x\"\n"
                            "content-artist:needs_review:\"Enemy sprites batch 2\"\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->blocked_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"blocked\":[]"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_empty_blocked_json_array) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->blocked_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"blocked\":[]"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_blocked_overlay_owner_match) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"),
                            "phase=02-production focus=\"x\"\n"
                            "gameplay-engineer:blocked:\"Combat system v2\":\"Waiting on final boss design\"\n"),
              0);
    ASSERT_EQ(gb_write_sys_expand(root), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/phases/01-preproduction/gdd.md"),
                            "status: draft\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->blocked_count, 1);
    ASSERT_STR_EQ(board->blocked[0].owner, "@gameplay-engineer");
    ASSERT_STR_EQ(board->blocked[0].task, "Combat system v2");
    ASSERT_STR_EQ(board->blocked[0].blocked_by, "Waiting on final boss design");
    ASSERT_EQ(board->production_count, 1);
    ASSERT_STR_EQ(board->production[0].work_state, "blocked");
    ASSERT_STR_EQ(board->production[0].blocked_by, "Waiting on final boss design");
    ASSERT_EQ(board->preproduction_count, 1);
    ASSERT_STR_EQ(board->preproduction[0].work_state, "pending");
    ASSERT_STR_EQ(board->preproduction[0].blocked_by, "");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(
        json, "{\"owner\":\"@gameplay-engineer\",\"task\":\"Combat system v2\","
              "\"blocked_by\":\"Waiting on final boss design\"}"));
    ASSERT_NOT_NULL(strstr(json, "\"work_state\":\"blocked\""));
    ASSERT_NOT_NULL(strstr(json, "\"blocked_by\":\"Waiting on final boss design\""));
    ASSERT_NOT_NULL(strstr(json, "\"work_state\":\"pending\""));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_expand_read_does_not_write_skill_trees) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char spec_path[1024];
    char tasks_path[1024];
    char state_path[1024];
    char *spec_before;
    char *tasks_before;
    char *state_before;
    char *spec_after;
    char *tasks_after;
    char *state_after;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"),
                            "phase=02-production focus=\"keep\"\n"
                            "gameplay-engineer:blocked:\"Combat system v2\":\"Waiting on final boss design\"\n"),
              0);
    ASSERT_EQ(gb_write_sys_expand(root), 0);
    snprintf(spec_path, sizeof(spec_path),
             "%s/.gamedev/phases/02-production/systems/SYS-001-movement/spec.md", root);
    snprintf(tasks_path, sizeof(tasks_path),
             "%s/.gamedev/phases/02-production/systems/SYS-001-movement/tasks.md", root);
    snprintf(state_path, sizeof(state_path), "%s/.gamedev/state.md", root);
    spec_before = gb_read_alloc(spec_path);
    tasks_before = gb_read_alloc(tasks_path);
    state_before = gb_read_alloc(state_path);
    ASSERT_NOT_NULL(spec_before);
    ASSERT_NOT_NULL(tasks_before);
    ASSERT_NOT_NULL(state_before);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    spec_after = gb_read_alloc(spec_path);
    tasks_after = gb_read_alloc(tasks_path);
    state_after = gb_read_alloc(state_path);
    ASSERT_NOT_NULL(spec_after);
    ASSERT_NOT_NULL(tasks_after);
    ASSERT_NOT_NULL(state_after);
    ASSERT_STR_EQ(spec_before, spec_after);
    ASSERT_STR_EQ(tasks_before, tasks_after);
    ASSERT_STR_EQ(state_before, state_after);
    ASSERT_FALSE(cbm_is_dir(TH_PATH(root, ".sdd-skill")));
    ASSERT_FALSE(cbm_file_exists(TH_PATH(root, ".sdd-skill")));
    free(spec_before);
    free(tasks_before);
    free(state_before);
    free(spec_after);
    free(tasks_after);
    free(state_after);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_parse_epics_registry_null_empty) {
    cbm_game_board_registry_row_t *rows;
    int count = 99;

    rows = calloc(CBM_GAME_BOARD_MAX_REGISTRY, sizeof(*rows));
    ASSERT_NOT_NULL(rows);
    cbm_game_board_parse_epics_registry(NULL, rows, &count);
    ASSERT_EQ(count, 0);
    count = 99;
    cbm_game_board_parse_epics_registry("", rows, &count);
    ASSERT_EQ(count, 0);
    free(rows);
    PASS();
}

TEST(game_board_parse_epics_registry_nnn_and_last_wins) {
    cbm_game_board_registry_row_t *rows;
    int count = 0;
    const char *md =
        "| Epic | Plan | Name | Origin | Status |\n"
        "|------|------|------|--------|--------|\n"
        "| 1 | inbox-plan | a | o | in_progress |\n"
        "| epic-001 | other-plan | b | o | parked |\n"
        "| 001 | inbox-plan | a | o | not_started |\n"
        "| n/a | inbox-plan | x | o | closed |\n"
        "| 0 | inbox-plan | z | o | evergreen |\n"
        "| 002 | native | n | o | closed |\n";

    rows = calloc(CBM_GAME_BOARD_MAX_REGISTRY, sizeof(*rows));
    ASSERT_NOT_NULL(rows);
    cbm_game_board_parse_epics_registry(md, rows, &count);
    ASSERT_EQ(count, 3);
    ASSERT_STR_EQ(rows[0].slug, "inbox-plan");
    ASSERT_EQ(rows[0].nnn, 1);
    ASSERT_FALSE(rows[0].hide);
    ASSERT_STR_EQ(rows[1].slug, "other-plan");
    ASSERT_EQ(rows[1].nnn, 1);
    ASSERT_TRUE(rows[1].hide);
    ASSERT_STR_EQ(rows[2].slug, "inbox-plan");
    ASSERT_EQ(rows[2].nnn, 0);
    ASSERT_FALSE(rows[2].hide);
    free(rows);
    PASS();
}

TEST(game_board_registry_in_progress_omits_leftover) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    char reg_path[1024];
    char epic_path[1024];
    char *reg_before;
    char *epic_before;
    char *reg_after;
    char *epic_after;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, "| 001 | inbox-plan | Inbox | o | in_progress |\n"), 0);
    snprintf(reg_path, sizeof(reg_path), "%s/.gamedev/epics_registry.md", root);
    snprintf(epic_path, sizeof(epic_path), "%s/%s", root, k_inbox_epic_id);
    reg_before = gb_read_alloc(reg_path);
    epic_before = gb_read_alloc(epic_path);
    ASSERT_NOT_NULL(reg_before);
    ASSERT_NOT_NULL(epic_before);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(gb_inbox_has_id(board, k_inbox_epic_id), 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, k_inbox_epic_id));
    reg_after = gb_read_alloc(reg_path);
    epic_after = gb_read_alloc(epic_path);
    ASSERT_NOT_NULL(reg_after);
    ASSERT_NOT_NULL(epic_after);
    ASSERT_STR_EQ(reg_before, reg_after);
    ASSERT_STR_EQ(epic_before, epic_after);
    free(reg_before);
    free(epic_before);
    free(reg_after);
    free(epic_after);
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_not_started_keeps) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, "| 001 | inbox-plan | Inbox | o | not_started |\n"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_closed_parked_omit_norow_stays) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-002-done.md", "done", "s", "pending"), 0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-003-paused.md", "paused", "s", "pending"),
              0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-004-leftover.md", "leftover", "s",
                              "pending"),
              0);
    ASSERT_EQ(gb_write_registry(root,
                                "| 002 | inbox-plan | Done | o | closed |\n"
                                "| 003 | inbox-plan | Paused | o | parked |\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(gb_inbox_has_id(board, ".grill/plans/inbox-plan/epics/epic-002-done.md"), 0);
    ASSERT_EQ(gb_inbox_has_id(board, ".grill/plans/inbox-plan/epics/epic-003-paused.md"), 0);
    ASSERT_EQ(gb_inbox_has_id(board, ".grill/plans/inbox-plan/epics/epic-004-leftover.md"), 1);
    ASSERT_EQ(gb_inbox_has_id(board, k_inbox_epic_id), 1);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_present_companion_does_not_hide) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, ""), 0);
    ASSERT_EQ(gb_write_gdd_companion(root), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_header_only_roadmap_does_not_hide) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, ""), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/roadmap.md"),
                            "Token inbox-plan is claimed.\n\n"
                            "| Epic | Status |\n"
                            "|------|--------|\n"
                            "| 001 | mapped |\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_last_duplicate_closed_omits) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root,
                                "| 001 | inbox-plan | Inbox | o | not_started |\n"
                                "| 001 | inbox-plan | Inbox | o | closed |\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(gb_inbox_has_id(board, k_inbox_epic_id), 0);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_last_duplicate_not_started_keeps) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root,
                                "| 001 | inbox-plan | Inbox | o | closed |\n"
                                "| 001 | inbox-plan | Inbox | o | not_started |\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_epic_cell_1_hides) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, "| 1 | inbox-plan | Inbox | o | in_progress |\n"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(gb_inbox_has_id(board, k_inbox_epic_id), 0);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_epic_cell_epic_001_hides) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, "| epic-001 | inbox-plan | Inbox | o | in_progress |\n"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(gb_inbox_has_id(board, k_inbox_epic_id), 0);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_plan_path_leftover_does_not_match) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, "| 001 | .grill/plans/inbox-plan | Inbox | o | closed |\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_inbox_65th_omitted_no_has_more_registry_absent) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    int i;
    char fn[64];

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_grill_index(root,
                                   "| slug | title | status |\n"
                                   "|------|-------|--------|\n"
                                   "| inbox-plan | Inbox Plan | draft |\n"),
              0);
    for (i = 1; i <= 65; i++) {
        snprintf(fn, sizeof(fn), "epic-%03d-item.md", i);
        ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", fn, "n", "s", "pending"), 0);
    }
    ASSERT_FALSE(cbm_file_exists(TH_PATH(root, ".gamedev/epics_registry.md")));
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 64);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(gb_array_len(json, "inbox"), 64);
    ASSERT_NULL(strstr(json, "epic-065-item.md"));
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_evergreen_epic_0_does_not_hide) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, "| 0 | inbox-plan | Evergreen | o | evergreen |\n"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_unknown_status_does_not_hide) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, "| 001 | inbox-plan | Inbox | o | closd |\n"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_plan_native_never_matches) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, "| 001 | native | Native | o | closed |\n"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_unreadable_present_companion_does_not_hide) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    FILE *f;
    char chunk[4096];
    size_t left;
    char path[1024];

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_gdd_companion(root), 0);
    snprintf(path, sizeof(path), "%s/.gamedev/epics_registry.md", root);
    f = fopen(path, "wb");
    ASSERT_NOT_NULL(f);
    memset(chunk, 'A', sizeof(chunk));
    left = (size_t)1024 * 1024 + 1;
    while (left > 0) {
        size_t n = left < sizeof(chunk) ? left : sizeof(chunk);
        ASSERT_EQ((long long)fwrite(chunk, 1, n, f), (long long)n);
        left -= n;
    }
    fclose(f);
    ASSERT_TRUE(cbm_file_exists(path));
    ASSERT_FALSE(cbm_is_dir(path));
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    ASSERT_EQ(board->inbox_count, 1);
    ASSERT_STR_EQ(board->inbox[0].id, k_inbox_epic_id);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, k_inbox_epic_id));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_registry_malformed_epic_skipped_parked_omits) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_epic_md(root, "inbox-plan", "epic-002-done.md", "done", "s", "pending"), 0);
    ASSERT_EQ(gb_write_registry(root,
                                "| n/a | inbox-plan | Inbox | o | closed |\n"
                                "| 002 | inbox-plan | Done | o | parked |\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(gb_inbox_has_id(board, k_inbox_epic_id), 1);
    ASSERT_EQ(gb_inbox_has_id(board, ".grill/plans/inbox-plan/epics/epic-002-done.md"), 0);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_parse_backlog_debt_null_empty) {
    cbm_game_board_debt_t *rows;
    int count = 99;

    rows = calloc(CBM_GAME_BOARD_MAX_DEBT, sizeof(*rows));
    ASSERT_NOT_NULL(rows);
    cbm_game_board_parse_backlog_debt(NULL, rows, &count);
    ASSERT_EQ(count, 0);
    count = 99;
    cbm_game_board_parse_backlog_debt("", rows, &count);
    ASSERT_EQ(count, 0);
    free(rows);
    PASS();
}

TEST(game_board_debt_open_comment) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "<!-- debt:gate-preproduction missing GDD lock "
                                    "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].id, "debt:gate-preproduction");
    ASSERT_STR_EQ(board->debt[0].title, "missing GDD lock");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(gb_array_len(json, "debt"), 1);
    ASSERT_NOT_NULL(strstr(json, "\"id\":\"debt:gate-preproduction\""));
    ASSERT_NOT_NULL(strstr(json, "\"title\":\"missing GDD lock\""));
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_NULL(strstr(json, "\"severity\""));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_list_item_open) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "- debt:save-slot no checkpoint -- gameplay -- M2\n"), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].id, "debt:save-slot");
    ASSERT_STR_EQ(board->debt[0].title, "no checkpoint");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_resolved_by_following_line) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "<!-- debt:gate-preproduction missing GDD lock "
                                    "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"
                                    "resolved-by: SYS-0.1\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_design_tech_without_prefix_omitted) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "<!-- design camera shake \xe2\x80\x94 gameplay \xe2\x80\x94 "
                                    "M1 -->\n"
                                    "<!-- tech nav mesh bake \xe2\x80\x94 tech-architect "
                                    "\xe2\x80\x94 M1 -->\n"
                                    "<!-- debt:adopt-gap-audio no sfx list \xe2\x80\x94 audio "
                                    "\xe2\x80\x94 M2 -->\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].id, "debt:adopt-gap-audio");
    ASSERT_STR_EQ(board->debt[0].title, "no sfx list");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_missing_file_empty) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_FALSE(cbm_file_exists(TH_PATH(root, ".gamedev/backlog.md")));
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    ASSERT_EQ(board->debt_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_FALSE(cbm_file_exists(TH_PATH(root, ".gamedev/backlog.md")));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_same_line_resolved_by) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "<!-- debt:gate-preproduction missing GDD lock resolved-by: "
                                    "SYS-0.1 \xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 0);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_blank_line_then_resolved_by) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "<!-- debt:gate-preproduction missing GDD lock "
                                    "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"
                                    "\n"
                                    "resolved-by: SYS-0.1\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 0);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_next_start_ends_previous) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "<!-- debt:gate-preproduction missing GDD lock "
                                    "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"
                                    "<!-- debt:adopt-gap-audio no sfx list \xe2\x80\x94 audio "
                                    "\xe2\x80\x94 M2 -->\n"
                                    "resolved-by: SYS-0.2\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].id, "debt:gate-preproduction");
    ASSERT_STR_EQ(board->debt[0].title, "missing GDD lock");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_17th_open_omitted_no_has_more) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    char body[2048];
    int i;
    int n = 0;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    body[0] = '\0';
    for (i = 1; i <= 17; i++) {
        n += snprintf(body + n, sizeof(body) - (size_t)n, "- debt:d%02d item %d\n", i, i);
        ASSERT_TRUE(n > 0 && n < (int)sizeof(body));
    }
    ASSERT_EQ(gb_write_backlog(root, body), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 16);
    ASSERT_STR_EQ(board->debt[0].id, "debt:d01");
    ASSERT_STR_EQ(board->debt[15].id, "debt:d16");
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(gb_array_len(json, "debt"), 16);
    ASSERT_NULL(strstr(json, "debt:d17"));
    ASSERT_NULL(strstr(json, "has_more"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_heading_counts) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "## debt:adopt-gap-audio no sfx list \xe2\x80\x94 audio "
                                    "\xe2\x80\x94 M2\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].id, "debt:adopt-gap-audio");
    ASSERT_STR_EQ(board->debt[0].title, "no sfx list");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_bare_colon_skipped) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "debt:\n"
                                    "<!-- debt:gate-preproduction missing GDD lock "
                                    "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].id, "debt:gate-preproduction");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_backlog_is_not_a_card) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    int i;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "<!-- debt:gate-preproduction missing GDD lock "
                                    "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_EQ(board->inbox_count, 0);
    ASSERT_EQ(board->preproduction_count, 0);
    ASSERT_EQ(board->production_count, 0);
    ASSERT_EQ(board->postproduction_count, 0);
    for (i = 0; i < board->inbox_count; i++) {
        ASSERT_NULL(strstr(board->inbox[i].id, "backlog.md"));
    }
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, "backlog.md"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_inbox_registry_hide_unchanged) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_inbox_plan_epic(root), 0);
    ASSERT_EQ(gb_write_registry(root, "| 001 | inbox-plan | Inbox | o | in_progress |\n"), 0);
    ASSERT_EQ(gb_write_backlog(root, "<!-- debt:gate-preproduction missing GDD lock "
                                    "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(gb_inbox_has_id(board, k_inbox_epic_id), 0);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].id, "debt:gate-preproduction");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_unreadable_oversize_empty) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;
    FILE *f;
    char chunk[4096];
    size_t left;
    char path[1024];

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    snprintf(path, sizeof(path), "%s/.gamedev/backlog.md", root);
    f = fopen(path, "wb");
    ASSERT_NOT_NULL(f);
    memset(chunk, 'A', sizeof(chunk));
    left = (size_t)1024 * 1024 + 1;
    while (left > 0) {
        size_t n = left < sizeof(chunk) ? left : sizeof(chunk);
        ASSERT_EQ((long long)fwrite(chunk, 1, n, f), (long long)n);
        left -= n;
    }
    fclose(f);
    ASSERT_TRUE(cbm_file_exists(path));
    ASSERT_FALSE(cbm_is_dir(path));
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    ASSERT_EQ(board->debt_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_directory_at_path_empty) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char *json;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_mkdir_p(TH_PATH(root, ".gamedev/backlog.md")), 0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_TRUE(board->gamedev_skill_present);
    ASSERT_EQ(board->debt_count, 0);
    json = cbm_game_board_to_json(board);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    free(json);
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_line_without_tag_skipped) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(gb_write_backlog(root, "random prose with no tag\n"
                                    "<!-- debt:gate-preproduction missing GDD lock "
                                    "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    ASSERT_STR_EQ(board->debt[0].id, "debt:gate-preproduction");
    ASSERT_STR_EQ(board->debt[0].title, "missing GDD lock");
    free(board);
    th_rmtree(root);
    PASS();
}

TEST(game_board_debt_read_leaves_skill_bytes) {
    char *root = gb_mkroot();
    cbm_game_board_t *board;
    char backlog_path[1024];
    char state_path[1024];
    char reg_path[1024];
    char *backlog_before;
    char *state_before;
    char *reg_before;
    char *backlog_after;
    char *state_after;
    char *reg_after;

    ASSERT_NOT_NULL(root);
    ASSERT_EQ(gb_seed_gamedev(root), 0);
    ASSERT_EQ(th_write_file(TH_PATH(root, ".gamedev/state.md"), "phase=02-production focus=\"x\"\n"),
              0);
    ASSERT_EQ(gb_write_registry(root, "| 001 | inbox-plan | Inbox | o | not_started |\n"), 0);
    ASSERT_EQ(gb_write_backlog(root, "<!-- debt:gate-preproduction missing GDD lock "
                                    "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    snprintf(backlog_path, sizeof(backlog_path), "%s/.gamedev/backlog.md", root);
    snprintf(state_path, sizeof(state_path), "%s/.gamedev/state.md", root);
    snprintf(reg_path, sizeof(reg_path), "%s/.gamedev/epics_registry.md", root);
    backlog_before = gb_read_alloc(backlog_path);
    state_before = gb_read_alloc(state_path);
    reg_before = gb_read_alloc(reg_path);
    ASSERT_NOT_NULL(backlog_before);
    ASSERT_NOT_NULL(state_before);
    ASSERT_NOT_NULL(reg_before);
    board = calloc(1, sizeof(*board));
    ASSERT_NOT_NULL(board);
    cbm_game_board_read(root, board);
    ASSERT_EQ(board->debt_count, 1);
    backlog_after = gb_read_alloc(backlog_path);
    state_after = gb_read_alloc(state_path);
    reg_after = gb_read_alloc(reg_path);
    ASSERT_NOT_NULL(backlog_after);
    ASSERT_NOT_NULL(state_after);
    ASSERT_NOT_NULL(reg_after);
    ASSERT_STR_EQ(backlog_before, backlog_after);
    ASSERT_STR_EQ(state_before, state_after);
    ASSERT_STR_EQ(reg_before, reg_after);
    ASSERT_FALSE(cbm_file_exists(TH_PATH(root, ".sdd-skill")));
    free(backlog_before);
    free(state_before);
    free(reg_before);
    free(backlog_after);
    free(state_after);
    free(reg_after);
    free(board);
    th_rmtree(root);
    PASS();
}

SUITE(game_board) {
    RUN_TEST(game_board_absent_gamedev);
    RUN_TEST(game_board_empty_dir_present_true);
    RUN_TEST(game_board_compact_phase_focus);
    RUN_TEST(game_board_prefers_compact_over_alias);
    RUN_TEST(game_board_legacy_equals_alias);
    RUN_TEST(game_board_legacy_colon_alias);
    RUN_TEST(game_board_unknown_phase_token_is_null);
    RUN_TEST(game_board_unreadable_state_md);
    RUN_TEST(game_board_gdd_artifact_json);
    RUN_TEST(game_board_missing_narrative_bible_not_placeholder);
    RUN_TEST(game_board_sys_dir_without_spec_md);
    RUN_TEST(game_board_two_in_progress_stay_in_production);
    RUN_TEST(game_board_status_ready_is_in_progress);
    RUN_TEST(game_board_65th_production_omitted_no_has_more);
    RUN_TEST(game_board_non_cards_and_postprod_exist_only);
    RUN_TEST(game_board_production_order_sys_lvl_art_qa);
    RUN_TEST(game_board_present_false_skips_phase_walk);
    RUN_TEST(game_board_unconverted_grill_epic_inbox);
    RUN_TEST(game_board_companion_to_exact_path_omits_epic);
    RUN_TEST(game_board_roadmap_slug_plus_table_nnn_omits);
    RUN_TEST(game_board_closed_grill_plan_leftover_stays);
    RUN_TEST(game_board_missing_conversion_sits_beside_sys);
    RUN_TEST(game_board_no_grill_directory_empty_inbox);
    RUN_TEST(game_board_inbox_order_index_then_nnn);
    RUN_TEST(game_board_plan_folder_cite_without_nnn_stays);
    RUN_TEST(game_board_kebab_name_does_not_convert);
    RUN_TEST(game_board_table_nnn_without_slug_does_not_convert);
    RUN_TEST(game_board_read_leaves_skill_trees);
    RUN_TEST(game_board_track_a_sys_blurb_tasks_inputs);
    RUN_TEST(game_board_track_b_gdd_header_only);
    RUN_TEST(game_board_what_it_does_wins_over_open);
    RUN_TEST(game_board_playtest_recent_last_round_only);
    RUN_TEST(game_board_level_recent_last_8_changelog);
    RUN_TEST(game_board_48_task_cap_omits_49th);
    RUN_TEST(game_board_needs_review_is_not_strip_row);
    RUN_TEST(game_board_empty_blocked_json_array);
    RUN_TEST(game_board_blocked_overlay_owner_match);
    RUN_TEST(game_board_expand_read_does_not_write_skill_trees);
    RUN_TEST(game_board_parse_epics_registry_null_empty);
    RUN_TEST(game_board_parse_epics_registry_nnn_and_last_wins);
    RUN_TEST(game_board_registry_in_progress_omits_leftover);
    RUN_TEST(game_board_registry_not_started_keeps);
    RUN_TEST(game_board_registry_closed_parked_omit_norow_stays);
    RUN_TEST(game_board_registry_present_companion_does_not_hide);
    RUN_TEST(game_board_registry_header_only_roadmap_does_not_hide);
    RUN_TEST(game_board_registry_last_duplicate_closed_omits);
    RUN_TEST(game_board_registry_last_duplicate_not_started_keeps);
    RUN_TEST(game_board_registry_epic_cell_1_hides);
    RUN_TEST(game_board_registry_epic_cell_epic_001_hides);
    RUN_TEST(game_board_registry_plan_path_leftover_does_not_match);
    RUN_TEST(game_board_inbox_65th_omitted_no_has_more_registry_absent);
    RUN_TEST(game_board_registry_evergreen_epic_0_does_not_hide);
    RUN_TEST(game_board_registry_unknown_status_does_not_hide);
    RUN_TEST(game_board_registry_plan_native_never_matches);
    RUN_TEST(game_board_registry_unreadable_present_companion_does_not_hide);
    RUN_TEST(game_board_registry_malformed_epic_skipped_parked_omits);
    RUN_TEST(game_board_parse_backlog_debt_null_empty);
    RUN_TEST(game_board_debt_open_comment);
    RUN_TEST(game_board_debt_list_item_open);
    RUN_TEST(game_board_debt_resolved_by_following_line);
    RUN_TEST(game_board_debt_design_tech_without_prefix_omitted);
    RUN_TEST(game_board_debt_missing_file_empty);
    RUN_TEST(game_board_debt_same_line_resolved_by);
    RUN_TEST(game_board_debt_blank_line_then_resolved_by);
    RUN_TEST(game_board_debt_next_start_ends_previous);
    RUN_TEST(game_board_debt_17th_open_omitted_no_has_more);
    RUN_TEST(game_board_debt_heading_counts);
    RUN_TEST(game_board_debt_bare_colon_skipped);
    RUN_TEST(game_board_debt_backlog_is_not_a_card);
    RUN_TEST(game_board_debt_inbox_registry_hide_unchanged);
    RUN_TEST(game_board_debt_unreadable_oversize_empty);
    RUN_TEST(game_board_debt_directory_at_path_empty);
    RUN_TEST(game_board_debt_line_without_tag_skipped);
    RUN_TEST(game_board_debt_read_leaves_skill_bytes);
}
