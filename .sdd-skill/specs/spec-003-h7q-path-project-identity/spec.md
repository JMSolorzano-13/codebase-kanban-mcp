# Spec-003-h7q: Path–Project Identity
Status: closed | Spec ID: spec-003-h7q-path-project-identity
Ref Ticket: none | KPI: A second identity for an already-indexed Path is refused (HTTP 409 / MCP isError) and the operator lands on the existing project's Graph; leftover same-Path clones show a conflict, enter the newest, and delete older only after confirm; Dashboard Reindex refreshes the existing row without a second store | Priority: P0
Created: 2026-08-29 | Tech Debt Ref: TD-002

## Executive Summary
Today the store key is the project name (`.db` filename). `root_path` is stored per project but is not unique. MCP `index_repository` still accepts `name`, so one repo can exist as two indexes. Spec-001 already removed the UI Project ID field; it did not reject a second index of the same Path.

This spec makes Path↔Project 1:1 going forward (grill ADR-001, ADR-012). A create of a Path that already has a project is blocked and the UI opens that project's Graph with a notice. Reindex of an existing row is a separate Dashboard action (and MCP/CLI); it does not go through the create modal. Legacy clones already on disk are not silent-deleted (grill ADR-009): Dashboard shows a conflict, Enter opens the newest `indexed_at`, and deleting an older clone is confirm-gated.

Business impact: the operator cannot invent a second index of the same repo; leftover aliases are resolved with an explicit delete.

## User Stories

### US-001: Duplicate Path create is blocked and redirected
As an operator, I want "Index This Folder" on a Path I already indexed to open that project instead of creating another, So that Path and Project stay 1:1.
Acceptance Criteria:
- [ ] POST `/api/index` that would create a new project name for a Path already owned by one or more projects returns HTTP 409, JSON includes `"code":"path_exists"` and `"existing_project":"<newest name>"`, and does not start an index job (no HTTP 202, no new IndexProgress slot)
- [ ] Newest name = the listed project whose canonical `root_path` equals the request Path and whose `indexed_at` is greatest (ISO-8601 string compare). Tie → lexicographically greater `name`
- [ ] Create-index modal on 409 `path_exists` closes, URL becomes `?tab=graph&project=<existing_project>`, and a visible status notice names that project
- [ ] If the UI list already contains that Path, the modal may skip POST; outcome is the same redirect + notice (no 202)
- [ ] Create-index still posts `{ "root_path": "<path>" }` only (no `project` / `project_name`). Spec-001 regression
- [ ] Bare POST `{ "root_path": "<owned path>" }` (no existing-project key) is a create attempt: HTTP 409 `path_exists` even when the derived name matches the owner

### US-002: Reindex of an existing project is allowed
As an operator or agent, I want to refresh an index that already exists, So that freshness updates and spec-004 can attach later.
Acceptance Criteria:
- [ ] HTTP reindex: POST `/api/index` with `{ "root_path": "<path>", "project": "<existing name>" }` where that name already owns that canonical Path → HTTP 202, updates that project only, no second `.db` / list row
- [ ] MCP `index_repository` with `repo_path` equal to an owned Path and no `name` override (or `name` equal to the owner) → not isError, same single project
- [ ] Create-index modal never sends `project` and never starts a reindex (US-001)
- [ ] Watcher auto-sync of an already-owned Path is not a create and is not 409'd by this spec

