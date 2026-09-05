# Spec-014-x7m: Game visibility filters
Status: completed | Spec ID: spec-014-x7m-filters
Ref Ticket: none | KPI: On Game first paint, done artifacts are hidden; Show Dones and Track A/B/All filter phase columns only; Inbox always visible; zero skill writes; no new HTTP | Priority: P1
Created: 2026-08-31 | Tech Debt Ref: none

## Executive Summary
spec-012 shipped Game expand, CBM archive, and a session Show archived toggle. A live board (bevy-tetris) still paints every unarchived done artifact next to pending work. Track A, B, and H sit in the same columns with no way to isolate one track.

This spec adds two client-side visibility controls in Game pane chrome, next to Show archived, with a visual separator: Show Dones (off by default) and Track A / B / All (All by default; All includes A, B, and H). Filters apply to phase columns only. Inbox cards stay visible. Show archived reveals an archived done card only when Show Dones is also on (AND).

Business impact: an operator scans active work without scrolling past Done, and can isolate Track A or B, without a new HTTP family, a persisted preference, or a skill-file write.

## User Stories

### US-001: Show Dones in Game chrome
As an operator on Game, I want a Show Dones control next to Show archived with a visual separator, So that I can reveal finished artifacts only when I ask.
Acceptance Criteria:
- [ ] Accessible name is exactly "Show Dones". It lives in Game pane chrome (same cluster as Show archived). Not Inbox header. Not a fifth column
- [ ] A visual separator (pipe text `|`) sits between Show archived and Show Dones
- [ ] `aria-pressed` is false on first paint of GameBoardTab for a project
- [ ] Activating it sets `aria-pressed` true. Activating again sets false
- [ ] When `aria-pressed` is false, a phase card whose JSON `work_state` is `"done"` is not shown, even if `archived` is false
- [ ] When `aria-pressed` is true, an unarchived done phase card is shown (subject to the Track filter)
- [ ] English copy is "Show Dones". i18n `en` and `zh`. Tests may assert English
- [ ] Specs tab does not gain a Show Dones control. Graph and ADR do not gain it

### US-002: Show archived AND Show Dones
As an operator, I want Show archived to reveal archived done cards only when Show Dones is also on, So that archived finished work does not leak through a single toggle.
Acceptance Criteria:
- [ ] An archived done phase card is shown iff Show Dones is pressed AND Show archived is pressed (AND, then Track filter)
- [ ] Show archived pressed and Show Dones unpressed: that card is not shown
- [ ] Show Dones pressed and Show archived unpressed: that card is not shown
- [ ] Both pressed: that card is shown (subject to Track)
- [ ] Leftover `archived` true on a card whose `work_state` is not `"done"`: still shown when Show Dones is off and Track allows it (spec-012 leftover rule stays)
- [ ] Archive / Unarchive eligibility and POST `/api/game-board` stay spec-012. After Archive 200 with Show archived off, that card is not shown even if Show Dones is on
- [ ] When every artifact in a phase column is hidden by these filters, that column is header only. No "No specs yet" copy

### US-003: Track A / B / All
As an operator on Game, I want a three-way Track filter in the same chrome cluster, So that I can see only Track A, only Track B, or every track including H.
Acceptance Criteria:
- [ ] Three controls with accessible names "Track A", "Track B", and "All" live in Game pane chrome after Show Dones, with a visual separator (pipe `|`) between Show Dones and the Track group
- [ ] They are exclusive. Exactly one has `aria-pressed` true at a time
- [ ] First paint: "All" has `aria-pressed` true; "Track A" and "Track B" are false
- [ ] All: phase cards with track `"A"`, `"B"`, or `"H"` are eligible (then Show Dones / Show archived)
- [ ] Track A: only phase cards with track `"A"`. Track `"B"` and `"H"` are not shown
- [ ] Track B: only phase cards with track `"B"`. Track `"A"` and `"H"` are not shown
- [ ] A phase artifact whose `track` is not `"A"` or `"B"` (including `"H"` and any other/null track) is shown only when All is pressed
- [ ] Activating a Track control does not send a new GET or POST
- [ ] i18n `en` and `zh` for the three names. Tests may assert English "Track A", "Track B", "All"
- [ ] Specs tab does not gain Track controls

