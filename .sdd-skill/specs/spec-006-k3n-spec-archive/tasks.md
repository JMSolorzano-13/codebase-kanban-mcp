# Work Breakdown — Spec-006: Spec archive
Total Tasks: 4 / Estimated Total Effort: 12h

## Task Dependency Graph
```
#1 store spec_archive + set/load/copy ──► #2 HTTP POST/GET merge + C Gherkin + publish copy
                                       └──► #3 SpecBoardTab filter + Archive/Unarchive
                                            └──► #4 Vitest Gherkin mapping
```
#3 may start after #2's JSON field + POST contract exist (types can be stubbed from plan). Critical path: #1 (3h) → #2 (4h) → #3 (3h) → #4 (2h) = 12h.

## Tasks

### Task #1 — Store spec_archive table + set/load/copy
Definition of Done:
- [x] `init_schema` DDL includes `CREATE TABLE IF NOT EXISTS spec_archive (spec_id TEXT PRIMARY KEY, archived INTEGER NOT NULL CHECK (archived IN (0, 1)), updated_at TEXT NOT NULL)`
- [x] `cbm_spec_archive_row_t` in `store.h`: `char spec_id[192]`, `int archived` (0/1)
- [x] `cbm_store_spec_archive_set(store, spec_id, archived)` UPSERT; empty/NULL spec_id → `CBM_STORE_ERR`; no DELETE on 0
- [x] `cbm_store_spec_archive_load(store, out, cap, &count)` — missing table (`sqlite_master` probe, same idea as `cbm_store_adr_get`) → count 0, `CBM_STORE_OK`
- [x] `cbm_store_spec_archive_copy(src, dst)` — load src, set each row on dst; missing src table → OK no-op
- [x] `tests/test_store_spec_archive.c` added to `TEST_STORE_SRCS`; memory-store tests cover Then clauses below
- [x] tests pass locally; breadcrumbs
User Stories Addressed: US-005 (persist shape)
Gherkin covered (unit-level Then):
- Limit — orphan flag: row remains after load of a board that does not contain that id (copy/load still returns the row; invent-card is HTTP)
- Limit — repeat archive is idempotent (set true twice → one row archived=1)
Dependencies: None
Estimated Effort: 3h
Subagent: yes
Path: full
Implementation Notes: Do not touch HTTP, pipeline, or graph-ui. Do not reuse `store_meta` / `project_summaries`. Do not add the table to `sqlite_writer.c`. Cap load at 64 (board max) or a small constant ≥64. `updated_at` via existing `iso_now`. Do not include `spec_board.h` from store.

### Task #2 — HTTP POST + GET merge + C Gherkin + publish copy
Definition of Done:
- [x] `cbm_spec_board_entry_t` has `bool archived`; `cbm_spec_board_read` never sets it (stays 0)
- [x] `cbm_spec_board_to_json` emits `"archived":true|false` on every spec
- [x] `dispatch_request`: `/api/spec-board*` GET → get handler; POST → post handler
- [x] GET: after `cbm_spec_board_read`, query-open + `cbm_store_spec_archive_load` + apply matching ids only; missing table/open fail → all false, 200
- [x] POST body max 4096; yyjson; `project`+`spec_id` strings required; `archived` must be JSON bool
- [x] POST errors: exact strings in plan.md API table (409 no persist; 404 spec/project; 400 invalid archived)
- [x] POST 200 `{"project":"...","spec_id":"...","archived":true|false}` (escaped); idempotent
- [x] Mutation lock on POST like `handle_adr_save` (423 if busy)
- [x] `cbm_pipeline_publish_staged` calls `cbm_store_spec_archive_copy` from query-open `final_db_path` onto the stage store after ADR write; missing live db/table is OK; copy fail fails publish. Do not change `adr_fill`
- [x] `test_httpd.c` + `test_spec_board.c` cover C Gherkin rows below; `active.json` byte-identical around success and 409 POST
- [x] Zero skill writes (no fopen write under `.sdd-skill/`)
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001 (POST 200), US-005, US-006
Gherkin covered:
- GET keeps archived specs in Done with archived true
- Limit — orphan flag does not invent a card
- Limit — repeat archive is idempotent
- Error — archive a Todo spec is 409 and writes nothing (+ active.json bytes)
- Error — unknown spec_id is 404
- Error — missing project on POST is 400
- Error — invalid archived on POST is 400
- Error — unknown project on POST is 404
- Archive POST 200 JSON has spec_id + archived true (HTTP half of happy path)
Dependencies: Task #1
Estimated Effort: 4h
Subagent: no
Path: full
Implementation Notes: `handle_spec_board` may split get/post. Do not return full board JSON on POST. Do not add MCP. Fixtures in `/tmp` only. Leftover-true-on-todo JSON is GET merge (set flag in store, board column todo) — assert in httpd if cheap. Do not edit `adr_fill` / `saved_adr`.

