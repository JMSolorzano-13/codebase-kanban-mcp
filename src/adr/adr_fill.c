/**
 * @sdd-task: Task #1 - XOR trio select + gamedev relatives
 * @sdd-spec: specs/spec-013-r9w-adr-fill-gamedev-trio/spec.md
 * @sdd-decision: SDD-ADR-058 XOR; SDD-ADR-059 local is-dir; SDD-ADR-060 NULL=neither dir; SDD-ADR-061 no ALWAYS_SKIP
 * @sdd-why: XOR trio select; pass relatives into build so gamedev and sdd paths cannot mix
 * @human-debug: NULL = neither skill dir. Missing H1 = that selected-trio path absent/empty/unreadable
 * (no sdd fallback). File-at-path .gamedev is not present — sdd fill applies if .sdd-skill/ is a dir
 */
#include "adr/adr_fill.h"

#include "foundation/compat_fs.h"
#include "foundation/platform.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* XOR select picks one of these two trios. Never DEV_LOG, GDD, TECH_DEBT,
 * constitution, human/, specs/, or history/. */
static const char *const CBM_ADR_REL_PURPOSE = ".sdd-skill/context_ai.md";
static const char *const CBM_ADR_REL_STACK = ".sdd-skill/baseline/TECH_STACK.md";
static const char *const CBM_ADR_REL_DECISIONS = ".sdd-skill/baseline/ARCHITECTURE_ADR.md";
static const char *const CBM_ADR_GAME_REL_PURPOSE = ".gamedev/game_context.md";
static const char *const CBM_ADR_GAME_REL_STACK = ".gamedev/baseline/TECH_STACK.md";
static const char *const CBM_ADR_GAME_REL_DECISIONS = ".gamedev/baseline/ARCHITECTURE_ADR.md";

typedef struct {
    const char *purpose;
    const char *stack;
    const char *decisions;
} adr_trio_rels_t;

static const char *const CBM_ADR_H1_PURPOSE = "# Purpose";
static const char *const CBM_ADR_H1_STACK = "# Stack";
static const char *const CBM_ADR_H1_DECISIONS = "# Decisions";

enum {
    CBM_ADR_UTF8_BOM_LEN = 3,
    CBM_ADR_BOM_0 = 0xEF,
    CBM_ADR_BOM_1 = 0xBB,
    CBM_ADR_BOM_2 = 0xBF,
    CBM_ADR_PATH_MAX = CBM_SZ_4K,
    CBM_ADR_GEN_BUF = CBM_SZ_8K,
};

static bool adr_join(const char *root, const char *rel, char *out, size_t out_sz) {
    int n;

    if (!root || !root[0] || !rel || !out || out_sz == 0) {
        return false;
    }
    n = snprintf(out, out_sz, "%s/%s", root, rel);
    return n > 0 && (size_t)n < out_sz;
}

static bool adr_dir_present(const char *root_path, const char *rel) {
    char path[CBM_ADR_PATH_MAX];

    if (!adr_join(root_path, rel, path, sizeof(path))) {
        return false;
    }
    return cbm_is_dir(path);
}

/* XOR: .gamedev/ directory wins (gamedev relatives only). Else .sdd-skill/.
 * File-at-path is not present. Relatives never mixed. */
static bool adr_select_trio(const char *root_path, adr_trio_rels_t *out) {
    if (!out) {
        return false;
    }
    if (adr_dir_present(root_path, ".gamedev")) {
        out->purpose = CBM_ADR_GAME_REL_PURPOSE;
        out->stack = CBM_ADR_GAME_REL_STACK;
        out->decisions = CBM_ADR_GAME_REL_DECISIONS;
        return true;
    }
    if (adr_dir_present(root_path, ".sdd-skill")) {
        out->purpose = CBM_ADR_REL_PURPOSE;
        out->stack = CBM_ADR_REL_STACK;
        out->decisions = CBM_ADR_REL_DECISIONS;
        return true;
    }
    return false;
}

/* Regular file + successful open/read. Missing, directory-at-path, or open/read
 * failure → NULL (omit extract and its H1). */
static char *adr_read_extract(const char *path) {
    cbm_path_info_t info;
    FILE *f;
    char *raw;
    size_t n;
    int err;
    unsigned char *u;

    if (!path || cbm_path_info_utf8(path, &info) != 0 || !info.is_regular) {
        return NULL;
    }
    f = cbm_fopen(path, "rb");
    if (!f) {
        return NULL;
    }
    raw = malloc((size_t)CBM_SZ_64K + 1);
    if (!raw) {
        fclose(f);
        return NULL;
    }
    n = fread(raw, 1, (size_t)CBM_SZ_64K, f);
    err = ferror(f);
    fclose(f);
    if (err) {
        free(raw);
        return NULL;
    }
    u = (unsigned char *)raw;
    if (n >= (size_t)CBM_ADR_UTF8_BOM_LEN && u[0] == (unsigned char)CBM_ADR_BOM_0 &&
        u[1] == (unsigned char)CBM_ADR_BOM_1 && u[2] == (unsigned char)CBM_ADR_BOM_2) {
        n -= (size_t)CBM_ADR_UTF8_BOM_LEN;
        memmove(raw, raw + CBM_ADR_UTF8_BOM_LEN, n);
    }
    if (n > (size_t)CBM_ADR_EXTRACT_MAX) {
        n = (size_t)CBM_ADR_EXTRACT_MAX;
        while (n > 0 && raw[n - 1] != '\n') {
            n--;
        }
        if (n == 0) {
            n = (size_t)CBM_ADR_EXTRACT_MAX;
        }
    }
    raw[n] = '\0';
    if (n == 0) {
        free(raw);
        return NULL;
    }
    return raw;
}

