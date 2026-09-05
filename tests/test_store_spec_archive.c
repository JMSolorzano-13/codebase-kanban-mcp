/**
 * @sdd-task: Task #1 - Store spec_archive table + set/load/copy
 * @sdd-spec: specs/spec-006-k3n-spec-archive/spec.md
 * @sdd-decision: SDD-ADR-029 - spec_archive table in the project .db
 * @sdd-why: Persist archive flags in the project .db; orphan rows stay; missing table is empty
 * @human-debug: Missing-table tests DROP spec_archive then load/copy — probe must be sqlite_master
 */
#include "test_framework.h"

#include <store/store.h>

#include <sqlite3.h>
#include <string.h>

static int sa_row_count(cbm_store_t *s) {
    sqlite3_stmt *stmt = NULL;
    int n = -1;

    if (sqlite3_prepare_v2(cbm_store_get_db(s), "SELECT COUNT(*) FROM spec_archive;", -1, &stmt,
                           NULL) != SQLITE_OK) {
        return -1;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        n = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return n;
}

static int sa_find(const cbm_spec_archive_row_t *rows, int n, const char *id) {
    int i;

    for (i = 0; i < n; i++) {
        if (strcmp(rows[i].spec_id, id) == 0) {
            return i;
        }
    }
    return -1;
}

TEST(spec_archive_schema_exists) {
    cbm_store_t *s = cbm_store_open_memory();
    sqlite3_stmt *stmt = NULL;
    const unsigned char *sql = NULL;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(sqlite3_prepare_v2(cbm_store_get_db(s),
                                 "SELECT sql FROM sqlite_master WHERE type='table' "
                                 "AND name='spec_archive' LIMIT 1;",
                                 -1, &stmt, NULL),
              SQLITE_OK);
    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    sql = sqlite3_column_text(stmt, 0);
    ASSERT_NOT_NULL(sql);
    ASSERT_NOT_NULL(strstr((const char *)sql, "spec_id TEXT PRIMARY KEY"));
    ASSERT_NOT_NULL(strstr((const char *)sql, "archived INTEGER NOT NULL"));
    ASSERT_NOT_NULL(strstr((const char *)sql, "CHECK (archived IN (0, 1))"));
    ASSERT_NOT_NULL(strstr((const char *)sql, "updated_at TEXT NOT NULL"));
    sqlite3_finalize(stmt);
    cbm_store_close(s);
    PASS();
}

TEST(spec_archive_set_load_roundtrip) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_spec_archive_row_t rows[CBM_SPEC_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(cbm_store_spec_archive_set(s, "spec-012-ccc-closed", 1), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_spec_archive_load(s, rows, CBM_SPEC_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    i = sa_find(rows, n, "spec-012-ccc-closed");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    cbm_store_close(s);
    PASS();
}

TEST(spec_archive_set_empty_id_err) {
    cbm_store_t *s = cbm_store_open_memory();

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(cbm_store_spec_archive_set(s, "", 1), CBM_STORE_ERR);
    ASSERT_EQ(cbm_store_spec_archive_set(s, NULL, 1), CBM_STORE_ERR);
    ASSERT_EQ(sa_row_count(s), 0);
    cbm_store_close(s);
    PASS();
}

TEST(spec_archive_unarchive_keeps_row) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_spec_archive_row_t rows[CBM_SPEC_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(cbm_store_spec_archive_set(s, "spec-012-ccc-closed", 1), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_spec_archive_set(s, "spec-012-ccc-closed", 0), CBM_STORE_OK);
    ASSERT_EQ(sa_row_count(s), 1);
    ASSERT_EQ(cbm_store_spec_archive_load(s, rows, CBM_SPEC_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    i = sa_find(rows, n, "spec-012-ccc-closed");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 0);
    cbm_store_close(s);
    PASS();
}

TEST(spec_archive_repeat_archive_idempotent) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_spec_archive_row_t rows[CBM_SPEC_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(cbm_store_spec_archive_set(s, "spec-012-ccc-closed", 1), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_spec_archive_set(s, "spec-012-ccc-closed", 1), CBM_STORE_OK);
    ASSERT_EQ(sa_row_count(s), 1);
    ASSERT_EQ(cbm_store_spec_archive_load(s, rows, CBM_SPEC_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    i = sa_find(rows, n, "spec-012-ccc-closed");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    cbm_store_close(s);
    PASS();
}

TEST(spec_archive_orphan_row_survives_load) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_spec_archive_row_t rows[CBM_SPEC_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(s);
    /* Store has no board: leftover id must still load (HTTP invent-card is later). */
    ASSERT_EQ(cbm_store_spec_archive_set(s, "spec-099-zzz-gone", 1), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_spec_archive_load(s, rows, CBM_SPEC_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    i = sa_find(rows, n, "spec-099-zzz-gone");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    cbm_store_close(s);
    PASS();
}

TEST(spec_archive_missing_table_load_ok) {
    cbm_store_t *s = cbm_store_open_memory();
    cbm_spec_archive_row_t rows[CBM_SPEC_ARCHIVE_CAP];
    int n = -1;

    ASSERT_NOT_NULL(s);
    ASSERT_EQ(sqlite3_exec(cbm_store_get_db(s), "DROP TABLE spec_archive;", NULL, NULL, NULL),
              SQLITE_OK);
    ASSERT_EQ(cbm_store_spec_archive_load(s, rows, CBM_SPEC_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 0);
    cbm_store_close(s);
    PASS();
}

TEST(spec_archive_copy_rows) {
    cbm_store_t *src = cbm_store_open_memory();
    cbm_store_t *dst = cbm_store_open_memory();
    cbm_spec_archive_row_t rows[CBM_SPEC_ARCHIVE_CAP];
    int n = -1;
    int i;

    ASSERT_NOT_NULL(src);
    ASSERT_NOT_NULL(dst);
    ASSERT_EQ(cbm_store_spec_archive_set(src, "spec-012-ccc-closed", 1), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_spec_archive_set(src, "spec-099-zzz-gone", 1), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_spec_archive_copy(src, dst), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_spec_archive_load(dst, rows, CBM_SPEC_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 2);
    i = sa_find(rows, n, "spec-012-ccc-closed");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    i = sa_find(rows, n, "spec-099-zzz-gone");
    ASSERT_GTE(i, 0);
    ASSERT_EQ(rows[i].archived, 1);
    cbm_store_close(src);
    cbm_store_close(dst);
    PASS();
}

TEST(spec_archive_copy_missing_src_noop) {
    cbm_store_t *src = cbm_store_open_memory();
    cbm_store_t *dst = cbm_store_open_memory();
    cbm_spec_archive_row_t rows[CBM_SPEC_ARCHIVE_CAP];
    int n = -1;

    ASSERT_NOT_NULL(src);
    ASSERT_NOT_NULL(dst);
    ASSERT_EQ(cbm_store_spec_archive_set(dst, "spec-017-hhh-visible", 0), CBM_STORE_OK);
    ASSERT_EQ(sqlite3_exec(cbm_store_get_db(src), "DROP TABLE spec_archive;", NULL, NULL, NULL),
              SQLITE_OK);
    ASSERT_EQ(cbm_store_spec_archive_copy(src, dst), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_spec_archive_load(dst, rows, CBM_SPEC_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(sa_find(rows, n, "spec-017-hhh-visible"), 0);
    ASSERT_EQ(rows[0].archived, 0);
    cbm_store_close(src);
    cbm_store_close(dst);
    PASS();
}

SUITE(store_spec_archive) {
    RUN_TEST(spec_archive_schema_exists);
    RUN_TEST(spec_archive_set_load_roundtrip);
    RUN_TEST(spec_archive_set_empty_id_err);
    RUN_TEST(spec_archive_unarchive_keeps_row);
    RUN_TEST(spec_archive_repeat_archive_idempotent);
    RUN_TEST(spec_archive_orphan_row_survives_load);
    RUN_TEST(spec_archive_missing_table_load_ok);
    RUN_TEST(spec_archive_copy_rows);
    RUN_TEST(spec_archive_copy_missing_src_noop);
}
