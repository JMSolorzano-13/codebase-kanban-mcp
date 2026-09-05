# Spec-009-t4x: Specs tab grill presence
Status: completed | Spec ID: spec-009-t4x-specs-tab-grill-presence
Ref Ticket: none | KPI: A project with `.grill/` and no `.sdd-skill/` shows the Specs tab; the Kanban paints grill Todo (or noSpecs); `?tab=specs` stays on Specs; neither skill still omits the tab | Priority: P0
Created: 2026-08-30 | Tech Debt Ref: none

## Executive Summary
spec-008 already walks `.grill/` on GET `/api/spec-board` and emits `grill_skill_present` plus `epics[]` even when `.sdd-skill/` is absent. The workspace strip still treats Specs as sdd-only: the one-shot presence hook is true only if `sdd_skill_present === true`, `?tab=specs` falls back to Graph, and SpecBoardTab replaces the Kanban with the `notSddSkill` empty state whenever sdd is false.

A grill-only path therefore cannot open the Kanban that spec-008 already fills. This spec changes presence to sdd OR grill on the same one-shot GET. Tab label stays "Specs". In progress and Done stay specs-only (empty + existing `noSpecs` when there are no specs). gamedev does not show or hide the tab.

Business impact: an operator whose repo has only grill-skill can reach Mixed Todo without adopting sdd first.

## User Stories

### US-001: Specs tab when grill exists without sdd
As an operator on a project that has `.grill/` and no `.sdd-skill/`, I want the Specs tab in the workspace strip, So that I can open the Kanban that already lists grill epics.
Acceptance Criteria:
- [ ] Presence still comes from one shot of GET `/api/spec-board?project=<name>` (not the ~4s poll, not `/api/skill-presence`)
- [ ] Specs is shown when the GET is HTTP 200 and (`sdd_skill_present === true` OR `grill_skill_present === true`)
- [ ] Tab label stays "Specs". Tab order when shown remains Graph | Specs | ADR
- [ ] Enter from Dashboard still opens Graph. This spec does not change the default workspace tab
- [ ] Loading, non-200, or network failure still omit Specs (not disabled; no strip hint)

### US-002: Grill-only Kanban is the board, not notSddSkill
As an operator on Specs for a grill-only project, I want the three-column Kanban, So that unconverted epics appear in Todo instead of a "doesn't use sdd-skill" empty pane.
Acceptance Criteria:
- [ ] When `grill_skill_present` is true and `sdd_skill_present` is false, SpecBoardTab paints the Kanban
- [ ] The visible pane must not show the `notSddSkill` copy ("This project doesn't use sdd-skill — nothing to show here")
- [ ] Todo paints `epics[]` from the same GET (spec-008 card rules unchanged: letter E, title, summary, plan; no expand/archive)
- [ ] In progress and Done paint zero epic cards. With zero specs they show the existing `noSpecs` copy ("No specs yet")
- [ ] Grill dir present and `epics` empty: tab still shown; Todo shows `noSpecs`

### US-003: Deep-link tab=specs stays when grill-only
As an operator, I want `?tab=specs&project=<name>` to stay on Specs when grill is present, So that a bookmark or share link does not bounce to Graph.
Acceptance Criteria:
- [ ] `?tab=specs&project=<name>` + HTTP 200 + `grill_skill_present === true` → URL stays `tab=specs`; SpecBoardTab is the pane
- [ ] `?tab=specs&project=<name>` + neither `sdd_skill_present` nor `grill_skill_present` (or GET not 200 / still in flight) → fallback to Graph (`?tab=graph&project=<name>`) unchanged
- [ ] sdd-only and sdd+grill deep-links still open Specs

### US-004: Omit rules stay for neither-skill and gamedev-only
As an operator, I want Specs hidden when the path has neither sdd nor grill, So that a dead tab does not appear for graph-only projects or a future gamedev-only tree.
Acceptance Criteria:
- [ ] `sdd_skill_present` false and `grill_skill_present` false → Specs omitted
- [ ] A `.gamedev/` directory alone does not show Specs. GET `/api/spec-board` still must not emit `gamedev_skill_present`
- [ ] Do not read `.gamedev/` to decide the strip. Do not lock sdd xor gamedev on a path
- [ ] sdd-only (no `.grill/`): Specs still shown (spec-002 / spec-008 regression)

