# Work Breakdown — Spec-010: Game tab silent win
Total Tasks: 5 / Estimated Total Effort: 13.5h

## Task Dependency Graph
```
#1 C/HTTP game-board contract ──► #2 TabId / route / strip / i18n
                                 └──► #3 GameBoardTab chrome
                                      └──► #4 App silent-win + dual fetch + deep-links
                                           └──► #5 remaining Vitest Gherkin
```
#2 and #3 may start after #1 JSON field names exist (types from plan). Critical path: #1 (4h) → #2 (2h) → #3 (2.5h) → #4 (3h) → #5 (2h) = 13.5h.

## Tasks

### Task #1 — C/HTTP GET /api/game-board contract
Definition of Done:
- [x] `src/ui/game_board.h` + `game_board.c`: `cbm_game_board_t` with `gamedev_skill_present`, `phase`, `focus`, `continue_cmd`, four card arrays + counts
- [x] `CBM_GAME_BOARD_MAX_CARDS` 64; this spec never appends cards (all counts 0)
- [x] `cbm_game_board_read` uses `cbm_spec_board_gamedev_skill_present` for the dir check; fopen `"rb"` on `root/.gamedev/state.md` only when present
- [x] Parse compact `phase=` / `focus=`; tolerate `active_phase` / `director_focus` (`=` or line-start `:`); prefer compact when both exist
- [x] JSON `phase` only if token is `01-preproduction` | `02-production` | `03-postproduction`; else null. Focus cap 512. Missing/unreadable → phase/focus empty
- [x] present true → `continue` `/gamedev-skill continue`; present false → `continue` `""`
- [x] `cbm_game_board_to_json` emits all US-005 keys; four arrays `[]`; never English phase labels
- [x] `handle_game_board_get` + `dispatch_request` GET `/api/game-board*` only; same 400/404 strings as spec-board; heap calloc; no POST
- [x] GET `/api/spec-board` still has no `gamedev_skill_present` field
- [x] GET 200 leaves `.gamedev/`, `.sdd-skill/`, `.grill/` byte-identical and does not create missing trees
- [x] Makefile.cbm: `UI_SRCS` += `src/ui/game_board.c`; `TEST_UI_SRCS` += `tests/test_game_board.c`
- [x] No MCP tool; do not edit `mcp.c`; do not edit `spec_board.c` read/to_json
- [x] `tests/test_game_board.c` + `test_httpd.c` cover Then clauses below
- [x] tests pass locally; breadcrumbs
User Stories Addressed: US-005, US-006 (C/HTTP half)
Gherkin covered:
- Limit — game-board 200 present true emits empty column arrays
- Limit — spec-board still has no gamedev field
- Error — unknown project is 404
- Error — missing project query is 400
- Error — GET does not write skill trees (C/HTTP bytes)
Dependencies: None
Estimated Effort: 4h
Subagent: yes
Path: full
Implementation Notes: SDD-ADR-044, SDD-ADR-045. Fixtures `/tmp` only. Mirror `handle_spec_board_get` without archive merge. Unknown phase token → null, not a 500. Do not add `.gamedev` to ALWAYS_SKIP this spec.

