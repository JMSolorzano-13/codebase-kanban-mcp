/**
 * @sdd-task: Task #2 - Admit create vs reindex (HTTP + MCP + jobs)
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-014 no UNIQUE SQL; SDD-ADR-015 project is reindex key
 * @sdd-why: Catalog + admit share one scan; store I/O stays out of foundation
 * @human-debug: Skip _*.db, :memory:, ghosts, and DBs without one primary projects row.
 */
#include "foundation/identity.h"

#include "foundation/compat_fs.h"
#include "store/store.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    ID_DB_EXT = 3,      /* strlen(".db") */
    ID_MIN_DB_NAME = 4, /* "x.db" */
    ID_CATALOG_INIT = 8,
    ID_CATALOG_GROW = 2,
};

static const char *id_or_empty(const char *s) {
    return s ? s : "";
}

static bool id_is_project_db_file(const char *name, size_t len) {
    if (!name || len < ID_MIN_DB_NAME || strcmp(name + len - ID_DB_EXT, ".db") != 0) {
        return false;
    }
    if (name[0] == '_' || strncmp(name, ":memory:", sizeof(":memory:") - 1) == 0) {
        return false;
    }
    return true;
}

static bool id_is_primary_name(const char *name) {
    return name && name[0] && strstr(name, "::") == NULL;
}

static bool id_fill_from_store(cbm_store_t *st, cbm_identity_entry_t *out) {
    cbm_project_t *projs = NULL;
    int n = 0;
    int primary = -1;
    int primary_count = 0;
    int i;

    if (cbm_store_list_projects(st, &projs, &n) != CBM_STORE_OK) {
        return false;
    }
    for (i = 0; i < n; i++) {
        if (id_is_primary_name(projs[i].name)) {
            primary = i;
            primary_count++;
        }
    }
    if (primary_count != 1) {
        cbm_store_free_projects(projs, n);
        return false;
    }
    snprintf(out->name, sizeof(out->name), "%s", id_or_empty(projs[primary].name));
    snprintf(out->root_path, sizeof(out->root_path), "%s", id_or_empty(projs[primary].root_path));
    snprintf(out->indexed_at, sizeof(out->indexed_at), "%s",
             id_or_empty(projs[primary].indexed_at));
    cbm_identity_canonical_root(out->root_path, out->canonical_root, sizeof(out->canonical_root));
    cbm_store_free_projects(projs, n);
    return out->name[0] != '\0';
}

int cbm_identity_catalog_load(const char *cache_dir, cbm_identity_entry_t **out, int *count) {
    cbm_dir_t *d;
    cbm_dirent_t *entry;
    cbm_identity_entry_t *arr;
    int cap = ID_CATALOG_INIT;
    int n = 0;

    if (!out || !count) {
        return -1;
    }
    *out = NULL;
    *count = 0;
    if (!cache_dir || !cache_dir[0]) {
        return -1;
    }

    d = cbm_opendir(cache_dir);
    if (!d) {
        return 0;
    }

    arr = calloc((size_t)cap, sizeof(*arr));
    if (!arr) {
        cbm_closedir(d);
        return -1;
    }

    while ((entry = cbm_readdir(d)) != NULL) {
        const char *name = entry->name;
        size_t len = name ? strlen(name) : 0;
        char full_path[CBM_SZ_2K];
        cbm_store_t *st;
        int written;

        if (!id_is_project_db_file(name, len)) {
            continue;
        }
        written = snprintf(full_path, sizeof(full_path), "%s/%s", cache_dir, name);
        if (written <= 0 || (size_t)written >= sizeof(full_path)) {
            continue;
        }
        st = cbm_store_open_path_query(full_path);
        if (!st) {
            continue;
        }
        if (n >= cap) {
            int next = cap * ID_CATALOG_GROW;
            cbm_identity_entry_t *grown = realloc(arr, (size_t)next * sizeof(*arr));
            if (!grown) {
                cbm_store_close(st);
                cbm_identity_catalog_free(arr);
                cbm_closedir(d);
                return -1;
            }
            memset(grown + cap, 0, (size_t)(next - cap) * sizeof(*arr));
            arr = grown;
            cap = next;
        }
        if (id_fill_from_store(st, &arr[n])) {
            n++;
        }
        cbm_store_close(st);
    }
    cbm_closedir(d);

    if (n == 0) {
        free(arr);
        *out = NULL;
        *count = 0;
        return 0;
    }
    *out = arr;
    *count = n;
    return 0;
}

