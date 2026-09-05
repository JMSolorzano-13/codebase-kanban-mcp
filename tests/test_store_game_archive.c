/**
 * @sdd-task: Task #1 - Store game_archive table + set/load/copy
 * @sdd-spec: specs/spec-012-m2k-game-expand-archive-deps/spec.md
 * @sdd-decision: SDD-ADR-052 - game_archive table; not spec_archive
 * @sdd-why: Game archive flags live in the project .db; query-open skips init_schema so missing table is empty not ERR
 * @human-debug: Missing-table tests DROP game_archive then load/copy — probe must be sqlite_master
 */
#include "test_framework.h"

#include <store/store.h>

#include <sqlite3.h>
#include <string.h>

static int ga_row_count(cbm_store_t *s) {
    sqlite3_stmt *stmt = NULL;
    int n = -1;

    if (sqlite3_prepare_v2(cbm_store_get_db(s), "SELECT COUNT(*) FROM game_archive;", -1, &stmt,
                           NULL) != SQLITE_OK) {
        return -1;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        n = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return n;
}

static int ga_find(const cbm_game_archive_row_t *rows, int n, const char *id) {
    int i;

    for (i = 0; i < n; i++) {
        if (strcmp(rows[i].card_id, id) == 0) {
            return i;
        }
    }
    return -1;
}

TEST(game_archive_schema_exists) {
    cbm_store_t *s = cbm_store_open_memory();
    sqlite3_stmt *stmt = NULL;
    const unsigned char *sql = NULL;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(sqlite3_prepare_v2(cbm_store_get_db(s),
                                 "SELECT sql FROM sqlite_master WHERE type='table' "
                                 "AND name='game_archive' LIMIT 1;",
                                 -1, &stmt, NULL),
              SQLITE_OK);
    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    sql = sqlite3_column_text(stmt, 0);
    ASSERT_NOT_NULL(sql);
    ASSERT_NOT_NULL(strstr((const char *)sql, "card_id TEXT PRIMARY KEY"));
    ASSERT_NOT_NULL(strstr((const char *)sql, "archived INTEGER NOT NULL"));
    ASSERT_NOT_NULL(strstr((const char *)sql, "CHECK (archived IN (0, 1))"));
    ASSERT_NOT_NULL(strstr((const char *)sql, "updated_at TEXT NOT NULL"));
    sqlite3_finalize(stmt);
    cbm_store_close(s);
    PASS();
}

TEST(game_archive_set_load_roundtrip) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_game_archive_row_t rows[CBM_GAME_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(cbm_store_game_archive_set(s, ".gamedev/phases/02-production/SYS-001-movement", 1),
              CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_load(s, rows, CBM_GAME_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    i = ga_find(rows, n, ".gamedev/phases/02-production/SYS-001-movement");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    cbm_store_close(s);
    PASS();
}

TEST(game_archive_set_empty_id_err) {
    cbm_store_t *s = cbm_store_open_memory();

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(cbm_store_game_archive_set(s, "", 1), CBM_STORE_ERR);
    ASSERT_EQ(cbm_store_game_archive_set(s, NULL, 1), CBM_STORE_ERR);
    ASSERT_EQ(ga_row_count(s), 0);
    cbm_store_close(s);
    PASS();
}

TEST(game_archive_set_card_id_too_long) {
    cbm_store_t *s = cbm_store_open_memory();
    char ok[256];
    char too_long[257];
    cbm_game_archive_row_t rows[CBM_GAME_ARCHIVE_CAP];
    int n = -1;

    ASSERT_NOT_NULL(s);
    memset(ok, 'a', 255);
    ok[255] = '\0';
    memset(too_long, 'b', 256);
    too_long[256] = '\0';
    ASSERT_EQ(cbm_store_game_archive_set(s, ok, 1), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_set(s, too_long, 1), CBM_STORE_ERR);
    ASSERT_EQ(ga_row_count(s), 1);
    ASSERT_EQ(cbm_store_game_archive_load(s, rows, CBM_GAME_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(ga_find(rows, n, ok), 0);
    cbm_store_close(s);
    PASS();
}

TEST(game_archive_unarchive_keeps_row) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_game_archive_row_t rows[CBM_GAME_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(cbm_store_game_archive_set(s, ".gamedev/phases/02-production/SYS-001-movement", 1),
              CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_set(s, ".gamedev/phases/02-production/SYS-001-movement", 0),
              CBM_STORE_OK);
    ASSERT_EQ(ga_row_count(s), 1);
    ASSERT_EQ(cbm_store_game_archive_load(s, rows, CBM_GAME_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    i = ga_find(rows, n, ".gamedev/phases/02-production/SYS-001-movement");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 0);
    cbm_store_close(s);
    PASS();
}

TEST(game_archive_repeat_archive_idempotent) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_game_archive_row_t rows[CBM_GAME_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(cbm_store_game_archive_set(s, ".gamedev/phases/02-production/SYS-001-movement", 1),
              CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_set(s, ".gamedev/phases/02-production/SYS-001-movement", 1),
              CBM_STORE_OK);
    ASSERT_EQ(ga_row_count(s), 1);
    ASSERT_EQ(cbm_store_game_archive_load(s, rows, CBM_GAME_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    i = ga_find(rows, n, ".gamedev/phases/02-production/SYS-001-movement");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    cbm_store_close(s);
    PASS();
}

TEST(game_archive_orphan_row_survives_load) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_game_archive_row_t rows[CBM_GAME_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(s);
    /* Store has no board: leftover id must still load (HTTP invent-card is later). */
    ASSERT_EQ(cbm_store_game_archive_set(
                  s, ".gamedev/phases/01-preproduction/missing-doc.md", 1),
              CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_load(s, rows, CBM_GAME_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    i = ga_find(rows, n, ".gamedev/phases/01-preproduction/missing-doc.md");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    cbm_store_close(s);
    PASS();
}

TEST(game_archive_missing_table_load_ok) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_game_archive_row_t rows[CBM_GAME_ARCHIVE_CAP];
    int n = -1;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(sqlite3_exec(cbm_store_get_db(s), "DROP TABLE game_archive;", NULL, NULL, NULL),
              SQLITE_OK);
    ASSERT_EQ(cbm_store_game_archive_load(s, rows, CBM_GAME_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 0);
    cbm_store_close(s);
    PASS();
}

TEST(game_archive_copy_rows) {
    cbm_store_t *src = cbm_store_open_memory();
    cbm_store_t *dst = cbm_store_open_memory();
    cbm_game_archive_row_t rows[CBM_GAME_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(src);
    ASSERT_NOT_NULL(dst);
    ASSERT_EQ(cbm_store_game_archive_set(src, ".gamedev/phases/02-production/SYS-001-movement", 1),
              CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_set(
                  src, ".gamedev/phases/01-preproduction/missing-doc.md", 1),
              CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_copy(src, dst), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_load(dst, rows, CBM_GAME_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 2);
    i = ga_find(rows, n, ".gamedev/phases/02-production/SYS-001-movement");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    i = ga_find(rows, n, ".gamedev/phases/01-preproduction/missing-doc.md");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    cbm_store_close(src);
    cbm_store_close(dst);
    PASS();
}

TEST(game_archive_copy_missing_src_noop) {
    cbm_store_t *src = cbm_store_open_memory();
    cbm_store_t *dst = cbm_store_open_memory();
    cbm_game_archive_row_t rows[CBM_GAME_ARCHIVE_CAP];
    int n = -1;

    ASSERT_NOT_NULL(src);
    ASSERT_NOT_NULL(dst);
    ASSERT_EQ(cbm_store_game_archive_set(dst, ".gamedev/design/gdd.md", 0), CBM_STORE_OK);
    ASSERT_EQ(sqlite3_exec(cbm_store_get_db(src), "DROP TABLE game_archive;", NULL, NULL, NULL),
              SQLITE_OK);
    ASSERT_EQ(cbm_store_game_archive_copy(src, dst), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_load(dst, rows, CBM_GAME_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(ga_find(rows, n, ".gamedev/design/gdd.md"), 0);
    ASSERT_EQ(rows[0].archived, 0);
    cbm_store_close(src);
    cbm_store_close(dst);
    PASS();
}

SUITE(store_game_archive) {
    RUN_TEST(game_archive_schema_exists);
    RUN_TEST(game_archive_set_load_roundtrip);
    RUN_TEST(game_archive_set_empty_id_err);
    RUN_TEST(game_archive_set_card_id_too_long);
    RUN_TEST(game_archive_unarchive_keeps_row);
    RUN_TEST(game_archive_repeat_archive_idempotent);
    RUN_TEST(game_archive_orphan_row_survives_load);
    RUN_TEST(game_archive_missing_table_load_ok);
    RUN_TEST(game_archive_copy_rows);
    RUN_TEST(game_archive_copy_missing_src_noop);
}
