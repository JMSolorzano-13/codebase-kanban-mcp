# Spec-010-c4h: Game tab silent win
Status: completed | Spec ID: spec-010-c4h-game-tab-silent-win
Ref Ticket: none | KPI: A project with `.gamedev/` shows the Game tab and omits Specs; chrome shows phase, focus, and `/gamedev-skill continue` from GET `/api/game-board`; `?tab=specs` on that path becomes `tab=game` | Priority: P0
Created: 2026-08-30 | Tech Debt Ref: none

## Executive Summary
The workspace already has Graph, Specs (sdd OR grill), and ADR. gamedev-skill is a different cycle. A path with `.gamedev/` must get its own Game tab and must not show Specs, even when `.sdd-skill/` or `.grill/` exist on the same root (silent win). Other paths stay sdd+grill as today.

This spec ships the tab, the silent-win omit, chrome (phase + focus + copyable continue), and a new GET. The board JSON includes empty column arrays so epic 002 can fill the same path. No artifact cards, Inbox, expand, archive, or ADR-trio fill.

Business impact: the operator opens a `.gamedev/` project (e.g. bevy-tetris) and sees Game-not-Specs plus how to resume the skill, without CBM writing `.gamedev/` or launching it.

## User Stories

### US-001: Game tab when `.gamedev/` exists
As an operator on a project whose root has `.gamedev/`, I want a Game tab in the workspace strip, So that I can open the gamedev surface instead of Specs.
Acceptance Criteria:
- [ ] Presence for Game comes from one shot of GET `/api/game-board?project=<name>` (not `/api/skill-presence`, not GET `/api/spec-board`)
- [ ] Game is shown when that GET is HTTP 200 and `gamedev_skill_present === true`
- [ ] Tab id is `game`. Visible label is "Game". Accessible name is "Game"
- [ ] Tab order when Game is shown: Graph then Game then ADR
- [ ] Enter from Dashboard still opens Graph. This spec does not change the default workspace tab
- [ ] Loading or `gamedev_skill_present !== true` omits Game (not disabled; no strip hint)

### US-002: Silent win omits Specs
As an operator on a path that has `.gamedev/` and also `.sdd-skill/` and/or `.grill/`, I want Specs hidden, So that one path has one lifecycle chrome and Track A is not duplicated in the Specs Kanban.
Acceptance Criteria:
- [ ] When Game is shown, Specs is not shown
- [ ] No conflict banner, no hybrid chrome, no "both tabs" state
- [ ] While GET `/api/game-board` has not returned, omit Game and omit Specs (no Specs flash)
- [ ] After HTTP 200 with `gamedev_skill_present === false`, Specs follows spec-009 (200 spec-board AND sdd OR grill)
- [ ] After HTTP 500 or network failure on game-board, omit Game; Specs follows spec-009 (do not hide Specs on a failed Game GET)
- [ ] sdd-only and grill-only paths without `.gamedev/` still show Specs and never show Game

### US-003: Chrome is a map, not a launcher
As an operator on Game, I want phase, focus, and `/gamedev-skill continue` visible, So that I know where the cycle is and what to paste into the IDE.
Acceptance Criteria:
- [ ] When `state.md` parses, chrome shows the mapped phase label, the focus string, and the exact text `/gamedev-skill continue`
- [ ] Phase labels (English, tests assert these): `01-preproduction` → "Pre-production"; `02-production` → "Production"; `03-postproduction` → "Post-production & Launch"
- [ ] No CBM button launches the skill, pastes into the IDE, or lists the full command catalog
- [ ] No gate-review hint in this spec
- [ ] Empty `.gamedev/` or missing/unreadable `state.md`: tab still shown; chrome shows "state.md missing" and `/gamedev-skill continue`; no phase label; no focus text
- [ ] This spec does not paint column headers or artifact cards (JSON arrays stay empty)

