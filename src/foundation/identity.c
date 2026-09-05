/**
 * @sdd-task: Task #1 - Identity catalog + list_projects fields
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-016 - catalog emits stored root_path plus canonical_root
 * @sdd-why: list and admit must agree on Path equality without a JS realpath
 * @human-debug: If two paths do not compare equal → one failed realpath and fell back to a
 * different stored string
 */
#include "foundation/identity.h"

#include "foundation/compat_fs.h"

#include <stdio.h>
#include <string.h>

static const char *id_or_empty(const char *s) {
    return s ? s : "";
}

void cbm_identity_canonical_root(const char *root_path, char *out, size_t out_sz) {
    char resolved[CBM_SZ_4K];

    if (!out || out_sz == 0) {
        return;
    }
    out[0] = '\0';
    if (!root_path || !root_path[0]) {
        return;
    }
    if (cbm_canonical_path(root_path, resolved, sizeof(resolved))) {
        snprintf(out, out_sz, "%s", resolved);
        return;
    }
    snprintf(out, out_sz, "%s", root_path);
}

int cbm_identity_cmp_newest(const char *indexed_at_a, const char *name_a, const char *indexed_at_b,
                            const char *name_b) {
    int by_time = strcmp(id_or_empty(indexed_at_a), id_or_empty(indexed_at_b));
    if (by_time != 0) {
        return by_time;
    }
    return strcmp(id_or_empty(name_a), id_or_empty(name_b));
}

const cbm_identity_entry_t *cbm_identity_newest_for_canonical(const cbm_identity_entry_t *entries,
                                                              int count,
                                                              const char *canonical_root) {
    const cbm_identity_entry_t *best = NULL;
    const char *key = id_or_empty(canonical_root);
    int i;

    if (!entries || count <= 0 || !key[0]) {
        return NULL;
    }
    for (i = 0; i < count; i++) {
        if (strcmp(entries[i].canonical_root, key) != 0) {
            continue;
        }
        if (!best || cbm_identity_cmp_newest(entries[i].indexed_at, entries[i].name,
                                             best->indexed_at, best->name) > 0) {
            best = &entries[i];
        }
    }
    return best;
}
