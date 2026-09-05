epic: 003
plan: executive-ui-ia
name: path-project-identity
status: detailed

summary: Path↔Project is 1:1. New index of an existing Path/name blocks and redirects. UI has no Project ID field. Legacy duplicates: conflict, enter newest, auth-delete older.
delivery-rationale: Operator cannot create a second index of the same repo; leftover clones are resolved with an explicit delete, not silently.

tech:
  facts:
    - Today identity = project name (.db file); root_path stored per project; no UNIQUE on path
    - Name from cbm_project_name_from_path or optional override (MCP + UI)
    - UI field copy: "Project ID (optional — permanent, cannot be renamed)"
    - delete via DELETE /api/project?name= and MCP delete_project
    - indexed_at exists and is the "newest" signal (ADR-010)
    - index can run via UI POST /api/index, MCP index_repository, CLI
    - Concurrent index serialized per project name (mutation guard), not per path
  open-qs:
    1. Canonical Path: realpath/symlink, trailing slash, Windows drive case, macOS case-insensitivity — what equality means?
    2. MCP/CLI project_name override: reject, ignore, or allow only when Path is new and name free (UI removed; agents may still send it)?
    3. Detect legacy dups: scan all stores' root_path after canonicalize — cost, when (UI load vs index)?
    4. Two in-flight indexes of same Path under different derived names — guard key becomes Path not name?
    5. Newest = indexed_at string compare — timezone/format guaranteed?
    6. After delete older: leftover .db/.corrupt files and watcher registration?

product:
  facts:
    - New create of existing Path or Project → block + redirect to existing workspace (Graph)
    - No Project ID field in create modal
    - Legacy: show conflict, enter newest, ask auth to delete older; yes deletes; no keeps older listed
    - Last indexed visible so human can see which is newer
  open-qs:
    1. Redirect UX: immediate Graph of existing project + notice, or confirm step first?
    2. Three-plus clones of one Path: enter newest, offer delete each older, or one multi-delete?
    3. User declines delete: persist conflict badge on Dashboard until resolved?
    4. Conflict copy: show both names + both dates + both paths (paths should match)?
    5. Name-collision different Paths (two folders derived to same slug): treat as legacy dup of name, not path — same UX or different?

deps: [epic-001 create modal + Dashboard list, epic-002 enter→Graph]
adrs: [ADR-001, ADR-009, ADR-010, ADR-012]
glossary-refs: [Path, Project, Duplicate-index attempt, Legacy duplicate, Last indexed]
