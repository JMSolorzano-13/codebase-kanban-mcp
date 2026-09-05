/**
 * @sdd-task: Task #1 - spec_board debt parse + additive JSON
 * @sdd-spec: specs/spec-015-s5k-specs-debt-and-path/spec.md
 * @sdd-decision: SDD-ADR-066 TECH_DEBT.md parse here; SDD-ADR-065 always-emit debt[] cap 16
 * @sdd-why: HTTP must not grow skill-tree IO; heading Status wins; conversion untouched
 * @human-debug: If debt stays [] with an open heading → Status: not line-start/pipe-cell or fopen rb failed
 *
 * spec_board.c — pure parsing for the spec board (see spec_board.h).
 *
 * No writes anywhere in this file. fopen "rb" only. Grill and debt fills are
 * independent of sdd presence. Do not read .gamedev/ from read/to_json.
 *
 * Dual done matcher (SDD-ADR-026): one shared history/test_results.log.
 * Active keeps the bare Task #N last-line-wins rule (a prior spec's Task #N
 * can still leak into the active card — documented, not hidden). Non-active
 * requires that spec's id on the same line as Task #N; bare Task #N does not
 * mark another spec's task.
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
                             size_t active_id_sz, char *source_grill_epic, size_t source_sz) {
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
    if (source_grill_epic && source_sz > 0) {
        source_grill_epic[0] = '\0';
        yyjson_val *v_source = yyjson_obj_get(root, "source");
        if (v_source && yyjson_is_obj(v_source)) {
            yyjson_val *v_ge = yyjson_obj_get(v_source, "grill_epic");
            if (v_ge && yyjson_is_str(v_ge)) {
                snprintf(source_grill_epic, source_sz, "%s", yyjson_get_str(v_ge));
            }
        }
    }
    append_specs_from_array(yyjson_obj_get(root, "planned_specs"), "todo", out);
    /* draft_specs: same backlog column as planned — sdd-skill keeps both as
     * non-active work items; no distinct column exists in the skill contract. */
    append_specs_from_array(yyjson_obj_get(root, "draft_specs"), "todo", out);
    append_specs_from_array(yyjson_obj_get(root, "completed_specs"), "done", out);
    yyjson_doc_free(doc);
}

/* ── spec.md title + blurb (one open; SDD-ADR-024 / SDD-ADR-028) ─
 * Title is the first-line H1 (same strip as the old active-only read).
 * Blurb is extract_blurb on the same buffer so spec.md is not opened twice.
 * Missing or unreadable (including a directory at that path) → both "". */

static void read_spec_md(const char *spec_dir, cbm_spec_board_entry_t *e) {
    char path[1280];
    snprintf(path, sizeof(path), "%s/spec.md", spec_dir);
    char *buf = read_whole_file(path);
    if (!buf) {
        e->title[0] = '\0';
        e->blurb[0] = '\0';
        return;
    }
    const char *line_end = strchr(buf, '\n');
    size_t first_len = line_end ? (size_t)(line_end - buf) : strlen(buf);
    char first[256];
    if (first_len >= sizeof(first)) {
        first_len = sizeof(first) - 1;
    }
    memcpy(first, buf, first_len);
    first[first_len] = '\0';
    const char *l = first;
    while (*l == '#' || *l == ' ') {
        l++;
    }
    snprintf(e->title, sizeof(e->title), "%s", l);
    cbm_spec_board_extract_blurb(buf, e->blurb, sizeof(e->blurb));

    /* First Companion-to .grill/plans/…md token (trailing notes after .md ignored).
     * The token is never fopen'd — conversion is strcmp against constructed ids. */
    e->companion_grill[0] = '\0';
    {
        const char *cp = buf;
        while (*cp && !e->companion_grill[0]) {
            const char *eol = strchr(cp, '\n');
            size_t linelen = eol ? (size_t)(eol - cp) : strlen(cp);
            char line[2048];
            size_t copy = linelen < sizeof(line) - 1 ? linelen : sizeof(line) - 1;
            memcpy(line, cp, copy);
            line[copy] = '\0';
            if (strstr(line, "Companion to:")) {
                const char *tok = strstr(line, ".grill/plans/");
                if (tok) {
                    const char *md_end = strstr(tok, ".md");
                    if (md_end) {
                        size_t n = (size_t)(md_end - tok) + 3;
                        if (n >= sizeof(e->companion_grill)) {
                            n = sizeof(e->companion_grill) - 1;
                        }
                        memcpy(e->companion_grill, tok, n);
                        e->companion_grill[n] = '\0';
                    }
                }
            }
            if (!eol) {
                break;
            }
            cp = eol + 1;
        }
    }
    free(buf);
}

/* ── spec.md Executive Summary blurb (SDD-ADR-025) ─────────────────
 * Line-start ## Executive Summary only: a mid-paragraph mention or ###/H1
 * must not become the source. Stop at the next H2 so ## KPI is never copied.
 * Links are stripped before sentence cuts so URL '.' cannot end a sentence.
 * Third sentence is dropped before the 512-byte cap so leftover bytes cannot
 * pull it (or KPI) back in. */

