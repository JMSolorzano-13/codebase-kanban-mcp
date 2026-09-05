# Work Breakdown — Spec-017: Game debt chrome
Total Tasks: 3 / Estimated Total Effort: 9h

## Task Dependency Graph
```
#1 game_board debt parse + JSON ──► #2 HTTP GET/POST leftover locks
                                 └──► #3 GameBoardTab strip + leftover Vitest
```
#3 may start after #1's JSON field names exist (types from plan). Critical path: #1 (4h) → #2 (2h) → #3 (3h) = 9h.

## Tasks

### Task #1 — game_board debt parse + additive JSON
Definition of Done:
- [ ] `CBM_GAME_BOARD_MAX_DEBT` 16 in `game_board.h` next to MAX_BLOCKED (independent of cards 64 / blocked 16 / spec-board 16)
- [ ] `cbm_game_board_debt_t`: `id[96]`, `title[256]`
- [ ] `cbm_game_board_t` has `debt[CBM_GAME_BOARD_MAX_DEBT]`, `int debt_count`
- [ ] `cbm_game_board_parse_backlog_debt(const char *md, cbm_game_board_debt_t *out, int *count)` exported; NULL/empty md → count 0; does not fopen
- [ ] `cbm_game_board_read` calls `debt_fill` after `overlay_blocked`: join `{root}/.gamedev/backlog.md`; not regular file or `read_whole_file` NULL → debt_count 0; else parse; never writes
- [ ] Start = first `debt:` + `[A-Za-z0-9][A-Za-z0-9_-]*` on the line (comment / list / ordered / ATX / prose). `id` is the full token
- [ ] `debt:` with no tag body (space, punctuation, EOL) skipped. `design` / `tech` without `debt:` are not entries
- [ ] Entry body = start line through (not including) next start, next ATX `##` (0–3 leading spaces), or EOF. Blank lines do not end it
- [ ] Closed iff body contains `resolved-by` (hyphen, lowercase) on the start line or a later body line
- [ ] Title = after tag, trim, strip wrapping `]`, strip trailing `-->`, cut at first ` — ` or ` -- `. Empty title allowed
- [ ] Order = file/start-line. Duplicate id: first start wins; later same-id skipped. Cap 16 open; 17th omitted; closed does not consume a slot
- [ ] `cbm_game_board_to_json` always emits `"debt":[{id,title}]` (empty array when count 0); never `has_more`; never owner/target/severity
- [ ] Do not call `cbm_spec_board_parse_tech_debt`. Do not change inbox hide / `fill_artifacts`. Do not put backlog.md on any card array
- [ ] `tests/test_game_board.c` covers Then clauses below; existing to_json tests accept additive `debt` (empty when no file)
- [ ] tests pass locally; breadcrumbs
User Stories Addressed: US-001, US-002, US-003, US-004 (reader half)
Gherkin covered (unit-level Then):
- Open comment: debt length 1; id `debt:gate-preproduction`; title `missing GDD lock`; no has_more
- List-item `- debt:save-slot … -- …` → id `debt:save-slot`; title `no checkpoint`
- resolved-by on following line → debt []
- design + tech + one debt → length 1; `debt:adopt-gap-audio`; title `no sfx list`
- Missing backlog.md → debt []
- Same-line resolved-by → debt []
- Blank line then resolved-by → debt []
- Next debt start ends previous: only first open; second closed
- 17th open omitted; no `debt:d17`; no has_more
- Heading `## debt:adopt-gap-audio …` counts
- Bare `debt:` skipped; well-formed kept
- backlog.md is not a card in inbox/pre/prod/post
- Inbox registry hide unchanged (keep green)
- Unreadable regular file → debt []
- Directory-at-path → debt []
- Line without a debt tag skipped; well-formed kept
- GET-equivalent read leaves fixture backlog.md / state.md / registry bytes identical; does not create backlog.md
Dependencies: None
Estimated Effort: 4h
Subagent: no
Path: full
Implementation Notes: All in `game_board.c` / `.h` (SDD-ADR-072, SDD-ADR-073). Do not touch HTTP, graph-ui, MCP, store, Makefile.cbm, spec_board.c. Heap calloc in tests (already). Stop appending at 16 open. Fixtures `/tmp` only. Reuse `read_whole_file` and `game_is_regular_file`. Unreadable = regular file + `read_whole_file` NULL; use oversize > `GAME_BOARD_MAX_FILE` (1 MiB), not a directory. Directory-at-path is the missing class. Do not parse a live backlog.md. Update `gb_assert_empty_arrays_json` to require `"debt":[]`.

