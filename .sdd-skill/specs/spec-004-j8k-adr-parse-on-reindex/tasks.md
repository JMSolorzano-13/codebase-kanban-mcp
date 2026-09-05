# Work Breakdown — Spec-004: ADR parse on reindex
Total Tasks: 4 / Estimated Total Effort: 13h

## Task Dependency Graph
```
#1 splice + extract helper ──► #2 pipeline hook + skip dir ──► #3 HTTP/MCP/watcher Gherkin
                         └──► #4 AdrTab stamp + warning
```
#4 is parallel after #1 (needs marker string only). Critical path: #1 (3h) → #2 (4h) → #3 (4h) = 11h if #4 finishes in the #2/#3 window.

## Tasks

### Task #1 — Splice + bounded extract helper
Definition of Done:
- [x] New `cbm_adr_*` module: extract trio, splice markers, no store / no HTTP
- [x] Markers exactly `<!-- CBM-GENERATED-START -->` / `END` and `<!-- CBM-MANUAL-START -->` / `END`; order generated then manual
- [x] Unmarked body → entire existing content becomes manual; generated prepended
- [x] Empty existing + skill present → generated filled; manual empty or whitespace
- [x] No `.sdd-skill/` dir → helper is a no-op (NULL / unchanged); no markers invented
- [x] Skill present, trio all missing → markers written; generated has no trio extracts
- [x] Per-file cap 1536 after 64KiB read; English H1s `# Purpose` `# Stack` `# Decisions`; omit H1 when that file is missing/empty/unreadable
- [x] DEV_LOG / TECH_DEBT / constitution / human / specs / history never opened
- [x] Unreadable = exists as regular file and open/read fails (`chmod 0` or directory-at-path fallback)
- [x] C tests cover the Then clauses listed below without running a full index
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-004
Gherkin covered (unit-level Then):
- migrate unmarked `# Existing ADR` into manual
- unique substrings from each readable trio file; SECRET-DEVLOG absent
- first-create empty manual
- partial context_ai only
- unreadable ARCHITECTURE_ADR omits that extract
- no `.sdd-skill` leaves unmarked
Dependencies: None
Estimated Effort: 3h
Subagent: yes
Path: full
Implementation Notes: Pattern is spec_board best-effort IO, not a copy of Kanban parse. No pipeline hook here. Do not raise HTTP limit here.

### Task #2 — Pipeline hook + `.sdd-skill` skip
Definition of Done:
- [x] `cbm_pipeline_new` leaves `adr_fill` false
- [x] After ADR capture, if `adr_fill`: replace `saved_adr` with `cbm_adr_fill_document`; then publish (full and incremental persist)
- [x] Fill OOM/error leaves prior `saved_adr`; pipeline rc still success
- [x] `handle_index_repository` in-process sets `adr_fill` true unless args JSON has `adr_fill: false`
- [x] Watcher job args include `"adr_fill": false`; `application_index_args_normalize_defaults` strips the key
- [x] `.sdd-skill` is in `ALWAYS_SKIP_DIRS`; trio files are not graph File nodes
- [x] C tests: pipeline/helper with flag true vs false; discover skip if an existing skip test pattern exists
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-003, US-005
Gherkin covered:
- watcher incremental does not fill (flag / watcher args)
- hook runs on user-triggered persist including incremental route
Dependencies: Task #1
Estimated Effort: 4h
Subagent: no
Path: full
Implementation Notes: Do not treat incremental route as skip. Do not fill in a post-job HTTP callback. Do not add `adr_fill` to the MCP tool schema. Do not wire HTTP job completion tests here if they need the full daemon — that is Task #3.

### Task #3 — HTTP + MCP + watcher Gherkin
Definition of Done:
- [x] Successful POST `/api/index` create and `{root_path, project}` Reindex fill the same store GET `/api/adr` reads
- [x] Successful `index_repository` fills; `manage_adr` get matches GET `/api/adr`
- [x] No `.sdd-skill/`: content unchanged, no `CBM-GENERATED`
- [x] Partial trio + unreadable sibling: job success; omitted extract absent; siblings present
- [x] `manage_adr` update of manual survives the next user-triggered index; generated refreshes
- [x] POST `/api/adr` generated-region edit is replaced on next Reindex; `PURPOSE-CANONICAL` wins
- [x] POST `/api/adr` body max is 32768 (16384 of generated+manual still 200)
- [x] Watcher / `adr_fill: false` path does not insert markers or trio strings
- [x] Existing `tool_index_repository_reports_store_backed_adr` still passes
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-003, US-004, US-005
Gherkin covered:
- Dashboard Reindex fills + migrates unmarked
- MCP index_repository fills the same ADR blob
- First create-index generated + empty manual
- Limit — no `.sdd-skill` leaves ADR unmarked
- Limit — only context_ai.md; job still succeeds
- Limit — watcher incremental does not fill ADR
- Error — unreadable ARCHITECTURE_ADR omits extract; job success
- Error — manage_adr manual edit survives the next Reindex
- Error — generated-region edits are replaced on the next Reindex
Dependencies: Task #2
Estimated Effort: 4h
Subagent: no
Path: full
Implementation Notes: Reuse `test_httpd.c` ADR helpers and `test_mcp.c` handle_tool style. Do not add a Dashboard ADR control. Do not change spec-003 409/202 admission.

### Task #4 — AdrTab generated-at + replace warning
Definition of Done:
- [x] When GET content includes `CBM-GENERATED-START`, AdrTab shows a visible datetime from `Project.indexed_at` via `formatIndexedAt` + `useProjects` list cache (`<time dateTime>`)
- [x] Markers absent → that stamp omitted
- [x] Visible warning (i18n en+zh) that generated-region edits are replaced on the next user-triggered index
- [x] One textarea of the whole blob; Save still POSTs `{project, content}`; no Dashboard ADR control
- [x] `window.confirm` dirty-leave unchanged
- [x] `AdrTab.test.tsx` (+ i18n.test) cover the Then clauses below
- [x] `colorForLabel("Function")` still `#06b6d4` if that test is in the suite run
- [x] tests pass; breadcrumbs
User Stories Addressed: US-006
Gherkin covered:
- ADR tab shows generated-at and replace warning
Dependencies: Task #1
Estimated Effort: 2h
Subagent: yes
Path: full
Implementation Notes: No second freshness field. Stamp is chrome, not a line in the blob. Generated H1s inside the textarea are English (server). Do not split the editor into two textareas.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | Splice + extract helper | 3h | None | done |
| 2 | Pipeline hook + skip dir | 4h | #1 | done |
| 3 | HTTP/MCP/watcher Gherkin | 4h | #2 | done |
| 4 | AdrTab stamp + warning | 2h | #1 | done |
Total: 13h

## Critical Path
#1 → #2 → #3 (11h). #4 parallel after #1.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Blocked: note in state.md, switch task, tell @architect.

## Test Coverage Requirements
Every Gherkin scenario mapped above. Target >80% on touched C fill + graph-ui AdrTab/i18n. C tests for hook, migrate, skip-no-skill, skip-watcher, partial, unreadable, MCP parity.

## Success Criteria for All Tasks
- [x] all DoD complete [x] tests>80% (review estimates) [x] passes @review [x] @tester approves [x] human docs complete