static const char *find_executive_summary_line(const char *md) {
    static const char k_heading[] = "## Executive Summary";
    const size_t klen = sizeof(k_heading) - 1;
    const char *p = md;
    while (p) {
        const char *t = p;
        while (*t == ' ' || *t == '\t') {
            t++;
        }
        if (strncmp(t, k_heading, klen) == 0) {
            unsigned char next = (unsigned char)t[klen];
            if (next == '\0' || isspace(next)) {
                return p;
            }
        }
        const char *nl = strchr(p, '\n');
        p = nl ? nl + 1 : NULL;
    }
    return NULL;
}

/* [text](url) → text. Unclosed brackets stay literal — no other fallbacks. */
static void strip_md_links(const char *in, size_t inlen, char *out, size_t outsz) {
    size_t oi = 0;
    size_t i = 0;
    while (i < inlen && oi + 1 < outsz) {
        if (in[i] == '[') {
            size_t j = i + 1;
            const char *rbr = NULL;
            while (j < inlen) {
                if (in[j] == ']' && j + 1 < inlen && in[j + 1] == '(') {
                    rbr = in + j;
                    break;
                }
                j++;
            }
            if (rbr) {
                const char *url = rbr + 2;
                size_t remain = (size_t)(in + inlen - url);
                const char *rpar = remain ? memchr(url, ')', remain) : NULL;
                if (rpar) {
                    size_t tlen = (size_t)(rbr - (in + i + 1));
                    if (oi + tlen >= outsz) {
                        tlen = outsz - 1 - oi;
                    }
                    if (tlen > 0) {
                        memcpy(out + oi, in + i + 1, tlen);
                        oi += tlen;
                    }
                    i = (size_t)(rpar - in) + 1;
                    continue;
                }
            }
        }
        out[oi++] = in[i++];
    }
    out[oi] = '\0';
}

/* Keep at most two .?! sentences; a third is dropped entirely. No terminator
 * means the remainder is the current sentence (still at most two). */
static void cut_after_two_sentences(char *s) {
    int found = 0;
    for (char *p = s; *p; p++) {
        if (*p == '.' || *p == '?' || *p == '!') {
            unsigned char nxt = (unsigned char)p[1];
            if (p[1] == '\0' || isspace(nxt)) {
                found++;
                if (found >= 2) {
                    p[1] = '\0';
                    return;
                }
            }
        }
    }
}

static void collapse_newlines_to_space(char *s) {
    char *r = s;
    char *w = s;
    while (*r) {
        if (*r == '\r') {
            *w++ = ' ';
            if (r[1] == '\n') {
                r++;
            }
        } else if (*r == '\n') {
            *w++ = ' ';
        } else {
            *w++ = *r;
        }
        r++;
    }
    *w = '\0';
}

static void trim_ascii_space(char *s) {
    char *a = s;
    while (*a && isspace((unsigned char)*a)) {
        a++;
    }
    if (a != s) {
        memmove(s, a, strlen(a) + 1);
    }
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[--n] = '\0';
    }
}

/* Byte-truncate then walk back to the last space so the 512 cap does not
 * tear a word. No space in the window → keep the hard cut. */
static void copy_blurb_truncated(const char *src, char *out, size_t outsz) {
    size_t n = strlen(src);
    if (n < outsz) {
        memcpy(out, src, n + 1);
        return;
    }
    memcpy(out, src, outsz - 1);
    out[outsz - 1] = '\0';
    for (size_t i = outsz - 1; i > 0; i--) {
        if (out[i - 1] == ' ') {
            out[i - 1] = '\0';
            break;
        }
    }
}