### Task #2 — HTTP GET additive + POST leftover locks
Definition of Done:
- [ ] `handle_game_board_get` still: read → archive merge on cards only → `to_json` (no backlog IO in HTTP)
- [ ] GET 200 body includes `debt` as Task #1 (always present; empty array when omit)
- [ ] GET 200 body has no field named `has_more`
- [ ] GET unknown project still 404 `{"error":"project not found"}`
- [ ] GET `/api/spec-board` does not require or fopen `backlog.md`; Specs `debt[]` still from TECH_DEBT.md
- [ ] GET 200 leaves `.gamedev/backlog.md` (when present), `.gamedev/state.md`, `.gamedev/epics_registry.md`, `.grill/index.md` byte-identical
- [ ] GET 200 when backlog.md does not exist does not create that file
- [ ] POST archive of a done artifact leaves backlog.md byte-identical; debt ids are not archive targets
- [ ] Inbox registry hide HTTP leftover stays green
- [ ] `test_httpd.c` covers C HTTP Gherkin rows below
- [ ] Zero skill writes; no new route; no MCP
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-001 (HTTP), US-004, US-005
Gherkin covered:
- Open comment entry GET 200 JSON (HTTP half)
- Limit — 17th omitted; no has_more (HTTP half if not fully proven in #1)
- Limit — Inbox registry hide is unchanged (HTTP leftover)
- Limit — Specs strip still uses spec-board debt only (HTTP leftover)
- Error — unknown project on GET is still 404
- Error — GET does not write skill trees or create backlog.md
- Error — POST archive does not write backlog.md
Dependencies: Task #1
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Prefer proving existing dispatch + new JSON over rewriting GET. Do not fopen backlog.md in `http_server.c`. Archive merge must not set fields on `debt[]`. Do not add a distinct error string for debt. Do not edit spec_board.c to “pass” the leftover.

### Task #3 — GameBoardTab strip + leftover Vitest
Definition of Done:
- [ ] `GameBoardDebt` type: `id`, `title`. `GameBoard.debt?: GameBoardDebt[]` (missing = [])
- [ ] Reuse `t.specBoard.openTechDebt` (`en` === `"Open tech debt"`). Do not add `gameBoard.openTechDebt`. Do not edit `i18n.ts`
- [ ] When `(board.debt ?? []).length > 0`, GameBoardTab paints `role="region"` `aria-label={t.specBoard.openTechDebt}` after `<BlockedStrip />` and before the four-column grid. Not inside WorkspaceHeader. Not on the filter row. Not above the phase line
- [ ] Each row is a `<p>` (or non-control) showing `id` then title. `whitespace-normal break-words`. No `truncate`. No onClick / clipboard / expand
- [ ] When debt is missing or `[]`, the region is not in the document (existing spec-015 Game mount test stays green)
- [ ] Show Dones / Track / Show archived do not hide debt rows. Inbox / Artifact / expand / archive / BlockedStrip stay
- [ ] `useGameBoard` / `SpecBoardTab.tsx` / `WorkspaceHeader.tsx` / `colors.ts` not edited
- [ ] Host: missing `debt` does not throw; empty `.gamedev/` chrome still shows Game; Inbox still shown
- [ ] `GameBoardTab.test.tsx` / `App.test.tsx` map UI Gherkin Thens below
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-004, US-005, US-006
Gherkin covered (implementation + tests):
- Open comment strip: region name after Blocked slot, `debt:gate-preproduction` + `missing GDD lock`, no director/M1, Inbox still shown, WorkspaceHeader does not show the tag
- resolved-by / missing file / empty gamedev: region not shown; Game tab still shown when present
- Limit — long debt title wraps; computed text-overflow is not `ellipsis`
- Limit — Graph ADR omit Game debt region; Specs leftover still spec-board only
- Limit — activating a debt row: no POST, no clipboard, no expand
- Limit — Show Dones off still shows the debt row
- Limit — Blocked region above Open tech debt; four columns below
- Limit — 17th omitted: no control named "Has more"
Dependencies: Task #1
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: Copy Specs `DebtStrip` JSX into GameBoardTab.tsx (do not import SpecBoardTab). SDD-ADR-074. Conversion/hide/parse stay C-owned; UI mocks already-filtered `debt`. jsdom: assert no class `truncate` + class `break-words`; computed `text-overflow` is not `"ellipsis"`. Placement: `compareDocumentPosition` or sibling order. Do not add Playwright unless @tester later requires CERTIFICATION. Existing "does not show Open tech debt on mount" stays for debt-missing mocks.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | game_board debt parse + additive JSON | 4h | None | done |
| 2 | HTTP GET additive + POST leftover locks | 2h | #1 | done |
| 3 | GameBoardTab strip + leftover Vitest | 3h | #1 | done |
Total: 9h

## Critical Path
#1 → #2 (9h with #3 after #1). #3 can run after #1 without #2 because UI mocks JSON.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `game_board` parse + httpd Gherkin + `GameBoardTab.tsx`. C owns comment/list/heading, resolved-by same-line and next-line, blank-line continuation, design/tech skip, cap, missing/unreadable/dir, non-card, bytes. Vitest owns strip paint/omit, placement vs Blocked, wrap, dead row, Show Dones lock, Graph/ADR/Specs/header absence.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