### US-004: Deep-links
As an operator, I want `?tab=game` and leftover `?tab=specs` bookmarks to resolve under silent win, So that a share link does not land on a hidden tab.
Acceptance Criteria:
- [ ] `?tab=game&project=<name>` + game-board 200 + `gamedev_skill_present === true` → URL stays `tab=game`; Game pane is shown
- [ ] `?tab=game&project=<name>` + not present (in flight, flag false, or non-200 except as US-002 error rule) → fallback to Graph (`?tab=graph&project=<name>`)
- [ ] `?tab=specs&project=<name>` + game-board 200 + `gamedev_skill_present === true` → replaceState to `tab=game` (not Graph)
- [ ] `?tab=specs` on a path without gamedev still follows spec-009
- [ ] Inbound `tab=game` is restored after omit-until-true once presence lands true (same restore idea as spec-002/009 for specs)

### US-005: GET `/api/game-board` contract
As an operator (and the UI), I want one new GET that reports gamedev presence plus chrome fields, So that spec-board stays free of gamedev and epic 002 can fill the same path.
Acceptance Criteria:
- [ ] `GET /api/game-board?project=<name>` is the only new HTTP path in this spec
- [ ] Known project + `.gamedev/` is a directory → HTTP 200, `gamedev_skill_present` true, chrome fields set as below, four arrays empty
- [ ] Known project + no `.gamedev/` directory → HTTP 200, `gamedev_skill_present` false; arrays still present and empty
- [ ] Unknown project → HTTP 404 `{"error":"project not found"}` (same string as spec-board)
- [ ] Missing or empty `project` query → HTTP 400 `{"error":"missing project parameter"}`
- [ ] GET 200 JSON keys: `gamedev_skill_present` (bool), `phase` (string or null), `focus` (string or null), `continue` (string), `inbox` (array), `preproduction` (array), `production` (array), `postproduction` (array)
- [ ] When present is true and `state.md` parses: `phase` is one of `01-preproduction` | `02-production` | `03-postproduction`; `focus` is the focus value; `continue` is `/gamedev-skill continue`
- [ ] When present is true and `state.md` is missing or unreadable: `phase` null, `focus` null, `continue` is `/gamedev-skill continue`
- [ ] When present is false: `phase` null, `focus` null; `continue` may be `""` or `/gamedev-skill continue`; arrays `[]`
- [ ] This spec: every card array is `[]` (length 0). No artifact objects yet
- [ ] GET `/api/spec-board` still must not emit `gamedev_skill_present`. graph-ui must not call `/api/skill-presence`
- [ ] No MCP game-board tool. No POST `/api/game-board` in this spec