void cbm_spec_board_extract_blurb(const char *spec_md, char *out, size_t outsz) {
    if (!out || outsz == 0) {
        return;
    }
    out[0] = '\0';
    if (!spec_md || !spec_md[0] || outsz < 2) {
        return;
    }

    const char *heading = find_executive_summary_line(spec_md);
    if (!heading) {
        return;
    }
    const char *nl = strchr(heading, '\n');
    if (!nl) {
        return;
    }
    const char *body = nl + 1;
    /* Next H2 immediately under the heading (## KPI with no body). */
    if (strncmp(body, "## ", 3) == 0) {
        return;
    }
    const char *h2 = strstr(body, "\n## ");
    size_t body_len = h2 ? (size_t)(h2 - body) : strlen(body);
    if (body_len == 0) {
        return;
    }

    char *stripped = malloc(body_len + 1);
    if (!stripped) {
        return;
    }
    strip_md_links(body, body_len, stripped, body_len + 1);
    cut_after_two_sentences(stripped);
    collapse_newlines_to_space(stripped);
    trim_ascii_space(stripped);
    if (!stripped[0]) {
        free(stripped);
        return;
    }
    copy_blurb_truncated(stripped, out, outsz);
    free(stripped);
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

/* ── history/test_results.log (one buffer, applied to every entry) ──
 * Walk lines without strtok so the same fopen can be reused (SDD-ADR-028).
 * Active: bare Task #N, last line wins, done = line contains PASS.
 * Non-active: same last-line-wins, but the line must also contain e->id. */

static void apply_test_results_log(const char *log_buf, cbm_spec_board_entry_t *e) {
    e->tasks_done = 0;
    if (!log_buf || e->task_count <= 0) {
        return;
    }

    const char *p = log_buf;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
        char line[2048];
        size_t copy = linelen < sizeof(line) - 1 ? linelen : sizeof(line) - 1;
        memcpy(line, p, copy);
        line[copy] = '\0';

        const char *marker = strstr(line, "Task #");
        if (marker) {
            bool qualified = e->active || (e->id[0] && strstr(line, e->id) != NULL);
            if (qualified) {
                int n = atoi(marker + 6);
                bool pass = strstr(line, "PASS") != NULL;
                for (int i = 0; i < e->task_count; i++) {
                    if (e->tasks[i].number == n) {
                        e->tasks[i].done = pass; /* last matching line wins */
                        break;
                    }
                }
            }
        }

        if (!eol) {
            break;
        }
        p = eol + 1;
    }

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

/* ── grill (.grill/ walk; SDD-ADR-036) ───────────────────────────
 * Zero-write. Dirent names with /, \, or .. are rejected. Companion-to
 * tokens are never fopen'd. Cap 64 after omit; unreadable epic skipped. */

enum { GRILL_MAX_PLANS = 96, GRILL_MAX_EPIC_FILES = 96 };

typedef struct {
    char slug[96];
    char title[256];
} grill_plan_ref_t;

typedef struct {
    int nnn;
    char filename[160];
} grill_epic_file_t;

static bool grill_dirent_rejected(const char *name) {
    if (!name || !name[0]) {
        return true;
    }
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return true;
    }
    if (strchr(name, '/') != NULL || strchr(name, '\\') != NULL) {
        return true;
    }
    if (strstr(name, "..") != NULL) {
        return true;
    }
    return false;
}

static int grill_cmp_slug(const void *a, const void *b) {
    const grill_plan_ref_t *pa = (const grill_plan_ref_t *)a;
    const grill_plan_ref_t *pb = (const grill_plan_ref_t *)b;
    return strcmp(pa->slug, pb->slug);
}

static int grill_cmp_nnn(const void *a, const void *b) {
    const grill_epic_file_t *ea = (const grill_epic_file_t *)a;
    const grill_epic_file_t *eb = (const grill_epic_file_t *)b;
    if (ea->nnn < eb->nnn) {
        return -1;
    }
    if (ea->nnn > eb->nnn) {
        return 1;
    }
    return strcmp(ea->filename, eb->filename);
}

static bool grill_parse_epic_filename(const char *name, int *nnn_out) {
    const char *p;
    int nnn = 0;
    size_t len;
    if (!name || strncmp(name, "epic-", 5) != 0) {
        return false;
    }
    p = name + 5;
    if (!isdigit((unsigned char)*p)) {
        return false;
    }
    while (isdigit((unsigned char)*p)) {
        nnn = nnn * 10 + (*p - '0');
        p++;
    }
    if (*p != '-') {
        return false;
    }
    len = strlen(name);
    if (len < 4 || strcmp(name + len - 3, ".md") != 0) {
        return false;
    }
    *nnn_out = nnn;
    return true;
}

static bool grill_epic_converted(const char *epic_id, const cbm_spec_board_t *board,
                                 const char *source_grill_epic) {
    int i;
    if (!epic_id || !epic_id[0]) {
        return false;
    }
    if (source_grill_epic && source_grill_epic[0] && strcmp(source_grill_epic, epic_id) == 0) {
        return true;
    }
    if (!board) {
        return false;
    }
    for (i = 0; i < board->spec_count; i++) {
        if (board->specs[i].companion_grill[0] &&
            strcmp(board->specs[i].companion_grill, epic_id) == 0) {
            return true;
        }
    }
    return false;
}