void cbm_identity_catalog_free(cbm_identity_entry_t *entries) {
    free(entries);
}

static const char *id_nonempty(const char *s) {
    return (s && s[0]) ? s : NULL;
}

static const cbm_identity_entry_t *id_find_name(const cbm_identity_entry_t *entries, int count,
                                                const char *name) {
    int i;
    if (!entries || !name || !name[0]) {
        return NULL;
    }
    for (i = 0; i < count; i++) {
        if (strcmp(entries[i].name, name) == 0) {
            return &entries[i];
        }
    }
    return NULL;
}

static const cbm_identity_inflight_t *id_inflight_for_path(const cbm_identity_inflight_t *inflight,
                                                           int count, const char *canonical) {
    int i;
    if (!inflight || !canonical || !canonical[0]) {
        return NULL;
    }
    for (i = 0; i < count; i++) {
        if (strcmp(inflight[i].canonical_root, canonical) == 0) {
            return &inflight[i];
        }
    }
    return NULL;
}

static int id_owner_count(const cbm_identity_entry_t *entries, int count, const char *canonical) {
    int n = 0;
    int i;
    if (!entries || !canonical || !canonical[0]) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (strcmp(entries[i].canonical_root, canonical) == 0) {
            n++;
        }
    }
    return n;
}

static bool id_name_owns_path(const cbm_identity_entry_t *entries, int count, const char *name,
                              const char *canonical) {
    const cbm_identity_entry_t *row = id_find_name(entries, count, name);
    return row && strcmp(row->canonical_root, canonical) == 0;
}

static void id_fill_conflict(cbm_identity_admit_result_t *out, cbm_identity_admit_verdict_t verdict,
                             const char *canonical, const char *name, const char *indexed_at) {
    out->verdict = verdict;
    snprintf(out->canonical_root, sizeof(out->canonical_root), "%s", id_or_empty(canonical));
    snprintf(out->existing_project, sizeof(out->existing_project), "%s", id_or_empty(name));
    snprintf(out->indexed_at, sizeof(out->indexed_at), "%s", id_or_empty(indexed_at));
}

static void id_fill_ok(cbm_identity_admit_result_t *out, cbm_identity_admit_verdict_t verdict,
                       const char *canonical, const char *bind) {
    out->verdict = verdict;
    snprintf(out->canonical_root, sizeof(out->canonical_root), "%s", id_or_empty(canonical));
    snprintf(out->bind_project, sizeof(out->bind_project), "%s", id_or_empty(bind));
}

static void id_path_exists_from_catalog(cbm_identity_admit_result_t *out,
                                        const cbm_identity_entry_t *entries, int count,
                                        const char *canonical, const cbm_identity_inflight_t *hit) {
    const cbm_identity_entry_t *newest =
        cbm_identity_newest_for_canonical(entries, count, canonical);
    if (newest) {
        id_fill_conflict(out, CBM_IDENTITY_ADMIT_PATH_EXISTS, canonical, newest->name,
                         newest->indexed_at);
        return;
    }
    if (hit) {
        id_fill_conflict(out, CBM_IDENTITY_ADMIT_PATH_EXISTS, canonical, hit->project, "");
        return;
    }
    id_fill_conflict(out, CBM_IDENTITY_ADMIT_PATH_EXISTS, canonical, "", "");
}

