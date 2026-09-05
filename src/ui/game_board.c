/**
 * @sdd-task: Task #1 - game_board debt parse + additive JSON
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-072 always-emit debt[{id,title}]; SDD-ADR-073 parse backlog.md here
 * @sdd-why: Open debt:* from .gamedev/backlog.md; do not call cbm_spec_board_parse_tech_debt
 * @human-debug: If Game debt stays [] with an open comment → tag charset / resolved-by in body / path not regular; if closed item appears → body ended at a blank line; if 17th shows → cap not applied
 *
 * game_board.c — dir check via spec_board helper; fopen rb on state.md
 * and artifact headers. Expand extract is local (do not call
 * cbm_spec_board_extract_blurb). Grill catalog and Game conversion live
 * here; do not call spec_board grill_* / active.json.
 */
#include "ui/game_board.h"
#include "ui/spec_board.h"
#include "foundation/compat_fs.h"
#include "foundation/platform.h"
#include "foundation/str_util.h"

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { GAME_BOARD_MAX_FILE = 1024 * 1024 };

static const char k_continue_cmd[] = "/gamedev-skill continue";

typedef enum {
    GB_PROD_SYS = 0,
    GB_PROD_LVL,
    GB_PROD_ART,
    GB_PROD_ANIM,
    GB_PROD_AUDIO,
    GB_PROD_UI,
    GB_PROD_QA
} gb_prod_kind_t;

typedef struct {
    char name[256];
    int nnn;
    gb_prod_kind_t kind;
} gb_prod_cand_t;

typedef struct {
    const char *basename;
    const char *owner;
    const char *track;
} gb_owner_row_t;

static const gb_owner_row_t k_file_owners[] = {
    {"gdd.md", "@game-designer", "B"},
    {"narrative-bible.md", "@narrative-designer", "B"},
    {"style-guide.md", "@art-director", "B"},
    {"tech-architecture.md", "@tech-architect", "A"},
    {"audio-direction.md", "@audio-director", "B"},
    {"production-plan.md", "@producer", "B"},
    {"playtest-log.md", "@qa-lead", "H"},
    {"optimization.md", "@performance-engineer", "A"},
    {"platform-integration.md", "@platform-integrator", "A"},
    {"release-plan.md", "@release-engineer", "A"},
    {"marketing-plan.md", "@marketing-strategist", "B"},
    {"postmortem.md", "@analyst", "B"},
};

static const char *k_preprod_files[] = {
    "gdd.md",
    "narrative-bible.md",
    "style-guide.md",
    "tech-architecture.md",
    "audio-direction.md",
    "production-plan.md",
};

static const char *k_postprod_files[] = {
    "optimization.md",
    "platform-integration.md",
    "release-plan.md",
    "marketing-plan.md",
    "postmortem.md",
};

