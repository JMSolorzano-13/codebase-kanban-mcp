/**
 * @sdd-task: Task #2 - Admit create vs reindex (HTTP + MCP + jobs)
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-014 admission-only Path 1:1; SDD-ADR-015 HTTP project is reindex key
 * @sdd-why: Catalog helpers plus admit/MCP Then clauses for Task #2 Gherkin
 * @human-debug: Isolate CBM_CACHE_DIR. isError is always present — check :true not the key.
 * derived_name is the full-path slug, not basename.
 */
#include "../src/foundation/compat.h"
#include "test_framework.h"
#include "test_helpers.h"
#include "foundation/compat_fs.h"
#include "foundation/identity.h"
#include "mcp/mcp.h"
#include "pipeline/pipeline.h"
#include "store/store.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif

static bool id_mcp_is_error(const char *blob) {
    return blob && (strstr(blob, "\"isError\":true") != NULL ||
                    strstr(blob, "\\\"isError\\\":true") != NULL);
}

static bool id_has_json_key(const char *blob, const char *key) {
    char plain[128];
    char escaped[128];
    if (!blob || !key) {
        return false;
    }
    snprintf(plain, sizeof(plain), "\"%s\"", key);
    if (strstr(blob, plain)) {
        return true;
    }
    snprintf(escaped, sizeof(escaped), "\\\"%s\\\"", key);
    return strstr(blob, escaped) != NULL;
}

static bool id_write_project_db(const char *cache, const char *name, const char *root,
                                const char *indexed_at) {
    char path[CBM_SZ_2K];
    cbm_store_t *st;
    bool ok;

    snprintf(path, sizeof(path), "%s/%s.db", cache, name);
    st = cbm_store_open_path(path);
    if (!st) {
        return false;
    }
    ok = cbm_store_upsert_project(st, name, root) == CBM_STORE_OK;
    if (ok && indexed_at && indexed_at[0]) {
        char sql[CBM_SZ_512];
        snprintf(sql, sizeof(sql), "UPDATE projects SET indexed_at='%s' WHERE name='%s';",
                 indexed_at, name);
        ok = sqlite3_exec(cbm_store_get_db(st), sql, NULL, NULL, NULL) == SQLITE_OK;
    }
    cbm_store_close(st);
    return ok;
}

static void id_restore_cache(const char *saved) {
    if (saved) {
        (void)cbm_setenv("CBM_CACHE_DIR", saved, 1);
    } else {
        (void)cbm_unsetenv("CBM_CACHE_DIR");
    }
}

TEST(id_newest_prefers_later_indexed_at) {
    ASSERT_GT(cbm_identity_cmp_newest("2026-08-29T10:00:00Z", "alpha-old", "2026-08-28T10:00:00Z",
                                      "alpha"),
              0);
    ASSERT_LT(cbm_identity_cmp_newest("2026-08-28T10:00:00Z", "alpha", "2026-08-29T10:00:00Z",
                                      "alpha-old"),
              0);
    PASS();
}

TEST(id_newest_tie_uses_greater_name) {
    ASSERT_GT(
        cbm_identity_cmp_newest("2026-08-29T10:00:00Z", "beta", "2026-08-29T10:00:00Z", "alpha"),
        0);
    ASSERT_LT(
        cbm_identity_cmp_newest("2026-08-29T10:00:00Z", "alpha", "2026-08-29T10:00:00Z", "beta"),
        0);
    PASS();
}

TEST(id_missing_path_falls_back_to_stored) {
    char out[CBM_SZ_4K];
    const char *missing = "/no/such/cbm-identity-missing-xyz";
    cbm_identity_canonical_root(missing, out, sizeof(out));
    ASSERT_STR_EQ(out, missing);
    PASS();
}

TEST(id_catalog_two_stores_same_path_newest_name) {
    char cache[256];
    char repo[256];
    cbm_identity_entry_t *ents = NULL;
    int n = 0;
    const cbm_identity_entry_t *newest;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-cache-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-repo-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);

    ASSERT(id_write_project_db(cache, "alpha-old", repo, "2026-08-28T10:00:00Z"));
    ASSERT(id_write_project_db(cache, "alpha", repo, "2026-08-29T10:00:00Z"));

    ASSERT_EQ(cbm_identity_catalog_load(cache, &ents, &n), 0);
    ASSERT_EQ(n, 2);
    ASSERT_NOT_NULL(ents);
    newest = cbm_identity_newest_for_canonical(ents, n, ents[0].canonical_root);
    ASSERT_NOT_NULL(newest);
    ASSERT_STR_EQ(newest->name, "alpha");
    ASSERT_STR_EQ(ents[0].canonical_root, ents[1].canonical_root);
    ASSERT_STR_EQ(ents[0].root_path, repo);

    cbm_identity_catalog_free(ents);
    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

