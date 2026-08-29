/*
 * spec_board.c — pure parsing for the sdd-skill spec board (see spec_board.h).
 *
 * No writes anywhere in this file. Every read degrades gracefully: a missing
 * file/dir just leaves the corresponding field at its zero value instead of
 * failing the whole read, matching the rest of the UI support endpoints.
 *
 * Known limitation (documented on purpose, not hidden): history/test_results.log
 * is a single append-only log shared across every spec a project has ever had.
 * Task "done" status is derived by matching "Task #N" entries against the
 * ACTIVE spec's own tasks.md task numbers, so a prior spec's Task #N could in
 * principle leak a stale PASS into a same-numbered task of the current spec.
 * Acceptable for a quick-glance board; revisit if it proves confusing in
 * practice (see docs/kanban-adapter-contract.md).
 */
#include "ui/spec_board.h"
#include "foundation/compat_fs.h"
#include "foundation/platform.h"
#include "foundation/str_util.h"

#include <yyjson/yyjson.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── File I/O ─────────────────────────────────────────────────── */

enum { SPEC_BOARD_MAX_FILE = 4 * 1024 * 1024 };

static char *read_whole_file(const char *path) {
    FILE *f = cbm_fopen(path, "rb");
    if (!f) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long sz = ftell(f);
    if (sz < 0 || sz > SPEC_BOARD_MAX_FILE) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    char *buf = malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[n] = '\0';
    return buf;
}

/* ── state.md (two known layouts) ────────────────────────────────
 * New (post token-reduction, skill_version >= 1.4.0): 2-line compact
 *   "role=@x task=\"...\" spec=... ..." / "notes=..."
 * Old (pre-1.4.0, still present on projects nobody has re-touched since —
 * per the skill's own documented self-migrating format-compat note, an
 * old-format state.md is left as-is until the next agent turn rewrites it):
 *   "current_role: @x" / "current_task: ..." as separate markdown lines,
 *   with free-text blocker notes usually under a trailing "## Notes"
 * section. Both are tried; whichever finds a value wins per field. */

static void kv_extract(const char *line, const char *key, char *out, size_t outsz) {
    out[0] = '\0';
    if (!line) {
        return;
    }
    size_t klen = strlen(key);
    const char *p = line;
    while ((p = strstr(p, key)) != NULL) {
        bool at_start = (p == line) || isspace((unsigned char)p[-1]);
        if (at_start && p[klen] == '=') {
            const char *v = p + klen + 1;
            bool quoted = (*v == '"');
            if (quoted) {
                v++;
            }
            size_t bi = 0;
            char tmp[512];
            while (*v && bi < sizeof(tmp) - 1) {
                if (quoted) {
                    if (*v == '"') {
                        break;
                    }
                } else if (isspace((unsigned char)*v)) {
                    break;
                }
                tmp[bi++] = *v++;
            }
            tmp[bi] = '\0';
            snprintf(out, outsz, "%s", tmp);
            return;
        }
        p += klen;
    }
}

/* Old-format "key: value" line, key must start the (trimmed) line. */
static void kv_extract_colon_line(const char *line, const char *key, char *out, size_t outsz) {
    const char *l = line;
    while (*l == ' ') {
        l++;
    }
    size_t klen = strlen(key);
    if (strncmp(l, key, klen) != 0) {
        return;
    }
    const char *v = l + klen;
    if (*v != ':') {
        return;
    }
    v++;
    while (*v == ' ') {
        v++;
    }
    size_t vl = strlen(v);
    while (vl > 0 && (v[vl - 1] == '\r' || v[vl - 1] == '\n')) {
        vl--;
    }
    if (vl >= outsz) {
        vl = outsz - 1;
    }
    memcpy(out, v, vl);
    out[vl] = '\0';
}