### Task #2 — TabId, route kernels, strip, i18n
Definition of Done:
- [x] `WORKSPACE_TABS` is `["graph", "specs", "adr", "game"]`; `TabId` / `isWorkspaceTab` / `readRoute` accept `tab=game` with project
- [x] `fallbackSpecsToGraph` signature unchanged
- [x] `fallbackGameToGraph(tab, present)`: `tab === "game" && !present` → `"graph"`
- [x] `resolveWorkspaceTab(tab, specsPresent, gamePresent)`: specs+gamePresent → `"game"`; then game kernel; then specs kernel
- [x] `WorkspaceTabStrip` gains `showGame`. When true: Graph | Game | ADR (label `t.tabs.game`, accessible name "Game"). When false: existing Specs rule. If both true, Game wins (defensive)
- [x] i18n en+zh: `tabs.game` ("Game" in en), `state.md missing`, phase labels EN exactly `Pre-production` / `Production` / `Post-production & Launch`
- [x] `route.test.ts` lock test updated (do not leave `["graph","specs","adr"]` green)
- [x] Strip unit tests: showGame order; omit Game when false; no disabled placeholder
- [x] Do not edit App / GameBoardTab / C this task
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001 (closed set + label), US-006 (strip reuse)
Gherkin covered: none as primary (strip order Then is App #4 / leftover #5). Kernel units required.
Dependencies: Task #1
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-043. Const order ≠ strip display order when Game is shown. `routeUrl("game", name)` must work. Do not call `/api/skill-presence`.

### Task #3 — GameBoardTab chrome only
Definition of Done:
- [x] New `GameBoardTab` (not `SpecBoardTab`). Props receive board JSON (no second fetch)
- [x] When `phase` is `02-production` and focus/continue set: pane shows "Production", the focus string, `/gamedev-skill continue`; does not show the other two EN phase labels; no launcher button
- [x] When phase/focus null: pane shows "state.md missing" and `/gamedev-skill continue`; no EN phase labels
- [x] No column headers, no artifact cards, no gate-review hint, no command catalog
- [x] `GameBoard` type in `types.ts` matches GET keys; `continue` field typed (do not paint arrays)
- [x] `SpecBoardTab.tsx` / `useSpecBoard` / `useSddSkillPresent` not edited
- [x] Component tests for chrome + missing-state (full App Gherkin is Task #4/#5)
- [x] tests pass; breadcrumbs
User Stories Addressed: US-003
Gherkin covered:
- Game chrome shows phase, focus, and continue
- Limit — empty gamedev directory still shows Game (pane Thens; tab Then owned by Task #4)
Dependencies: Task #2
Estimated Effort: 2.5h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-042. Grayscale tokens only. Continue is selectable text, not a button that launches the skill. Phase map lives in UI/i18n, not C.

### Task #4 — App silent-win, dual fetch, deep-links
Definition of Done:
- [x] `useGameBoard(project)` one-shot GET `/api/game-board?project=`; `{ settled, present, board }`; `present` = 200 AND `gamedev_skill_present === true`; hang/4xx/5xx/throw → settled true except hang stays settled false; never `/api/skill-presence`; no 4s interval
- [x] `showGame = settled && present`; `showSpecs = settled && !showGame && specsPresent`
- [x] In-flight: omit Game and Specs even if spec-board already 200
- [x] game-board 500/network: omit Game; Specs follows spec-009
- [x] When Game shown: Specs omitted; no conflict banner; order Graph then Game then ADR
- [x] `?tab=game` + present → URL stays game; Game pane shown
- [x] `?tab=game` + !present (in flight / false / non-200) → URL `tab=graph`; inbound restore after omit-until-true when present lands true
- [x] `?tab=specs` + present → replaceState `tab=game` (not Graph, not Specs)
- [x] `?tab=specs` without gamedev still follows spec-009
- [x] Enter on a gamedev project still `tab=graph` + GraphTab + Game tab shown
- [x] grill-only / sdd-only without gamedev: Specs shown, Game not shown
- [x] `mockAppFetch` default game-board 200 present false; hang/500/true opt-in
- [x] `useSddSkillPresent` / `fallbackSpecsToGraph` signature / Enter `navigate("graph")` unchanged
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-004
Gherkin covered:
- gamedev-only project shows the Game tab
- silent win hides Specs when sdd and grill also exist
- Deep-link tab=game stays when gamedev is present
- Deep-link tab=specs on a gamedev path becomes tab=game
- grill-only without gamedev still shows Specs and not Game
- Limit — GET 200 with present false omits Game
- Limit — Enter still opens Graph on a gamedev project
- Limit — sdd-only without gamedev still shows Specs
- Limit — inbound tab=game restores after omit-until-true
- Error — game-board still in flight omits Game and Specs
- Error — game-board 500 omits Game and does not hide Specs
- Error — tab=game without gamedev falls back to Graph
Dependencies: Task #3
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-043. Pane uses `showGame`/`showSpecs`, not raw specs `present` while game `!settled`. `pendingSpecsDeepLink` + gamedev → game. Do not boot GraphTab/Three (keep mock).

### Task #5 — Remaining Vitest Gherkin mapping
Definition of Done:
- [x] Every UI Gherkin Then not already asserted in #3/#4 has a Vitest
- [x] Strip: Game accessible name "Game"; order Graph then Game then ADR when shown
- [x] Empty-dir tab shown + chrome (if #3 pane-only, assert tab in App here)
- [x] App fetch log: no request path contains `/api/skill-presence`
- [x] Existing spec-009 grill-only / neither / GET 500 Specs tests stay green with default game-board 200 false
- [x] No columns/cards/launcher/gate-review in Game pane
- [x] `colorForLabel("Function") === "#06b6d4"` still locked if that test is in the suite run
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001–US-006 (remaining UI Thens + no skill-presence)
Gherkin covered:
- Error — GET does not write skill trees (graph-ui no skill-presence half)
- Any #3/#4 Then still missing after implementation
Dependencies: Task #4
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Test-only if #4 product is complete. Do not add Playwright unless @tester later requires CERTIFICATION. Do not weaken spec-009 omit-until.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | C/HTTP GET /api/game-board contract | 4h | None | done |
| 2 | TabId, route kernels, strip, i18n | 2h | #1 | done |
| 3 | GameBoardTab chrome only | 2.5h | #2 | done |
| 4 | App silent-win, dual fetch, deep-links | 3h | #3 | done |
| 5 | Remaining Vitest Gherkin mapping | 2h | #4 | done |
Total: 13.5h

## Critical Path
#1 → #2 → #3 → #4 → #5 (13.5h). #2 types can be stubbed from plan.md before #1 lands if needed; JSON keys must match Task #1.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `game_board` + httpd Gherkin + `GameBoardTab` + `App` strip/route. C owns GET status/fields/bytes/no spec-board gamedev key. Vitest owns strip, silent win, deep-links, chrome, in-flight, no skill-presence.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