### US-003: MCP/CLI cannot mint a second identity
As an agent, I want `index_repository` to obey the same 1:1 rule as the UI, So that a `name` override cannot clone a Path.
Acceptance Criteria:
- [ ] `index_repository` with a Path already owned and a `name` different from the owning project → isError, text contains `path_exists` and the owning/newest project name; no new store
- [ ] `index_repository` with a new Path and `name` that already exists for a different Path → isError, text contains `name_exists` and the existing project name; no new store
- [ ] `index_repository` with a new Path and free `name` (override or derived) → index starts (today's success path)
- [ ] Mutation/concurrency guard for in-flight index is the canonical Path, not only the project name: a second create of the same Path under another derived name is refused while the first job is running

### US-004: Legacy same-Path clones are a visible conflict
As an operator, I want leftover aliases of one Path marked on Dashboard, So that I can see the clash and enter the newest.
Acceptance Criteria:
- [ ] Two or more `list_projects` rows whose `root_path` canonicalize to the same string are a legacy duplicate group
- [ ] Dashboard shows a conflict region for the group: every name, every last-indexed datetime (`formatIndexedAt` / existing `indexed_at`), and the Path once
- [ ] Enter (or the group's primary enter) opens `?tab=graph&project=<newest name>`
- [ ] Rows in a group remain listed until deleted; the conflict badge/region stays while two or more members remain
- [ ] Three or more clones: enter newest; each older has its own delete affordance (no single multi-delete)

### US-005: Older clone deletes only after confirm
As an operator, I want to authorize deleting the older clone, So that history is not dropped silently.
Acceptance Criteria:
- [ ] Delete-older uses the existing confirm + `DELETE /api/project?name=<older>`
- [ ] Confirm yes → older name is gone from the next `list_projects`; newest remains
- [ ] Confirm dismiss → no DELETE; older stays; conflict remains
- [ ] Existing per-row Delete on a non-conflict row is unchanged
- [ ] This spec does not rename custom-named DBs

### US-006: Name collision on a different Path is not a redirect
As an operator, I want indexing `/tmp/b/foo` to fail clearly when project `foo` already owns `/tmp/a/foo`, So that I am not dropped into the wrong repo.
Acceptance Criteria:
- [ ] POST `/api/index` whose derived name exists and whose canonical Path differs from that project's Path returns HTTP 409, `"code":"name_exists"`, `"existing_project":"<name>"`
- [ ] UI keeps the create modal open, shows the error text, does not change to `?tab=graph&project=<existing>`
- [ ] This is not the US-004 conflict UI (paths differ)

### US-007: Dashboard has a Reindex action
As an operator, I want Reindex on a Dashboard row, So that I can refresh that project without the create modal.
Acceptance Criteria:
- [ ] Each project row (including each member of a conflict group) has a control with accessible name "Reindex"
- [ ] Activating it POSTs `{ "root_path": "<row.root_path>", "project": "<row.name>" }` to `/api/index`
- [ ] HTTP 202 → IndexProgress on Dashboard; URL stays `?tab=dashboard` (no workspace navigation)
- [ ] list_projects still has that same name once (no new row)
- [ ] HTTP error → visible error on Dashboard, row remains, no redirect
- [ ] i18n en+zh for the control and error
- [ ] Workspace header does not gain a Reindex control in this spec

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Path–Project Identity

  # Happy Paths
  Scenario: Create of an already-indexed Path redirects to the existing Graph
    Given list_projects returns one project named "alpha" with root_path "/tmp/alpha" and indexed_at "2026-08-29T10:00:00Z"
    And the create-index modal is open
    When the operator activates "Index This Folder" for "/tmp/alpha"
    Then no index job is started (no HTTP 202 from POST /api/index, or POST is skipped)
    And if POST /api/index is sent it returns HTTP 409 with JSON code "path_exists" and existing_project "alpha"
    And the URL query includes "tab=graph" and "project=alpha"
    And a visible status notice contains "alpha"
    And GraphTab for "alpha" is shown
    And Dashboard IndexProgress is not shown for a new job

  Scenario: Dashboard Reindex starts a job for the existing project
    Given list_projects returns one project named "alpha" with root_path "/tmp/alpha"
    When the operator activates the control named "Reindex" on the "alpha" row
    Then POST /api/index is sent with JSON {"root_path":"/tmp/alpha","project":"alpha"}
    And the response is HTTP 202
    And Dashboard shows IndexProgress
    And the URL query does not include "tab=graph"
    And list_projects still contains exactly one project named "alpha"

  Scenario: MCP reindex of the single owner is allowed
    Given list_projects returns one project named "alpha" with root_path "/tmp/alpha"
    When index_repository is called with repo_path "/tmp/alpha" and no name override
    Then the tool result is not isError
    And list_projects still contains exactly one project named "alpha"
    And no second cache store is created for that Path

  Scenario: Legacy same-Path clones show conflict and Enter opens newest
    Given list_projects returns
      | name      | root_path  | indexed_at           |
      | alpha-old | /tmp/alpha | 2026-08-28T10:00:00Z |
      | alpha     | /tmp/alpha | 2026-08-29T10:00:00Z |
    When the operator opens Dashboard
    Then a conflict region contains "alpha-old" and "alpha"
    And the conflict region contains a visible datetime derived from "2026-08-28T10:00:00Z"
    And the conflict region contains a visible datetime derived from "2026-08-29T10:00:00Z"
    When the operator activates Enter for that conflict group
    Then the URL query includes "tab=graph" and "project=alpha"
    And GraphTab for "alpha" is shown

  Scenario: Confirm delete older removes only the older clone
    Given list_projects returns "alpha-old" at "/tmp/alpha" indexed_at "2026-08-28T10:00:00Z" and "alpha" at "/tmp/alpha" indexed_at "2026-08-29T10:00:00Z"
    And Dashboard shows the conflict region
    When the operator activates Delete on "alpha-old" and accepts the confirm dialog
    Then DELETE /api/project?name=alpha-old is sent
    And after the list refreshes the document does not contain a project row named "alpha-old"
    And the document still contains a project row named "alpha"

  # Limit Cases
  Scenario: Limit Case — trailing slash is the same Path
    Given list_projects returns one project named "alpha" with root_path "/tmp/alpha"
    When POST /api/index is sent with JSON {"root_path":"/tmp/alpha/"}
    Then the response is HTTP 409 with JSON code "path_exists" and existing_project "alpha"
    And no index job slot is allocated

  Scenario: Limit Case — three clones enter newest and delete is per older
    Given list_projects returns
      | name | root_path  | indexed_at           |
      | a1   | /tmp/alpha | 2026-08-27T10:00:00Z |
      | a2   | /tmp/alpha | 2026-08-28T10:00:00Z |
      | a3   | /tmp/alpha | 2026-08-29T10:00:00Z |
    When the operator activates Enter for that conflict group
    Then the URL query includes "project=a3"
    And the conflict region has a Delete control for "a1" and a Delete control for "a2"
    And the document does not show a single control that deletes both "a1" and "a2" in one request

  Scenario: Limit Case — indexed_at tie uses greater name
    Given list_projects returns "alpha" at "/tmp/alpha" indexed_at "2026-08-29T10:00:00Z" and "beta" at "/tmp/alpha" indexed_at "2026-08-29T10:00:00Z"
    When POST /api/index is sent with JSON {"root_path":"/tmp/alpha"}
    Then the response is HTTP 409 with existing_project "beta"

  Scenario: Limit Case — MCP name override on a new Path with a free name starts index
    Given list_projects returns {"projects":[]}
    When index_repository is called with repo_path "/tmp/newrepo" and name "custom"
    Then the tool result is not isError
    And a project named "custom" is the one being indexed

  Scenario: Limit Case — create modal still has no Project ID field
    Given the create-index modal is open
    Then the document does not contain the accessible name "Project ID (optional — permanent, cannot be renamed)"
    And POST /api/index bodies from this modal have no "project" property
    And POST /api/index bodies from this modal have no "project_name" property

  Scenario: Limit Case — Reindex of a custom-named project keeps that name
    Given list_projects returns one project named "custom" with root_path "/tmp/newrepo"
    When the operator activates the control named "Reindex" on the "custom" row
    Then POST /api/index is sent with JSON {"root_path":"/tmp/newrepo","project":"custom"}
    And the response is HTTP 202
    And list_projects still contains exactly one project named "custom"

  # Error Scenarios
  Scenario: Error — MCP name override clones an existing Path
    Given list_projects returns one project named "alpha" with root_path "/tmp/alpha"
    When index_repository is called with repo_path "/tmp/alpha" and name "alpha-alias"
    Then the tool result is isError
    And the error text contains "path_exists"
    And the error text contains "alpha"
    And list_projects still contains exactly one project named "alpha"

  Scenario: Error — derived name exists on a different Path
    Given list_projects returns one project named "foo" with root_path "/tmp/a/foo"
    When the operator activates "Index This Folder" for "/tmp/b/foo"
    Then POST /api/index returns HTTP 409 with JSON code "name_exists" and existing_project "foo"
    And the create-index modal stays open
    And the document contains a visible error derived from the response
    And the URL query does not include "project=foo"

  Scenario: Error — delete older cancelled keeps the conflict
    Given Dashboard lists "alpha-old" and "alpha" as a same-Path conflict
    When the operator activates Delete on "alpha-old" and dismisses the confirm dialog
    Then no DELETE request to "/api/project?name=alpha-old" is sent
    And the conflict region still contains "alpha-old" and "alpha"

  Scenario: Error — in-flight index of the same Path refuses a second create
    Given an index job is running for canonical Path "/tmp/alpha" under project "alpha"
    When POST /api/index is sent with JSON {"root_path":"/tmp/alpha"}
    Then the response is not HTTP 202 for a second job
    And the response is HTTP 409 with JSON code "path_exists" and existing_project "alpha"

  Scenario: Error — Dashboard Reindex failure stays on Dashboard
    Given list_projects returns one project named "alpha" with root_path "/tmp/alpha"
    And POST /api/index will return HTTP 500
    When the operator activates the control named "Reindex" on the "alpha" row
    Then a visible error region is non-empty
    And the URL query does not include "tab=graph"
    And the document still contains a project row named "alpha"
```

## Success Metrics
| Metric | Target | Current | Status |
| Second identity for an owned Path | 0 new rows | name-keyed, override allowed | not met |
| Create-existing-Path UI outcome | Graph of existing + notice | starts another job | not met |
| Dashboard Reindex | 202, same name, IndexProgress, stay home | create modal is only UI path | not met |
| Legacy same-Path clones | conflict + enter newest + confirm-delete | two rows, no grouping | not met |
| name_exists different Path | 409, modal stays, no redirect | derived-name overwrite / second job | not met |
Primary KPI: first metric plus redirect-on-create and confirm-gated older delete.

## Constraints & Assumptions
Technical:
- Identity check uses existing `cbm_canonical_path` (POSIX realpath / Windows final path). Equality is strcmp of those strings. No extra Unicode/case-fold layer on macOS.
- `indexed_at` remains the newest signal (grill ADR-010). Do not add a second freshness field.
- Prefer extending POST `/api/index` + `index_repository` + `list_projects` consumers. New endpoints only if architect proves 409 + list grouping cannot meet AC.
- Store may stay name-keyed on disk; uniqueness is enforced at create/reindex admission. A UNIQUE `root_path` migration is an architect call, not required by Gherkin if admission + list grouping hold.
- graph-ui Vitest + C tests for daemon/HTTP/MCP admission. Every Gherkin maps to at least one test.
- Chrome stays spec-001 grayscale. GraphTab `colorForLabel` hex unchanged.
- Delete remains confirm-gated (constitution VI.3).

Business:
- Grill epic 003 only. ADR parse-on-reindex is spec-004.
- Existing custom names are not bulk-renamed.

Planner defaults (reject a Gherkin scenario to change these):
1. Create-index modal on an owned Path always redirects (US-001). It never starts a reindex. Bare POST `{root_path}` on an owned Path is 409 even if the derived name matches.
2. Dashboard row Reindex POSTs `{root_path, project}` and stays on Dashboard with IndexProgress. No workspace Reindex control.
3. MCP `name` allowed only when Path is new and name is free, or when it matches the owner (reindex). MCP without `name` on an owned Path is reindex.
4. Redirect is immediate Graph + status notice. No extra confirm step.
5. Three-plus clones: enter newest; delete each older separately.
6. Decline delete: conflict stays on Dashboard until one name remains.
7. Conflict copy: all names + all last-indexed datetimes + Path once.
8. name_exists (different Path) ≠ path_exists. No workspace redirect.
9. Newest = `indexed_at` string compare; tie = greater `name`.
10. Concurrent guard key = canonical Path.
11. After delete older, leftover `.db` / watcher cleanup is existing `delete_project` behavior; architect must not leave the older name watchable.

## Out of Scope
- Recolor 3D nodes/edges or redesign GraphTab / Specs Kanban
- ADR parse of context_ai.md + TECH_STACK.md + ARCHITECTURE_ADR.md — spec-004
- Workspace-header Reindex control
- Bulk rename of existing custom project names
- Silent auto-delete of older clones
- Merging two SQLite stores into one
- Writing back to `.sdd-skill/` or source
- Changing Control polls, HealthDot, or spec-001/002 chrome/routing except redirect + conflict UI
- Case-insensitive Path equality beyond `cbm_canonical_path`

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% on touched graph-ui + C admission tests [ ] review approved [ ] human docs confirmed [ ] TD-002 AC met [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- Admission gate shared by POST `/api/index` and `index_repository` (same helper as workspace-boundary sharing)
- Canonical Path as job/mutation key
- 409 JSON: `error`, `code` (`path_exists` | `name_exists`), `existing_project`, `indexed_at` when known
- Reindex vs create: `project` present and matching an existing owner of that Path → 202; absent → create admission (409 if owned)
- Dashboard groups `list_projects` by canonical Path (client canonicalize must match server, or server lists a group id — architect chooses)
- i18n en+zh for notice, conflict, name_exists, Reindex
- Constitution IV.1 must be rewritten at close (store key vs Path uniqueness)

## Questions for Architect (answered in plan.md)
- Enforce UNIQUE `root_path` in SQLite vs admission-only
- How UI canonicalize matches C (`cbm_canonical_path`) without a new endpoint
- Exact MCP isError string vs structured content
- Whether in-flight same-Path 409 uses `path_exists` or a distinct busy code (Gherkin uses `path_exists`)
- Field name `project` vs reuse `project_name` for reindex admission (Gherkin uses `project` so create modal stays path-only)
- Watcher + leftover `.db` after delete older

## Questions for @implementer
- [ ] Map every Gherkin scenario to a test (C for 409/MCP; Vitest for redirect/conflict/delete)
- [ ] Do not send `project_name` from CreateIndexModal
- [ ] Do not silent-delete older clones
- [ ] Do not redirect on `name_exists`
- [ ] Do not change `colorForLabel` hex
- [ ] CreateIndexModal must not send `project` or `project_name`
- [ ] Dashboard Reindex must send `{root_path, project}` and must not navigate to Graph on 202

## Related Specs
Depends on: spec-001-w3q-executive-dashboard (create modal path-only, Dashboard list, last-indexed), spec-002-p8w-project-workspace (Enter → Graph workspace)
Blocks: none required; spec-004 needs a working reindex of the single owner (US-002)
Supersedes: constitution IV.1 "Path uniqueness is out of spec-001" — this spec owns it
Companion to: .grill/plans/executive-ui-ia/epics/epic-003-path-project-identity.md (ADR-001, ADR-009, ADR-010, ADR-012)

## Revision History
| Date | Author | Change |
| 2026-08-29 | @planner | draft from grill epic-003 + ADR-001/009/010/012; debt=TD-002 |
| 2026-08-29 | @planner | User approved + added Dashboard Reindex (US-007). Status=approved. Handoff @architect. |

## Approval Sign-off
Spec Owner(@planner): approved 2026-08-29 / Gherkin scenarios(user): approved 2026-08-29 (plus Reindex) / Product Owner: approved via Gherkin / Architecture(@architect): pending