static void read_state_md(const char *sdd_dir, char *role_out, size_t role_sz, char *task_out,
                          size_t task_sz, char *notes_out, size_t notes_sz) {
    char path[1152];
    snprintf(path, sizeof(path), "%s/state.md", sdd_dir);
    char *buf = read_whole_file(path);
    if (!buf) {
        return;
    }

    /* Try the new compact 2-line format first (cheap: just the first two lines). */
    char *nl = strchr(buf, '\n');
    char line1[1024] = {0};
    char line2[1024] = {0};
    if (nl) {
        size_t l1 = (size_t)(nl - buf);
        if (l1 >= sizeof(line1)) {
            l1 = sizeof(line1) - 1;
        }
        memcpy(line1, buf, l1);
        line1[l1] = '\0';
        char *line2_start = nl + 1;
        char *nl2 = strchr(line2_start, '\n');
        size_t l2 = nl2 ? (size_t)(nl2 - line2_start) : strlen(line2_start);
        if (l2 >= sizeof(line2)) {
            l2 = sizeof(line2) - 1;
        }
        memcpy(line2, line2_start, l2);
        line2[l2] = '\0';
    } else {
        snprintf(line1, sizeof(line1), "%s", buf);
    }

    kv_extract(line1, "role", role_out, role_sz);
    kv_extract(line1, "task", task_out, task_sz);
    const char *p = strstr(line2, "notes=");
    if (!p) {
        p = strstr(line2, "notes:");
    }
    if (p) {
        const char *v = p + 6;
        while (*v == ' ' || *v == '"') {
            v++;
        }
        snprintf(notes_out, notes_sz, "%s", v);
        size_t l = strlen(notes_out);
        while (l > 0 && (notes_out[l - 1] == '"' || notes_out[l - 1] == '\r')) {
            notes_out[--l] = '\0';
        }
    }

    /* Fall back to the old multi-line "current_role:"/"current_task:" layout
     * whenever the compact parse above found nothing — this is the format
     * still on disk for any project not re-touched since the skill's 1.4.0
     * state.md migration (bevy-tetris is one such project). */
    if (!role_out[0] || !task_out[0] || !notes_out[0]) {
        bool in_notes = false;
        char *saveptr = NULL;
        char *line = strtok_r(buf, "\n", &saveptr);
        while (line) {
            if (!role_out[0]) {
                kv_extract_colon_line(line, "current_role", role_out, role_sz);
            }
            if (!task_out[0]) {
                kv_extract_colon_line(line, "current_task", task_out, task_sz);
            }
            const char *trimmed = line;
            while (*trimmed == ' ') {
                trimmed++;
            }
            if (strncmp(trimmed, "## Notes", 8) == 0) {
                in_notes = true;
            } else if (in_notes && !notes_out[0] &&
                      (strstr(trimmed, "block") || strstr(trimmed, "Block"))) {
                while (*trimmed == '-' || *trimmed == ' ') {
                    trimmed++;
                }
                snprintf(notes_out, notes_sz, "%s", trimmed);
            }
            line = strtok_r(NULL, "\n", &saveptr);
        }
    }

    free(buf);
}

/* "Task #7 — ..." → 7. Returns -1 when no "#<digits>" is found. */
static int extract_task_number(const char *task_field) {
    const char *p = strchr(task_field, '#');
    if (!p) {
        return -1;
    }
    return atoi(p + 1);
}

/* ── active.json ──────────────────────────────────────────────── */

static void append_specs_from_array(yyjson_val *arr, const char *column,
                                    cbm_spec_board_t *out) {
    if (!arr || !yyjson_is_arr(arr)) {
        return;
    }
    size_t idx, max;
    yyjson_val *item;
    yyjson_arr_foreach(arr, idx, max, item) {
        if (out->spec_count >= CBM_SPEC_BOARD_MAX_SPECS) {
            break;
        }
        if (!yyjson_is_str(item)) {
            continue;
        }
        cbm_spec_board_entry_t *e = &out->specs[out->spec_count++];
        memset(e, 0, sizeof(*e));
        snprintf(e->id, sizeof(e->id), "%s", yyjson_get_str(item));
        snprintf(e->column, sizeof(e->column), "%s", column);
        e->checklist_percent = -1;
    }
}

static void read_active_json(const char *sdd_dir, cbm_spec_board_t *out, char *active_id,
                             size_t active_id_sz) {
    char path[1152];
    snprintf(path, sizeof(path), "%s/specs/active.json", sdd_dir);
    char *buf = read_whole_file(path);
    if (!buf) {
        return;
    }
    yyjson_doc *doc = yyjson_read(buf, strlen(buf), 0);
    free(buf);
    if (!doc) {
        return;
    }
    yyjson_val *root = yyjson_doc_get_root(doc);
    yyjson_val *v_active = yyjson_obj_get(root, "active_spec");
    if (v_active && yyjson_is_str(v_active)) {
        snprintf(active_id, active_id_sz, "%s", yyjson_get_str(v_active));
    }
    append_specs_from_array(yyjson_obj_get(root, "planned_specs"), "todo", out);
    /* draft_specs: same backlog column as planned — sdd-skill keeps both as
     * non-active work items; no distinct column exists in the skill contract. */
    append_specs_from_array(yyjson_obj_get(root, "draft_specs"), "todo", out);
    append_specs_from_array(yyjson_obj_get(root, "completed_specs"), "done", out);
    yyjson_doc_free(doc);
}

/* ── spec.md title (active spec only — keeps this a single extra read) ──── */