### US-006: Zero skill writes and regressions
As an operator, I want CBM to only read `.gamedev/`, So that the skill remains the writer and sdd+grill paths do not change.
Acceptance Criteria:
- [ ] GET `/api/game-board` 200 leaves `.gamedev/`, `.sdd-skill/`, and `.grill/` byte-identical (and does not create them)
- [ ] Graph, Dashboard, ADR tab, Path 1:1, ADR fill, spec-005 expand, spec-006 archive, spec-007 `formatIndexedAt`, spec-008 epic cards, spec-009 Specs presence stay unchanged on non-gamedev paths
- [ ] i18n: `tabs.game` in `en` ("Game") and `zh`; tests may assert English. Add `state.md missing` in both locales
- [ ] Reuse WorkspaceTabStrip (extend it). Closed set: WORKSPACE_TABS / TabId / readRoute include `game`

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Game tab silent win

  # Happy Paths
  Scenario: gamedev-only project shows the Game tab
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    And "/tmp/bevy/.sdd-skill/" does not exist
    And "/tmp/bevy/.grill/" does not exist
    And GET /api/game-board?project=bevy returns HTTP 200
    And that body has gamedev_skill_present true
    When the operator is on the workspace for project "bevy"
    Then the tab named "Game" is shown
    And the tab named "Game" has accessible name "Game"
    And the tab named "Specs" is not shown
    And the tab order is Graph then Game then ADR

  Scenario: silent win hides Specs when sdd and grill also exist
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    And "/tmp/bevy/.sdd-skill/" exists as a directory
    And "/tmp/bevy/.grill/" exists as a directory
    And GET /api/game-board?project=bevy returns HTTP 200 with gamedev_skill_present true
    And GET /api/spec-board?project=bevy returns HTTP 200 with sdd_skill_present true and grill_skill_present true
    When the operator is on the workspace for project "bevy"
    Then the tab named "Game" is shown
    And the tab named "Specs" is not shown
    And the pane does not show the text "Path conflict"
    And the pane does not show a conflict banner

  Scenario: Game chrome shows phase, focus, and continue
    Given GET /api/game-board?project=bevy returns HTTP 200
    And gamedev_skill_present is true
    And phase is "02-production"
    And focus is "Triaging playtest round 3 feedback on movement feel"
    And continue is "/gamedev-skill continue"
    When the operator activates the tab named "Game"
    Then the pane shows the text "Production"
    And the pane shows the text "Triaging playtest round 3 feedback on movement feel"
    And the pane shows the text "/gamedev-skill continue"
    And the pane does not show the text "Pre-production"
    And the pane does not show the text "Post-production & Launch"
    And the pane does not contain a button that launches gamedev-skill

  Scenario: Deep-link tab=game stays when gamedev is present
    Given GET /api/game-board?project=bevy returns HTTP 200 with gamedev_skill_present true
    When the operator opens "?tab=game&project=bevy"
    Then the URL query includes "tab=game" and "project=bevy"
    And the Game pane is shown
    And the URL query does not include "tab=graph"

  Scenario: Deep-link tab=specs on a gamedev path becomes tab=game
    Given GET /api/game-board?project=bevy returns HTTP 200 with gamedev_skill_present true
    When the operator opens "?tab=specs&project=bevy"
    Then the URL query includes "tab=game" and "project=bevy"
    And the Game pane is shown
    And the URL query does not include "tab=specs"
    And the tab named "Specs" is not shown

  Scenario: grill-only without gamedev still shows Specs and not Game
    Given GET /api/game-board?project=alpha returns HTTP 200 with gamedev_skill_present false
    And GET /api/spec-board?project=alpha returns HTTP 200 with sdd_skill_present false and grill_skill_present true
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is shown
    And the tab named "Game" is not shown
    And the tab order is Graph then Specs then ADR

  # Limit Cases
  Scenario: Limit Case — empty gamedev directory still shows Game
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    And "/tmp/bevy/.gamedev/state.md" does not exist
    And GET /api/game-board?project=bevy returns HTTP 200
    And gamedev_skill_present is true
    And phase is null
    And focus is null
    And continue is "/gamedev-skill continue"
    When the operator activates the tab named "Game"
    Then the tab named "Game" is shown
    And the pane shows the text "state.md missing"
    And the pane shows the text "/gamedev-skill continue"
    And the pane does not show the text "Pre-production"
    And the pane does not show the text "Production"
    And the pane does not show the text "Post-production & Launch"

  Scenario: Limit Case — GET 200 with present false omits Game
    Given GET /api/game-board?project=alpha returns HTTP 200 with gamedev_skill_present false
    And inbox is []
    And preproduction is []
    And production is []
    And postproduction is []
    When the operator is on the workspace for project "alpha"
    Then the tab named "Game" is not shown

  Scenario: Limit Case — Enter still opens Graph on a gamedev project
    Given GET /api/game-board?project=bevy returns HTTP 200 with gamedev_skill_present true
    When the operator activates Enter on the "bevy" row
    Then the URL query includes "tab=graph" and "project=bevy"
    And the tab named "Game" is shown
    And GraphTab for "bevy" is shown

  Scenario: Limit Case — game-board 200 present true emits empty column arrays
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And gamedev_skill_present is true
    And inbox is []
    And preproduction is []
    And production is []
    And postproduction is []
    And continue is "/gamedev-skill continue"

  Scenario: Limit Case — spec-board still has no gamedev field
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    When GET /api/spec-board?project=bevy is read
    Then the response status is 200
    And the response JSON has no field whose name is "gamedev_skill_present"

  Scenario: Limit Case — sdd-only without gamedev still shows Specs
    Given GET /api/game-board?project=alpha returns HTTP 200 with gamedev_skill_present false
    And GET /api/spec-board?project=alpha returns HTTP 200 with sdd_skill_present true and grill_skill_present false
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is shown
    And the tab named "Game" is not shown

  Scenario: Limit Case — inbound tab=game restores after omit-until-true
    Given the operator opened "?tab=game&project=bevy"
    And GET /api/game-board?project=bevy has not returned
    Then the tab named "Game" is not shown
    And the URL query includes "tab=graph"
    When GET /api/game-board?project=bevy returns HTTP 200 with gamedev_skill_present true
    Then the URL query includes "tab=game" and "project=bevy"
    And the Game pane is shown

  # Error Scenarios
  Scenario: Error — game-board still in flight omits Game and Specs
    Given GET /api/game-board?project=bevy has not returned
    And GET /api/spec-board?project=bevy returns HTTP 200 with sdd_skill_present true and grill_skill_present true
    When the operator is on the workspace for project "bevy"
    Then the tab named "Game" is not shown
    And the tab named "Specs" is not shown

  Scenario: Error — game-board 500 omits Game and does not hide Specs
    Given GET /api/game-board?project=alpha returns HTTP 500
    And GET /api/spec-board?project=alpha returns HTTP 200 with sdd_skill_present true and grill_skill_present false
    When the operator is on the workspace for project "alpha"
    Then the tab named "Game" is not shown
    And the tab named "Specs" is shown
    And GraphTab remains usable

  Scenario: Error — unknown project is 404
    Given no catalog project named "missing"
    When GET /api/game-board?project=missing is read
    Then the response status is 404
    And the response body is {"error":"project not found"}

  Scenario: Error — missing project query is 400
    When GET /api/game-board is read
    Then the response status is 400
    And the response body is {"error":"missing project parameter"}

  Scenario: Error — tab=game without gamedev falls back to Graph
    Given GET /api/game-board?project=alpha returns HTTP 200 with gamedev_skill_present false
    When the operator opens "?tab=game&project=alpha"
    Then the URL query includes "tab=graph" and "project=alpha"
    And GraphTab for "alpha" is shown
    And the tab named "Game" is not shown

  Scenario: Error — GET does not write skill trees
    Given project "bevy" root_path is "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And "/tmp/bevy/.gamedev/" files are byte-identical to their contents before the GET
    And "/tmp/bevy/.sdd-skill/" does not get created
    And "/tmp/bevy/.grill/" does not get created
    And graph-ui issues no request whose path contains "/api/skill-presence"
