# Work Breakdown — Spec-008: Grill epic Todo
Total Tasks: 4 / Estimated Total Effort: 11h

## Task Dependency Graph
```
#1 spec_board grill read + JSON ──► #2 HTTP GET/POST C Gherkin (bytes + 404)
                                 └──► #3 SpecBoardTab EpicCard + Todo order
                                      └──► #4 Vitest Gherkin mapping
```
#3 may start after #1's JSON field names exist (types from plan). Critical path: #1 (4h) → #2 (2h) → #3 (3h) → #4 (2h) = 11h.

## Tasks

### Task #1 — spec_board grill read + additive JSON
Definition of Done:
- [x] `CBM_SPEC_BOARD_MAX_EPICS` 64 in `spec_board.h` next to spec/task caps
- [x] `cbm_spec_board_epic_t`: `kind` conceptually epic; `id[256]`, `title[256]`, `summary[512]`, `plan_title[256]`, `column[]` always `"todo"`
- [x] `cbm_spec_board_t` has `bool grill_skill_present`, `epics[CBM_SPEC_BOARD_MAX_EPICS]`, `int epic_count`
- [x] Internal conversion scratch is not JSON: listed spec `Companion to:` token + `active.json` `source.grill_epic`
- [x] `cbm_spec_board_grill_skill_present` (or inline `cbm_is_dir`) true iff `root/.grill` is a directory
- [x] `cbm_spec_board_read` fills grill even when sdd is absent; sdd fill remains as today; fopen `"rb"` only; never writes
- [x] index.md GFM table row order + title; unlisted plan dirs after, slug asc; within plan, `epic-NNN-*.md` by NNN; other names skipped
- [x] plan_title = index title else plan.md frontmatter `title:` else slug
- [x] title = epic.md `name:`; summary = epic.md `summary:` (not blurb extract)
- [x] Converted omit: exact `strcmp` Companion-to first `.grill/plans/`…`.md` OR `source.grill_epic`; trailing notes after `.md` ignored; Done/in_progress claim still omits
- [x] Unreadable epic skipped; cap 64 after omit; no `has_more` key; spec cap 64 unchanged
- [x] `cbm_spec_board_to_json` emits `"grill_skill_present"` and `"epics":[{kind,id,title,summary,plan_title,column}]`; spec objects have no `"kind"`
- [x] Dirent names with `/`, `\`, or `..` rejected; Companion-to token is never fopen'd
- [x] Do not read `.gamedev/` here; do not emit `gamedev_skill_present`
- [x] `tests/test_spec_board.c` covers Then clauses below; existing to_json tests accept additive keys (empty epics when no `.grill/`)
- [x] tests pass locally; breadcrumbs
User Stories Addressed: US-001, US-002, US-004, US-005, US-006 (reader half)
Gherkin covered (unit-level Then):
- Mixed Todo JSON: unconverted epic fields + kind epic + column todo
- Companion-to exact path omits
- active.json source.grill_epic omits
- Companion-to trailing notes still match
- Done spec claim still omits from epics
- two plans: index.md then NNN (struct order)
- closed plan leftover stays
- pending and detailed both listed
- missing Companion-to keeps epic
- 65th omitted; specs length still 64; JSON has no has_more
- no .grill → flag false, epics []
- unreadable epic skipped
- GET-equivalent read leaves fixture bytes identical (index.md / epic.md / active.json)
Dependencies: None
Estimated Effort: 4h
Subagent: yes
Path: full
Implementation Notes: All in `spec_board.c` / `.h` (SDD-ADR-036). Do not touch HTTP, graph-ui, MCP, store. Heap calloc in tests (already). Stop appending at 64. Fixtures `/tmp` only. Reuse `read_whole_file` / `kv_extract_colon_line`. Do not add Makefile sources.

### Task #2 — HTTP GET additive + POST epic-id 404
Definition of Done:
- [x] `handle_spec_board_get` still: read → archive merge on specs only → `to_json` (no grill IO in HTTP)
- [x] GET 200 body includes `grill_skill_present` + `epics` as Task #1
- [x] GET unknown project still 404 `{"error":"project not found"}`
- [x] POST with `spec_id` equal to an epic `id` that is not in `specs[]` → 404 `{"error":"spec not found"}`; `spec_board_find` unchanged (specs only)
- [x] That POST does not call `cbm_store_spec_archive_set`; epic.md and active.json byte-identical
- [x] GET 200 leaves `.grill/index.md`, epic.md, and `.sdd-skill/specs/active.json` byte-identical
- [x] `test_httpd.c` covers C HTTP Gherkin rows below
- [x] Zero skill writes; no new route; no MCP
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001 (HTTP), US-006
Gherkin covered:
- Mixed Todo GET 200 JSON (HTTP half of happy path)
- Error — POST archive with an epic id is 404 and writes nothing
- Error — unknown project on GET is still 404
- Error — GET does not write skill trees
Dependencies: Task #1
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Prefer proving existing dispatch + new JSON over rewriting GET. Do not fopen `.grill/` in `http_server.c`. Do not add a distinct error string for epics. Archive merge must not set fields on `epics[]`.

### Task #3 — SpecBoardTab EpicCard + Todo epics-then-specs
Definition of Done:
- [x] `SpecBoardEpic` type: `kind: "epic"`, `id`, `title`, `summary`, `plan_title`, `column: "todo"`
- [x] `SpecBoard` has `grill_skill_present?: boolean` (missing = false) and `epics?: SpecBoardEpic[]` (missing = [])
- [x] `--color-epic-mark: #7d8ec9` in `globals.css` `@theme inline`; EpicCard letter class uses `text-[var(--color-epic-mark)]`
- [x] Letter in the DOM is the single character `E` (not i18n, not the word Epic, not a filled pill/badge)
- [x] Todo column document order: epic cards, then spec cards with column todo; In progress and Done render zero EpicCards
- [x] EpicCard always shows title, summary, plan_title, and id text (Gherkin id Then); no title control that expands tasks/blurb/Archive/Unarchive
- [x] Activating the epic card does not send POST `/api/spec-board`
- [x] Spec cards keep spec-005 expand and spec-006 Archive/Unarchive
- [x] `useSddSkillPresent` / poll interval / `formatIndexedAt` / `colors.ts` not edited
- [x] Host: missing `epics` does not throw; not-sdd-skill copy still shows when `sdd_skill_present` false even if grill true
- [x] Component tests for paint order + no expand/archive (full table is Task #4)
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-003, US-004, US-006 (tab presence)
Gherkin covered (implementation + partial tests):
- Mixed Todo paints epic then spec (UI half)
- Limit — epic card does not expand or archive
- Limit — Specs tab still requires sdd_skill_present (board host when sdd false)
Dependencies: Task #2
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: Keep EpicCard in SpecBoardTab.tsx (no new CSS file). Todo `pendingCount` includes epics + spec todos. Three Column children only. Do not change WorkspaceTabStrip.

### Task #4 — Vitest Gherkin mapping remaining UI scenarios
Definition of Done:
- [x] `SpecBoardTab.test.tsx` maps every UI Gherkin Then below
- [x] Existing host tests: no `selectProject` when `project` is set; loading / not-sdd-skill still hold with `grill_skill_present: true`
- [x] Document order of card id texts matches the two-plan Gherkin
- [x] Done / In progress do not show epic id text
- [x] No control named "Has more"; `colorForLabel("Function") === "#06b6d4"` still locked if that test is in the suite run
- [x] Click/activate epic card: no Archive/Unarchive, no "No tasks planned yet", `fetch` POST not called
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001–US-006 (UI Thens)
Gherkin covered:
- Mixed Todo paints an unconverted epic then a planned spec (card texts E, title, summary, plan, ids, order)
- Companion-to omit: Todo does not show epic id (when mock epics already omitted — UI does not re-match)
- Done spec claiming epic: Done shows spec id, not epic id; In progress has no epic id
- Limit — two plans follow index.md then epic-NNN then specs (document order)
- Limit — missing Companion-to shows both cards
- Limit — 65th omitted in mock of 64 epics; no Has more
- Limit — epic card does not expand or archive
- Limit — Specs tab still requires sdd_skill_present
Dependencies: Task #3
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: English assertions. Conversion is C-owned; UI tests mock already-filtered `epics`. Do not boot GraphTab/Three. Do not add Playwright unless @tester later requires CERTIFICATION. Do not weaken host not-sdd-skill: grill flag true + sdd false → tab copy / strip still hide Specs.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | spec_board grill read + additive JSON | 4h | None | complete |
| 2 | HTTP GET additive + POST epic-id 404 | 2h | #1 | complete |
| 3 | SpecBoardTab EpicCard + Todo order | 3h | #2 | complete |
| 4 | Vitest Gherkin mapping | 2h | #3 | complete |
Total: 11h

## Critical Path
#1 → #2 → #3 → #4 (11h). #3 can stub types from plan.md before #2 lands if HTTP is tests-only, but JSON contract must match Task #1.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `spec_board` + httpd Gherkin + `SpecBoardTab.tsx`. C owns parse/match/order/cap/skip/bytes/POST 404. Vitest owns Mixed Todo paint, E token, no expand/archive, column isolation, has-more absent, tab still sdd-only.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