static void grill_trim_copy(char *dst, size_t dstsz, const char *src, size_t len) {
    while (len > 0 && isspace((unsigned char)*src)) {
        src++;
        len--;
    }
    while (len > 0 && isspace((unsigned char)src[len - 1])) {
        len--;
    }
    if (len >= dstsz) {
        len = dstsz - 1;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static int grill_split_gfm_row(const char *line, char cells[][256], int max_cells) {
    const char *p = line;
    int n = 0;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (*p != '|') {
        return 0;
    }
    /* Leading empty cell before the first pipe (GFM). */
    cells[0][0] = '\0';
    n = 1;
    p++;
    while (*p && n < max_cells) {
        const char *start = p;
        while (*p && *p != '|' && *p != '\n' && *p != '\r') {
            p++;
        }
        grill_trim_copy(cells[n], 256, start, (size_t)(p - start));
        n++;
        if (*p == '|') {
            p++;
        } else {
            break;
        }
    }
    return n;
}

static bool grill_slug_is_header_or_sep(const char *slug) {
    const char *p;
    if (!slug || !slug[0]) {
        return true;
    }
    if (strcmp(slug, "slug") == 0) {
        return true;
    }
    p = slug;
    while (*p == '-' || *p == ':' || *p == ' ') {
        p++;
    }
    /* Separator cells are dashes (possibly with colons). */
    if (p > slug && *p == '\0') {
        return true;
    }
    if (strstr(slug, "---") != NULL) {
        return true;
    }
    return false;
}

static void grill_read_plan_md_title(const char *root, const char *slug, char *out, size_t outsz) {
    char path[1400];
    char *buf;
    const char *p;
    out[0] = '\0';
    snprintf(path, sizeof(path), "%s/.grill/plans/%s/plan.md", root, slug);
    buf = read_whole_file(path);
    if (!buf) {
        return;
    }
    p = buf;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (strncmp(p, "---", 3) != 0) {
        free(buf);
        return;
    }
    p = strchr(p, '\n');
    if (!p) {
        free(buf);
        return;
    }
    p++;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
        char line[512];
        const char *t;
        size_t copy = linelen < sizeof(line) - 1 ? linelen : sizeof(line) - 1;
        memcpy(line, p, copy);
        line[copy] = '\0';
        t = line;
        while (*t == ' ' || *t == '\t') {
            t++;
        }
        if (strncmp(t, "---", 3) == 0) {
            break;
        }
        kv_extract_colon_line(line, "title", out, outsz);
        if (!eol) {
            break;
        }
        p = eol + 1;
    }
    free(buf);
}

static void grill_collect_plans(const char *root, grill_plan_ref_t *plans, int *nplans) {
    char index_path[1152];
    char *idx;
    grill_plan_ref_t unlisted[GRILL_MAX_PLANS];
    int nun = 0;
    int indexed;
    char plans_dir[1200];
    cbm_dir_t *d;

    *nplans = 0;
    snprintf(index_path, sizeof(index_path), "%s/.grill/index.md", root);
    idx = read_whole_file(index_path);
    if (idx) {
        const char *p = idx;
        while (*p && *nplans < GRILL_MAX_PLANS) {
            const char *eol = strchr(p, '\n');
            size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
            char line[1024];
            char cells[8][256];
            int ncells;
            size_t copy = linelen < sizeof(line) - 1 ? linelen : sizeof(line) - 1;
            memcpy(line, p, copy);
            line[copy] = '\0';
            ncells = grill_split_gfm_row(line, cells, 8);
            if (ncells >= 3 && !grill_slug_is_header_or_sep(cells[1]) && cells[1][0]) {
                if (!grill_dirent_rejected(cells[1])) {
                    memset(&plans[*nplans], 0, sizeof(plans[*nplans]));
                    snprintf(plans[*nplans].slug, sizeof(plans[*nplans].slug), "%s", cells[1]);
                    snprintf(plans[*nplans].title, sizeof(plans[*nplans].title), "%s", cells[2]);
                    (*nplans)++;
                }
            }
            if (!eol) {
                break;
            }
            p = eol + 1;
        }
        free(idx);
    }

    indexed = *nplans;
    snprintf(plans_dir, sizeof(plans_dir), "%s/.grill/plans", root);
    d = cbm_opendir(plans_dir);
    if (d) {
        cbm_dirent_t *ent;
        while ((ent = cbm_readdir(d)) != NULL) {
            char full[1400];
            int i;
            bool found = false;
            if (grill_dirent_rejected(ent->name)) {
                continue;
            }
            snprintf(full, sizeof(full), "%s/%s", plans_dir, ent->name);
            if (!cbm_is_dir(full)) {
                continue;
            }
            for (i = 0; i < indexed; i++) {
                if (strcmp(plans[i].slug, ent->name) == 0) {
                    found = true;
                    break;
                }
            }
            if (found || nun >= GRILL_MAX_PLANS) {
                continue;
            }
            memset(&unlisted[nun], 0, sizeof(unlisted[nun]));
            snprintf(unlisted[nun].slug, sizeof(unlisted[nun].slug), "%s", ent->name);
            nun++;
        }
        cbm_closedir(d);
    }
    if (nun > 1) {
        qsort(unlisted, (size_t)nun, sizeof(unlisted[0]), grill_cmp_slug);
    } else if (nun == 1) {
        /* single unlisted: no sort needed */
    }
    {
        int i;
        for (i = 0; i < nun && *nplans < GRILL_MAX_PLANS; i++) {
            plans[(*nplans)++] = unlisted[i];
        }
    }
}

static void grill_append_epic(const char *root, const char *slug, const char *filename,
                              const char *plan_title, const char *source_grill_epic,
                              cbm_spec_board_t *out) {
    char rel_id[256];
    char path[1600];
    char *buf;
    const char *p;
    int n;
    cbm_spec_board_epic_t *e;

    if (out->epic_count >= CBM_SPEC_BOARD_MAX_EPICS) {
        return;
    }
    n = snprintf(rel_id, sizeof(rel_id), ".grill/plans/%s/epics/%s", slug, filename);
    if (n < 0 || n >= (int)sizeof(rel_id)) {
        return;
    }
    if (grill_epic_converted(rel_id, out, source_grill_epic)) {
        return;
    }
    snprintf(path, sizeof(path), "%s/%s", root, rel_id);
    /* fopen rb on a directory can succeed on POSIX; skip dir-at-path. */
    if (cbm_is_dir(path)) {
        return;
    }
    buf = read_whole_file(path);
    if (!buf) {
        return;
    }

    e = &out->epics[out->epic_count];
    memset(e, 0, sizeof(*e));
    snprintf(e->id, sizeof(e->id), "%s", rel_id);
    snprintf(e->column, sizeof(e->column), "todo");
    snprintf(e->plan_title, sizeof(e->plan_title), "%s", plan_title);

    p = buf;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
        char line[1024];
        size_t copy = linelen < sizeof(line) - 1 ? linelen : sizeof(line) - 1;
        memcpy(line, p, copy);
        line[copy] = '\0';
        kv_extract_colon_line(line, "name", e->title, sizeof(e->title));
        kv_extract_colon_line(line, "summary", e->summary, sizeof(e->summary));
        if (!eol) {
            break;
        }
        p = eol + 1;
    }
    free(buf);
    out->epic_count++;
}

