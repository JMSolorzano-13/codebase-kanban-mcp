# Work Breakdown — Spec-012: Game expand, archive, and deps
Total Tasks: 5 / Estimated Total Effort: 19h

## Task Dependency Graph
```
#1 store game_archive ──► #3 HTTP POST/GET merge + C Gherkin + publish copy
#2 C expand parse + blocked overlay + JSON ──► #3
                                            └──► #4 GameBoardTab expand + blocked strip + i18n
                                                 └──► #5 Archive UI + refresh + remaining Vitest
```
#1 and #2 are parallel. #4 may start after #2 JSON field names exist (types from plan). Critical path: #2 (5h) → #3 (4h) → #4 (3.5h) → #5 (3.5h) = 16h plus #1 (3h, hidden under #2).

## Tasks

### Task #1 — Store game_archive table + set/load/copy
Definition of Done:
- [x] `init_schema` DDL includes `CREATE TABLE IF NOT EXISTS game_archive (card_id TEXT PRIMARY KEY, archived INTEGER NOT NULL CHECK (archived IN (0, 1)), updated_at TEXT NOT NULL)`
- [x] `cbm_game_archive_row_t` in `store.h`: `char card_id[256]`, `int archived` (0/1)
- [x] `CBM_GAME_ARCHIVE_CAP` 512
- [x] `cbm_store_game_archive_set(store, card_id, archived)` UPSERT; empty/NULL card_id → `CBM_STORE_ERR`; card_id longer than 255 → ERR; no DELETE on 0
- [x] `cbm_store_game_archive_load(store, out, cap, &count)` — missing table (`sqlite_master` probe, same as spec_archive) → count 0, `CBM_STORE_OK`
- [x] `cbm_store_game_archive_copy(src, dst)` — load src, set each row on dst; missing src table → OK no-op
- [x] Do not reuse `spec_archive` / `store_meta` / `project_summaries`. Do not add the table to `sqlite_writer.c`
- [x] `tests/test_store_game_archive.c` added to `TEST_STORE_SRCS`; memory-store tests cover Then clauses below
- [x] tests pass locally; breadcrumbs
User Stories Addressed: US-003 (persist shape)
Gherkin covered (unit-level Then):
- Limit — orphan flag: row remains after load of a board that does not contain that id (copy/load still returns the row; invent-card is HTTP)
- Limit — repeat archive is idempotent (set true twice → one row archived=1)
Dependencies: None
Estimated Effort: 3h
Subagent: yes
Path: full
Implementation Notes: SDD-ADR-052. Do not touch HTTP, pipeline, game_board.c, or graph-ui. Store does not include `game_board.h`. `updated_at` via existing `iso_now`. Mirror `spec_archive_table_probe` as `game_archive_table_probe`.

