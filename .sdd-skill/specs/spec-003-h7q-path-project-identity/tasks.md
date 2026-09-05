# Work Breakdown — Spec-003: Path–Project Identity
Total Tasks: 5 / Estimated Total Effort: 16h

## Task Dependency Graph
```
#1 catalog + list fields ──┬──► #2 HTTP/MCP admit + path-keyed jobs ──► #5 Reindex + Gherkin compose
                           └──► #3 Dashboard groups + conflict ──► #4 create 409 redirect + notice ──► #5
```
#2 and #3 are parallel after #1. Critical path: #1 (4h) → #2 (4h) → #5 (3h) = 11h if #3/#4 finish in the #2 window.

## Tasks

### Task #1 — Identity catalog + list_projects fields
Definition of Done:
- [x] Shared C catalog: scan cache `.db` files, read `name`, `root_path`, `indexed_at` via existing store APIs
- [x] `canonical_root` = `cbm_canonical_path(root_path)` or stored path if resolve fails
- [x] Newest helper: max `indexed_at` string; tie → greater `name`
- [x] `list_projects` JSON each project includes `indexed_at` and `canonical_root` (always, not metadata_only-gated)
- [x] Display `root_path` still the stored value
- [x] C tests: two stores same canonical path → newest name; trailing slash / symlink collapse if fixture can; missing path falls back
- [x] `graph-ui` `Project` type adds `canonical_root?: string`
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001 (newest), US-004 (group key)
Gherkin covered: newest/tie helpers; list fields used by later UI tasks
Dependencies: None
Estimated Effort: 4h
Subagent: no
Path: full
Implementation Notes: No UNIQUE SQL. Extract scan from `handle_list_projects` / `build_project_json_entry` rather than opening DBs twice if easy. Do not implement 409 yet.

### Task #2 — Admit create vs reindex (HTTP + MCP + jobs)
Definition of Done:
- [x] Bare `{root_path}`: Path owned → HTTP 409 `code=path_exists` `existing_project=<newest>` no slot; derived name exists other Path → 409 `name_exists`
- [x] `{root_path, project}` where that name owns that Path → HTTP 202, same name only
- [x] HTTP `project_name` treated as `project` (reindex key). Never creates a new name
- [x] MCP rules per plan.md (single-owner no-name = reindex; multi-owner no-name = isError `path_exists`; alias `name` = `path_exists`; name on other Path = `name_exists`)
- [x] In-flight job for same canonical Path: second bare POST → 409 `path_exists` (not 202). Same-`project` reindex may subscribe
- [x] C tests cover Gherkin HTTP/MCP Then clauses listed below
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-003, US-006
Gherkin covered:
- trailing slash → 409 path_exists
- indexed_at tie → existing_project greater name
- MCP reindex single owner
- MCP name override clones Path
- MCP name on new Path free
- derived name other Path 409 (HTTP)
- in-flight second create 409
Dependencies: Task #1
Estimated Effort: 4h
Subagent: no
Path: full
Implementation Notes: Shared helper with workspace-boundary style. Allocate HTTP slot only after admit allows. Watcher path is reindex. Do not build Dashboard UI here.

### Task #3 — Dashboard conflict group + Enter newest + delete older
Definition of Done:
- [x] Group `useProjects` rows by `canonical_root` or `root_path` if field absent
- [x] Group size ≥ 2 → conflict region: all names, all last-indexed `<time>`, Path once
- [x] Enter on the group → `onSelectProject(newestName)`
- [x] Each member keeps its own Delete (existing confirm + `DELETE /api/project?name=`)
- [x] No control that deletes two names in one request
- [x] Solo rows unchanged (Enter + Delete)
- [x] `Dashboard.test.tsx` covers two-clone conflict, three-clone Enter a3 + two Deletes, delete cancel
- [x] tests pass; breadcrumbs
User Stories Addressed: US-004, US-005
Gherkin covered:
- Legacy same-Path clones + Enter newest
- Confirm delete older
- three clones enter newest, per-older delete
- delete older cancelled
Dependencies: Task #1
Estimated Effort: 3h
Subagent: yes
Path: full
Implementation Notes: Reindex button is Task #5. Do not navigate on delete. i18n conflict copy here or #5; English accessible names required for tests.

### Task #4 — Create modal path_exists redirect + notice
Definition of Done:
- [x] Modal POST still `{root_path}` only (no `project` / `project_name`)
- [x] HTTP 409 `path_exists` → close modal, call `onPathExists(existing_project)` (or equivalent). No `onCreated` / IndexProgress
- [x] App sets `?tab=graph&project=<existing>` and a `role="status"` notice containing that name
- [x] HTTP 409 `name_exists` → modal stays open, visible error from body, no Graph navigation
- [x] Optional: if list already has that `canonical_root`, skip POST and take the same redirect
- [x] CreateIndexModal + App tests for both 409 codes; no Project ID field
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-006
Gherkin covered:
- Create already-indexed Path redirects
- create modal no Project ID / no project key
- derived name other Path stays in modal
Dependencies: Task #2 (409 shape), Task #3 (Dashboard still home)
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Notice can clear on next navigation. Do not 202-then-redirect.

### Task #5 — Dashboard Reindex + i18n + remaining Gherkin
Definition of Done:
- [x] Every row (including conflict members) has control accessible name "Reindex"
- [x] Click → POST `{"root_path":row.root_path,"project":row.name}` ; 202 → IndexProgress, stay Dashboard
- [x] 500 → visible error, stay Dashboard, row remains
- [x] No Reindex on workspace header
- [x] i18n en+zh: Reindex, conflict, path-exists notice, name_exists, reindex error
- [x] App/Dashboard tests: Reindex happy, custom-named `custom`, Reindex 500
- [x] Full suite green; `colorForLabel("Function")` still `#06b6d4` if that test exists
- [x] tests pass; breadcrumbs
User Stories Addressed: US-002, US-007
Gherkin covered:
- Dashboard Reindex starts job
- Reindex custom-named project
- Reindex failure stays on Dashboard
Dependencies: Task #2, Task #3, Task #4
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: Create modal must not gain Reindex. Do not send `project_name`.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | Catalog + list fields | 4h | None | done |
| 2 | Admit HTTP/MCP/jobs | 4h | #1 | done |
| 3 | Conflict UI | 3h | #1 | done |
| 4 | Create 409 redirect | 2h | #2 #3 | done |
| 5 | Reindex + compose | 3h | #2 #3 #4 | done |
Total: 16h

## Critical Path
#1 → #2 → #5 (11h). #3/#4 parallel after #1 / after #2+#3.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Blocked: note in state.md, switch task, tell @architect.

## Test Coverage Requirements
Every Gherkin scenario mapped above. Target >80% on touched graph-ui files. C tests for admission.

## Success Criteria for All Tasks
- [x] all DoD complete [x] tests>80% touched (57/57 targeted Vitest; reporter not installed) [x] passes @review [x] @tester approves (DEV all 5; CERT later if required) [x] human docs complete (awaiting human close phrase)