static void grill_fill_epics(const char *root, cbm_spec_board_t *out, const char *source_grill_epic) {
    grill_plan_ref_t plans[GRILL_MAX_PLANS];
    int nplans = 0;
    int pi;

    grill_collect_plans(root, plans, &nplans);
    for (pi = 0; pi < nplans; pi++) {
        char plan_title[256];
        char epics_dir[1400];
        cbm_dir_t *d;
        grill_epic_file_t files[GRILL_MAX_EPIC_FILES];
        int nfiles = 0;
        int fi;

        if (out->epic_count >= CBM_SPEC_BOARD_MAX_EPICS) {
            break;
        }
        if (plans[pi].title[0]) {
            snprintf(plan_title, sizeof(plan_title), "%s", plans[pi].title);
        } else {
            grill_read_plan_md_title(root, plans[pi].slug, plan_title, sizeof(plan_title));
            if (!plan_title[0]) {
                snprintf(plan_title, sizeof(plan_title), "%s", plans[pi].slug);
            }
        }

        snprintf(epics_dir, sizeof(epics_dir), "%s/.grill/plans/%s/epics", root, plans[pi].slug);
        d = cbm_opendir(epics_dir);
        if (!d) {
            continue;
        }
        {
            cbm_dirent_t *ent;
            while ((ent = cbm_readdir(d)) != NULL && nfiles < GRILL_MAX_EPIC_FILES) {
                int nnn = 0;
                if (grill_dirent_rejected(ent->name)) {
                    continue;
                }
                if (ent->is_dir) {
                    /* dir-at-path still listed if the name matches; append will skip unreadable */
                }
                if (!grill_parse_epic_filename(ent->name, &nnn)) {
                    continue;
                }
                memset(&files[nfiles], 0, sizeof(files[nfiles]));
                files[nfiles].nnn = nnn;
                snprintf(files[nfiles].filename, sizeof(files[nfiles].filename), "%s", ent->name);
                nfiles++;
            }
        }
        cbm_closedir(d);
        if (nfiles > 1) {
            qsort(files, (size_t)nfiles, sizeof(files[0]), grill_cmp_nnn);
        }
        for (fi = 0; fi < nfiles; fi++) {
            if (out->epic_count >= CBM_SPEC_BOARD_MAX_EPICS) {
                break;
            }
            grill_append_epic(root, plans[pi].slug, files[fi].filename, plan_title, source_grill_epic,
                              out);
        }
    }
}

/* ── TECH_DEBT.md (heading Status wins; Debt Summary table fallback) ─ */

enum { DEBT_TABLE_CAP = 128, DEBT_GFM_CELLS = 16 };

typedef struct {
    char id[32];
    char status[64];
} debt_table_row_t;

static int debt_is_h2_at(const char *p) {
    return p[0] == '#' && p[1] == '#' && p[2] == ' ';
}

static const char *debt_line_end(const char *p) {
    const char *eol = strchr(p, '\n');
    return eol ? eol : p + strlen(p);
}

static const char *debt_skip_ws(const char *p, const char *end) {
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\r')) {
        p++;
    }
    return p;
}

static void debt_copy_token(const char *p, const char *end, char *tok, size_t toksz) {
    size_t n = 0;

    while (p < end && *p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != '|') {
        if (n + 1 < toksz) {
            tok[n++] = *p;
        }
        p++;
    }
    tok[n] = '\0';
}

static int debt_extract_status_span(const char *p, const char *end, char *tok, size_t toksz) {
    const char *s = debt_skip_ws(p, end);
    const char *q;
    int saw_bar = 0;

    if ((size_t)(end - s) >= 7 && memcmp(s, "Status:", 7) == 0) {
        s = debt_skip_ws(s + 7, end);
        debt_copy_token(s, end, tok, toksz);
        return 1;
    }

    for (q = p; q < end; q++) {
        if (*q == '|') {
            saw_bar = 1;
            break;
        }
    }
    if (!saw_bar) {
        return 0;
    }

    q = p;
    while (q < end) {
        const char *bar = q;
        const char *cell;
        const char *cell_end;

        while (bar < end && *bar != '|') {
            bar++;
        }
        cell = debt_skip_ws(q, bar);
        cell_end = bar;
        while (cell_end > cell &&
               (cell_end[-1] == ' ' || cell_end[-1] == '\t' || cell_end[-1] == '\r')) {
            cell_end--;
        }
        if ((size_t)(cell_end - cell) >= 7 && memcmp(cell, "Status:", 7) == 0) {
            const char *tokp = debt_skip_ws(cell + 7, cell_end);
            debt_copy_token(tokp, cell_end, tok, toksz);
            return 1;
        }
        if (bar >= end) {
            break;
        }
        q = bar + 1;
    }
    return 0;
}