### Task #2 — C expand parse + blocked overlay + JSON
Definition of Done:
- [x] `cbm_game_board_card_t` widened: `blurb[512]`, `inputs[512]`, `last_decision[512]`, `open[512]`, `recent[512]`, `blocked_by[512]`, `archived` bool (read leaves archived 0), `tasks[48]` of `{number, name[192], done}`, `task_count`
- [x] `cbm_game_board_t` has `blocked[16]` of `{owner[48], task[256], blocked_by[512]}` + `blocked_count`. Cap 16. No `has_more`
- [x] Track A / B / H / playtest / Inbox parse per plan.md "Expand parse". fopen `"rb"` only. Missing file degrades that card
- [x] `## What it does` wins over header `open` / `last_decision` for blurb. Fallback skips `none` / `—`
- [x] Tasks: checkbox dialect only; cap 48; no `has_more`; skip Subagent:/Path:/HTML comments/empty
- [x] Blocked strip from state.md `slug:blocked:"task":"blocked-by"` document order. `needs_review`/`in_progress`/`done` not rows
- [x] Overlay in C: artifact cards whose `owner` equals strip `owner` (`@`+slug) get `work_state` `"blocked"` and `blocked_by` text. Inbox never overlaid
- [x] `cbm_game_board_to_json` emits additive card keys + top-level `blocked` (always an array). Empty `blocked_by` → JSON null. `archived` false until HTTP merge
- [x] GET `/api/spec-board` still has no `gamedev_skill_present`. No POST this task. Do not edit `spec_board.c` / `mcp.c`
- [x] `tests/test_game_board.c` cover Then clauses below; skill-tree bytes around GET
- [x] tests pass; breadcrumbs
User Stories Addressed: US-002, US-005 (C/JSON half)
Gherkin covered:
- Track A SYS blurb/tasks/inputs (JSON)
- Track B gdd last_decision/open, blurb "", tasks []
- Limit — What it does wins over header open
- Limit — playtest recent is the last Round only
- Limit — level recent is the last 8 changelog lines
- Limit — 48 task cap omits the 49th; no has_more
- Limit — needs_review is not a strip row
- Limit — empty blocked JSON `[]`
- blocked overlay: owner match → work_state blocked + blocked_by; other owner pending + null
- Error — GET does not write skill trees (C bytes; HTTP POST bytes are Task #3)
Dependencies: None
Estimated Effort: 5h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-055, SDD-ADR-056. Export extract helpers for unit tests if cheaper than a full tree. Parse blocked from the same state.md buffer as chrome (extend `parse_state_md` or a sibling). Overlay after `fill_artifacts`, before return. Do not fopen `roadmap.md` for deps. Inbox expand fields stay empty.

### Task #3 — HTTP POST + GET merge + C Gherkin + publish copy
Definition of Done:
- [x] `dispatch_request`: `/api/game-board*` GET → get handler; POST → post handler
- [x] GET: after `cbm_game_board_read`, query-open + `cbm_store_game_archive_load` + apply matching ids only; missing table/open fail → all false, 200
- [x] POST body max 4096; yyjson; `project`+`card_id` strings required; `archived` must be JSON bool
- [x] POST errors: exact strings in plan.md API table (409 no persist; 404 card/project; 400 invalid archived)
- [x] Inbox epic id → 404 `card not found`. Overlay `work_state` blocked → 409 `card not done`
- [x] POST 200 `{"project":"...","card_id":"...","archived":true|false}` (escaped); idempotent
- [x] Mutation lock on POST like `handle_spec_board_post` (423 if busy)
- [x] POST 200 leaves `.gamedev/`, `.sdd-skill/`, `.grill/` byte-identical
- [x] `cbm_pipeline_publish_staged` calls `cbm_store_game_archive_copy` from the same live query-open as spec_archive; missing live db/table is OK; copy fail fails publish. Do not change `adr_fill`
- [x] POST `/api/spec-board` with a Game card_id still 404 `spec not found` and does not set game_archive
- [x] `test_httpd.c` (+ board JSON if needed) cover C Gherkin rows below
- [x] Zero skill writes. No MCP. Do not edit `spec_board.c`
- [x] tests pass; breadcrumbs
User Stories Addressed: US-003, US-006 (C/HTTP)
Gherkin covered:
- Archive POST 200 JSON has card_id + archived true (HTTP half)
- Limit — Inbox POST epic is 404
- Limit — leftover archived on pending JSON may be true (GET merge)
- Limit — orphan game_archive row does not invent a card
- Limit — repeat archive is idempotent
- Error — archive a pending artifact is 409 and writes nothing (+ skill-file bytes)
- Error — archive a blocked overlay card is 409
- Error — unknown card_id is 404
- Error — missing project on POST is 400
- Error — invalid archived on POST is 400
- Error — unknown project on POST is 404
- Error — POST spec-board with a Game card_id does not archive
Dependencies: Task #1, Task #2
Estimated Effort: 4h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-053. Find card across four arrays; kind epic → 404 even if somehow done. Do not return full board JSON on POST. Fixtures in `/tmp` only.

### Task #4 — GameBoardTab expand + blocked strip + i18n
Definition of Done:
- [x] Title control on artifact and Inbox cards (`<button>` + `aria-expanded`). Not a modal/popover/side panel
- [x] `expandedIds` Set keyed by GET `id`. All start collapsed. Multi-open. Project change / remount clears
- [x] Continue `stopPropagation` stays; activating continue does not toggle expand
- [x] Chrome `/gamedev-skill continue` `<p>` is not an expand control
- [x] Track A expand: blurb, `#N name` for every task (or English "No tasks planned yet"), Inputs region only when `inputs` non-empty
- [x] Track B: last_decision + open when non-empty; no "No tasks planned yet"; no Inputs
- [x] Inbox expand: summary + plan_title only; no Archive/Unarchive; no "No tasks planned yet"
- [x] Non-null `blocked_by` shown collapsed and expanded with prefix "Blocked"
- [x] Non-empty `board.blocked` → region named "Blocked" above columns, below spec-010 chrome. Click does nothing. Empty → region omitted
- [x] i18n: reuse specBoard archive/unarchive/showArchived/noTasksYet; add gameBoard.inputs + blockedStrip en+zh
- [x] `parseGameBoard` additive fields; missing `archived` → false; missing `blocked` → `[]`; skip invalid kind
- [x] `useGameBoard` still one-shot. Archive POST / Show archived / App project+refresh are Task #5
- [x] Invert spec-011 tests that lock title/card-activate never expands. Keep body-click (not title) does not expand; no-drag; clipboard
- [x] `SpecBoardTab.tsx` / `useSddSkillPresent` / `useSpecBoard` not edited
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-005 (paint)
Gherkin covered:
- Track A SYS card expands (pane Thens)
- Track B gdd expand (pane)
- blocked strip and overlay (pane Thens)
- Limit — Inbox expand has no Archive (UI half)
- Limit — empty Track A tasks still expands
- Limit — missing Inputs heading omits the region
- Limit — two cards stay expanded and continue does not toggle
- Limit — empty blocked omits the strip
Dependencies: Task #2
Estimated Effort: 3.5h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-055, SDD-ADR-057. Show archived control may exist unpressed this task but hide/filter Gherkin is Task #5. Do not POST this task. LVL/playtest expand bodies: if JSON `recent` is set, show it in expand (needed for completeness even if Gherkin GET-owned).

### Task #5 — Archive UI + refresh + remaining Vitest
Definition of Done:
- [x] `useGameBoard.refresh` is `() => Promise<void>` (same GET). Must not set `settled` or `present` to false
- [x] App passes `project` + `refresh` into `GameBoardTab`
- [x] Show archived in Game pane chrome (not Inbox header, not a fourth column). `aria-pressed` false on first paint / project change / remount. No localStorage. GET refetch after POST does not reset the toggle
- [x] Phase filter: hide `work_state==="done" && archived && !showArchived`. Inbox never filtered. Leftover archived on pending: show; no Archive/Unarchive
- [x] Archive only expanded done && !archived. Unarchive only expanded done && archived. POST `/api/game-board` `{project, card_id, archived}`. No dialog / `window.confirm`
- [x] After POST 200, `await refresh()`. Unarchive while hide still shows the card
- [x] All-archived phase column = header only; Show archived control remains
- [x] Every UI Gherkin Then not already asserted in #4 has a Vitest
- [x] spec-010/011 silent-win / Enter Graph / leftover `tab=specs`→game / no `/api/skill-presence` stay green
- [x] `colorForLabel("Function") === "#06b6d4"` still locked if that test is in the suite run
- [x] tests pass; breadcrumbs
User Stories Addressed: US-003, US-004, US-006 (UI)
Gherkin covered:
- Archive hides a done artifact without a confirm dialog (UI)
- Limit — Show archived is session-only and remount starts hidden
- Limit — Unarchive restores the card while the toggle is off
- Limit — leftover archived on pending does not hide (pane)
- Limit — all artifacts in a column archived leaves header only
- Any #4 Then still missing after implementation
Dependencies: Task #3, Task #4
Estimated Effort: 3.5h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-054. Stub `fetch` for POST. Mock refresh by resolving then rerendering with the GET payload. Do not add Playwright unless @tester later requires CERTIFICATION. Do not weaken silent win.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | Store game_archive + set/load/copy | 3h | None | done |
| 2 | C expand parse + blocked overlay + JSON | 5h | None | done |
| 3 | HTTP POST/GET merge + C Gherkin + publish copy | 4h | #1, #2 | done |
| 4 | GameBoardTab expand + blocked strip + i18n | 3.5h | #2 | done |
| 5 | Archive UI + refresh + remaining Vitest | 3.5h | #3, #4 | done |
Total: 19h

## Critical Path
#2 → #3 → #4 → #5 (16h). #1 parallel with #2. #4 may run after #2 in parallel with #3 once card JSON keys are stable (Archive POST still waits on #3).

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `game_archive` store + `game_board` + httpd Gherkin + `GameBoardTab` + `useGameBoard`. C owns table/merge/overlay/parse/409/404/400/idempotent/orphan/zero writes. Vitest owns expand/strip/archive/show-archived/unarchive/remount/continue/no-confirm/silent-win leftovers.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
