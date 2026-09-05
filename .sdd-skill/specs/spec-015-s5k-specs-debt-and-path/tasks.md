# Work Breakdown — Spec-015: Specs debt and path
Total Tasks: 4 / Estimated Total Effort: 11h

## Task Dependency Graph
```
#1 spec_board debt parse + JSON ──► #2 HTTP GET/POST C Gherkin (bytes + 404)
                                 └──► #3 SpecBoardTab strip + EpicCard wrap + i18n
                                      └──► #4 leftover Vitest locks
```
#3 may start after #1's JSON field names exist (types from plan). Critical path: #1 (4h) → #2 (2h) → #3 (3h) → #4 (2h) = 11h.

## Tasks

### Task #1 — spec_board debt parse + additive JSON
Definition of Done:
- [ ] `CBM_SPEC_BOARD_MAX_DEBT` 16 in `spec_board.h` next to spec/epic/task caps
- [ ] `cbm_spec_board_debt_t`: `id[32]`, `title[256]`
- [ ] `cbm_spec_board_t` has `debt[CBM_SPEC_BOARD_MAX_DEBT]`, `int debt_count`
- [ ] `cbm_spec_board_parse_tech_debt(const char *md, cbm_spec_board_debt_t *out, int *count)` exported; NULL/empty md → count 0; does not fopen
- [ ] `cbm_spec_board_read` calls `debt_fill` after grill fill: join `{root}/.sdd-skill/baseline/TECH_DEBT.md`, `read_whole_file` (`"rb"`); NULL → debt_count 0; else parse; never writes
- [ ] Heading id = `## ` + `TD-` + one or more digits; no id → skip block; GET-equivalent read still succeeds
- [ ] Title = trimmed remainder after `## TD-NNN:` on that line; not a `Title:` field; not the summary-table title cell
- [ ] Status: first `Status:` that is trimmed line-start or a `|` cell start, inside the heading block (until next `## `). Token until whitespace, `|`, or EOL. Case-sensitive `Status:`
- [ ] If no heading Status token, use `## Debt Summary` GFM table row for that id (`ID` + `Status` header cells; skip `---` rows). No that heading → no table map
- [ ] Include iff token `strcmp` ≠ `"resolved"`. Unknown/typo/empty/missing-both → open. Heading token wins over table
- [ ] Order = first `## TD-NNN` in file first. Duplicate id: first heading wins. Cap 16 open; 17th omitted; resolved does not consume a slot
- [ ] `cbm_spec_board_to_json` always emits `"debt":[{id,title}]` (empty array when count 0); never `has_more`; never severity/category/status
- [ ] Do not change `grill_epic_converted` / Companion-to / `source.grill_epic`. Do not read `.gamedev/`
- [ ] `tests/test_spec_board.c` covers Then clauses below; existing to_json tests accept additive `debt` (empty when no file)
- [ ] tests pass locally; breadcrumbs
User Stories Addressed: US-001, US-002, US-003, US-006 (reader half)
Gherkin covered (unit-level Then):
- Open heading item: debt length 1; id TD-005; title leftover cache; no has_more
- All-resolved → debt []
- Limit — missing TECH_DEBT.md → debt []
- Limit — heading Status wins when table says resolved
- Limit — table Status used when heading has no Status token
- Limit — unknown Status token is open
- Limit — 17th open omitted; no TD-017; no has_more
- Error — unreadable TECH_DEBT.md → debt []
- Error — heading without TD-NNN skipped; well-formed TD-005 kept
- Companion-to exact omit still holds (leftover; do not rewrite matcher)
- GET-equivalent read leaves fixture TECH_DEBT.md / active.json / epic.md bytes identical
Dependencies: None
Estimated Effort: 4h
Subagent: yes
Path: full
Implementation Notes: All in `spec_board.c` / `.h` (SDD-ADR-065, SDD-ADR-066). Do not touch HTTP, graph-ui, MCP, store, Makefile.cbm. Heap calloc in tests (already). Stop appending at 16 open. Fixtures `/tmp` only. Reuse `read_whole_file`. Unreadable = dir-at-path or fopen fail (same class as unreadable epic). Do not parse this repo's live TECH_DEBT.md.