static int debt_heading_status(const char *block, const char *block_end, char *tok, size_t toksz) {
    const char *p = block;

    tok[0] = '\0';
    while (p < block_end && *p) {
        const char *eol = debt_line_end(p);

        if (eol > block_end) {
            eol = block_end;
        }
        if (debt_extract_status_span(p, eol, tok, toksz)) {
            return 1;
        }
        if (*eol != '\n') {
            break;
        }
        p = eol + 1;
    }
    return 0;
}

static int debt_parse_td_heading(const char *p, const char *eol, char *id, size_t idsz, char *title,
                                 size_t titlesz) {
    const char *s;
    const char *digits;
    const char *dend;
    const char *t;
    const char *tend;
    size_t idlen;
    size_t tlen;

    id[0] = '\0';
    title[0] = '\0';
    if (!debt_is_h2_at(p)) {
        return 0;
    }
    s = p + 3;
    if ((size_t)(eol - s) < 4 || memcmp(s, "TD-", 3) != 0) {
        return 0;
    }
    digits = s + 3;
    if (digits >= eol || !isdigit((unsigned char)*digits)) {
        return 0;
    }
    dend = digits;
    while (dend < eol && isdigit((unsigned char)*dend)) {
        dend++;
    }
    idlen = (size_t)(dend - s);
    if (idlen >= idsz) {
        idlen = idsz - 1;
    }
    memcpy(id, s, idlen);
    id[idlen] = '\0';

    t = debt_skip_ws(dend, eol);
    if (t < eol && *t == ':') {
        t = debt_skip_ws(t + 1, eol);
        tend = eol;
        while (tend > t && (tend[-1] == ' ' || tend[-1] == '\t' || tend[-1] == '\r')) {
            tend--;
        }
        tlen = (size_t)(tend - t);
        if (tlen >= titlesz) {
            tlen = titlesz - 1;
        }
        memcpy(title, t, tlen);
        title[tlen] = '\0';
    }
    return 1;
}

static int debt_is_summary_heading(const char *p, const char *eol) {
    static const char key[] = "## Debt Summary";
    const size_t klen = sizeof(key) - 1;
    const char *s;

    if ((size_t)(eol - p) < klen || memcmp(p, key, klen) != 0) {
        return 0;
    }
    s = debt_skip_ws(p + klen, eol);
    return s >= eol;
}

static const char *debt_next_h2(const char *p) {
    while (*p) {
        if (debt_is_h2_at(p)) {
            return p;
        }
        p = (*debt_line_end(p) == '\n') ? debt_line_end(p) + 1 : p + strlen(p);
    }
    return p;
}

static int debt_gfm_nth_cell(const char *p, const char *eol, int idx, char *out, size_t outsz) {
    int n = 0;
    const char *q = p;

    out[0] = '\0';
    if (q < eol && *q == '|') {
        q++;
    }
    while (q < eol) {
        const char *bar = q;
        const char *a;
        const char *b;
        size_t len;

        while (bar < eol && *bar != '|') {
            bar++;
        }
        if (n == idx) {
            a = debt_skip_ws(q, bar);
            b = bar;
            while (b > a && (b[-1] == ' ' || b[-1] == '\t' || b[-1] == '\r')) {
                b--;
            }
            len = (size_t)(b - a);
            if (len >= outsz) {
                len = outsz - 1;
            }
            memcpy(out, a, len);
            out[len] = '\0';
            return 1;
        }
        n++;
        if (bar >= eol) {
            break;
        }
        q = bar + 1;
        if (q >= eol) {
            break;
        }
    }
    return 0;
}

static int debt_gfm_header_cols(const char *p, const char *eol, int *id_col, int *status_col) {
    char cell[64];
    int i;

    *id_col = -1;
    *status_col = -1;
    for (i = 0; i < DEBT_GFM_CELLS; i++) {
        if (!debt_gfm_nth_cell(p, eol, i, cell, sizeof(cell))) {
            break;
        }
        if (strcmp(cell, "ID") == 0) {
            *id_col = i;
        } else if (strcmp(cell, "Status") == 0) {
            *status_col = i;
        }
    }
    return *id_col >= 0 && *status_col >= 0;
}

static int debt_gfm_is_sep(const char *p, const char *eol) {
    char cell[64];
    int i;
    int any = 0;

    for (i = 0; i < DEBT_GFM_CELLS; i++) {
        const char *c;

        if (!debt_gfm_nth_cell(p, eol, i, cell, sizeof(cell))) {
            break;
        }
        if (!cell[0]) {
            continue;
        }
        any = 1;
        for (c = cell; *c; c++) {
            if (*c != '-' && *c != ':') {
                return 0;
            }
        }
    }
    return any;
}

static int debt_line_has_bar(const char *p, const char *eol) {
    const char *t;

    for (t = p; t < eol; t++) {
        if (*t == '|') {
            return 1;
        }
    }
    return 0;
}