static void read_spec_title(const char *spec_dir, char *out, size_t outsz) {
    char path[1280];
    snprintf(path, sizeof(path), "%s/spec.md", spec_dir);
    char *buf = read_whole_file(path);
    if (!buf) {
        return;
    }
    char *nl = strchr(buf, '\n');
    if (nl) {
        *nl = '\0';
    }
    const char *l = buf;
    while (*l == '#' || *l == ' ') {
        l++;
    }
    snprintf(out, outsz, "%s", l);
    free(buf);
}

/* ── tasks.md ("### Task #N — Name" headings) ────────────────────── */

static void read_tasks_md(const char *spec_dir, cbm_spec_board_entry_t *e) {
    char path[1280];
    snprintf(path, sizeof(path), "%s/tasks.md", spec_dir);
    char *buf = read_whole_file(path);
    if (!buf) {
        return;
    }
    char *saveptr = NULL;
    char *line = strtok_r(buf, "\n", &saveptr);
    while (line && e->task_count < CBM_SPEC_BOARD_MAX_TASKS) {
        const char *l = line;
        while (*l == ' ') {
            l++;
        }
        if (strncmp(l, "### Task #", 10) == 0) {
            cbm_spec_task_t *t = &e->tasks[e->task_count++];
            memset(t, 0, sizeof(*t));
            t->number = atoi(l + 10);
            /* Name follows the first '-'-family separator (ASCII '-' or the
             * UTF-8 em dash "—" used by the template); tolerate either. */
            const char *sep = strstr(l, "—");
            if (!sep) {
                sep = strchr(l, '-');
            }
            if (sep) {
                const char *namep = sep;
                while (*namep == '-' || (unsigned char)*namep >= 0x80) {
                    namep++;
                }
                while (*namep == ' ') {
                    namep++;
                }
                snprintf(t->name, sizeof(t->name), "%s", namep);
            }
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }
    free(buf);
}

/* ── history/test_results.log ("Task #N ... PASS|FAIL") ─────────────── */

static void read_test_results(const char *sdd_dir, cbm_spec_board_entry_t *e) {
    char path[1152];
    snprintf(path, sizeof(path), "%s/history/test_results.log", sdd_dir);
    char *buf = read_whole_file(path);
    if (!buf) {
        return;
    }
    char *saveptr = NULL;
    char *line = strtok_r(buf, "\n", &saveptr);
    while (line) {
        const char *marker = strstr(line, "Task #");
        if (marker) {
            int n = atoi(marker + 6);
            bool pass = strstr(line, "PASS") != NULL;
            for (int i = 0; i < e->task_count; i++) {
                if (e->tasks[i].number == n) {
                    e->tasks[i].done = pass; /* chronological — last entry wins */
                    break;
                }
            }
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }
    free(buf);
    e->tasks_done = 0;
    for (int i = 0; i < e->task_count; i++) {
        if (e->tasks[i].done) {
            e->tasks_done++;
        }
    }
}

/* ── checklist.md ("## Feature Status" ... "TOTAL ... NN%") ─────────── */

static double read_checklist_percent(const char *spec_dir) {
    char path[1280];
    snprintf(path, sizeof(path), "%s/checklist.md", spec_dir);
    char *buf = read_whole_file(path);
    if (!buf) {
        return -1;
    }
    double result = -1;
    char *saveptr = NULL;
    char *line = strtok_r(buf, "\n", &saveptr);
    while (line) {
        if (strstr(line, "TOTAL")) {
            const char *p = line;
            while ((p = strchr(p, '%')) != NULL) {
                const char *start = p;
                while (start > line && (isdigit((unsigned char)start[-1]) || start[-1] == '.')) {
                    start--;
                }
                if (start < p) {
                    result = atof(start);
                    break;
                }
                p++;
            }
            if (result >= 0) {
                break;
            }
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }
    free(buf);
    return result;
}

/* ── Public API ───────────────────────────────────────────────── */

bool cbm_spec_board_sdd_skill_present(const char *root_path) {
    if (!root_path || !root_path[0]) {
        return false;
    }
    char path[1152];
    snprintf(path, sizeof(path), "%s/.sdd-skill", root_path);
    return cbm_is_dir(path);
}

bool cbm_spec_board_gamedev_skill_present(const char *root_path) {
    if (!root_path || !root_path[0]) {
        return false;
    }
    char path[1152];
    snprintf(path, sizeof(path), "%s/.gamedev", root_path);
    return cbm_is_dir(path);
}

void cbm_spec_board_read(const char *root_path, cbm_spec_board_t *out) {
    memset(out, 0, sizeof(*out));
    if (!cbm_spec_board_sdd_skill_present(root_path)) {
        return;
    }
    out->sdd_skill_present = true;

    char sdd_dir[1024];
    snprintf(sdd_dir, sizeof(sdd_dir), "%s/.sdd-skill", root_path);

    char active_id[192] = {0};
    read_active_json(sdd_dir, out, active_id, sizeof(active_id));

    if (active_id[0] && out->spec_count < CBM_SPEC_BOARD_MAX_SPECS) {
        cbm_spec_board_entry_t *e = &out->specs[out->spec_count++];
        memset(e, 0, sizeof(*e));
        snprintf(e->id, sizeof(e->id), "%s", active_id);
        snprintf(e->column, sizeof(e->column), "in_progress");
        e->active = true;
        e->checklist_percent = -1;

        char spec_dir[1280];
        snprintf(spec_dir, sizeof(spec_dir), "%s/specs/%s", sdd_dir, active_id);
        read_spec_title(spec_dir, e->title, sizeof(e->title));
        read_tasks_md(spec_dir, e);
        read_test_results(sdd_dir, e);
        e->checklist_percent = read_checklist_percent(spec_dir);

        char role[32] = {0}, task_field[256] = {0}, notes[256] = {0};
        read_state_md(sdd_dir, role, sizeof(role), task_field, sizeof(task_field), notes,
                      sizeof(notes));
        snprintf(e->current_agent, sizeof(e->current_agent), "%s", role);
        if (notes[0] && (strstr(notes, "block") || strstr(notes, "Block"))) {
            snprintf(e->blocked_note, sizeof(e->blocked_note), "%s", notes);
        }
        int cur_task = extract_task_number(task_field);
        if (cur_task > 0) {
            for (int i = 0; i < e->task_count; i++) {
                if (e->tasks[i].number == cur_task) {
                    e->tasks[i].current = true;
                    break;
                }
            }
        }
    }
}

/* Append-with-growth helper for the JSON writer below. */
static bool board_json_append(char **buf, size_t *cap, int *pos, const char *fmt, ...) {
    va_list ap;
    for (;;) {
        va_start(ap, fmt);
        int need = vsnprintf(*buf + *pos, *cap - (size_t)*pos, fmt, ap);
        va_end(ap);
        if (need < 0) {
            return false;
        }
        if ((size_t)(*pos + need) < *cap) {
            *pos += need;
            return true;
        }
        size_t new_cap = *cap * 2;
        char *nb = realloc(*buf, new_cap);
        if (!nb) {
            return false;
        }
        *buf = nb;
        *cap = new_cap;
    }
}

char *cbm_spec_board_to_json(const cbm_spec_board_t *b) {
    size_t cap = 65536;
    char *buf = malloc(cap);
    if (!buf) {
        return NULL;
    }
    int pos = 0;

#define APP(...)                                          \
    if (!board_json_append(&buf, &cap, &pos, __VA_ARGS__)) { \
        free(buf);                                        \
        return NULL;                                       \
    }

    APP("{\"sdd_skill_present\":%s,\"specs\":[", b->sdd_skill_present ? "true" : "false");
    for (int i = 0; i < b->spec_count; i++) {
        const cbm_spec_board_entry_t *e = &b->specs[i];
        char esc_id[384], esc_title[512], esc_agent[64], esc_blocked[512];
        cbm_json_escape(esc_id, (int)sizeof(esc_id), e->id);
        cbm_json_escape(esc_title, (int)sizeof(esc_title), e->title);
        cbm_json_escape(esc_agent, (int)sizeof(esc_agent), e->current_agent);
        cbm_json_escape(esc_blocked, (int)sizeof(esc_blocked), e->blocked_note);
        if (i > 0) {
            APP(",");
        }
        APP("{\"id\":\"%s\",\"title\":\"%s\",\"column\":\"%s\",\"active\":%s,"
            "\"current_agent\":\"%s\",\"blocked_note\":\"%s\","
            "\"task_count\":%d,\"tasks_done\":%d,\"checklist_percent\":%.1f,\"tasks\":[",
            esc_id, esc_title, e->column, e->active ? "true" : "false", esc_agent, esc_blocked,
            e->task_count, e->tasks_done, e->checklist_percent);
        for (int j = 0; j < e->task_count; j++) {
            const cbm_spec_task_t *t = &e->tasks[j];
            char esc_tname[512];
            cbm_json_escape(esc_tname, (int)sizeof(esc_tname), t->name);
            if (j > 0) {
                APP(",");
            }
            APP("{\"number\":%d,\"name\":\"%s\",\"done\":%s,\"current\":%s}", t->number,
                esc_tname, t->done ? "true" : "false", t->current ? "true" : "false");
        }
        APP("]}");
    }
    APP("]}");
#undef APP
    return buf;
}
