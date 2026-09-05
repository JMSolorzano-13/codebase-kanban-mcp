/**
 * @sdd-task: Task #2 - Admit create vs reindex (HTTP + MCP + jobs)
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-014 admission-only; SDD-ADR-015 project is reindex key
 * @sdd-why: Shared admit + catalog; no UNIQUE SQL
 * @human-debug: CREATE vs REINDEX vs MCP intents. derived_name is full-path slug.
 */
#ifndef CBM_FOUNDATION_IDENTITY_H
#define CBM_FOUNDATION_IDENTITY_H

#include "foundation/constants.h"

#include <stddef.h>

/*
 * Path↔Project catalog. No global projects table: scan cache .db files.
 * Admit (Task #2) and list_projects share these helpers.
 */

typedef struct {
    char name[CBM_SZ_1K];
    char root_path[CBM_SZ_1K]; /* stored display path, not rewritten */
    char indexed_at[CBM_SZ_64];
    char canonical_root[CBM_SZ_4K];
} cbm_identity_entry_t;

/* realpath when the path exists; otherwise copy stored root_path. */
void cbm_identity_canonical_root(const char *root_path, char *out, size_t out_sz);

/* >0 if a is newer (or tie with greater name), <0 if b wins, 0 if equal. */
int cbm_identity_cmp_newest(const char *indexed_at_a, const char *name_a, const char *indexed_at_b,
                            const char *name_b);

const cbm_identity_entry_t *cbm_identity_newest_for_canonical(const cbm_identity_entry_t *entries,
                                                              int count,
                                                              const char *canonical_root);

/* Scan cache_dir for resolvable project DBs. 0 on success (empty dir is ok). */
int cbm_identity_catalog_load(const char *cache_dir, cbm_identity_entry_t **out, int *count);
void cbm_identity_catalog_free(cbm_identity_entry_t *entries);

typedef enum {
    CBM_IDENTITY_INTENT_CREATE = 0, /* HTTP bare {root_path} */
    CBM_IDENTITY_INTENT_REINDEX,    /* HTTP project / project_name / watcher */
    CBM_IDENTITY_INTENT_MCP,        /* index_repository name rules */
} cbm_identity_intent_t;

typedef enum {
    CBM_IDENTITY_ADMIT_CREATE = 0,
    CBM_IDENTITY_ADMIT_REINDEX,
    CBM_IDENTITY_ADMIT_PATH_EXISTS,
    CBM_IDENTITY_ADMIT_NAME_EXISTS,
} cbm_identity_admit_verdict_t;

typedef struct {
    char canonical_root[CBM_SZ_4K];
    char project[CBM_SZ_1K];
} cbm_identity_inflight_t;

typedef struct {
    cbm_identity_admit_verdict_t verdict;
    char existing_project[CBM_SZ_1K];
    char indexed_at[CBM_SZ_64];
    char canonical_root[CBM_SZ_4K];
    char bind_project[CBM_SZ_1K];
} cbm_identity_admit_result_t;

/* derived_name = cbm_project_name_from_path(request_path). request_name NULL if none. */
void cbm_identity_admit(const char *cache_dir, const char *request_path, const char *derived_name,
                        const char *request_name, cbm_identity_intent_t intent,
                        const cbm_identity_inflight_t *inflight, int inflight_count,
                        cbm_identity_admit_result_t *out);

#endif /* CBM_FOUNDATION_IDENTITY_H */
