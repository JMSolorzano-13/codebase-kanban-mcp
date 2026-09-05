# Work Breakdown — Spec-016: Game Inbox registry
Total Tasks: 3 / Estimated Total Effort: 8h

## Task Dependency Graph
```
#1 game_board registry parse + hide predicate ──► #2 HTTP GET leftover locks
                                               └──► #3 InboxCard wrap + leftover Vitest
```
#3 may start after #1's omit contract is known (UI mocks already-filtered `inbox`). Critical path: #1 (4h) → #2 (2h) → #3 (2h) = 8h.

## Tasks

### Task #1 — game_board registry parse + hide predicate
Definition of Done:
- [ ] `CBM_GAME_BOARD_MAX_REGISTRY` 256 in `game_board.h`
- [ ] `cbm_game_board_registry_row_t`: slug[96], nnn, hide
- [ ] `cbm_game_board_parse_epics_registry(const char *md, cbm_game_board_registry_row_t *out, int *count)` exported; NULL/empty md → count 0; does not fopen
- [ ] `game_grill_fill_inbox`: `present = game_is_regular_file("{root}/.gamedev/epics_registry.md")`
- [ ] If present: `read_whole_file` (`"rb"`); NULL → 0 rows; else parse; do not call `game_grill_load_conv`
- [ ] If absent (missing or directory / non-regular): `game_grill_load_conv` as today; hide via `game_grill_epic_converted`
- [ ] If present: `game_grill_append_epic` omits iff Plan exact slug AND Epic integer NNN AND hide==true. Does not call `game_grill_epic_converted`
- [ ] Hide-set Status tokens: `in_progress`, `closed`, `parked`. `not_started` / unknown / evergreen → hide false
- [ ] Epic NNN 0 never hide. Plan cell `native` skipped. Origin unused
- [ ] Epic cell: trim; optional `epic-`; all-digits → int. Else skip row
- [ ] Plan cell: trim; exact slug. Path leftover does not match
- [ ] Duplicate Plan+NNN: last data row wins (overwrite). Header/separator/short row (n < 6 after split) skipped
- [ ] Walk and cap 64 unchanged. No `has_more`. Zero writes. Never create the file
- [ ] Do not edit spec_board.c / http_server.c / graph-ui this task
- [ ] `tests/test_game_board.c` covers Then clauses below
- [ ] tests pass locally; breadcrumbs
User Stories Addressed: US-001, US-002, US-003, US-004, US-005 (walk/cap), US-006 (C half)
Gherkin covered (unit-level Then):
- Registry in_progress omits matching leftover
- not_started keeps
- closed and parked omit; no-row stays
- File present + Companion-to does not hide unlisted
- File absent still hides via Companion-to (keep green)
- Empty/header-only registry present: roadmap does not hide
- Last duplicate wins (closed after not_started → omit; not_started after closed → keep)
- Epic cell `1` and `epic-001` match NNN 1
- Plan path leftover does not match
- File absent still hides via roadmap slug+NNN (keep green)
- Cap 64 / no has_more when file absent (keep green)
- evergreen Epic 0 does not hide
- unknown Status does not hide
- Plan native never matches
- Unreadable regular file present: Companion-to does not hide; GET-equivalent read 200
- Malformed Epic `n/a` skipped; well-formed parked row omits
- Read leaves fixture registry / epic.md bytes identical
Dependencies: None
Estimated Effort: 4h
Subagent: yes
Path: full
Implementation Notes: All in `game_board.c` / `.h` (SDD-ADR-069, SDD-ADR-070). Reuse `game_is_regular_file` and `game_grill_split_gfm_row` (Epic=cells[1], Plan=cells[2], Status=cells[5]). Unreadable = regular file + `read_whole_file` NULL; use oversize > `GAME_BOARD_MAX_FILE` (1 MiB), not a directory. Fixtures `/tmp` only. Heap calloc in tests (already). Do not parse this repo’s live registry.

### Task #2 — HTTP GET leftover locks
Definition of Done:
- [ ] `handle_game_board_get` still: read → archive merge → `to_json` (no registry IO in HTTP)
- [ ] GET 200 body has no field named `has_more` and no `epics_registry` / `registry` key
- [ ] GET unknown project still 404 `{"error":"project not found"}`
- [ ] GET 200 with a hide-set row omits that inbox id (HTTP half of happy path)
- [ ] GET `/api/spec-board` still lists a grill epic that the Game registry marks `closed` (no Companion-to / no source.grill_epic)
- [ ] GET 200 leaves `.gamedev/epics_registry.md` (when present), `.gamedev/state.md`, `.grill/index.md`, `.sdd-skill/specs/active.json` byte-identical
- [ ] GET 200 when the registry path does not exist does not create that file
- [ ] POST `/api/game-board` existing tests stay green (Inbox epic still 404)
- [ ] `test_httpd.c` covers C HTTP Gherkin rows below
- [ ] Zero skill writes; no new route; no MCP
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-001 (HTTP), US-006
Gherkin covered:
- Registry in_progress omits (HTTP half)
- Error — unknown project on GET is still 404
- Error — GET does not write skill trees or create the registry
- Limit — Specs Todo does not read the registry
Dependencies: Task #1
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Prefer proving existing dispatch. Do not fopen the registry in `http_server.c`. Do not edit spec_board.c to “pass” the leftover — the lock is that spec-board already lists the epic.

### Task #3 — InboxCard wrap + leftover Vitest
Definition of Done:
- [ ] InboxCard id `<p>` drops `truncate`, uses `whitespace-normal break-all`, shows full `card.id`
- [ ] InboxCard title stays TitleControl (may truncate). Letter E unchanged
- [ ] ArtifactCard id `<p>` still has `truncate`
- [ ] Empty Inbox: column header shown; 0 cards; no text “all tracked”; no text “no artifacts in this phase”
- [ ] Show Dones off still shows a mocked Inbox card (spec-014 leftover)
- [ ] `useGameBoard` / i18n / types / SpecBoardTab.tsx / WorkspaceHeader / colors.ts not edited
- [ ] `GameBoardTab.test.tsx` maps UI Gherkin Thens below
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-005, US-001 (paint), US-006 (UI lock)
Gherkin covered:
- Inbox id wraps the full path under the short name
- Limit — empty Inbox after hide keeps header only
- Limit — artifact id line still truncates
- Limit — Show Dones off still shows a visible Inbox card
Dependencies: Task #1
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: One class change on InboxCard (SDD-ADR-071). Conversion/hide is C-owned; UI tests mock already-filtered `inbox`. jsdom: assert no class `truncate` + class `break-all`; computed `text-overflow` is not `"ellipsis"`. Do not add Playwright unless @tester later requires CERTIFICATION. Do not add an i18n empty-state string.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | game_board registry parse + hide predicate | 4h | None | pending |
| 2 | HTTP GET leftover locks | 2h | #1 | pending |
| 3 | InboxCard wrap + leftover Vitest | 2h | #1 | pending |
Total: 8h

## Critical Path
#1 → #2 (8h with #3 after #1). #3 can run after #1 without #2 because UI mocks JSON.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `game_board` hide + httpd Gherkin + InboxCard. C owns present/absent, hide-set, last-wins, NNN, slug, unreadable, malformed, bytes. Vitest owns wrap vs artifact truncate, empty-column copy, Show Dones leftover.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