TEST(id_trailing_slash_collapses) {
    char cache[256];
    char repo[256];
    char slashed[300];
    char canon_plain[CBM_SZ_4K];
    char canon_slash[CBM_SZ_4K];
    cbm_identity_entry_t *ents = NULL;
    int n = 0;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-slash-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-slashrepo-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    snprintf(slashed, sizeof(slashed), "%s/", repo);

    ASSERT(id_write_project_db(cache, "alpha", slashed, "2026-08-29T10:00:00Z"));
    cbm_identity_canonical_root(repo, canon_plain, sizeof(canon_plain));
    cbm_identity_canonical_root(slashed, canon_slash, sizeof(canon_slash));
    ASSERT_STR_EQ(canon_plain, canon_slash);

    ASSERT_EQ(cbm_identity_catalog_load(cache, &ents, &n), 0);
    ASSERT_EQ(n, 1);
    ASSERT_STR_EQ(ents[0].root_path, slashed);
    ASSERT_STR_EQ(ents[0].canonical_root, canon_plain);

    cbm_identity_catalog_free(ents);
    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

#ifndef _WIN32
TEST(id_symlink_collapses) {
    char cache[256];
    char real_repo[256];
    char link_parent[256];
    char link_path[300];
    cbm_identity_entry_t *ents = NULL;
    int n = 0;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-linkc-XXXXXX");
    snprintf(real_repo, sizeof(real_repo), "/tmp/cbm-id-linkr-XXXXXX");
    snprintf(link_parent, sizeof(link_parent), "/tmp/cbm-id-linkp-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(real_repo) != NULL);
    ASSERT(cbm_mkdtemp(link_parent) != NULL);
    snprintf(link_path, sizeof(link_path), "%s/proj", link_parent);
    ASSERT_EQ(symlink(real_repo, link_path), 0);

    ASSERT(id_write_project_db(cache, "via-real", real_repo, "2026-08-28T10:00:00Z"));
    ASSERT(id_write_project_db(cache, "via-link", link_path, "2026-08-29T10:00:00Z"));

    ASSERT_EQ(cbm_identity_catalog_load(cache, &ents, &n), 0);
    ASSERT_EQ(n, 2);
    ASSERT_STR_EQ(ents[0].canonical_root, ents[1].canonical_root);
    ASSERT_STR_EQ(cbm_identity_newest_for_canonical(ents, n, ents[0].canonical_root)->name,
                  "via-link");

    cbm_identity_catalog_free(ents);
    th_cleanup(cache);
    th_cleanup(real_repo);
    unlink(link_path);
    th_cleanup(link_parent);
    PASS();
}
#endif

TEST(id_list_projects_emits_indexed_at_and_canonical_root) {
    char cache[256];
    char repo[256];
    char slashed[300];
    char canon[CBM_SZ_4K];
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    cbm_mcp_server_t *srv;
    char *resp;
    char *meta;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-list-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-listrepo-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    snprintf(slashed, sizeof(slashed), "%s/", repo);
    cbm_identity_canonical_root(repo, canon, sizeof(canon));

    ASSERT(id_write_project_db(cache, "alpha", slashed, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    srv = cbm_mcp_server_new(NULL);
    ASSERT_NOT_NULL(srv);
    resp = cbm_mcp_handle_tool(srv, "list_projects", "{}");
    ASSERT_NOT_NULL(resp);
    ASSERT(id_has_json_key(resp, "indexed_at"));
    ASSERT(id_has_json_key(resp, "canonical_root"));
    ASSERT_NOT_NULL(strstr(resp, "2026-08-29T10:00:00Z"));
    ASSERT_NOT_NULL(strstr(resp, "alpha"));
    ASSERT_NOT_NULL(strstr(resp, slashed));
    ASSERT_NOT_NULL(strstr(resp, canon));

    meta = cbm_mcp_handle_tool(srv, "list_projects", "{\"metadata_only\":true}");
    ASSERT_NOT_NULL(meta);
    ASSERT(id_has_json_key(meta, "indexed_at"));
    ASSERT(id_has_json_key(meta, "canonical_root"));
    ASSERT_NOT_NULL(strstr(meta, "2026-08-29T10:00:00Z"));

    free(resp);
    free(meta);
    cbm_mcp_server_free(srv);
    id_restore_cache(saved_copy);
    free(saved_copy);
    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

static char *id_fake_index_ok(void *ctx, const char *repo, const char *args) {
    (void)ctx;
    (void)repo;
    (void)args;
    return cbm_mcp_text_result("{\"status\":\"ok\"}", false);
}

TEST(id_admit_create_owned_path_is_path_exists) {
    char cache[256];
    char repo[256];
    char *derived;
    cbm_identity_admit_result_t adm;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-adm-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-admrepo-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    derived = cbm_project_name_from_path(repo);
    ASSERT(id_write_project_db(cache, derived, repo, "2026-08-29T10:00:00Z"));

    cbm_identity_admit(cache, repo, derived, NULL, CBM_IDENTITY_INTENT_CREATE, NULL, 0, &adm);
    ASSERT_EQ(adm.verdict, CBM_IDENTITY_ADMIT_PATH_EXISTS);
    ASSERT_STR_EQ(adm.existing_project, derived);

    free(derived);
    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

TEST(id_admit_trailing_slash_same_path) {
    char cache[256];
    char repo[256];
    char slashed[300];
    char *derived;
    cbm_identity_admit_result_t adm;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-adm2-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-adm2r-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    snprintf(slashed, sizeof(slashed), "%s/", repo);
    derived = cbm_project_name_from_path(repo);
    ASSERT(id_write_project_db(cache, "alpha", repo, "2026-08-29T10:00:00Z"));

    cbm_identity_admit(cache, slashed, derived, NULL, CBM_IDENTITY_INTENT_CREATE, NULL, 0, &adm);
    ASSERT_EQ(adm.verdict, CBM_IDENTITY_ADMIT_PATH_EXISTS);
    ASSERT_STR_EQ(adm.existing_project, "alpha");

    free(derived);
    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

TEST(id_admit_tie_picks_greater_name) {
    char cache[256];
    char repo[256];
    cbm_identity_admit_result_t adm;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-tie-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-tier-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    ASSERT(id_write_project_db(cache, "alpha", repo, "2026-08-29T10:00:00Z"));
    ASSERT(id_write_project_db(cache, "beta", repo, "2026-08-29T10:00:00Z"));

    cbm_identity_admit(cache, repo, "ignored", NULL, CBM_IDENTITY_INTENT_CREATE, NULL, 0, &adm);
    ASSERT_EQ(adm.verdict, CBM_IDENTITY_ADMIT_PATH_EXISTS);
    ASSERT_STR_EQ(adm.existing_project, "beta");

    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

TEST(id_admit_derived_name_other_path) {
    char cache[256];
    char owned[256];
    char request[256];
    char *derived;
    cbm_identity_admit_result_t adm;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-nm-XXXXXX");
    snprintf(owned, sizeof(owned), "/tmp/cbm-id-nmo-XXXXXX");
    snprintf(request, sizeof(request), "/tmp/cbm-id-nmr-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(owned) != NULL);
    ASSERT(cbm_mkdtemp(request) != NULL);
    derived = cbm_project_name_from_path(request);
    ASSERT(id_write_project_db(cache, derived, owned, "2026-08-29T10:00:00Z"));

    cbm_identity_admit(cache, request, derived, NULL, CBM_IDENTITY_INTENT_CREATE, NULL, 0, &adm);
    ASSERT_EQ(adm.verdict, CBM_IDENTITY_ADMIT_NAME_EXISTS);
    ASSERT_STR_EQ(adm.existing_project, derived);

    free(derived);
    th_cleanup(cache);
    th_cleanup(owned);
    th_cleanup(request);
    PASS();
}

TEST(id_admit_reindex_same_owner) {
    char cache[256];
    char repo[256];
    cbm_identity_admit_result_t adm;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-rx-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-rxr-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    ASSERT(id_write_project_db(cache, "custom", repo, "2026-08-29T10:00:00Z"));

    cbm_identity_admit(cache, repo, "other", "custom", CBM_IDENTITY_INTENT_REINDEX, NULL, 0, &adm);
    ASSERT_EQ(adm.verdict, CBM_IDENTITY_ADMIT_REINDEX);
    ASSERT_STR_EQ(adm.bind_project, "custom");

    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

TEST(id_admit_inflight_create_is_path_exists) {
    char repo[256];
    cbm_identity_inflight_t flight;
    cbm_identity_admit_result_t adm;

    snprintf(repo, sizeof(repo), "/tmp/cbm-id-if-XXXXXX");
    ASSERT(cbm_mkdtemp(repo) != NULL);
    cbm_identity_canonical_root(repo, flight.canonical_root, sizeof(flight.canonical_root));
    snprintf(flight.project, sizeof(flight.project), "%s", "alpha");

    cbm_identity_admit(NULL, repo, "alpha", NULL, CBM_IDENTITY_INTENT_CREATE, &flight, 1, &adm);
    ASSERT_EQ(adm.verdict, CBM_IDENTITY_ADMIT_PATH_EXISTS);
    ASSERT_STR_EQ(adm.existing_project, "alpha");

    th_cleanup(repo);
    PASS();
}

TEST(id_mcp_reindex_single_owner_not_error) {
    char cache[256];
    char repo[256];
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    char args[CBM_SZ_2K];
    cbm_mcp_server_t *srv;
    char *resp;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-mre-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-mrer-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    ASSERT(id_write_project_db(cache, "alpha", repo, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    srv = cbm_mcp_server_new(NULL);
    ASSERT_NOT_NULL(srv);
    cbm_mcp_server_set_index_executor(srv, id_fake_index_ok, NULL);
    snprintf(args, sizeof(args), "{\"repo_path\":\"%s\"}", repo);
    resp = cbm_mcp_handle_tool(srv, "index_repository", args);
    ASSERT_NOT_NULL(resp);
    ASSERT_FALSE(id_mcp_is_error(resp));

    free(resp);
    cbm_mcp_server_free(srv);
    id_restore_cache(saved_copy);
    free(saved_copy);
    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

TEST(id_mcp_name_override_clone_is_error) {
    char cache[256];
    char repo[256];
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    char args[CBM_SZ_2K];
    cbm_mcp_server_t *srv;
    char *resp;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-mcl-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-mclr-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    ASSERT(id_write_project_db(cache, "alpha", repo, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    srv = cbm_mcp_server_new(NULL);
    ASSERT_NOT_NULL(srv);
    cbm_mcp_server_set_index_executor(srv, id_fake_index_ok, NULL);
    snprintf(args, sizeof(args), "{\"repo_path\":\"%s\",\"name\":\"alpha-alias\"}", repo);
    resp = cbm_mcp_handle_tool(srv, "index_repository", args);
    ASSERT_NOT_NULL(resp);
    ASSERT_TRUE(id_mcp_is_error(resp));
    ASSERT_NOT_NULL(strstr(resp, "path_exists"));
    ASSERT_NOT_NULL(strstr(resp, "alpha"));

    free(resp);
    cbm_mcp_server_free(srv);
    id_restore_cache(saved_copy);
    free(saved_copy);
    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

TEST(id_mcp_name_on_new_path_free) {
    char cache[256];
    char repo[256];
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    char args[CBM_SZ_2K];
    cbm_mcp_server_t *srv;
    char *resp;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-mnw-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-mnwr-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    srv = cbm_mcp_server_new(NULL);
    ASSERT_NOT_NULL(srv);
    cbm_mcp_server_set_index_executor(srv, id_fake_index_ok, NULL);
    snprintf(args, sizeof(args), "{\"repo_path\":\"%s\",\"name\":\"custom\"}", repo);
    resp = cbm_mcp_handle_tool(srv, "index_repository", args);
    ASSERT_NOT_NULL(resp);
    ASSERT_FALSE(id_mcp_is_error(resp));
    ASSERT_NULL(strstr(resp, "path_exists"));
    ASSERT_NULL(strstr(resp, "name_exists"));

    free(resp);
    cbm_mcp_server_free(srv);
    id_restore_cache(saved_copy);
    free(saved_copy);
    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

TEST(id_mcp_multi_owner_no_name_is_path_exists) {
    char cache[256];
    char repo[256];
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    char args[CBM_SZ_2K];
    cbm_mcp_server_t *srv;
    char *resp;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-mmul-XXXXXX");
    snprintf(repo, sizeof(repo), "/tmp/cbm-id-mmulr-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(repo) != NULL);
    ASSERT(id_write_project_db(cache, "alpha", repo, "2026-08-28T10:00:00Z"));
    ASSERT(id_write_project_db(cache, "beta", repo, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    srv = cbm_mcp_server_new(NULL);
    ASSERT_NOT_NULL(srv);
    cbm_mcp_server_set_index_executor(srv, id_fake_index_ok, NULL);
    snprintf(args, sizeof(args), "{\"repo_path\":\"%s\"}", repo);
    resp = cbm_mcp_handle_tool(srv, "index_repository", args);
    ASSERT_NOT_NULL(resp);
    ASSERT_TRUE(id_mcp_is_error(resp));
    ASSERT_NOT_NULL(strstr(resp, "path_exists"));
    ASSERT_NOT_NULL(strstr(resp, "beta"));

    free(resp);
    cbm_mcp_server_free(srv);
    id_restore_cache(saved_copy);
    free(saved_copy);
    th_cleanup(cache);
    th_cleanup(repo);
    PASS();
}

TEST(id_mcp_name_on_other_path_is_name_exists) {
    char cache[256];
    char owned[256];
    char request[256];
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    char args[CBM_SZ_2K];
    cbm_mcp_server_t *srv;
    char *resp;

    snprintf(cache, sizeof(cache), "/tmp/cbm-id-mnm-XXXXXX");
    snprintf(owned, sizeof(owned), "/tmp/cbm-id-mnmo-XXXXXX");
    snprintf(request, sizeof(request), "/tmp/cbm-id-mnmr-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(owned) != NULL);
    ASSERT(cbm_mkdtemp(request) != NULL);
    ASSERT(id_write_project_db(cache, "taken", owned, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    srv = cbm_mcp_server_new(NULL);
    ASSERT_NOT_NULL(srv);
    cbm_mcp_server_set_index_executor(srv, id_fake_index_ok, NULL);
    snprintf(args, sizeof(args), "{\"repo_path\":\"%s\",\"name\":\"taken\"}", request);
    resp = cbm_mcp_handle_tool(srv, "index_repository", args);
    ASSERT_NOT_NULL(resp);
    ASSERT_TRUE(id_mcp_is_error(resp));
    ASSERT_NOT_NULL(strstr(resp, "name_exists"));
    ASSERT_NOT_NULL(strstr(resp, "taken"));

    free(resp);
    cbm_mcp_server_free(srv);
    id_restore_cache(saved_copy);
    free(saved_copy);
    th_cleanup(cache);
    th_cleanup(owned);
    th_cleanup(request);
    PASS();
}

SUITE(identity) {
    RUN_TEST(id_newest_prefers_later_indexed_at);
    RUN_TEST(id_newest_tie_uses_greater_name);
    RUN_TEST(id_missing_path_falls_back_to_stored);
    RUN_TEST(id_catalog_two_stores_same_path_newest_name);
    RUN_TEST(id_trailing_slash_collapses);
#ifndef _WIN32
    RUN_TEST(id_symlink_collapses);
#endif
    RUN_TEST(id_list_projects_emits_indexed_at_and_canonical_root);
    RUN_TEST(id_admit_create_owned_path_is_path_exists);
    RUN_TEST(id_admit_trailing_slash_same_path);
    RUN_TEST(id_admit_tie_picks_greater_name);
    RUN_TEST(id_admit_derived_name_other_path);
    RUN_TEST(id_admit_reindex_same_owner);
    RUN_TEST(id_admit_inflight_create_is_path_exists);
    RUN_TEST(id_mcp_reindex_single_owner_not_error);
    RUN_TEST(id_mcp_name_override_clone_is_error);
    RUN_TEST(id_mcp_name_on_new_path_free);
    RUN_TEST(id_mcp_multi_owner_no_name_is_path_exists);
    RUN_TEST(id_mcp_name_on_other_path_is_name_exists);
}