static void debt_fill_summary_table(const char *md, debt_table_row_t *rows, int *nrow) {
    const char *p = md;

    *nrow = 0;
    while (*p) {
        const char *eol = debt_line_end(p);

        if (debt_is_h2_at(p) && debt_is_summary_heading(p, eol)) {
            const char *q = (*eol == '\n') ? eol + 1 : eol;
            int id_col = -1;
            int status_col = -1;
            int have_header = 0;

            while (*q) {
                const char *qeol = debt_line_end(q);
                char id[32];
                char st[64];
                char tok[64];
                int dup;
                int i;

                if (debt_is_h2_at(q)) {
                    break;
                }
                if (debt_line_has_bar(q, qeol)) {
                    if (!have_header) {
                        if (debt_gfm_header_cols(q, qeol, &id_col, &status_col)) {
                            have_header = 1;
                        }
                    } else if (!debt_gfm_is_sep(q, qeol) && *nrow < DEBT_TABLE_CAP) {
                        if (debt_gfm_nth_cell(q, qeol, id_col, id, sizeof(id)) &&
                            debt_gfm_nth_cell(q, qeol, status_col, st, sizeof(st)) && id[0]) {
                            debt_copy_token(st, st + strlen(st), tok, sizeof(tok));
                            dup = 0;
                            for (i = 0; i < *nrow; i++) {
                                if (strcmp(rows[i].id, id) == 0) {
                                    dup = 1;
                                    break;
                                }
                            }
                            if (!dup) {
                                snprintf(rows[*nrow].id, sizeof(rows[*nrow].id), "%s", id);
                                snprintf(rows[*nrow].status, sizeof(rows[*nrow].status), "%s", tok);
                                (*nrow)++;
                            }
                        }
                    }
                }
                if (*qeol != '\n') {
                    break;
                }
                q = qeol + 1;
            }
            return;
        }
        if (*eol != '\n') {
            break;
        }
        p = eol + 1;
    }
}

static const char *debt_table_status(const debt_table_row_t *rows, int nrow, const char *id) {
    int i;

    for (i = 0; i < nrow; i++) {
        if (strcmp(rows[i].id, id) == 0) {
            return rows[i].status;
        }
    }
    return NULL;
}

static int debt_id_seen(char seen[][32], int nseen, const char *id) {
    int i;

    for (i = 0; i < nseen; i++) {
        if (strcmp(seen[i], id) == 0) {
            return 1;
        }
    }
    return 0;
}

void cbm_spec_board_parse_tech_debt(const char *md, cbm_spec_board_debt_t *out, int *count) {
    debt_table_row_t table[DEBT_TABLE_CAP];
    char seen[DEBT_TABLE_CAP][32];
    int ntable = 0;
    int nseen = 0;
    const char *p;

    if (!count) {
        return;
    }
    *count = 0;
    if (!out || !md || !md[0]) {
        return;
    }

    debt_fill_summary_table(md, table, &ntable);
    p = md;
    while (*p) {
        const char *eol = debt_line_end(p);
        char id[32];
        char title[256];
        char status[64];
        const char *tbl;

        if (debt_parse_td_heading(p, eol, id, sizeof(id), title, sizeof(title))) {
            const char *block = (*eol == '\n') ? eol + 1 : eol;
            const char *block_end = debt_next_h2(block);

            if (!debt_id_seen(seen, nseen, id)) {
                if (nseen < DEBT_TABLE_CAP) {
                    snprintf(seen[nseen], sizeof(seen[nseen]), "%s", id);
                    nseen++;
                }
                status[0] = '\0';
                if (!debt_heading_status(block, block_end, status, sizeof(status))) {
                    tbl = debt_table_status(table, ntable, id);
                    if (tbl) {
                        snprintf(status, sizeof(status), "%s", tbl);
                    }
                }
                if (strcmp(status, "resolved") != 0 && *count < CBM_SPEC_BOARD_MAX_DEBT) {
                    snprintf(out[*count].id, sizeof(out[*count].id), "%s", id);
                    snprintf(out[*count].title, sizeof(out[*count].title), "%s", title);
                    (*count)++;
                }
            }
            p = block_end;
            continue;
        }
        if (*eol != '\n') {
            break;
        }
        p = eol + 1;
    }
}