### Task #2 — HTTP GET additive + POST leftover locks
Definition of Done:
- [ ] `handle_spec_board_get` still: read → archive merge on specs only → `to_json` (no TECH_DEBT IO in HTTP)
- [ ] GET 200 body includes `debt` as Task #1 (always present; empty array when omit)
- [ ] GET 200 body has no field named `has_more`
- [ ] GET unknown project still 404 `{"error":"project not found"}`
- [ ] POST with `spec_id` equal to an epic `id` that is not in `specs[]` → 404 `{"error":"spec not found"}`; `spec_board_find` unchanged
- [ ] That POST does not call `cbm_store_spec_archive_set`; epic.md byte-identical
- [ ] GET 200 leaves `.sdd-skill/baseline/TECH_DEBT.md`, `.sdd-skill/specs/active.json`, and `.grill/index.md` byte-identical
- [ ] GET `/api/game-board` existing tests stay green (no debt key required on that JSON)
- [ ] `test_httpd.c` covers C HTTP Gherkin rows below
- [ ] Zero skill writes; no new route; no MCP
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-001 (HTTP), US-005 (game GET lock), US-006
Gherkin covered:
- Open heading item GET 200 JSON (HTTP half)
- Limit — 17th omitted; no has_more (HTTP half if not fully proven in #1)
- Error — POST archive with an epic id is 404 and writes nothing
- Error — unknown project on GET is still 404
- Error — GET does not write skill trees or game files
Dependencies: Task #1
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Prefer proving existing dispatch + new JSON over rewriting GET. Do not fopen TECH_DEBT.md in `http_server.c`. Archive merge must not set fields on `debt[]`. Do not add a distinct error string for debt.

### Task #3 — SpecBoardTab strip + EpicCard wrap + i18n
Definition of Done:
- [ ] `SpecBoardDebt` type: `id`, `title`. `SpecBoard.debt?: SpecBoardDebt[]` (missing = [])
- [ ] `messages.en.specBoard.openTechDebt` === `"Open tech debt"`; zh filled; `i18n.test.ts` locks English
- [ ] When `(board.debt ?? []).length > 0`, SpecBoardTab paints `role="region"` `aria-label={t.specBoard.openTechDebt}` above the three-column row. Not inside WorkspaceHeader. Not inside a column
- [ ] Each row is a `<p>` (or non-control) showing `id` then title. `whitespace-normal break-words`. No `truncate`. No onClick / clipboard / expand
- [ ] When debt is missing or `[]`, the region is not in the document
- [ ] EpicCard title line keeps `truncate`. EpicCard id line drops `truncate`, uses `whitespace-normal break-all`, shows full `epic.id`
- [ ] SpecCard id line still has `truncate`
- [ ] `useSpecBoard` / `useSddSkillPresent` / poll / `formatIndexedAt` / `colors.ts` / `GameBoardTab.tsx` / `WorkspaceHeader.tsx` not edited
- [ ] Host: missing `debt` does not throw; grill-only still paints Kanban; notSddSkill last-resort when both flags false
- [ ] Component tests for strip paint/omit + wrap (full leftover table is Task #4)
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-004, US-005 (Specs-only strip)
Gherkin covered (implementation + partial tests):
- Open heading item strip: region name, TD-005 + leftover cache, Todo still shown
- All-resolved / missing file: region not shown
- Todo epic id wraps; title may still truncate
- Limit — spec card id still truncates
- Limit — long debt title wraps
Dependencies: Task #2
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: Keep EpicCard and the strip in SpecBoardTab.tsx (no new CSS file). Wrap the existing `flex gap-6` column row in a parent `flex flex-col`. Three Column children only. Letter E unchanged. SDD-ADR-067, SDD-ADR-068.

### Task #4 — leftover Vitest locks
Definition of Done:
- [x] `SpecBoardTab.test.tsx` / `App.test.tsx` / `GameBoardTab.test.tsx` / `WorkspaceHeader.test.tsx` map every remaining UI Gherkin Then below
- [x] WorkspaceHeader does not show `TD-005` or a region named "Open tech debt"
- [x] Activating Graph or ADR (App tab) does not show a region named "Open tech debt"
- [x] GameBoardTab mount does not show a region named "Open tech debt"
- [x] Grill-only (sdd false, grill true, no debt): Specs pane shown; region omitted; `notSddSkill` copy absent
- [x] Companion-to: Todo does not show the converted epic id (mock already omitted — UI does not re-match)
- [x] Click/activate a debt row: `fetch` POST not called; `navigator.clipboard.writeText` not called; no card expand region
- [x] No control named "Has more"
- [x] `colorForLabel("Function") === "#06b6d4"` still locked if that test is in the suite run
- [x] Existing host tests stay green (grill-only Kanban, notSddSkill last-resort, 64-epic no Has more, spec-014 Specs lock)
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001–US-006 (UI leftover Thens)
Gherkin covered:
- Open heading item: WorkspaceHeader does not show TD-005
- Companion-to still omits the converted epic (Todo)
- Limit — grill-only without TECH_DEBT.md still shows Specs
- Limit — 17th omitted: no Has more control
- Limit — Graph ADR and Game do not show the strip
- Limit — activating a debt row does nothing
Dependencies: Task #3
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: English assertions. Conversion is C-owned; UI tests mock already-filtered `epics`. GraphTab stays mocked in App.test — assert the region unmounts when Specs is left. Do not boot GraphScene/Three. Do not add Playwright unless @tester later requires CERTIFICATION. Do not edit GameBoardTab.tsx to "pass" the lock.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | spec_board debt parse + additive JSON | 4h | None | done |
| 2 | HTTP GET additive + POST leftover locks | 2h | #1 | done |
| 3 | SpecBoardTab strip + EpicCard wrap + i18n | 3h | #2 | done |
| 4 | leftover Vitest locks | 2h | #3 | done |
Total: 11h

## Critical Path
#1 → #2 → #3 → #4 (11h). #3 can stub types from plan.md before #2 lands if HTTP is tests-only, but JSON contract must match Task #1.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `spec_board` + httpd Gherkin + `SpecBoardTab.tsx`. C owns parse/heading-vs-table/unknown/cap/skip/bytes/POST 404. Vitest owns strip paint/omit, wrap vs spec-card truncate, conversion leftover, dead row, Graph/ADR/Game/header absence.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