static bool phase_token_ok(const char *token) {
    return token && (strcmp(token, "01-preproduction") == 0 ||
                     strcmp(token, "02-production") == 0 ||
                     strcmp(token, "03-postproduction") == 0);
}

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
    if (sz < 0 || sz > GAME_BOARD_MAX_FILE) {
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

/* Compact key=value. Same rules as spec_board kv_extract (quoted or unquoted). */
static void kv_extract(const char *text, const char *key, char *out, size_t outsz) {
    out[0] = '\0';
    if (!text || !key || outsz == 0) {
        return;
    }
    size_t klen = strlen(key);
    const char *p = text;
    while ((p = strstr(p, key)) != NULL) {
        bool at_start = (p == text) || isspace((unsigned char)p[-1]);
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

/* Line-start "key:" (leading spaces allowed). Does not clear out on miss. */
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

static void scan_colon_aliases(const char *buf, char *phase, size_t phase_sz, char *focus,
                               size_t focus_sz) {
    const char *line = buf;
    while (*line) {
        const char *nl = strchr(line, '\n');
        char tmp[1024];
        size_t llen = nl ? (size_t)(nl - line) : strlen(line);
        if (llen >= sizeof(tmp)) {
            llen = sizeof(tmp) - 1;
        }
        memcpy(tmp, line, llen);
        tmp[llen] = '\0';
        if (!phase[0]) {
            kv_extract_colon_line(tmp, "active_phase", phase, phase_sz);
        }
        if (!focus[0]) {
            kv_extract_colon_line(tmp, "director_focus", focus, focus_sz);
        }
        if (!nl) {
            break;
        }
        line = nl + 1;
    }
}

static void gb_trim_ascii_space(char *s) {
    char *a;
    size_t n;

    if (!s) {
        return;
    }
    a = s;
    while (*a && isspace((unsigned char)*a)) {
        a++;
    }
    if (a != s) {
        memmove(s, a, strlen(a) + 1);
    }
    n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[--n] = '\0';
    }
}

/* Byte-truncate then walk back to the last space so the 512 cap does not
 * tear a word. No space in the window → keep the hard cut. */
static void gb_copy_truncated(const char *src, char *out, size_t outsz) {
    size_t n;

    if (!out || outsz == 0) {
        return;
    }
    out[0] = '\0';
    if (!src) {
        return;
    }
    n = strlen(src);
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

static const char *gb_find_h2_line(const char *md, const char *heading, bool prefix_only) {
    const size_t klen = heading ? strlen(heading) : 0;
    const char *p = md;

    if (!md || !heading || klen == 0) {
        return NULL;
    }
    while (p) {
        const char *t = p;
        while (*t == ' ' || *t == '\t') {
            t++;
        }
        if (strncmp(t, heading, klen) == 0) {
            unsigned char next = (unsigned char)t[klen];
            if (prefix_only || next == '\0' || isspace(next)) {
                return p;
            }
        }
        const char *nl = strchr(p, '\n');
        p = nl ? nl + 1 : NULL;
    }
    return NULL;
}

/* [text](url) → text. Unclosed brackets stay literal — no other fallbacks. */
static void gb_strip_md_links(const char *in, size_t inlen, char *out, size_t outsz) {
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
static void gb_cut_after_two_sentences(char *s) {
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

static void gb_collapse_newlines_to_space(char *s) {
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

static void gb_extract_h2_body(const char *md, const char *heading, bool prefix_only, char *out,
                               size_t outsz, bool two_sentences) {
    const char *found;
    const char *nl;
    const char *body;
    const char *h2;
    size_t body_len;

    if (!out || outsz == 0) {
        return;
    }
    out[0] = '\0';
    if (!md || !md[0] || outsz < 2) {
        return;
    }
    found = gb_find_h2_line(md, heading, prefix_only);
    if (!found) {
        return;
    }
    nl = strchr(found, '\n');
    if (!nl) {
        return;
    }
    body = nl + 1;
    if (strncmp(body, "## ", 3) == 0) {
        return;
    }
    h2 = strstr(body, "\n## ");
    body_len = h2 ? (size_t)(h2 - body) : strlen(body);
    if (body_len == 0) {
        return;
    }

    if (two_sentences) {
        char *stripped = malloc(body_len + 1);
        if (!stripped) {
            return;
        }
        gb_strip_md_links(body, body_len, stripped, body_len + 1);
        gb_cut_after_two_sentences(stripped);
        gb_collapse_newlines_to_space(stripped);
        gb_trim_ascii_space(stripped);
        if (stripped[0]) {
            gb_copy_truncated(stripped, out, outsz);
        }
        free(stripped);
        return;
    }

    {
        char *tmp = malloc(body_len + 1);
        if (!tmp) {
            return;
        }
        memcpy(tmp, body, body_len);
        tmp[body_len] = '\0';
        gb_trim_ascii_space(tmp);
        if (tmp[0]) {
            gb_copy_truncated(tmp, out, outsz);
        }
        free(tmp);
    }
}

void cbm_game_board_extract_what_it_does(const char *md, char *out, size_t outsz) {
    gb_extract_h2_body(md, "## What it does", false, out, outsz, true);
}

void cbm_game_board_extract_last_round(const char *md, char *out, size_t outsz) {
    const char *last = NULL;
    const char *p;
    const char *next = NULL;
    size_t len;

    if (!out || outsz == 0) {
        return;
    }
    out[0] = '\0';
    if (!md || !md[0]) {
        return;
    }
    p = md;
    while (p) {
        const char *t = p;
        while (*t == ' ' || *t == '\t') {
            t++;
        }
        if (strncmp(t, "## Round ", 9) == 0) {
            last = p;
        }
        {
            const char *nl = strchr(p, '\n');
            p = nl ? nl + 1 : NULL;
        }
    }
    if (!last) {
        return;
    }
    {
        const char *nl = strchr(last, '\n');
        p = nl ? nl + 1 : last + strlen(last);
    }
    while (p && *p) {
        const char *t = p;
        while (*t == ' ' || *t == '\t') {
            t++;
        }
        if (strncmp(t, "## Round ", 9) == 0) {
            next = p;
            break;
        }
        {
            const char *nl = strchr(p, '\n');
            p = nl ? nl + 1 : NULL;
        }
    }
    len = next ? (size_t)(next - last) : strlen(last);
    {
        char *tmp = malloc(len + 1);
        if (!tmp) {
            return;
        }
        memcpy(tmp, last, len);
        tmp[len] = '\0';
        gb_trim_ascii_space(tmp);
        gb_copy_truncated(tmp, out, outsz);
        free(tmp);
    }
}

void cbm_game_board_extract_changelog_tail(const char *md, char *out, size_t outsz) {
    char *lines[8];
    int nkeep = 0;
    const char *p;
    char joined[CBM_GAME_BOARD_BLURB_MAX * 2];
    int pos = 0;
    int i;

    if (!out || outsz == 0) {
        return;
    }
    out[0] = '\0';
    if (!md) {
        return;
    }
    memset(lines, 0, sizeof(lines));
    p = md;
    while (*p) {
        const char *nl = strchr(p, '\n');
        size_t llen = nl ? (size_t)(nl - p) : strlen(p);
        char tmp[1024];
        char *copy;

        if (llen >= sizeof(tmp)) {
            llen = sizeof(tmp) - 1;
        }
        memcpy(tmp, p, llen);
        tmp[llen] = '\0';
        gb_trim_ascii_space(tmp);
        if (tmp[0] && strncmp(tmp, "<!--", 4) != 0) {
            copy = malloc(strlen(tmp) + 1);
            if (copy) {
                memcpy(copy, tmp, strlen(tmp) + 1);
                if (nkeep < 8) {
                    lines[nkeep++] = copy;
                } else {
                    free(lines[0]);
                    memmove(lines, lines + 1, 7 * sizeof(lines[0]));
                    lines[7] = copy;
                }
            }
        }
        if (!nl) {
            break;
        }
        p = nl + 1;
    }
    joined[0] = '\0';
    for (i = 0; i < nkeep; i++) {
        size_t remain = sizeof(joined) - (size_t)pos;
        int n;

        if (remain <= 1) {
            break;
        }
        n = snprintf(joined + pos, remain, "%s%s", i ? "\n" : "", lines[i]);
        if (n < 0) {
            break;
        }
        if ((size_t)n >= remain) {
            joined[sizeof(joined) - 1] = '\0';
            break;
        }
        pos += n;
        free(lines[i]);
        lines[i] = NULL;
    }
    for (; i < nkeep; i++) {
        free(lines[i]);
    }
    gb_copy_truncated(joined, out, outsz);
}

static bool gb_parse_quoted(const char **pp, char *out, size_t outsz) {
    const char *p = *pp;
    size_t n = 0;

    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (*p != '"') {
        return false;
    }
    p++;
    while (*p && *p != '"' && n + 1 < outsz) {
        out[n++] = *p++;
    }
    if (*p != '"') {
        return false;
    }
    out[n] = '\0';
    *pp = p + 1;
    return true;
}

static bool gb_parse_blocked_line(const char *line, cbm_game_board_blocked_t *row) {
    const char *l = line;
    const char *tag;
    size_t slen;
    char slug[48];
    const char *p;

    while (*l == ' ' || *l == '\t') {
        l++;
    }
    if (*l == '\0' || *l == '#' || strncmp(l, "<!--", 4) == 0) {
        return false;
    }
    if (strncmp(l, "phase=", 6) == 0 || strncmp(l, "focus=", 6) == 0) {
        return false;
    }
    if (strncmp(l, "active_phase", 12) == 0 || strncmp(l, "director_focus", 14) == 0) {
        return false;
    }
    tag = strstr(l, ":blocked:");
    if (!tag) {
        return false;
    }
    slen = (size_t)(tag - l);
    if (slen == 0 || slen >= sizeof(slug)) {
        return false;
    }
    memcpy(slug, l, slen);
    slug[slen] = '\0';
    gb_trim_ascii_space(slug);
    if (!slug[0]) {
        return false;
    }
    p = tag + 9;
    memset(row, 0, sizeof(*row));
    if (!gb_parse_quoted(&p, row->task, sizeof(row->task))) {
        return false;
    }
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (*p != ':') {
        return false;
    }
    p++;
    if (!gb_parse_quoted(&p, row->blocked_by, sizeof(row->blocked_by))) {
        return false;
    }
    if (slug[0] == '@') {
        snprintf(row->owner, sizeof(row->owner), "%s", slug);
    } else {
        snprintf(row->owner, sizeof(row->owner), "@%s", slug);
    }
    return true;
}

static void parse_state_md(const char *buf, cbm_game_board_t *out) {
    char compact_phase[32];
    char compact_focus[512];
    const char *line;

    kv_extract(buf, "phase", compact_phase, sizeof(compact_phase));
    if (compact_phase[0]) {
        snprintf(out->phase, sizeof(out->phase), "%s", compact_phase);
    } else {
        kv_extract(buf, "active_phase", out->phase, sizeof(out->phase));
    }

    kv_extract(buf, "focus", compact_focus, sizeof(compact_focus));
    if (compact_focus[0]) {
        snprintf(out->focus, sizeof(out->focus), "%s", compact_focus);
    } else {
        kv_extract(buf, "director_focus", out->focus, sizeof(out->focus));
    }

    if (!out->phase[0] || !out->focus[0]) {
        scan_colon_aliases(buf, out->phase, sizeof(out->phase), out->focus, sizeof(out->focus));
    }

    if (!phase_token_ok(out->phase)) {
        out->phase[0] = '\0';
    }

    line = buf;
    out->blocked_count = 0;
    while (*line && out->blocked_count < CBM_GAME_BOARD_MAX_BLOCKED) {
        const char *nl = strchr(line, '\n');
        char tmp[2048];
        size_t llen = nl ? (size_t)(nl - line) : strlen(line);

        if (llen >= sizeof(tmp)) {
            llen = sizeof(tmp) - 1;
        }
        memcpy(tmp, line, llen);
        tmp[llen] = '\0';
        if (gb_parse_blocked_line(tmp, &out->blocked[out->blocked_count])) {
            out->blocked_count++;
        }
        if (!nl) {
            break;
        }
        line = nl + 1;
    }
}

static bool game_is_regular_file(const char *path) {
    return path && path[0] && cbm_file_exists(path) && !cbm_is_dir(path);
}

static bool game_dirent_rejected(const char *name) {
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

static bool lookup_file_owner(const char *basename, const char **owner, const char **track) {
    size_t i;
    for (i = 0; i < sizeof(k_file_owners) / sizeof(k_file_owners[0]); i++) {
        if (strcmp(k_file_owners[i].basename, basename) == 0) {
            *owner = k_file_owners[i].owner;
            *track = k_file_owners[i].track;
            return true;
        }
    }
    return false;
}

static void owner_track_for_prod(gb_prod_kind_t kind, const char **owner, const char **track) {
    switch (kind) {
    case GB_PROD_SYS:
        *owner = "@gameplay-engineer";
        *track = "A";
        break;
    case GB_PROD_LVL:
        *owner = "@level-designer";
        *track = "H";
        break;
    case GB_PROD_ART:
        *owner = "@content-artist";
        *track = "B";
        break;
    case GB_PROD_ANIM:
        *owner = "@animator";
        *track = "B";
        break;
    case GB_PROD_AUDIO:
        *owner = "@sound-designer";
        *track = "B";
        break;
    case GB_PROD_UI:
        *owner = "@ui-designer";
        *track = "B";
        break;
    case GB_PROD_QA:
        *owner = "@qa-lead";
        *track = "H";
        break;
    }
}

static void map_status_token(const char *token, char *out, size_t outsz) {
    if (!token || !token[0] || strcmp(token, "draft") == 0) {
        snprintf(out, outsz, "pending");
        return;
    }
    if (strcmp(token, "in_review") == 0 || strcmp(token, "needs_review") == 0 ||
        strcmp(token, "wip") == 0 || strcmp(token, "ready") == 0) {
        snprintf(out, outsz, "in_progress");
        return;
    }
    if (strcmp(token, "approved") == 0 || strcmp(token, "done") == 0) {
        snprintf(out, outsz, "done");
        return;
    }
    if (strcmp(token, "blocked") == 0) {
        snprintf(out, outsz, "blocked");
        return;
    }
    snprintf(out, outsz, "pending");
}

static void first_status_token(const char *buf, char *out, size_t outsz) {
    const char *line;

    out[0] = '\0';
    if (!buf || outsz == 0) {
        return;
    }
    line = buf;
    while (*line) {
        const char *nl = strchr(line, '\n');
        char tmp[1024];
        char *hit;
        size_t llen = nl ? (size_t)(nl - line) : strlen(line);
        size_t bi;
        const char *v;

        if (llen >= sizeof(tmp)) {
            llen = sizeof(tmp) - 1;
        }
        memcpy(tmp, line, llen);
        tmp[llen] = '\0';
        hit = strstr(tmp, "status:");
        if (hit) {
            v = hit + 7;
            while (*v == ' ' || *v == '\t') {
                v++;
            }
            bi = 0;
            while (*v && *v != ' ' && *v != '\t' && *v != '\r' && bi + 1 < outsz) {
                out[bi++] = *v++;
            }
            out[bi] = '\0';
            return;
        }
        if (!nl) {
            break;
        }
        line = nl + 1;
    }
}

static void fill_artifact(cbm_game_board_card_t *card, const char *rel_id, const char *title,
                          const char *owner, const char *track, const char *work_state) {
    memset(card, 0, sizeof(*card));
    snprintf(card->kind, sizeof(card->kind), "artifact");
    snprintf(card->id, sizeof(card->id), "%s", rel_id);
    snprintf(card->title, sizeof(card->title), "%s", title);
    snprintf(card->track, sizeof(card->track), "%s", track);
    snprintf(card->work_state, sizeof(card->work_state), "%s", work_state);
    snprintf(card->owner, sizeof(card->owner), "%s", owner);
    snprintf(card->continue_cmd, sizeof(card->continue_cmd), "%s %s", k_continue_cmd, owner);
}

static void gb_path_join_sibling(const char *file_path, const char *sibling, char *out,
                                 size_t outsz) {
    const char *slash;

    if (!out || outsz == 0) {
        return;
    }
    out[0] = '\0';
    if (!file_path || !sibling) {
        return;
    }
    slash = strrchr(file_path, '/');
    if (!slash) {
        snprintf(out, outsz, "%s", sibling);
        return;
    }
    snprintf(out, outsz, "%.*s/%s", (int)(slash - file_path), file_path, sibling);
}

static bool gb_path_ends_with(const char *path, const char *suffix) {
    size_t plen;
    size_t slen;

    if (!path || !suffix) {
        return false;
    }
    plen = strlen(path);
    slen = strlen(suffix);
    return plen >= slen && strcmp(path + plen - slen, suffix) == 0;
}

static void gb_extract_header_kv(const char *buf, cbm_game_board_card_t *card) {
    const char *line;

    if (!buf || !card) {
        return;
    }
    line = buf;
    while (*line) {
        const char *nl = strchr(line, '\n');
        char tmp[2048];
        size_t llen = nl ? (size_t)(nl - line) : strlen(line);

        if (llen >= sizeof(tmp)) {
            llen = sizeof(tmp) - 1;
        }
        memcpy(tmp, line, llen);
        tmp[llen] = '\0';
        if (!card->last_decision[0]) {
            kv_extract_colon_line(tmp, "last_decision", card->last_decision,
                                  sizeof(card->last_decision));
        }
        if (!card->open[0]) {
            kv_extract_colon_line(tmp, "open", card->open, sizeof(card->open));
        }
        if (!nl) {
            break;
        }
        line = nl + 1;
    }
    gb_trim_ascii_space(card->last_decision);
    gb_trim_ascii_space(card->open);
}

static void gb_blurb_fallback(cbm_game_board_card_t *card) {
    if (card->open[0] && strcmp(card->open, "none") != 0) {
        gb_copy_truncated(card->open, card->blurb, sizeof(card->blurb));
        return;
    }
    if (card->last_decision[0] && strcmp(card->last_decision, "—") != 0) {
        gb_copy_truncated(card->last_decision, card->blurb, sizeof(card->blurb));
    }
}

static bool gb_task_line_skipped(const char *name) {
    if (!name || !name[0]) {
        return true;
    }
    if (strncmp(name, "<!--", 4) == 0) {
        return true;
    }
    if (strstr(name, "Subagent:") != NULL || strstr(name, "Path:") != NULL) {
        return true;
    }
    return false;
}

static void gb_read_tasks_md(const char *header_path, cbm_game_board_card_t *card) {
    char path[2048];
    char *buf;
    char *saveptr = NULL;
    char *line;
    int number = 1;

    gb_path_join_sibling(header_path, "tasks.md", path, sizeof(path));
    buf = read_whole_file(path);
    if (!buf) {
        return;
    }
    line = strtok_r(buf, "\n", &saveptr);
    while (line && card->task_count < CBM_GAME_BOARD_MAX_TASKS) {
        char tmp[1024];
        const char *l;
        bool done = false;
        const char *name;
        cbm_game_board_task_t *t;

        snprintf(tmp, sizeof(tmp), "%s", line);
        gb_trim_ascii_space(tmp);
        l = tmp;
        if (strncmp(l, "- [ ]", 5) == 0) {
            done = false;
            name = l + 5;
        } else if (strncmp(l, "- [x]", 5) == 0 || strncmp(l, "- [X]", 5) == 0) {
            done = true;
            name = l + 5;
        } else {
            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }
        while (*name == ' ' || *name == '\t') {
            name++;
        }
        if (gb_task_line_skipped(name)) {
            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }
        t = &card->tasks[card->task_count];
        memset(t, 0, sizeof(*t));
        t->done = done;
        snprintf(t->name, sizeof(t->name), "%s", name);
        gb_trim_ascii_space(t->name);
        if (!t->name[0]) {
            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }
        t->number = number++;
        card->task_count++;
        line = strtok_r(NULL, "\n", &saveptr);
    }
    free(buf);
}

static void apply_header_expand(const char *header_path, cbm_game_board_card_t *card) {
    char *buf = NULL;

    if (!card) {
        return;
    }
    if (game_is_regular_file(header_path)) {
        buf = read_whole_file(header_path);
    }
    if (buf) {
        char token[64];

        first_status_token(buf, token, sizeof(token));
        map_status_token(token, card->work_state, sizeof(card->work_state));
        gb_extract_header_kv(buf, card);
        if (strcmp(card->track, "A") == 0) {
            cbm_game_board_extract_what_it_does(buf, card->blurb, sizeof(card->blurb));
            if (!card->blurb[0]) {
                gb_blurb_fallback(card);
            }
            gb_extract_h2_body(buf, "## Inputs", true, card->inputs, sizeof(card->inputs), false);
        } else if (strcmp(card->track, "H") == 0 && gb_path_ends_with(header_path, "playtest-log.md")) {
            cbm_game_board_extract_last_round(buf, card->recent, sizeof(card->recent));
        }
        free(buf);
    } else {
        snprintf(card->work_state, sizeof(card->work_state), "pending");
        if (strcmp(card->track, "A") == 0) {
            gb_blurb_fallback(card);
        }
    }
    if (strcmp(card->track, "A") == 0) {
        gb_read_tasks_md(header_path, card);
    }
    if (strcmp(card->track, "H") == 0 && gb_path_ends_with(header_path, "/level.md")) {
        char clog[2048];
        char *cbuf;

        gb_path_join_sibling(header_path, "changelog.md", clog, sizeof(clog));
        cbuf = read_whole_file(clog);
        if (cbuf) {
            cbm_game_board_extract_changelog_tail(cbuf, card->recent, sizeof(card->recent));
            free(cbuf);
        }
    }
}

static void overlay_blocked_on_array(cbm_game_board_card_t *arr, int count,
                                     const cbm_game_board_blocked_t *row) {
    int i;

    for (i = 0; i < count; i++) {
        if (strcmp(arr[i].kind, "artifact") != 0) {
            continue;
        }
        if (strcmp(arr[i].owner, row->owner) != 0) {
            continue;
        }
        snprintf(arr[i].work_state, sizeof(arr[i].work_state), "blocked");
        snprintf(arr[i].blocked_by, sizeof(arr[i].blocked_by), "%s", row->blocked_by);
    }
}

static void overlay_blocked(cbm_game_board_t *out) {
    int i;

    if (!out) {
        return;
    }
    for (i = 0; i < out->blocked_count; i++) {
        overlay_blocked_on_array(out->preproduction, out->preproduction_count, &out->blocked[i]);
        overlay_blocked_on_array(out->production, out->production_count, &out->blocked[i]);
        overlay_blocked_on_array(out->postproduction, out->postproduction_count, &out->blocked[i]);
    }
}

static void append_card(cbm_game_board_card_t *arr, int *count, const cbm_game_board_card_t *card) {
    if (*count >= CBM_GAME_BOARD_MAX_CARDS) {
        return;
    }
    arr[(*count)++] = *card;
}

static void fill_fixed_files(const char *root, const char *phase_rel, const char *const *names,
                             size_t nnames, cbm_game_board_card_t *arr, int *count) {
    size_t i;
    for (i = 0; i < nnames; i++) {
        char abs[2048];
        char rel[512];
        const char *owner = NULL;
        const char *track = NULL;
        cbm_game_board_card_t card;

        snprintf(abs, sizeof(abs), "%s/%s/%s", root, phase_rel, names[i]);
        if (!game_is_regular_file(abs)) {
            continue;
        }
        if (!lookup_file_owner(names[i], &owner, &track)) {
            continue;
        }
        snprintf(rel, sizeof(rel), "%s/%s", phase_rel, names[i]);
        fill_artifact(&card, rel, names[i], owner, track, "pending");
        apply_header_expand(abs, &card);
        append_card(arr, count, &card);
    }
}

static int parse_prefix_nnn(const char *name, const char *prefix) {
    size_t plen;
    const char *p;
    long v;

    if (!name || !prefix) {
        return INT_MAX;
    }
    plen = strlen(prefix);
    if (strncmp(name, prefix, plen) != 0) {
        return INT_MAX;
    }
    p = name + plen;
    if (!isdigit((unsigned char)*p)) {
        return INT_MAX;
    }
    v = 0;
    while (isdigit((unsigned char)*p)) {
        if (v > (long)(INT_MAX / 10)) {
            return INT_MAX;
        }
        v = v * 10 + (*p - '0');
        p++;
    }
    return (int)v;
}

static int prod_cand_cmp(const void *a, const void *b) {
    const gb_prod_cand_t *pa = (const gb_prod_cand_t *)a;
    const gb_prod_cand_t *pb = (const gb_prod_cand_t *)b;

    if (pa->kind != pb->kind) {
        return (int)pa->kind - (int)pb->kind;
    }
    if ((pa->kind == GB_PROD_SYS || pa->kind == GB_PROD_LVL) && pa->nnn != pb->nnn) {
        return pa->nnn < pb->nnn ? -1 : 1;
    }
    return strcmp(pa->name, pb->name);
}

static void prod_try_add(gb_prod_cand_t *list, int *n, const char *name, gb_prod_kind_t kind,
                         const char *nnn_prefix) {
    gb_prod_cand_t *c;

    if (*n >= CBM_GAME_BOARD_LIST_MAX) {
        return;
    }
    if (game_dirent_rejected(name)) {
        return;
    }
    c = &list[*n];
    memset(c, 0, sizeof(*c));
    snprintf(c->name, sizeof(c->name), "%s", name);
    c->kind = kind;
    c->nnn = nnn_prefix ? parse_prefix_nnn(name, nnn_prefix) : INT_MAX;
    (*n)++;
}

static void collect_child_dirs(const char *parent, const char *prefix, gb_prod_kind_t kind,
                              gb_prod_cand_t *list, int *n) {
    cbm_dir_t *d;
    cbm_dirent_t *ent;

    d = cbm_opendir(parent);
    if (!d) {
        return;
    }
    while ((ent = cbm_readdir(d)) != NULL) {
        char full[2048];

        if (*n >= CBM_GAME_BOARD_LIST_MAX) {
            break;
        }
        if (game_dirent_rejected(ent->name)) {
            continue;
        }
        if (prefix && strncmp(ent->name, prefix, strlen(prefix)) != 0) {
            continue;
        }
        snprintf(full, sizeof(full), "%s/%s", parent, ent->name);
        if (!cbm_is_dir(full)) {
            continue;
        }
        prod_try_add(list, n, ent->name, kind, prefix);
    }
    cbm_closedir(d);
}

static void prod_header_path(const char *root, const gb_prod_cand_t *c, char *out, size_t outsz) {
    switch (c->kind) {
    case GB_PROD_SYS:
        snprintf(out, outsz, "%s/.gamedev/phases/02-production/systems/%s/spec.md", root, c->name);
        break;
    case GB_PROD_LVL:
        snprintf(out, outsz, "%s/.gamedev/phases/02-production/levels/%s/level.md", root, c->name);
        break;
    case GB_PROD_ART:
        snprintf(out, outsz, "%s/.gamedev/phases/02-production/art/%s/context.md", root, c->name);
        break;
    case GB_PROD_ANIM:
        snprintf(out, outsz, "%s/.gamedev/phases/02-production/animation/%s/context.md", root,
                 c->name);
        break;
    case GB_PROD_AUDIO:
        snprintf(out, outsz, "%s/.gamedev/phases/02-production/audio/%s/context.md", root, c->name);
        break;
    case GB_PROD_UI:
        snprintf(out, outsz, "%s/.gamedev/phases/02-production/ui/%s/context.md", root, c->name);
        break;
    case GB_PROD_QA:
        snprintf(out, outsz, "%s/.gamedev/phases/02-production/qa/%s", root, c->name);
        break;
    }
}

static void prod_rel_id(const gb_prod_cand_t *c, char *out, size_t outsz) {
    switch (c->kind) {
    case GB_PROD_SYS:
        snprintf(out, outsz, ".gamedev/phases/02-production/systems/%s", c->name);
        break;
    case GB_PROD_LVL:
        snprintf(out, outsz, ".gamedev/phases/02-production/levels/%s", c->name);
        break;
    case GB_PROD_ART:
        snprintf(out, outsz, ".gamedev/phases/02-production/art/%s", c->name);
        break;
    case GB_PROD_ANIM:
        snprintf(out, outsz, ".gamedev/phases/02-production/animation/%s", c->name);
        break;
    case GB_PROD_AUDIO:
        snprintf(out, outsz, ".gamedev/phases/02-production/audio/%s", c->name);
        break;
    case GB_PROD_UI:
        snprintf(out, outsz, ".gamedev/phases/02-production/ui/%s", c->name);
        break;
    case GB_PROD_QA:
        snprintf(out, outsz, ".gamedev/phases/02-production/qa/%s", c->name);
        break;
    }
}

static void fill_production(const char *root, cbm_game_board_t *out) {
    gb_prod_cand_t *list;
    int n = 0;
    int i;
    char parent[2048];
    char qa_path[2048];

    list = calloc(CBM_GAME_BOARD_LIST_MAX, sizeof(*list));
    if (!list) {
        return;
    }

    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/systems", root);
    collect_child_dirs(parent, "SYS-", GB_PROD_SYS, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/levels", root);
    collect_child_dirs(parent, "LVL-", GB_PROD_LVL, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/art", root);
    collect_child_dirs(parent, NULL, GB_PROD_ART, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/animation", root);
    collect_child_dirs(parent, NULL, GB_PROD_ANIM, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/audio", root);
    collect_child_dirs(parent, NULL, GB_PROD_AUDIO, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/ui", root);
    collect_child_dirs(parent, NULL, GB_PROD_UI, list, &n);

    snprintf(qa_path, sizeof(qa_path), "%s/.gamedev/phases/02-production/qa/playtest-log.md", root);
    if (game_is_regular_file(qa_path)) {
        prod_try_add(list, &n, "playtest-log.md", GB_PROD_QA, NULL);
    }

    if (n > 1) {
        qsort(list, (size_t)n, sizeof(list[0]), prod_cand_cmp);
    }

    for (i = 0; i < n && out->production_count < CBM_GAME_BOARD_MAX_CARDS; i++) {
        char header[2048];
        char rel[512];
        const char *owner = NULL;
        const char *track = NULL;
        cbm_game_board_card_t card;

        owner_track_for_prod(list[i].kind, &owner, &track);
        prod_header_path(root, &list[i], header, sizeof(header));
        prod_rel_id(&list[i], rel, sizeof(rel));
        fill_artifact(&card, rel, list[i].name, owner, track, "pending");
        apply_header_expand(header, &card);
        append_card(out->production, &out->production_count, &card);
    }
    free(list);
}

static void fill_artifacts(const char *root, cbm_game_board_t *out) {
    fill_fixed_files(root, ".gamedev/phases/01-preproduction", k_preprod_files,
                     sizeof(k_preprod_files) / sizeof(k_preprod_files[0]), out->preproduction,
                     &out->preproduction_count);
    fill_production(root, out);
    fill_fixed_files(root, ".gamedev/phases/03-postproduction", k_postprod_files,
                     sizeof(k_postprod_files) / sizeof(k_postprod_files[0]), out->postproduction,
                     &out->postproduction_count);
}

/* ── game_grill_* (.grill catalog + Game conversion; SDD-ADR-046, 047)
 * Copy of spec-008 index.md + epic-NNN walk. Conversion is Game-local:
 * Companion-to exact path on scanned artifacts, or slug token + roadmap
 * table NNN. Token is never fopen'd. Do not read active.json. */

enum { GAME_GRILL_MAX_PLANS = 96, GAME_GRILL_MAX_EPIC_FILES = 96, GAME_GRILL_MAX_TOKENS = 320 };

typedef struct {
    char slug[96];
    char title[256];
} game_grill_plan_ref_t;

typedef struct {
    int nnn;
    char filename[160];
} game_grill_epic_file_t;

typedef struct {
    char tokens[GAME_GRILL_MAX_TOKENS][256];
    int ntokens;
    char *roadmap;
    char *game_context;
} game_grill_conv_t;

static bool game_grill_dirent_rejected(const char *name) {
    return game_dirent_rejected(name);
}

static int game_grill_cmp_slug(const void *a, const void *b) {
    const game_grill_plan_ref_t *pa = (const game_grill_plan_ref_t *)a;
    const game_grill_plan_ref_t *pb = (const game_grill_plan_ref_t *)b;
    return strcmp(pa->slug, pb->slug);
}

static int game_grill_cmp_nnn(const void *a, const void *b) {
    const game_grill_epic_file_t *ea = (const game_grill_epic_file_t *)a;
    const game_grill_epic_file_t *eb = (const game_grill_epic_file_t *)b;
    if (ea->nnn < eb->nnn) {
        return -1;
    }
    if (ea->nnn > eb->nnn) {
        return 1;
    }
    return strcmp(ea->filename, eb->filename);
}

static bool game_grill_parse_epic_filename(const char *name, int *nnn_out) {
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

static void game_grill_trim_copy(char *dst, size_t dstsz, const char *src, size_t len) {
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

static int game_grill_split_gfm_row(const char *line, char cells[][256], int max_cells) {
    const char *p = line;
    int n = 0;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (*p != '|') {
        return 0;
    }
    cells[0][0] = '\0';
    n = 1;
    p++;
    while (*p && n < max_cells) {
        const char *start = p;
        while (*p && *p != '|' && *p != '\n' && *p != '\r') {
            p++;
        }
        game_grill_trim_copy(cells[n], 256, start, (size_t)(p - start));
        n++;
        if (*p == '|') {
            p++;
        } else {
            break;
        }
    }
    return n;
}

static bool game_grill_parse_registry_epic_cell(const char *cell, int *nnn_out) {
    const char *p = cell;
    unsigned long nnn = 0;
    int digits = 0;

    if (!p || !p[0] || !nnn_out) {
        return false;
    }
    if (strncmp(p, "epic-", 5) == 0) {
        p += 5;
    }
    if (!isdigit((unsigned char)*p)) {
        return false;
    }
    while (isdigit((unsigned char)*p)) {
        nnn = nnn * 10UL + (unsigned long)(*p - '0');
        if (nnn > (unsigned long)INT_MAX) {
            return false;
        }
        digits++;
        p++;
    }
    if (*p != '\0' || digits == 0) {
        return false;
    }
    *nnn_out = (int)nnn;
    return true;
}

static int game_grill_registry_find(const cbm_game_board_registry_row_t *rows, int n,
                                   const char *slug, int nnn) {
    int i;
    if (!rows || !slug) {
        return -1;
    }
    for (i = 0; i < n; i++) {
        if (rows[i].nnn == nnn && strcmp(rows[i].slug, slug) == 0) {
            return i;
        }
    }
    return -1;
}

void cbm_game_board_parse_epics_registry(const char *md, cbm_game_board_registry_row_t *out,
                                         int *count) {
    const char *p;

    if (!count) {
        return;
    }
    *count = 0;
    if (!out || !md || !md[0]) {
        return;
    }

    p = md;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
        char line[1024];
        char cells[8][256];
        int ncells;
        int nnn = 0;
        int slot;
        bool hide;
        size_t copy;

        if (linelen > 0 && p[linelen - 1] == '\r') {
            linelen--;
        }
        copy = linelen < sizeof(line) - 1 ? linelen : sizeof(line) - 1;
        memcpy(line, p, copy);
        line[copy] = '\0';
        ncells = game_grill_split_gfm_row(line, cells, 8);
        if (ncells >= 6 && game_grill_parse_registry_epic_cell(cells[1], &nnn) && cells[2][0] &&
            strcmp(cells[2], "native") != 0) {
            hide = (nnn != 0) && (strcmp(cells[5], "in_progress") == 0 ||
                                  strcmp(cells[5], "closed") == 0 ||
                                  strcmp(cells[5], "parked") == 0);
            slot = game_grill_registry_find(out, *count, cells[2], nnn);
            if (slot >= 0) {
                out[slot].hide = hide;
            } else if (*count < CBM_GAME_BOARD_MAX_REGISTRY) {
                memset(&out[*count], 0, sizeof(out[*count]));
                snprintf(out[*count].slug, sizeof(out[*count].slug), "%s", cells[2]);
                out[*count].nnn = nnn;
                out[*count].hide = hide;
                (*count)++;
            }
        }
        if (!eol) {
            break;
        }
        p = eol + 1;
    }
}

static bool game_grill_slug_is_header_or_sep(const char *slug) {
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
    if (p > slug && *p == '\0') {
        return true;
    }
    if (strstr(slug, "---") != NULL) {
        return true;
    }
    return false;
}

static bool game_grill_cell_is_header_or_sep(const char *cell) {
    return game_grill_slug_is_header_or_sep(cell);
}

static void game_grill_read_plan_md_title(const char *root, const char *slug, char *out,
                                          size_t outsz) {
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

static void game_grill_collect_plans(const char *root, game_grill_plan_ref_t *plans, int *nplans) {
    char index_path[1152];
    char *idx;
    game_grill_plan_ref_t unlisted[GAME_GRILL_MAX_PLANS];
    int nun = 0;
    int indexed;
    char plans_dir[1200];
    cbm_dir_t *d;

    *nplans = 0;
    snprintf(index_path, sizeof(index_path), "%s/.grill/index.md", root);
    idx = read_whole_file(index_path);
    if (idx) {
        const char *p = idx;
        while (*p && *nplans < GAME_GRILL_MAX_PLANS) {
            const char *eol = strchr(p, '\n');
            size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
            char line[1024];
            char cells[8][256];
            int ncells;
            size_t copy = linelen < sizeof(line) - 1 ? linelen : sizeof(line) - 1;
            memcpy(line, p, copy);
            line[copy] = '\0';
            ncells = game_grill_split_gfm_row(line, cells, 8);
            if (ncells >= 3 && !game_grill_slug_is_header_or_sep(cells[1]) && cells[1][0]) {
                if (!game_grill_dirent_rejected(cells[1])) {
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
            if (game_grill_dirent_rejected(ent->name)) {
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
            if (found || nun >= GAME_GRILL_MAX_PLANS) {
                continue;
            }
            memset(&unlisted[nun], 0, sizeof(unlisted[nun]));
            snprintf(unlisted[nun].slug, sizeof(unlisted[nun].slug), "%s", ent->name);
            nun++;
        }
        cbm_closedir(d);
    }
    if (nun > 1) {
        qsort(unlisted, (size_t)nun, sizeof(unlisted[0]), game_grill_cmp_slug);
    }
    {
        int i;
        for (i = 0; i < nun && *nplans < GAME_GRILL_MAX_PLANS; i++) {
            plans[(*nplans)++] = unlisted[i];
        }
    }
}

static bool game_grill_slug_char(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-';
}

static bool game_grill_has_slug_token(const char *hay, const char *slug) {
    const char *p;
    size_t slen;

    if (!hay || !slug || !slug[0]) {
        return false;
    }
    slen = strlen(slug);
    p = hay;
    while ((p = strstr(p, slug)) != NULL) {
        bool left_ok = (p == hay) || !game_grill_slug_char((unsigned char)p[-1]);
        bool right_ok = !game_grill_slug_char((unsigned char)p[slen]);
        if (left_ok && right_ok) {
            return true;
        }
        p += slen;
    }
    return false;
}

static bool game_grill_roadmap_has_nnn_cell(const char *roadmap, int nnn) {
    char want_nnn[16];
    char want_epic[24];
    const char *p;

    if (!roadmap) {
        return false;
    }
    snprintf(want_nnn, sizeof(want_nnn), "%03d", nnn);
    snprintf(want_epic, sizeof(want_epic), "epic-%03d", nnn);
    p = roadmap;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
        char line[1024];
        char cells[8][256];
        int ncells;
        int i;
        bool data = false;
        size_t copy = linelen < sizeof(line) - 1 ? linelen : sizeof(line) - 1;
        memcpy(line, p, copy);
        line[copy] = '\0';
        ncells = game_grill_split_gfm_row(line, cells, 8);
        for (i = 0; i < ncells; i++) {
            if (cells[i][0] && !game_grill_cell_is_header_or_sep(cells[i])) {
                data = true;
                break;
            }
        }
        if (data) {
            for (i = 0; i < ncells; i++) {
                if (strcmp(cells[i], want_nnn) == 0 || strcmp(cells[i], want_epic) == 0) {
                    return true;
                }
            }
        }
        if (!eol) {
            break;
        }
        p = eol + 1;
    }
    return false;
}

static void game_grill_extract_companion(const char *buf, char *out, size_t outsz) {
    const char *cp;

    out[0] = '\0';
    if (!buf || outsz == 0) {
        return;
    }
    cp = buf;
    while (*cp && !out[0]) {
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
                    if (n >= outsz) {
                        n = outsz - 1;
                    }
                    memcpy(out, tok, n);
                    out[n] = '\0';
                }
            }
        }
        if (!eol) {
            break;
        }
        cp = eol + 1;
    }
}

static void game_grill_add_token(game_grill_conv_t *conv, const char *path) {
    char *buf;
    char tok[256];

    if (!conv || conv->ntokens >= GAME_GRILL_MAX_TOKENS) {
        return;
    }
    if (!game_is_regular_file(path)) {
        return;
    }
    buf = read_whole_file(path);
    if (!buf) {
        return;
    }
    game_grill_extract_companion(buf, tok, sizeof(tok));
    free(buf);
    if (!tok[0]) {
        return;
    }
    snprintf(conv->tokens[conv->ntokens], sizeof(conv->tokens[0]), "%s", tok);
    conv->ntokens++;
}

static void game_grill_scan_listed_headers(const char *root, game_grill_conv_t *conv) {
    gb_prod_cand_t *list;
    int n = 0;
    int i;
    char parent[2048];

    list = calloc(CBM_GAME_BOARD_LIST_MAX, sizeof(*list));
    if (!list) {
        return;
    }
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/systems", root);
    collect_child_dirs(parent, "SYS-", GB_PROD_SYS, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/levels", root);
    collect_child_dirs(parent, "LVL-", GB_PROD_LVL, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/art", root);
    collect_child_dirs(parent, NULL, GB_PROD_ART, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/animation", root);
    collect_child_dirs(parent, NULL, GB_PROD_ANIM, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/audio", root);
    collect_child_dirs(parent, NULL, GB_PROD_AUDIO, list, &n);
    snprintf(parent, sizeof(parent), "%s/.gamedev/phases/02-production/ui", root);
    collect_child_dirs(parent, NULL, GB_PROD_UI, list, &n);
    for (i = 0; i < n; i++) {
        char header[2048];
        prod_header_path(root, &list[i], header, sizeof(header));
        game_grill_add_token(conv, header);
    }
    free(list);
}

static void game_grill_load_conv(const char *root, game_grill_conv_t *conv) {
    char path[1400];
    size_t i;

    memset(conv, 0, sizeof(*conv));
    for (i = 0; i < sizeof(k_preprod_files) / sizeof(k_preprod_files[0]); i++) {
        snprintf(path, sizeof(path), "%s/.gamedev/phases/01-preproduction/%s", root,
                 k_preprod_files[i]);
        game_grill_add_token(conv, path);
    }
    for (i = 0; i < sizeof(k_postprod_files) / sizeof(k_postprod_files[0]); i++) {
        snprintf(path, sizeof(path), "%s/.gamedev/phases/03-postproduction/%s", root,
                 k_postprod_files[i]);
        game_grill_add_token(conv, path);
    }
    snprintf(path, sizeof(path), "%s/.gamedev/phases/02-production/qa/playtest-log.md", root);
    game_grill_add_token(conv, path);
    game_grill_scan_listed_headers(root, conv);

    snprintf(path, sizeof(path), "%s/.gamedev/roadmap.md", root);
    conv->roadmap = read_whole_file(path);
    snprintf(path, sizeof(path), "%s/.gamedev/game_context.md", root);
    conv->game_context = read_whole_file(path);
}

static void game_grill_free_conv(game_grill_conv_t *conv) {
    if (!conv) {
        return;
    }
    free(conv->roadmap);
    free(conv->game_context);
    conv->roadmap = NULL;
    conv->game_context = NULL;
}

static bool game_grill_epic_converted(const char *epic_id, const char *slug, int nnn,
                                     const game_grill_conv_t *conv) {
    int i;
    bool slug_hit;

    if (!epic_id || !epic_id[0] || !conv) {
        return false;
    }
    for (i = 0; i < conv->ntokens; i++) {
        if (strcmp(conv->tokens[i], epic_id) == 0) {
            return true;
        }
    }
    slug_hit = game_grill_has_slug_token(conv->roadmap, slug) ||
               game_grill_has_slug_token(conv->game_context, slug);
    if (!slug_hit) {
        return false;
    }
    return game_grill_roadmap_has_nnn_cell(conv->roadmap, nnn);
}

static bool game_grill_registry_hides(const cbm_game_board_registry_row_t *reg, int nreg,
                                     const char *slug, int nnn) {
    int slot = game_grill_registry_find(reg, nreg, slug, nnn);
    if (slot < 0) {
        return false;
    }
    return reg[slot].hide;
}

static void game_grill_append_epic(const char *root, const char *slug, const char *filename,
                                  const char *plan_title, const game_grill_conv_t *conv,
                                  const cbm_game_board_registry_row_t *reg, int nreg,
                                  bool registry_present, cbm_game_board_t *out) {
    char rel_id[256];
    char path[1600];
    char *buf;
    const char *p;
    int n;
    int nnn = 0;
    cbm_game_board_card_t *c;

    if (out->inbox_count >= CBM_GAME_BOARD_MAX_CARDS) {
        return;
    }
    if (!game_grill_parse_epic_filename(filename, &nnn)) {
        return;
    }
    n = snprintf(rel_id, sizeof(rel_id), ".grill/plans/%s/epics/%s", slug, filename);
    if (n < 0 || n >= (int)sizeof(rel_id)) {
        return;
    }
    if (registry_present) {
        if (game_grill_registry_hides(reg, nreg, slug, nnn)) {
            return;
        }
    } else if (game_grill_epic_converted(rel_id, slug, nnn, conv)) {
        return;
    }
    snprintf(path, sizeof(path), "%s/%s", root, rel_id);
    if (cbm_is_dir(path)) {
        return;
    }
    buf = read_whole_file(path);
    if (!buf) {
        return;
    }

    c = &out->inbox[out->inbox_count];
    memset(c, 0, sizeof(*c));
    snprintf(c->kind, sizeof(c->kind), "epic");
    snprintf(c->id, sizeof(c->id), "%s", rel_id);
    snprintf(c->plan_title, sizeof(c->plan_title), "%s", plan_title);
    snprintf(c->continue_cmd, sizeof(c->continue_cmd), "%s", k_continue_cmd);
    /* track / work_state stay empty → JSON null. owner stays "". */

    p = buf;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
        char line[1024];
        size_t copy = linelen < sizeof(line) - 1 ? linelen : sizeof(line) - 1;
        memcpy(line, p, copy);
        line[copy] = '\0';
        kv_extract_colon_line(line, "name", c->title, sizeof(c->title));
        kv_extract_colon_line(line, "summary", c->summary, sizeof(c->summary));
        if (!eol) {
            break;
        }
        p = eol + 1;
    }
    free(buf);
    out->inbox_count++;
}

static void game_grill_fill_inbox(const char *root, cbm_game_board_t *out) {
    char grill_dir[1200];
    char reg_path[1400];
    game_grill_plan_ref_t plans[GAME_GRILL_MAX_PLANS];
    int nplans = 0;
    int pi;
    game_grill_conv_t *conv = NULL;
    cbm_game_board_registry_row_t reg[CBM_GAME_BOARD_MAX_REGISTRY];
    int nreg = 0;
    bool registry_present;

    snprintf(grill_dir, sizeof(grill_dir), "%s/.grill", root);
    if (!cbm_is_dir(grill_dir)) {
        return;
    }

    snprintf(reg_path, sizeof(reg_path), "%s/.gamedev/epics_registry.md", root);
    registry_present = game_is_regular_file(reg_path);
    memset(reg, 0, sizeof(reg));
    if (registry_present) {
        char *md = read_whole_file(reg_path);
        if (md) {
            cbm_game_board_parse_epics_registry(md, reg, &nreg);
            free(md);
        }
    } else {
        conv = calloc(1, sizeof(*conv));
        if (!conv) {
            return;
        }
        game_grill_load_conv(root, conv);
    }

    game_grill_collect_plans(root, plans, &nplans);
    for (pi = 0; pi < nplans; pi++) {
        char plan_title[256];
        char epics_dir[1400];
        cbm_dir_t *d;
        game_grill_epic_file_t files[GAME_GRILL_MAX_EPIC_FILES];
        int nfiles = 0;
        int fi;

        if (out->inbox_count >= CBM_GAME_BOARD_MAX_CARDS) {
            break;
        }
        if (plans[pi].title[0]) {
            snprintf(plan_title, sizeof(plan_title), "%s", plans[pi].title);
        } else {
            game_grill_read_plan_md_title(root, plans[pi].slug, plan_title, sizeof(plan_title));
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
            while ((ent = cbm_readdir(d)) != NULL && nfiles < GAME_GRILL_MAX_EPIC_FILES) {
                int nnn = 0;
                if (game_grill_dirent_rejected(ent->name)) {
                    continue;
                }
                if (!game_grill_parse_epic_filename(ent->name, &nnn)) {
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
            qsort(files, (size_t)nfiles, sizeof(files[0]), game_grill_cmp_nnn);
        }
        for (fi = 0; fi < nfiles; fi++) {
            if (out->inbox_count >= CBM_GAME_BOARD_MAX_CARDS) {
                break;
            }
            game_grill_append_epic(root, plans[pi].slug, files[fi].filename, plan_title, conv, reg,
                                   nreg, registry_present, out);
        }
    }
    if (conv) {
        game_grill_free_conv(conv);
        free(conv);
    }
}

enum { GAME_DEBT_SEEN_CAP = 256 };

static int game_debt_is_tag_first(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

static int game_debt_is_tag_rest(unsigned char c) {
    return game_debt_is_tag_first(c) || c == '_' || c == '-';
}

static const char *game_debt_find_token(const char *line, size_t linelen, size_t *id_len) {
    size_t i;

    if (!line || !id_len || linelen < 6) {
        return NULL;
    }
    for (i = 0; i + 5 < linelen; i++) {
        size_t j;

        if (memcmp(line + i, "debt:", 5) != 0) {
            continue;
        }
        if (!game_debt_is_tag_first((unsigned char)line[i + 5])) {
            continue;
        }
        j = i + 6;
        while (j < linelen && game_debt_is_tag_rest((unsigned char)line[j])) {
            j++;
        }
        *id_len = j - i;
        return line + i;
    }
    return NULL;
}

static int game_debt_line_is_atx(const char *line, size_t linelen) {
    size_t i = 0;

    while (i < linelen && i < 3 && line[i] == ' ') {
        i++;
    }
    return (linelen >= i + 2) && line[i] == '#' && line[i + 1] == '#';
}

static void game_debt_rtrim(char *s) {
    size_t n;

    if (!s) {
        return;
    }
    n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r')) {
        s[--n] = '\0';
    }
}

static void game_debt_ltrim_inplace(char *s) {
    char *p;

    if (!s) {
        return;
    }
    p = s;
    while (*p == ' ' || *p == '\t' || *p == '\r') {
        p++;
    }
    if (p != s) {
        memmove(s, p, strlen(p) + 1);
    }
}

static void game_debt_trim(char *s) {
    game_debt_ltrim_inplace(s);
    game_debt_rtrim(s);
}

static void game_debt_title_from_after(const char *after, size_t after_len, char *out, size_t outsz) {
    char tmp[1024];
    size_t n;
    char *em;
    char *ascii;
    char *cut;

    if (!out || outsz == 0) {
        return;
    }
    out[0] = '\0';
    n = after && after_len > 0 ? after_len : 0;
    if (n >= sizeof(tmp)) {
        n = sizeof(tmp) - 1;
    }
    if (n > 0) {
        memcpy(tmp, after, n);
    }
    tmp[n] = '\0';
    game_debt_trim(tmp);

    n = strlen(tmp);
    if (n > 0 && tmp[n - 1] == ']') {
        tmp[n - 1] = '\0';
    }
    n = strlen(tmp);
    if (n >= 3 && memcmp(tmp + n - 3, "-->", 3) == 0) {
        tmp[n - 3] = '\0';
    }
    game_debt_trim(tmp);

    em = strstr(tmp, " \xe2\x80\x94 ");
    ascii = strstr(tmp, " -- ");
    cut = NULL;
    if (em && ascii) {
        cut = em < ascii ? em : ascii;
    } else if (em) {
        cut = em;
    } else if (ascii) {
        cut = ascii;
    }
    if (cut) {
        *cut = '\0';
    }
    game_debt_trim(tmp);
    snprintf(out, outsz, "%s", tmp);
}

static int game_debt_id_seen(char seen[][96], int nseen, const char *id) {
    int i;

    if (!seen || !id) {
        return 0;
    }
    for (i = 0; i < nseen; i++) {
        if (strcmp(seen[i], id) == 0) {
            return 1;
        }
    }
    return 0;
}

static int game_debt_mem_contains(const char *s, size_t n, const char *needle, size_t nlen) {
    size_t i;

    if (!s || !needle || n < nlen) {
        return 0;
    }
    for (i = 0; i + nlen <= n; i++) {
        if (memcmp(s + i, needle, nlen) == 0) {
            return 1;
        }
    }
    return 0;
}

void cbm_game_board_parse_backlog_debt(const char *md, cbm_game_board_debt_t *out, int *count) {
    char seen[GAME_DEBT_SEEN_CAP][96];
    int nseen = 0;
    const char *p;

    if (!count) {
        return;
    }
    *count = 0;
    if (!out || !md || !md[0]) {
        return;
    }

    p = md;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
        size_t id_len = 0;
        const char *tok;
        const char *next;
        const char *q;
        const char *body_end;
        char id[96];
        char title[256];
        int closed;

        if (linelen > 0 && p[linelen - 1] == '\r') {
            linelen--;
        }
        tok = game_debt_find_token(p, linelen, &id_len);
        if (tok) {
            size_t copy = id_len < sizeof(id) - 1 ? id_len : sizeof(id) - 1;

            memcpy(id, tok, copy);
            id[copy] = '\0';
            game_debt_title_from_after(tok + id_len, (size_t)((p + linelen) - (tok + id_len)), title,
                                       sizeof(title));

            next = eol ? eol + 1 : p + strlen(p);
            body_end = next;
            q = next;
            while (*q) {
                const char *qeol = strchr(q, '\n');
                size_t qlen = qeol ? (size_t)(qeol - q) : strlen(q);
                size_t qid_len = 0;

                if (qlen > 0 && q[qlen - 1] == '\r') {
                    qlen--;
                }
                if (game_debt_find_token(q, qlen, &qid_len) || game_debt_line_is_atx(q, qlen)) {
                    body_end = q;
                    break;
                }
                q = qeol ? qeol + 1 : q + strlen(q);
                body_end = q;
            }
            if (!game_debt_id_seen(seen, nseen, id)) {
                if (nseen < GAME_DEBT_SEEN_CAP) {
                    snprintf(seen[nseen], sizeof(seen[nseen]), "%s", id);
                    nseen++;
                }
                closed = game_debt_mem_contains(p, (size_t)(body_end - p), "resolved-by", 11);
                if (!closed && *count < CBM_GAME_BOARD_MAX_DEBT) {
                    memset(&out[*count], 0, sizeof(out[*count]));
                    snprintf(out[*count].id, sizeof(out[*count].id), "%s", id);
                    snprintf(out[*count].title, sizeof(out[*count].title), "%s", title);
                    (*count)++;
                }
            }
        }
        if (!eol) {
            break;
        }
        p = eol + 1;
    }
}

static void game_debt_fill(const char *root_path, cbm_game_board_t *out) {
    char path[1152];
    char *md;

    out->debt_count = 0;
    if (!root_path || !root_path[0]) {
        return;
    }
    snprintf(path, sizeof(path), "%s/.gamedev/backlog.md", root_path);
    if (!game_is_regular_file(path)) {
        return;
    }
    md = read_whole_file(path);
    if (!md) {
        return;
    }
    cbm_game_board_parse_backlog_debt(md, out->debt, &out->debt_count);
    free(md);
}

void cbm_game_board_read(const char *root_path, cbm_game_board_t *out) {
    char path[1152];
    char *buf;

    if (!out) {
        return;
    }
    memset(out, 0, sizeof(*out));
    if (!root_path || !root_path[0]) {
        return;
    }

    out->gamedev_skill_present = cbm_spec_board_gamedev_skill_present(root_path);
    if (!out->gamedev_skill_present) {
        return;
    }

    snprintf(out->continue_cmd, sizeof(out->continue_cmd), "%s", k_continue_cmd);
    snprintf(path, sizeof(path), "%s/.gamedev/state.md", root_path);
    buf = read_whole_file(path);
    if (buf) {
        parse_state_md(buf, out);
        free(buf);
    }
    fill_artifacts(root_path, out);
    game_grill_fill_inbox(root_path, out);
    overlay_blocked(out);
    game_debt_fill(root_path, out);
}

static bool game_json_append(char **buf, size_t *cap, int *pos, const char *fmt, ...) {
    va_list ap;
    for (;;) {
        int need;
        size_t new_cap;
        char *nb;

        va_start(ap, fmt);
        need = vsnprintf(*buf + *pos, *cap - (size_t)*pos, fmt, ap);
        va_end(ap);
        if (need < 0) {
            return false;
        }
        if ((size_t)(*pos + need) < *cap) {
            *pos += need;
            return true;
        }
        new_cap = *cap * 2;
        if (new_cap < *cap) {
            return false;
        }
        nb = realloc(*buf, new_cap);
        if (!nb) {
            return false;
        }
        *buf = nb;
        *cap = new_cap;
    }
}

static bool game_json_emit_card(char **buf, size_t *cap, int *pos, const cbm_game_board_card_t *c,
                               int index) {
    char esc_kind[32];
    char esc_id[384];
    char esc_title[512];
    char esc_track[16];
    char esc_ws[32];
    char esc_owner[96];
    char esc_cont[192];
    char esc_sum[1024];
    char esc_plan[512];
    char esc_blurb[1024];
    char esc_inputs[1024];
    char esc_last[1024];
    char esc_open[1024];
    char esc_recent[1024];
    char esc_bby[1024];
    int ti;

    cbm_json_escape(esc_kind, (int)sizeof(esc_kind), c->kind);
    cbm_json_escape(esc_id, (int)sizeof(esc_id), c->id);
    cbm_json_escape(esc_title, (int)sizeof(esc_title), c->title);
    cbm_json_escape(esc_track, (int)sizeof(esc_track), c->track);
    cbm_json_escape(esc_ws, (int)sizeof(esc_ws), c->work_state);
    cbm_json_escape(esc_owner, (int)sizeof(esc_owner), c->owner);
    cbm_json_escape(esc_cont, (int)sizeof(esc_cont), c->continue_cmd);
    cbm_json_escape(esc_sum, (int)sizeof(esc_sum), c->summary);
    cbm_json_escape(esc_plan, (int)sizeof(esc_plan), c->plan_title);
    cbm_json_escape(esc_blurb, (int)sizeof(esc_blurb), c->blurb);
    cbm_json_escape(esc_inputs, (int)sizeof(esc_inputs), c->inputs);
    cbm_json_escape(esc_last, (int)sizeof(esc_last), c->last_decision);
    cbm_json_escape(esc_open, (int)sizeof(esc_open), c->open);
    cbm_json_escape(esc_recent, (int)sizeof(esc_recent), c->recent);
    cbm_json_escape(esc_bby, (int)sizeof(esc_bby), c->blocked_by);

    if (index > 0 && !game_json_append(buf, cap, pos, ",")) {
        return false;
    }
    if (!game_json_append(buf, cap, pos, "{\"kind\":\"%s\",\"id\":\"%s\",\"title\":\"%s\",",
                          esc_kind, esc_id, esc_title)) {
        return false;
    }
    if (c->track[0]) {
        if (!game_json_append(buf, cap, pos, "\"track\":\"%s\",", esc_track)) {
            return false;
        }
    } else if (!game_json_append(buf, cap, pos, "\"track\":null,")) {
        return false;
    }
    if (c->work_state[0]) {
        if (!game_json_append(buf, cap, pos, "\"work_state\":\"%s\",", esc_ws)) {
            return false;
        }
    } else if (!game_json_append(buf, cap, pos, "\"work_state\":null,")) {
        return false;
    }
    if (!game_json_append(buf, cap, pos,
                          "\"owner\":\"%s\",\"continue\":\"%s\",\"summary\":\"%s\","
                          "\"plan_title\":\"%s\",\"blurb\":\"%s\",\"tasks\":[",
                          esc_owner, esc_cont, esc_sum, esc_plan, esc_blurb)) {
        return false;
    }
    for (ti = 0; ti < c->task_count; ti++) {
        char esc_tname[512];

        cbm_json_escape(esc_tname, (int)sizeof(esc_tname), c->tasks[ti].name);
        if (ti > 0 && !game_json_append(buf, cap, pos, ",")) {
            return false;
        }
        if (!game_json_append(buf, cap, pos, "{\"number\":%d,\"name\":\"%s\",\"done\":%s}",
                              c->tasks[ti].number, esc_tname,
                              c->tasks[ti].done ? "true" : "false")) {
            return false;
        }
    }
    if (!game_json_append(buf, cap, pos,
                          "],\"inputs\":\"%s\",\"last_decision\":\"%s\",\"open\":\"%s\","
                          "\"recent\":\"%s\",",
                          esc_inputs, esc_last, esc_open, esc_recent)) {
        return false;
    }
    if (c->blocked_by[0]) {
        if (!game_json_append(buf, cap, pos, "\"blocked_by\":\"%s\",", esc_bby)) {
            return false;
        }
    } else if (!game_json_append(buf, cap, pos, "\"blocked_by\":null,")) {
        return false;
    }
    return game_json_append(buf, cap, pos, "\"archived\":%s}", c->archived ? "true" : "false");
}

static bool game_json_emit_array(char **buf, size_t *cap, int *pos, const cbm_game_board_card_t *arr,
                                int count) {
    int i;
    for (i = 0; i < count; i++) {
        if (!game_json_emit_card(buf, cap, pos, &arr[i], i)) {
            return false;
        }
    }
    return true;
}

static bool game_json_emit_blocked(char **buf, size_t *cap, int *pos, const cbm_game_board_t *b) {
    int i;

    for (i = 0; i < b->blocked_count; i++) {
        char esc_owner[96];
        char esc_task[512];
        char esc_by[1024];

        cbm_json_escape(esc_owner, (int)sizeof(esc_owner), b->blocked[i].owner);
        cbm_json_escape(esc_task, (int)sizeof(esc_task), b->blocked[i].task);
        cbm_json_escape(esc_by, (int)sizeof(esc_by), b->blocked[i].blocked_by);
        if (i > 0 && !game_json_append(buf, cap, pos, ",")) {
            return false;
        }
        if (!game_json_append(buf, cap, pos,
                              "{\"owner\":\"%s\",\"task\":\"%s\",\"blocked_by\":\"%s\"}",
                              esc_owner, esc_task, esc_by)) {
            return false;
        }
    }
    return true;
}

static bool game_json_emit_debt(char **buf, size_t *cap, int *pos, const cbm_game_board_t *b) {
    int i;

    for (i = 0; i < b->debt_count; i++) {
        char esc_id[192];
        char esc_title[640];

        cbm_json_escape(esc_id, (int)sizeof(esc_id), b->debt[i].id);
        cbm_json_escape(esc_title, (int)sizeof(esc_title), b->debt[i].title);
        if (i > 0 && !game_json_append(buf, cap, pos, ",")) {
            return false;
        }
        if (!game_json_append(buf, cap, pos, "{\"id\":\"%s\",\"title\":\"%s\"}", esc_id,
                              esc_title)) {
            return false;
        }
    }
    return true;
}

char *cbm_game_board_to_json(const cbm_game_board_t *b) {
    char esc_phase[64];
    char esc_focus[4096];
    char esc_cont[128];
    char phase_json[80];
    char focus_json[4112];
    size_t cap = 8192;
    char *buf;
    int pos = 0;

    if (!b) {
        return NULL;
    }
    cbm_json_escape(esc_phase, (int)sizeof(esc_phase), b->phase);
    cbm_json_escape(esc_focus, (int)sizeof(esc_focus), b->focus);
    cbm_json_escape(esc_cont, (int)sizeof(esc_cont), b->continue_cmd);
    if (b->phase[0]) {
        snprintf(phase_json, sizeof(phase_json), "\"%s\"", esc_phase);
    } else {
        snprintf(phase_json, sizeof(phase_json), "null");
    }
    if (b->focus[0]) {
        snprintf(focus_json, sizeof(focus_json), "\"%s\"", esc_focus);
    } else {
        snprintf(focus_json, sizeof(focus_json), "null");
    }

    buf = malloc(cap);
    if (!buf) {
        return NULL;
    }

#define APP(...)                                                       \
    if (!game_json_append(&buf, &cap, &pos, __VA_ARGS__)) {             \
        free(buf);                                                     \
        return NULL;                                                   \
    }

    APP("{\"gamedev_skill_present\":%s,\"phase\":%s,\"focus\":%s,\"continue\":\"%s\",\"blocked\":[",
        b->gamedev_skill_present ? "true" : "false", phase_json, focus_json, esc_cont);
    if (!game_json_emit_blocked(&buf, &cap, &pos, b)) {
        free(buf);
        return NULL;
    }
    APP("],\"inbox\":[");
    if (!game_json_emit_array(&buf, &cap, &pos, b->inbox, b->inbox_count)) {
        free(buf);
        return NULL;
    }
    APP("],\"preproduction\":[");
    if (!game_json_emit_array(&buf, &cap, &pos, b->preproduction, b->preproduction_count)) {
        free(buf);
        return NULL;
    }
    APP("],\"production\":[");
    if (!game_json_emit_array(&buf, &cap, &pos, b->production, b->production_count)) {
        free(buf);
        return NULL;
    }
    APP("],\"postproduction\":[");
    if (!game_json_emit_array(&buf, &cap, &pos, b->postproduction, b->postproduction_count)) {
        free(buf);
        return NULL;
    }
    APP("],\"debt\":[");
    if (!game_json_emit_debt(&buf, &cap, &pos, b)) {
        free(buf);
        return NULL;
    }
    APP("]}");
#undef APP
    return buf;
}