### Task #3 — SpecBoardTab filter + session toggle + Archive/Unarchive
Definition of Done:
- [x] `SpecBoardEntry.archived: boolean` (missing treated as `false`)
- [x] i18n en+zh: `archive` "Archive" / "归档"; `unarchive` "Unarchive" / "取消归档"; `showArchived` "Show archived" / "显示已归档"
- [x] `useSpecBoard.refresh` is `() => Promise<void>` (same `fetchBoard`)
- [x] `showArchived` useState(false); project change clears it (with expandedIds reset); no localStorage
- [x] Done header only: control accessible name "Show archived", `aria-pressed` bound
- [x] Done entries shown = `column==="done"` and not (`archived && !showArchived`); header count = shown length; empty → existing `t.specBoard.noSpecs` ("No specs yet")
- [x] Archive only on expanded Done with `archived===false`; Unarchive only on expanded Done with `archived===true`; todo/in_progress expands have neither
- [x] Archive/Unarchive POST `/api/spec-board` JSON `{project, spec_id, archived}`; no dialog, alertdialog, or `window.confirm`
- [x] After POST 200, `await refresh()`; Unarchive while hide still shows the card (archived now false)
- [x] spec-005 expand Set / blurb / TaskList filter unchanged
- [x] Component tests for filter + toggle + Archive POST (full table is Task #4)
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-003, US-004
Gherkin covered (implementation + partial tests):
- Archive hides without confirm (UI half)
- Show archived + Unarchive
- Fresh visit hidden
- Done count ignores hidden
- leftover Todo flag does not hide / no Archive
- all Done archived empty copy + toggle remains
Dependencies: Task #2
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: Mock `useSpecBoard` and stub `fetch` for POST. Do not change poll interval. Do not touch `formatIndexedAt` or Graph colors. Do not add a fourth Column. Chrome stays grayscale tokens.

### Task #4 — Vitest Gherkin mapping remaining UI scenarios
Definition of Done:
- [x] `SpecBoardTab.test.tsx` maps every UI Gherkin Then below
- [x] Existing host tests: no `selectProject` when `project` is set; loading / not-sdd-skill still hold
- [x] Poll: after archive hide, rerender with GET payload `archived: true`; card stays hidden while toggle off
- [x] spec-005 "document has no Archive/Unarchive" replaced: those names exist only in the Done cases above
- [x] `colorForLabel("Function") === "#06b6d4"` still locked if that test is in the suite run
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001–US-004, US-006 (poll)
Gherkin covered:
- Archive hides a Done card without a confirm dialog
- Show archived reveals the card and Unarchive restores it
- Fresh visit starts with archived hidden
- Limit — Done count ignores hidden archived
- Limit — leftover flag on a Todo spec does not hide it
- Limit — all Done archived shows empty copy and keeps the toggle
- Limit — poll after archive does not resurrect the card
Dependencies: Task #3
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: English assertions. Click the title control that contains the spec id text, then Archive/Unarchive. Do not boot GraphTab/Three. Do not add Playwright unless @tester later requires CERTIFICATION.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | Store spec_archive + set/load/copy | 3h | None | done |
| 2 | HTTP POST/GET merge + C Gherkin + publish copy | 4h | #1 | done |
| 3 | SpecBoardTab filter + Archive/Unarchive | 3h | #2 | done |
| 4 | Vitest Gherkin mapping | 2h | #3 | done |
Total: 12h

## Critical Path
#1 → #2 → #3 → #4 (12h). No safe parallel after #1 (HTTP needs store APIs; UI needs POST contract).

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched store/HTTP/`spec_board` JSON + `SpecBoardTab.tsx`. C owns table/merge/409/404/400/idempotent/orphan. Vitest owns hide/show/unarchive/fresh/count/poll/no-confirm.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