### US-004: Inbox always visible; session-only; no new HTTP
As an operator, I want Inbox grill cards always visible and these filters session-only, So that leftover funnel items never disappear and a remount starts clean.
Acceptance Criteria:
- [ ] Inbox cards are never hidden by Show Dones, Show archived, or Track
- [ ] Changing `?project=` or remounting GameBoardTab starts Show Dones unpressed, Show archived unpressed, Track All pressed
- [ ] No localStorage. No URL query param for these filters
- [ ] A GET refetch after POST does not reset the three controls
- [ ] GET `/api/game-board` path, query, and JSON stay spec-012. No filter query params. No new HTTP path. No new POST fields
- [ ] POST `/api/game-board` archive contract unchanged
- [ ] GET `/api/spec-board` and Specs Show archived stay spec-006/008/009
- [ ] graph-ui must not call `/api/skill-presence`. No MCP filter tool
- [ ] GET 200 and POST 200 leave `.gamedev/`, `.sdd-skill/`, and `.grill/` byte-identical
- [ ] Silent win, four columns, expand, clipboard, no drag, Enter→Graph stay

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Game visibility filters

  # Happy Paths
  Scenario: First paint hides done artifacts and selects All
    Given GET /api/game-board?project=bevy returns HTTP 200
    And gamedev_skill_present is true
    And preproduction includes kind "artifact" id ".gamedev/phases/01-preproduction/gdd.md" track "B" work_state "done" archived false
    And preproduction includes kind "artifact" id ".gamedev/phases/01-preproduction/narrative-bible.md" track "B" work_state "pending" archived false
    And production includes kind "artifact" id ".gamedev/phases/02-production/systems/SYS-001-movement" track "A" work_state "pending" archived false
    And production includes kind "artifact" id ".gamedev/phases/02-production/levels/LVL-001-well" track "H" work_state "in_progress" archived false
    When GameBoardTab mounts for project "bevy"
    Then the control named "Show Dones" has aria-pressed false
    And the control named "Show archived" has aria-pressed false
    And the control named "All" has aria-pressed true
    And the control named "Track A" has aria-pressed false
    And the control named "Track B" has aria-pressed false
    And the Pre-production column does not show a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"
    And the Pre-production column shows a card whose id text is ".gamedev/phases/01-preproduction/narrative-bible.md"
    And the Production column shows a card whose id text is ".gamedev/phases/02-production/systems/SYS-001-movement"
    And the Production column shows a card whose id text is ".gamedev/phases/02-production/levels/LVL-001-well"

  Scenario: Show Dones reveals an unarchived done card
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/gdd.md" track "B" work_state "done" archived false
    And GameBoardTab is mounted for project "bevy"
    And the control named "Show Dones" has aria-pressed false
    When the operator activates the control named "Show Dones"
    Then that control has aria-pressed true
    And the Pre-production column shows a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"
    When the operator activates the control named "Show Dones"
    Then that control has aria-pressed false
    And the Pre-production column does not show a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"

  Scenario: Track A hides B and H
    Given GET /api/game-board?project=bevy production includes id ".gamedev/phases/02-production/systems/SYS-001-movement" track "A" work_state "pending"
    And production includes id ".gamedev/phases/02-production/art/piece-set" track "B" work_state "pending"
    And production includes id ".gamedev/phases/02-production/levels/LVL-001-well" track "H" work_state "pending"
    And GameBoardTab is mounted for project "bevy"
    When the operator activates the control named "Track A"
    Then the control named "Track A" has aria-pressed true
    And the control named "All" has aria-pressed false
    And the Production column shows a card whose id text is ".gamedev/phases/02-production/systems/SYS-001-movement"
    And the Production column does not show a card whose id text is ".gamedev/phases/02-production/art/piece-set"
    And the Production column does not show a card whose id text is ".gamedev/phases/02-production/levels/LVL-001-well"
    And no GET /api/game-board request is sent as a result of that activation
    And no POST /api/game-board request is sent as a result of that activation

  Scenario: Both toggles on reveal an archived done card
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/audio-direction.md" track "B" work_state "done" archived true
    And GameBoardTab is mounted for project "bevy"
    When the operator activates the control named "Show Dones"
    And the operator activates the control named "Show archived"
    Then the control named "Show Dones" has aria-pressed true
    And the control named "Show archived" has aria-pressed true
    And the Pre-production column shows a card whose id text is ".gamedev/phases/01-preproduction/audio-direction.md"

  # Limit Cases
  Scenario: Limit Case — Inbox stays visible under every filter
    Given GET /api/game-board?project=bevy inbox includes id ".grill/plans/bevy-funnel/epics/epic-003-juice.md" kind "epic" track null work_state null
    And GameBoardTab is mounted for project "bevy"
    When the operator activates the control named "Track A"
    Then the Inbox column shows a card whose id text is ".grill/plans/bevy-funnel/epics/epic-003-juice.md"
    When the operator activates the control named "Track B"
    Then the Inbox column shows a card whose id text is ".grill/plans/bevy-funnel/epics/epic-003-juice.md"
    And the control named "Show Dones" has aria-pressed false
    And the Inbox column still shows that card

  Scenario: Limit Case — Track B then All restores H
    Given GET /api/game-board?project=bevy production includes id ".gamedev/phases/02-production/levels/LVL-001-well" track "H" work_state "pending"
    And GameBoardTab is mounted for project "bevy"
    When the operator activates the control named "Track B"
    Then the Production column does not show a card whose id text is ".gamedev/phases/02-production/levels/LVL-001-well"
    When the operator activates the control named "All"
    Then the control named "All" has aria-pressed true
    And the Production column shows a card whose id text is ".gamedev/phases/02-production/levels/LVL-001-well"

  Scenario: Limit Case — remount resets all three controls
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/gdd.md" track "B" work_state "done" archived false
    And the operator had Show Dones pressed and Track A pressed in a previous mount
    When GameBoardTab mounts for project "bevy"
    Then the control named "Show Dones" has aria-pressed false
    And the control named "Show archived" has aria-pressed false
    And the control named "All" has aria-pressed true
    And the Pre-production column does not show a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"

  Scenario: Limit Case — project change resets filters; GET refetch does not
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/gdd.md" track "B" work_state "done" archived false
    And GameBoardTab is mounted for project "bevy"
    And the operator has activated Show Dones and Track B
    When GET /api/game-board?project=bevy is refetched and the same board is passed in
    Then the control named "Show Dones" has aria-pressed true
    And the control named "Track B" has aria-pressed true
    And the Pre-production column shows a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"
    When the project prop becomes "other-game"
    Then the control named "Show Dones" has aria-pressed false
    And the control named "All" has aria-pressed true

  Scenario: Limit Case — all phase cards hidden leaves header only
    Given every preproduction card has work_state "done" archived false
    And Show Dones is not pressed
    When the Game tab paints Pre-production
    Then the pane shows a column named "Pre-production"
    And that column does not show a card
    And the pane shows a control named "Show Dones"

  Scenario: Limit Case — leftover archived pending stays visible with Show Dones off
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/gdd.md" track "B" work_state "pending" archived true
    When GameBoardTab mounts for project "bevy"
    Then the control named "Show Dones" has aria-pressed false
    And the Pre-production column shows a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"

  Scenario: Limit Case — chrome has separators and Specs does not copy the controls
    Given GameBoardTab is mounted for project "bevy"
    Then the Game pane chrome shows the text "|"
    And the Game pane shows a control named "Show archived"
    And the Game pane shows a control named "Show Dones"
    And the Game pane shows a control named "Track A"
    And the Game pane shows a control named "Track B"
    And the Game pane shows a control named "All"
    And within the Inbox column there is no control named "Show Dones"
    And within the Inbox column there is no control named "Track A"
    When SpecBoardTab is mounted
    Then the document does not show a control named "Show Dones"
    And the document does not show a control named "Track A"
    And the document does not show a control named "All"

  # Error Scenarios
  Scenario: Error — Show archived alone does not reveal an archived done card
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/audio-direction.md" track "B" work_state "done" archived true
    And GameBoardTab is mounted for project "bevy"
    When the operator activates the control named "Show archived"
    Then the control named "Show archived" has aria-pressed true
    And the control named "Show Dones" has aria-pressed false
    And the Pre-production column does not show a card whose id text is ".gamedev/phases/01-preproduction/audio-direction.md"

  Scenario: Error — Show Dones alone does not reveal an archived done card
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/audio-direction.md" track "B" work_state "done" archived true
    And GameBoardTab is mounted for project "bevy"
    When the operator activates the control named "Show Dones"
    Then the control named "Show Dones" has aria-pressed true
    And the control named "Show archived" has aria-pressed false
    And the Pre-production column does not show a card whose id text is ".gamedev/phases/01-preproduction/audio-direction.md"

  Scenario: Error — Track A plus Show Dones still hides a Track B done card
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/gdd.md" track "B" work_state "done" archived false
    And GameBoardTab is mounted for project "bevy"
    When the operator activates the control named "Show Dones"
    And the operator activates the control named "Track A"
    Then the Pre-production column does not show a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"

  Scenario: Error — filters do not add query params or write skill trees
    Given GameBoardTab is mounted for project "bevy"
    When the operator activates the control named "Show Dones"
    And the operator activates the control named "Track B"
    Then no GET /api/game-board request is sent with a query param named "track"
    And no GET /api/game-board request is sent with a query param named "show_dones"
    And no GET /api/game-board request is sent with a query param named "show_archived"
    And "/tmp/bevy/.gamedev/state.md" is byte-identical to its contents before those activations
    And window.localStorage has no key whose name contains "showDones" or "gameTrack"