static void adr_append(char *buf, size_t cap, size_t *len, const char *s) {
    size_t n;

    if (!buf || !len || !s || cap == 0) {
        return;
    }
    n = strlen(s);
    if (*len + n >= cap) {
        return;
    }
    memcpy(buf + *len, s, n);
    *len += n;
    buf[*len] = '\0';
}

static void adr_append_section(char *buf, size_t cap, size_t *len, const char *h1,
                              const char *extract) {
    if (!extract || !extract[0]) {
        return;
    }
    if (*len > 0) {
        adr_append(buf, cap, len, "\n");
    }
    adr_append(buf, cap, len, h1);
    adr_append(buf, cap, len, "\n");
    adr_append(buf, cap, len, extract);
    if (extract[strlen(extract) - 1] != '\n') {
        adr_append(buf, cap, len, "\n");
    }
}

static void adr_build_generated(const char *root_path, const adr_trio_rels_t *trio, char *buf,
                               size_t cap) {
    char path[CBM_ADR_PATH_MAX];
    char *extract;
    size_t len = 0;

    if (!buf || cap == 0 || !trio) {
        return;
    }
    buf[0] = '\0';
    if (adr_join(root_path, trio->purpose, path, sizeof(path))) {
        extract = adr_read_extract(path);
        adr_append_section(buf, cap, &len, CBM_ADR_H1_PURPOSE, extract);
        free(extract);
    }
    if (adr_join(root_path, trio->stack, path, sizeof(path))) {
        extract = adr_read_extract(path);
        adr_append_section(buf, cap, &len, CBM_ADR_H1_STACK, extract);
        free(extract);
    }
    if (adr_join(root_path, trio->decisions, path, sizeof(path))) {
        extract = adr_read_extract(path);
        adr_append_section(buf, cap, &len, CBM_ADR_H1_DECISIONS, extract);
        free(extract);
    }
}

static bool adr_manual_span(const char *existing, const char **out, size_t *out_len) {
    const char *start;
    const char *end;
    size_t start_len;

    if (!existing) {
        *out = "";
        *out_len = 0;
        return false;
    }
    start = strstr(existing, CBM_ADR_MANUAL_START);
    if (!start) {
        *out = existing;
        *out_len = strlen(existing);
        return false;
    }
    start_len = strlen(CBM_ADR_MANUAL_START);
    start += start_len;
    end = strstr(start, CBM_ADR_MANUAL_END);
    if (!end) {
        *out = existing;
        *out_len = strlen(existing);
        return false;
    }
    *out = start;
    *out_len = (size_t)(end - start);
    return true;
}

static void adr_copy(char **cursor, const char *s) {
    size_t n = strlen(s);
    memcpy(*cursor, s, n);
    *cursor += n;
}

static char *adr_splice(const char *generated_inner, const char *manual, size_t manual_len) {
    const char *gen = generated_inner ? generated_inner : "";
    size_t gen_len = strlen(gen);
    size_t extra_nl = (gen_len > 0 && gen[gen_len - 1] != '\n') ? 1 : 0;
    size_t cap = strlen(CBM_ADR_GENERATED_START) + 1 + gen_len + extra_nl +
                 strlen(CBM_ADR_GENERATED_END) + 1 + strlen(CBM_ADR_MANUAL_START) + manual_len +
                 strlen(CBM_ADR_MANUAL_END) + 1;
    char *out;
    char *p;

    out = malloc(cap);
    if (!out) {
        return NULL;
    }
    p = out;
    adr_copy(&p, CBM_ADR_GENERATED_START);
    *p++ = '\n';
    if (gen_len > 0) {
        memcpy(p, gen, gen_len);
        p += gen_len;
        if (extra_nl) {
            *p++ = '\n';
        }
    }
    adr_copy(&p, CBM_ADR_GENERATED_END);
    *p++ = '\n';
    adr_copy(&p, CBM_ADR_MANUAL_START);
    if (manual_len > 0 && manual) {
        memcpy(p, manual, manual_len);
        p += manual_len;
    }
    adr_copy(&p, CBM_ADR_MANUAL_END);
    *p = '\0';
    return out;
}

char *cbm_adr_fill_document(const char *root_path, const char *existing) {
    char generated[CBM_ADR_GEN_BUF];
    const char *manual;
    size_t manual_len;
    adr_trio_rels_t trio;

    if (!adr_select_trio(root_path, &trio)) {
        return NULL;
    }
    adr_build_generated(root_path, &trio, generated, sizeof(generated));
    (void)adr_manual_span(existing, &manual, &manual_len);
    return adr_splice(generated, manual, manual_len);
}