void cbm_identity_admit(const char *cache_dir, const char *request_path, const char *derived_name,
                        const char *request_name, cbm_identity_intent_t intent,
                        const cbm_identity_inflight_t *inflight, int inflight_count,
                        cbm_identity_admit_result_t *out) {
    cbm_identity_entry_t *entries = NULL;
    int count = 0;
    char canonical[CBM_SZ_4K];
    const char *name = id_nonempty(request_name);
    const char *derived = id_nonempty(derived_name);
    const cbm_identity_inflight_t *flight = NULL;
    const cbm_identity_entry_t *named = NULL;
    int owners = 0;

    if (!out) {
        return;
    }
    memset(out, 0, sizeof(*out));
    cbm_identity_canonical_root(request_path, canonical, sizeof(canonical));
    if (cache_dir && cache_dir[0]) {
        (void)cbm_identity_catalog_load(cache_dir, &entries, &count);
    }
    flight = id_inflight_for_path(inflight, inflight_count, canonical);
    owners = id_owner_count(entries, count, canonical);
    named = name ? id_find_name(entries, count, name) : NULL;

    if (intent == CBM_IDENTITY_INTENT_CREATE) {
        if (flight || owners > 0) {
            id_path_exists_from_catalog(out, entries, count, canonical, flight);
            cbm_identity_catalog_free(entries);
            return;
        }
        if (derived) {
            const cbm_identity_entry_t *row = id_find_name(entries, count, derived);
            if (row && strcmp(row->canonical_root, canonical) != 0) {
                id_fill_conflict(out, CBM_IDENTITY_ADMIT_NAME_EXISTS, canonical, row->name,
                                 row->indexed_at);
                cbm_identity_catalog_free(entries);
                return;
            }
        }
        id_fill_ok(out, CBM_IDENTITY_ADMIT_CREATE, canonical, derived ? derived : "");
        cbm_identity_catalog_free(entries);
        return;
    }

    if (intent == CBM_IDENTITY_INTENT_REINDEX) {
        if (name && id_name_owns_path(entries, count, name, canonical)) {
            id_fill_ok(out, CBM_IDENTITY_ADMIT_REINDEX, canonical, name);
            cbm_identity_catalog_free(entries);
            return;
        }
        if (flight && name && strcmp(flight->project, name) == 0) {
            id_fill_ok(out, CBM_IDENTITY_ADMIT_REINDEX, canonical, name);
            cbm_identity_catalog_free(entries);
            return;
        }
        if (flight || owners > 0) {
            id_path_exists_from_catalog(out, entries, count, canonical, flight);
            cbm_identity_catalog_free(entries);
            return;
        }
        if (named) {
            id_fill_conflict(out, CBM_IDENTITY_ADMIT_NAME_EXISTS, canonical, named->name,
                             named->indexed_at);
            cbm_identity_catalog_free(entries);
            return;
        }
        id_fill_conflict(out, CBM_IDENTITY_ADMIT_NAME_EXISTS, canonical, name ? name : "", "");
        cbm_identity_catalog_free(entries);
        return;
    }

    /* MCP */
    if (owners == 0 && !flight) {
        if (name && named && strcmp(named->canonical_root, canonical) != 0) {
            id_fill_conflict(out, CBM_IDENTITY_ADMIT_NAME_EXISTS, canonical, named->name,
                             named->indexed_at);
            cbm_identity_catalog_free(entries);
            return;
        }
        id_fill_ok(out, CBM_IDENTITY_ADMIT_CREATE, canonical,
                   name ? name : (derived ? derived : ""));
        cbm_identity_catalog_free(entries);
        return;
    }

    if (!name) {
        if (owners >= 2) {
            id_path_exists_from_catalog(out, entries, count, canonical, flight);
            cbm_identity_catalog_free(entries);
            return;
        }
        if (owners == 1) {
            const cbm_identity_entry_t *newest =
                cbm_identity_newest_for_canonical(entries, count, canonical);
            id_fill_ok(out, CBM_IDENTITY_ADMIT_REINDEX, canonical,
                       newest ? newest->name : (flight ? flight->project : ""));
            cbm_identity_catalog_free(entries);
            return;
        }
        /* catalog empty, in-flight only: subscribe/reindex that job */
        id_fill_ok(out, CBM_IDENTITY_ADMIT_REINDEX, canonical, flight ? flight->project : "");
        cbm_identity_catalog_free(entries);
        return;
    }

    if (id_name_owns_path(entries, count, name, canonical) ||
        (flight && strcmp(flight->project, name) == 0)) {
        id_fill_ok(out, CBM_IDENTITY_ADMIT_REINDEX, canonical, name);
        cbm_identity_catalog_free(entries);
        return;
    }
    id_path_exists_from_catalog(out, entries, count, canonical, flight);
    cbm_identity_catalog_free(entries);
}