### US-005: Same GET, zero skill writes
As an operator, I want this change to stay a read of flags already on the board GET, So that presence does not add a poll, an endpoint, or a skill-file write.
Acceptance Criteria:
- [ ] No new HTTP path. No MCP board/presence tool
- [ ] GET 200 leaves `.grill/` and `.sdd-skill/` byte-identical
- [ ] POST `/api/spec-board` stays spec-only (epic id still 404 `spec not found`)
- [ ] Graph, Dashboard, ADR tab, Path 1:1, ADR fill, spec-005 expand, spec-006 archive, spec-007 `formatIndexedAt`, spec-008 epic cards/order/cap stay unchanged
- [ ] Presence flags stay additive (`sdd_skill_present` + `grill_skill_present`)

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Specs tab grill presence

  # Happy Paths
  Scenario: Grill-only project shows the Specs tab
    Given project "alpha" has root_path "/tmp/alpha"
    And "/tmp/alpha/.grill/" exists as a directory
    And "/tmp/alpha/.sdd-skill/" does not exist
    And GET /api/spec-board?project=alpha returns HTTP 200
    And that body has sdd_skill_present false
    And that body has grill_skill_present true
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is shown
    And the tab named "Specs" has accessible name "Specs"
    And the tab order is Graph then Specs then ADR

  Scenario: Grill-only Kanban paints an epic in Todo and empty spec columns
    Given GET /api/spec-board?project=alpha returns HTTP 200
    And sdd_skill_present is false
    And grill_skill_present is true
    And epics has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And that epic entry has title "inbox"
    And specs is []
    When the operator activates the tab named "Specs"
    Then the pane does not show the text "This project doesn't use sdd-skill — nothing to show here"
    And the Todo column shows a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And that card shows the text "E"
    And that card shows the text "inbox"
    And the In progress column shows the text "No specs yet"
    And the Done column shows the text "No specs yet"
    And the In progress column does not show a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And the Done column does not show a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Deep-link tab=specs stays on Specs when grill-only
    Given GET /api/spec-board?project=alpha returns HTTP 200 with sdd_skill_present false and grill_skill_present true
    When the operator opens "?tab=specs&project=alpha"
    Then the URL query includes "tab=specs" and "project=alpha"
    And SpecBoardTab is shown
    And the URL query does not include "tab=graph"

  Scenario: sdd plus grill still shows Specs
    Given GET /api/spec-board?project=alpha returns HTTP 200 with sdd_skill_present true and grill_skill_present true
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is shown

  # Limit Cases
  Scenario: Limit Case — grill directory with no epics still shows Specs
    Given GET /api/spec-board?project=alpha returns HTTP 200
    And sdd_skill_present is false
    And grill_skill_present is true
    And epics is []
    And specs is []
    When the operator activates the tab named "Specs"
    Then the tab named "Specs" is shown
    And the pane does not show the text "This project doesn't use sdd-skill — nothing to show here"
    And the Todo column shows the text "No specs yet"
    And the In progress column shows the text "No specs yet"
    And the Done column shows the text "No specs yet"

  Scenario: Limit Case — neither skill omits Specs and deep-link falls back to Graph
    Given GET /api/spec-board?project=alpha returns HTTP 200 with sdd_skill_present false and grill_skill_present false
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is not shown
    And the tab order is Graph then ADR
    When the operator opens "?tab=specs&project=alpha"
    Then the URL query includes "tab=graph" and "project=alpha"
    And GraphTab for "alpha" is shown

  Scenario: Limit Case — sdd-only without grill still shows Specs
    Given GET /api/spec-board?project=alpha returns HTTP 200 with sdd_skill_present true and grill_skill_present false
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is shown

  Scenario: Limit Case — gamedev directory alone does not show Specs
    Given project "alpha" has root_path "/tmp/alpha"
    And "/tmp/alpha/.gamedev/" exists as a directory
    And "/tmp/alpha/.sdd-skill/" does not exist
    And "/tmp/alpha/.grill/" does not exist
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And sdd_skill_present is false
    And grill_skill_present is false
    And the response JSON has no field whose name is "gamedev_skill_present"
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is not shown

  Scenario: Limit Case — Enter still opens Graph on a grill-only project
    Given GET /api/spec-board?project=alpha returns HTTP 200 with sdd_skill_present false and grill_skill_present true
    When the operator activates Enter on the "alpha" row
    Then the URL query includes "tab=graph" and "project=alpha"
    And the tab named "Specs" is shown
    And GraphTab for "alpha" is shown

  # Error Scenarios
  Scenario: Error — GET 500 omits Specs
    Given GET /api/spec-board?project=alpha returns HTTP 500
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is not shown
    And GraphTab remains usable

  Scenario: Error — GET 404 omits Specs
    Given GET /api/spec-board?project=alpha returns HTTP 404
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is not shown

  Scenario: Error — request still in flight omits Specs
    Given GET /api/spec-board?project=alpha has not returned
    When the operator is on the workspace for project "alpha"
    Then the tab named "Specs" is not shown

  Scenario: Error — GET does not write skill trees
    Given project "alpha" root_path is "/tmp/alpha"
    And "/tmp/alpha/.grill/" exists as a directory
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And "/tmp/alpha/.grill/" files are byte-identical to their contents before the GET
    And "/tmp/alpha/.sdd-skill/" does not get created