```

## Success Metrics
| Metric | Target | Current | Status |
| Done artifacts hidden on first Game paint | 100% of work_state done | shown today | not met |
| Archived done visible only if both toggles on | 100% | Show archived alone reveals | not met |
| Track A/B hides the other tracks and H | 100% | no track filter | not met |
| Inbox cards hidden by filters | 0 | 0 | met (must stay 0) |
| Skill-file writes / new HTTP | 0 | 0 | met (must stay 0) |
Primary KPI: first paint hides dones; both toggles required for archived dones; Track A/B/All; Inbox always visible.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + Vite + Tailwind tokens. No C, no new SQLite, no new HTTP.
- Filter is client-side over the existing GET `/api/game-board` arrays (spec-011/012).
- Constitution II.3: user-visible strings in `graph-ui/src/lib/i18n.ts` en+zh.
- Constitution III.1: chrome grayscale. GraphTab `colorForLabel` hex stays.
- useGameBoard stays one-shot. Filter state is React state on GameBoardTab.
- spec-012 leftover archived-on-pending and Archive POST stay.

Business:
- Game tab only. Specs / Graph / ADR unchanged.
- Default All includes H. H has no dedicated control.
- Session-only, same lifetime as Show archived.

Planner defaults A (reject a Gherkin scenario to change these):
1. Ref ticket none. No TD.
2. Game only. No Specs Show Dones / Track.
3. Show archived AND Show Dones for archived done. Inbox never filtered.
4. Track exclusive A | B | All. Default All. H only under All.
5. Session-only. Remount / `?project=` reset. GET refetch keeps state. No localStorage. No URL params.
6. Client filter only. Zero skill writes. No new GET/POST fields.
7. Separator is the text `|` in chrome. Accessible names: "Show Dones", "Track A", "Track B", "All".
8. Empty filtered column = header only.

## Out of Scope
- Specs tab filters (Done column, grill epics)
- A dedicated Track H control
- Persisted preference (localStorage, CBM, URL)
- New HTTP path, GET query params, or POST fields
- Writing `.gamedev/`, `.sdd-skill/`, or `.grill/`
- Changing Archive / Unarchive / expand / blocked strip / silent win / four columns / conversion / clipboard / no drag
- MCP game-board or filter tool
- Drag / mark-done / launcher / toast

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% [ ] review approved [ ] human docs confirmed [ ] zero skill writes [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- graph-ui only: GameBoardTab chrome + `visiblePhaseCards` (or successor) takes showDones + showArchived + track
- i18n `gameBoard.showDones` / track labels en+zh
- Vitest: first paint, AND rule, Track A/B/All, Inbox, remount, project change, Specs absence
- No C tests unless architect proves a daemon change is required (planner default: none)

## Questions for Architect (answered in plan.md)
- Segmented `aria-pressed` trio vs radiogroup named "Track" → aria-pressed trio (SDD-ADR-064)
- Whether `|` is a decorative span (`aria-hidden`) → yes, two spans
- Keep Show archived i18n on `specBoard.showArchived` vs copy into `gameBoard` → keep specBoard (SDD-ADR-064)
- Invert spec-012 tests that assume Show archived alone reveals an archived done card → yes (SDD-ADR-063)

## Questions for @implementer
- [ ] Map every Gherkin scenario to a graph-ui Vitest
- [ ] Invert spec-012 cases where Show archived alone revealed a done archived card
- [ ] Do not add GET query params or C changes
- [ ] Do not filter Inbox
- [ ] Do not add Show Dones / Track to SpecBoardTab
- [ ] Keep silent win, four columns, expand, archive POST, clipboard, no drag

## Related Specs
Depends on: spec-012-m2k-game-expand-archive-deps (Show archived, archive AND hide, leftover pending)
Companion to: spec-011-q5n-game-phase-board (track A/B/H, work_state, four columns)
Does not include: Specs filters, Track H control, persisted prefs

## Revision History
| Date | Author | Change |
| 2026-08-31 | @planner | draft from human answers: Game only; All includes H; archived AND dones; Inbox always; ticket=none; debt=no |
| 2026-09-01 | @planner | Gherkin approved by human. Status → approved. Handoff @architect. |

## Approval Sign-off
Spec Owner(@planner): approved / Gherkin scenarios(user): approved / Product Owner: pending / Architecture(@architect): pending