static void debt_fill(const char *root_path, cbm_spec_board_t *out) {
    char path[1152];
    char *md;

    out->debt_count = 0;
    if (!root_path || !root_path[0]) {
        return;
    }
    snprintf(path, sizeof(path), "%s/.sdd-skill/baseline/TECH_DEBT.md", root_path);
    md = read_whole_file(path);
    if (!md) {
        return;
    }
    cbm_spec_board_parse_tech_debt(md, out->debt, &out->debt_count);
    free(md);
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

bool cbm_spec_board_grill_skill_present(const char *root_path) {
    if (!root_path || !root_path[0]) {
        return false;
    }
    char path[1152];
    snprintf(path, sizeof(path), "%s/.grill", root_path);
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
    char source_grill_epic[256] = {0};
    memset(out, 0, sizeof(*out));

    if (cbm_spec_board_sdd_skill_present(root_path)) {
        char sdd_dir[1024];
        char active_id[192] = {0};
        char log_path[1152];
        char *log_buf;
        int i;

        out->sdd_skill_present = true;
        snprintf(sdd_dir, sizeof(sdd_dir), "%s/.sdd-skill", root_path);

        read_active_json(sdd_dir, out, active_id, sizeof(active_id), source_grill_epic,
                         sizeof(source_grill_epic));

        if (active_id[0] && out->spec_count < CBM_SPEC_BOARD_MAX_SPECS) {
            cbm_spec_board_entry_t *e = &out->specs[out->spec_count++];
            memset(e, 0, sizeof(*e));
            snprintf(e->id, sizeof(e->id), "%s", active_id);
            snprintf(e->column, sizeof(e->column), "in_progress");
            e->active = true;
            e->checklist_percent = -1;
        }

        /* One shared log, then title+blurb+tasks for every listed id. state.md
         * and checklist stay active-only (SDD-ADR-024, SDD-ADR-028). */
        snprintf(log_path, sizeof(log_path), "%s/history/test_results.log", sdd_dir);
        log_buf = read_whole_file(log_path);

        for (i = 0; i < out->spec_count; i++) {
            cbm_spec_board_entry_t *e = &out->specs[i];
            char spec_dir[1280];
            snprintf(spec_dir, sizeof(spec_dir), "%s/specs/%s", sdd_dir, e->id);
            read_spec_md(spec_dir, e);
            read_tasks_md(spec_dir, e);
            apply_test_results_log(log_buf, e);

            if (!e->active) {
                continue;
            }

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
                for (int t = 0; t < e->task_count; t++) {
                    if (e->tasks[t].number == cur_task) {
                        e->tasks[t].current = true;
                        break;
                    }
                }
            }
        }
        free(log_buf);
    }

    /* Grill fill is independent of sdd. Do not read .gamedev/ here. */
    out->grill_skill_present = cbm_spec_board_grill_skill_present(root_path);
    if (out->grill_skill_present) {
        grill_fill_epics(root_path, out, source_grill_epic);
    }

    /* Debt fill is independent of sdd. fopen rb only; missing/unreadable → []. */
    debt_fill(root_path, out);
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

    APP("{\"sdd_skill_present\":%s,\"grill_skill_present\":%s,\"specs\":[",
        b->sdd_skill_present ? "true" : "false", b->grill_skill_present ? "true" : "false");
    for (int i = 0; i < b->spec_count; i++) {
        const cbm_spec_board_entry_t *e = &b->specs[i];
        char esc_id[384], esc_title[512], esc_blurb[1024], esc_agent[64], esc_blocked[512];
        cbm_json_escape(esc_id, (int)sizeof(esc_id), e->id);
        cbm_json_escape(esc_title, (int)sizeof(esc_title), e->title);
        cbm_json_escape(esc_blurb, (int)sizeof(esc_blurb), e->blurb);
        cbm_json_escape(esc_agent, (int)sizeof(esc_agent), e->current_agent);
        cbm_json_escape(esc_blocked, (int)sizeof(esc_blocked), e->blocked_note);
        if (i > 0) {
            APP(",");
        }
        APP("{\"id\":\"%s\",\"title\":\"%s\",\"blurb\":\"%s\",\"column\":\"%s\","
            "\"archived\":%s,\"active\":%s,"
            "\"current_agent\":\"%s\",\"blocked_note\":\"%s\","
            "\"task_count\":%d,\"tasks_done\":%d,\"checklist_percent\":%.1f,\"tasks\":[",
            esc_id, esc_title, esc_blurb, e->column, e->archived ? "true" : "false",
            e->active ? "true" : "false", esc_agent, esc_blocked, e->task_count, e->tasks_done,
            e->checklist_percent);
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
    APP("],\"epics\":[");
    for (int i = 0; i < b->epic_count; i++) {
        const cbm_spec_board_epic_t *e = &b->epics[i];
        char esc_id[512], esc_title[512], esc_summary[1024], esc_plan[512];
        cbm_json_escape(esc_id, (int)sizeof(esc_id), e->id);
        cbm_json_escape(esc_title, (int)sizeof(esc_title), e->title);
        cbm_json_escape(esc_summary, (int)sizeof(esc_summary), e->summary);
        cbm_json_escape(esc_plan, (int)sizeof(esc_plan), e->plan_title);
        if (i > 0) {
            APP(",");
        }
        APP("{\"kind\":\"epic\",\"id\":\"%s\",\"title\":\"%s\",\"summary\":\"%s\","
            "\"plan_title\":\"%s\",\"column\":\"todo\"}",
            esc_id, esc_title, esc_summary, esc_plan);
    }
    APP("],\"debt\":[");
    for (int i = 0; i < b->debt_count; i++) {
        const cbm_spec_board_debt_t *d = &b->debt[i];
        char esc_did[80], esc_dtitle[640];
        cbm_json_escape(esc_did, (int)sizeof(esc_did), d->id);
        cbm_json_escape(esc_dtitle, (int)sizeof(esc_dtitle), d->title);
        if (i > 0) {
            APP(",");
        }
        APP("{\"id\":\"%s\",\"title\":\"%s\"}", esc_did, esc_dtitle);
    }
    APP("]}");
#undef APP
    return buf;
}