```

## Success Metrics
| Metric | Target | Current | Status |
| Grill-only path shows Specs tab | 100% when GET 200 and grill_skill_present | 0 (sdd-only hook) | not met |
| Grill-only Specs pane is Kanban not notSddSkill | 100% | 0 (host gates on sdd) | not met |
| Deep-link tab=specs grill-only stays Specs | 100% | falls back to Graph | not met |
| Neither-skill / gamedev-only Specs tab | 0 | 0 | met (must stay 0) |
| Skill-file writes from GET or presence UI | 0 | 0 | met (must stay 0) |
| Second presence HTTP path | 0 | 0 | met (must stay 0) |
Primary KPI: first three metrics plus zero skill writes.

## Constraints & Assumptions
Technical:
- GET `/api/spec-board` already emits `grill_skill_present` and fills `epics[]` without sdd (spec-008 / `cbm_spec_board_read`). This spec does not change that JSON shape unless a flag is missing on an existing 200 (it is not).
- One-shot GET for the strip. `useSpecBoard` 4s poll stays board refresh only.
- Chrome grayscale. Tab label i18n key `tabs.specs` unchanged. Tests may assert English.
- Constitution I.2: indexer/graph-ui do not write skill cycle files.
- Constitution IV.3: no new endpoint.

Business:
- Grill epic-002. spec-008 Mixed Todo stays the epic-card contract.
- gamedev out (grill ADR-006). Presence stays additive.

Planner defaults (reject a Gherkin scenario to change these):
1. Present iff HTTP 200 and (`sdd_skill_present === true` OR `grill_skill_present === true`).
2. Same one-shot GET `/api/spec-board`. Never `/api/skill-presence` for the strip.
3. Grill-only paints the Kanban. `notSddSkill` must not appear on that pane.
4. Empty spec columns keep existing `noSpecs` copy. No new empty-state string required.
5. Deep-link `tab=specs` + grill-only stays on Specs. Loading / non-200 / neither-skill still fallback to Graph.
6. `.gamedev/` alone does not show Specs. Do not emit `gamedev_skill_present` on this GET.
7. Enter / default workspace tab stays Graph.
8. Zero skill writes. No MCP tool. No second poll.
9. Tab label stays "Specs".
10. spec-008 scenario "Specs tab still requires sdd_skill_present" is superseded (must be updated or removed, not left green against the old Then).

## Out of Scope
- Changing Mixed Todo card rules, conversion match, order, or epic cap (spec-008)
- Drag, button, or CBM write that creates a spec or deletes/moves an epic
- Reading `.gamedev/`, painting or hiding via gamedev
- Deciding whether a path may hold sdd and gamedev at once
- Renaming the tab away from "Specs"
- Changing default Enter tab away from Graph
- New HTTP path or MCP presence tool
- Changing spec-005 expand, spec-006 archive, spec-007 `formatIndexedAt`

## Acceptance Checklist
- [x] all US implemented [x] all AC met [x] ALL Gherkin scenarios pass [x] tests>80% on touched graph-ui (reporter may be absent) [x] review approved [x] human docs confirmed [x] zero skill writes [x] security passed [ ] manual test by owner done

## Architecture Considerations
- Presence hook today: `sdd_skill_present === true` only. Must OR `grill_skill_present === true` on the same 200 body
- `fallbackSpecsToGraph` / App strip consume that present flag; grill-only must not rewrite to Graph
- SpecBoardTab host gate today: `!board.sdd_skill_present` → `notSddSkill`. Grill-only must reach Column render
- C GET path likely unchanged; add tests only if a regression in flag emission is found
- i18n: no required new keys; `notSddSkill` may remain unused on the grill-only path
- Vitest: hook + App/strip deep-link + SpecBoardTab host. Update spec-008 tests that assert the old omit

## Questions for Architect (answered in plan.md)
- Keep hook name `useSddSkillPresent` or rename to a skill-presence hook (behavior is sdd OR grill)
- Keep `notSddSkill` as last-resort when both flags are false on an already-mounted pane, or drop that branch
- Any C/HTTP work at all, or graph-ui only

## Questions for @implementer
- [ ] Map every Gherkin scenario to a Vitest (and C only if GET flags change)
- [ ] Invert or update spec-008 "Specs tab still requires sdd_skill_present"
- [ ] Do not write `.grill/` or `.sdd-skill/`
- [ ] Do not show Specs for gamedev-only
- [ ] Do not add a second GET or an MCP tool
- [ ] Do not rename the tab
- [ ] Do not change Enter default away from Graph

## Related Specs
Depends on: spec-002-p8w-project-workspace (strip + omit-until), spec-008-g8r-grill-epic-todo (GET flags + epics)
Supersedes: spec-008 US-006 / Gherkin "Specs tab still requires sdd_skill_present" (tab omit is now sdd OR grill)
Companion to: .grill/plans/add-epics-plans-kanban/epics/epic-002-specs-tab-grill-presence.md
Does not include: spec-008 Mixed Todo card/conversion/cap work (already closed)

## Revision History
| Date | Author | Change |
| 2026-08-30 | @planner | draft from grill epic-002 + ADR-001,006,008; debt=no; ticket=none |
| 2026-08-30 | @planner | Gherkin approved; status approved; handoff @architect |

## Approval Sign-off
Spec Owner(@planner): ✓ 2026-08-30 / Gherkin scenarios(user): ✓ 2026-08-30 / Product Owner: ✓ 2026-08-30 / Architecture(@architect): pending