```

## Success Metrics
| Metric | Target | Current | Status |
| Game tab on `.gamedev/` path | 100% when game-board 200 and present | 0 (no tab id) | not met |
| Specs hidden when Game shown | 100% | Specs can show with sdd/grill | not met |
| `?tab=specs` on gamedev → `tab=game` | 100% | falls back to Graph or stays Specs | not met |
| Chrome phase + focus + continue when state.md parses | 100% | 0 | not met |
| Empty dir still shows Game | 100% | 0 | not met |
| In-flight Specs flash | 0 | possible | not met |
| Skill-file writes from GET or Game UI | 0 | 0 | met (must stay 0) |
| spec-board emits gamedev_skill_present | 0 | 0 | met (must stay 0) |
Primary KPI: first three metrics plus zero skill writes.

## Constraints & Assumptions
Technical:
- `cbm_spec_board_gamedev_skill_present` already checks `root/.gamedev` is a directory. spec-board read/to_json must not call it or emit the flag (existing lock).
- GET `/api/skill-presence` exists and is unused by graph-ui; this spec does not start using it.
- Constitution I.2: indexer/graph-ui do not write skill cycle files. Read of `.gamedev/state.md` is allowed.
- Constitution IV.3: new endpoint required because spec-board must stay gamedev-free.
- Constitution IX.2 spec-009: Specs present = spec-board 200 AND (sdd OR grill). This spec adds AND NOT gamedev (gamedev from game-board only).
- Chrome grayscale. Tests may assert English.
- Path = Project 1:1. Unknown project 404 string matches spec-board.

Business:
- Grill plan add-gamedev-skill epic-001. ADR-001 silent win. ADR-009 map not launcher.
- Epic 002 fills the four arrays and paints columns. Epic 003 expand/archive/deps. Epic 004 ADR trio.

Planner defaults (reject a Gherkin scenario to change these):
1. Presence source is GET `/api/game-board` only. Never `/api/skill-presence` for the strip.
2. Known project without `.gamedev/`: 200 + `gamedev_skill_present` false (not 404).
3. Tab id `game`, label "Game". Order Graph | Game | ADR.
4. `?tab=specs` + gamedev present → `tab=game`. `?tab=game` without gamedev → Graph.
5. Empty `.gamedev/` still shows the tab. Missing/unreadable state.md → "state.md missing" + continue.
6. In flight: omit Game and Specs. game-board 500: omit Game; Specs follows spec-009.
7. Enter / default workspace tab stays Graph.
8. JSON 001 includes empty `inbox`/`preproduction`/`production`/`postproduction`. UI 001 does not paint columns or cards.
9. Phase chrome labels are the three English strings above. JSON `phase` stays the skill token.
10. Zero skill writes. No MCP tool. No POST. No gate-review hint. No launcher button.

## Out of Scope
- Artifact cards, four visible columns, Inbox grill, conversion map (epic 002)
- Expand, archive, blocked-by strip, Track A Inputs (epic 003)
- ADR fill from the gamedev trio (epic 004)
- Two tabs Specs + Game on the same path
- Conflict banner if `.gamedev/` and `.sdd-skill/` coexist
- Writing `.gamedev/` or invoking gamedev-skill from CBM
- Using GET `/api/skill-presence` in graph-ui
- Emitting `gamedev_skill_present` on GET `/api/spec-board`
- Changing Enter default away from Graph
- Renaming Specs, Graph, or ADR
- POST `/api/game-board` / MCP game-board tool

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% [ ] review approved [ ] human docs confirmed [ ] zero skill writes [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- New HTTP GET family (C handler + JSON). Reuse `cbm_spec_board_gamedev_skill_present` for the dir check. New read of `.gamedev/state.md` for phase/focus (best-effort fopen; never write)
- graph-ui: WORKSPACE_TABS + TabId + readRoute + WorkspaceTabStrip + App fallback/restore. Game pane host (chrome only)
- Specs present becomes (spec-009 predicate) AND game-board settled AND NOT gamedev
- Dual fetch: spec-board (Specs) + game-board (Game). In-flight rule is the anti-flash
- i18n: `tabs.game`, missing-state string, phase labels
- Vitest: strip/order, silent win, deep-links, chrome, in-flight. C tests: GET 200/400/404, empty arrays, no spec-board gamedev field, no skill writes
- Existing `fallbackSpecsToGraph` is not enough: specs+gamedev must go to `game`, not `graph`

## Questions for Architect (answered in plan.md)
- One Game pane component vs reuse SpecBoardTab chrome patterns only
- Whether `fallbackSpecsToGraph` gains a sibling (`fallbackGameToGraph` + specs-on-gamedev → game) or one presence router
- state.md parse: tolerate legacy field names (active_phase / director_focus) this spec or compact `phase=` / `focus=` only
- Heap: dedicated `cbm_game_board_t` now vs thin JSON builder until epic 002 fills arrays

## Questions for @implementer
- [ ] Map every Gherkin scenario to a Vitest and every GET status/field scenario to a C test
- [ ] Do not write `.gamedev/`, `.sdd-skill/`, or `.grill/`
- [ ] Do not call `/api/skill-presence` from graph-ui
- [ ] Do not emit `gamedev_skill_present` on spec-board
- [ ] Do not paint columns or cards
- [ ] Do not add a launcher button or gate-review hint
- [ ] Do not change Enter default away from Graph
- [ ] Update WORKSPACE_TABS tests that lock `["graph","specs","adr"]`

## Related Specs
Depends on: spec-002-p8w-project-workspace (strip + omit-until + Enter Graph), spec-009-t4x-specs-tab-grill-presence (Specs = sdd OR grill; this spec adds AND NOT gamedev)
Companion to: .grill/plans/add-gamedev-skill/epics/epic-001-game-tab-silent-win.md
Does not include: epic 002 board cards, epic 003 expand/archive, epic 004 ADR trio

## Revision History
| Date | Author | Change |
| 2026-08-30 | @planner | draft from grill epic-001 + ADR-001,009; defaults A; debt=no; ticket=none |
| 2026-08-31 | @planner | Gherkin approved by user ("scenarios approved") |

## Approval Sign-off
Spec Owner(@planner): approved / Gherkin scenarios(user): approved / Product Owner: pending / Architecture(@architect): pending
