# Spec-002-p8w: Project Workspace
Status: closed | Spec ID: spec-002-p8w-project-workspace
Ref Ticket: none | KPI: Entering a project shows a workspace whose default tab is Graph, last-indexed is visible in the workspace header, Specs appears only when .sdd-skill/ is present, and ADR is always a full tab (not a Dashboard modal) | Priority: P0
Created: 2026-08-29 | Tech Debt Ref: none

## Executive Summary
After spec-001, account home is Dashboard. Enter still dumps the operator into a bare GraphTab with a name chip and ×. Specs is unrouted. ADR exists only as an extracted modal (`AdrButton`) that Dashboard must not mount.

This spec adds the project interior: a workspace shell with a small tab strip (Graph always, Specs only if `.sdd-skill/` exists, ADR always), last-indexed in the header, and leave-to-Dashboard. The 3D canvas and Specs Kanban stay; ADR moves from modal-on-card to a first-class tab using the same `/api/adr` blob.

Business impact: one project is usable and extensible without the old global-tab IA. Phase 2 parse (spec-004) can attach later because Phase 1 treats the whole ADR document as human-editable and does not invent a generated-region marker.

## User Stories

### US-001: Enter opens a workspace on Graph
As an operator, I want Enter on a Dashboard row to open that project on Graph, So that I land on the product surface instead of a lab default.
Acceptance Criteria:
- [ ] Enter navigates to `?tab=graph&project=<name>`
- [ ] The workspace chrome (header + tab strip) is visible; GraphTab for that name is the main pane
- [ ] Default workspace tab is Graph (grill ADR-008)
- [ ] × / leave-project returns to Dashboard (`?tab=dashboard`, no `project=`)
- [ ] There is no in-workspace project picker; changing project is Dashboard-only

### US-002: Workspace tab strip is Graph, Specs?, ADR
As an operator, I want named tabs inside a project, So that Graph, Specs, and ADR are reachable without the old global Specs/Graph/Projects/Control bar.
Acceptance Criteria:
- [ ] Tab order when Specs is present: Graph | Specs | ADR
- [ ] Tab order when Specs is absent: Graph | ADR
- [ ] Activating a tab updates `?tab=` to `graph` | `specs` | `adr` and keeps `project=<name>`
- [ ] The selected tab is distinguishable (aria-current or equivalent accessible selected state)
- [ ] Tab ids are a closed set plus a documented extension point in code (hardcoded list now; no plugin runtime). Future tabs (e.g. gamedev) are not built here

### US-003: Specs tab exists only when .sdd-skill/ is present
As an operator, I want Specs hidden when the project has no sdd-skill, So that the strip does not show a dead or disabled placeholder.
Acceptance Criteria:
- [ ] Presence comes from the existing GET `/api/spec-board?project=<name>` field `sdd_skill_present`
- [ ] `sdd_skill_present === true` → Specs tab is in the strip; activating it shows the existing SpecBoard Kanban for that project (no ProjectPicker)
- [ ] `sdd_skill_present === false` or the request fails or has not returned yet → Specs tab is omitted (not disabled, no "not sdd-skill" hint in the strip)
- [ ] Graph and ADR remain usable while spec-board is loading or failing
- [ ] `?tab=specs&project=<name>` when Specs is omitted falls back to Graph (`?tab=graph&project=<name>`)

### US-004: ADR is always a workspace tab
As an operator, I want to read and edit the project ADR on a full tab, So that the record is not a Dashboard-row modal.
Acceptance Criteria:
- [ ] ADR tab is always in the workspace strip
- [ ] GET `/api/adr?project=<name>` loads the editor; POST `/api/adr` body is `{ "project": "<name>", "content": "<markdown>" }` (same contract as today's AdrButton)
- [ ] Empty document: textarea is empty; placeholder is the current AdrButton headings (`# Architecture Decision Record` / Context / Decision / Consequences). Placeholder is not persisted until Save
- [ ] Save shows visible success or stays on the error path below; Delete (existing: POST empty content) remains available when `has_adr` is true
- [ ] Phase 1 writes the whole blob as human-editable. No generated-region marker, HTML comment fence, or "CBM-GENERATED" split (spec-004 / grill ADR-007)
- [ ] Dashboard still has no ADR control (spec-001 regression)

### US-005: Last indexed is visible in the workspace header
As an operator, I want the same last-indexed datetime in the workspace header, So that I can judge freshness without returning to Dashboard.
Acceptance Criteria:
- [ ] Header shows project name and a visible datetime derived from `Project.indexed_at` via the existing `formatIndexedAt` helper
- [ ] Value comes from the `list_projects` cache already used by Dashboard (`useProjects`). No extra `/api/index-status` or health call for this field
- [ ] If the deep-linked name is not in the list, the name still shows and the datetime is omitted (do not invent a second freshness field)

### US-006: Deep links, aliases, and unsaved ADR
As an operator, I want old Dashboard bookmarks to keep working and ADR drafts not to vanish, So that navigation is predictable.
Acceptance Criteria:
- [ ] `?tab=graph&project=<name>` opens workspace Graph (spec-001 deep link still valid)
- [ ] `?tab=adr&project=<name>` opens workspace ADR
- [ ] `?tab=specs&project=<name>` opens Specs when present, else Graph
- [ ] `?tab=dashboard`, `?tab=stats`, `?tab=control`, unknown tab, or any workspace tab without `project=` → Dashboard (spec-001 aliases unchanged)
- [ ] Dirty ADR editor (content ≠ last loaded/saved) + navigate to another workspace tab or leave to Dashboard → browser `confirm`; dismiss keeps the ADR tab and the draft; accept completes the navigation
- [ ] GraphTab `colorForLabel` / layout hex stay unchanged; workspace chrome uses spec-001 grayscale tokens

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Project Workspace

  # Happy Paths
  Scenario: Enter opens workspace Graph with last-indexed and Graph+ADR tabs
    Given list_projects returns one project named "alpha" with root_path "/tmp/alpha" and indexed_at "2026-08-29T10:00:00Z"
    And GET /api/spec-board?project=alpha returns {"sdd_skill_present":false}
    When the operator activates Enter on the "alpha" row
    Then the URL query includes "tab=graph" and "project=alpha"
    And GraphTab for "alpha" is shown
    And the document shows a workspace tab named "Graph" and a workspace tab named "ADR"
    And the document does not show a workspace tab named "Specs"
    And the workspace header contains "alpha" and a visible datetime derived from "2026-08-29T10:00:00Z"
    And the document does not show a header tab named "Projects" or "Control"

  Scenario: Specs tab appears when sdd-skill is present
    Given list_projects returns one project named "alpha"
    And GET /api/spec-board?project=alpha returns {"sdd_skill_present":true,"columns":{"backlog":[],"active":[],"done":[]}}
    When the operator enters "alpha" and activates the workspace tab named "Specs"
    Then the URL query includes "tab=specs" and "project=alpha"
    And SpecBoard Kanban for "alpha" is shown
    And the document does not contain the SpecBoard project-picker copy

  Scenario: ADR tab loads and saves the existing blob
    Given list_projects returns one project named "alpha"
    And GET /api/adr?project=alpha returns {"has_adr":true,"content":"# Existing ADR\n","updated_at":"2026-08-29T09:00:00Z"}
    When the operator enters "alpha" and activates the workspace tab named "ADR"
    Then the URL query includes "tab=adr" and "project=alpha"
    And a textarea contains "# Existing ADR"
    When the operator replaces the textarea with "# Edited ADR" and activates Save
    Then POST /api/adr is sent with JSON {"project":"alpha","content":"# Edited ADR"}
    And the document does not contain a generated-region marker or the text "CBM-GENERATED"

  Scenario: Leave project returns to Dashboard
    Given the workspace is open on "?tab=graph&project=alpha" with a clean ADR editor
    When the operator activates the control that returns home
    Then the main view is Dashboard
    And the URL query is "?tab=dashboard" or equivalent with no "project=" parameter

  # Limit Cases
  Scenario: Limit Case — spec-board still loading omits Specs
    Given list_projects returns one project named "alpha"
    And GET /api/spec-board?project=alpha has not responded
    When the operator enters "alpha"
    Then GraphTab for "alpha" is shown
    And the document shows a workspace tab named "ADR"
    And the document does not show a workspace tab named "Specs"

  Scenario: Limit Case — empty ADR shows placeholder and does not persist it
    Given GET /api/adr?project=alpha returns {"has_adr":false,"content":""}
    When the operator opens "?tab=adr&project=alpha"
    Then a textarea is empty
    And the textarea placeholder contains "Architecture Decision Record"
    And no POST /api/adr is sent until the operator activates Save

  Scenario: Limit Case — specs deep link without sdd-skill falls back to Graph
    Given GET /api/spec-board?project=alpha returns {"sdd_skill_present":false}
    When the operator opens "?tab=specs&project=alpha"
    Then GraphTab for "alpha" is shown
    And the URL query includes "tab=graph" and "project=alpha"
    And the document does not show a workspace tab named "Specs"

  Scenario: Limit Case — indexed_at missing from list omits datetime
    Given the operator opens "?tab=graph&project=ghost"
    And list_projects returns {"projects":[]}
    Then GraphTab for "ghost" is shown
    And the workspace header contains "ghost"
    And the workspace header does not render a time element for last-indexed

  Scenario: Limit Case — Dashboard aliases still home
    Given the operator opens "?tab=stats"
    When the first paint completes
    Then Dashboard is visible
    And "Control Panel" is visible
    And the document does not show a workspace tab named "ADR"

  # Error Scenarios
  Scenario: Error — ADR save fails keeps the draft
    Given the ADR tab is open for "alpha" with textarea "# Draft"
    And POST /api/adr returns HTTP 500
    When the operator activates Save
    Then a visible error region is non-empty
    And the textarea still contains "# Draft"
    And the URL query still includes "tab=adr" and "project=alpha"

  Scenario: Error — unsaved ADR leave cancelled stays on ADR
    Given the ADR tab is open for "alpha"
    And the textarea differs from the last loaded content
    When the operator activates the control that returns home and dismisses the confirm dialog
    Then the URL query still includes "tab=adr" and "project=alpha"
    And the textarea still contains the dirty draft
    And Dashboard is not the main view

  Scenario: Error — spec-board request fails omits Specs
    Given list_projects returns one project named "alpha"
    And GET /api/spec-board?project=alpha returns HTTP 500
    When the operator enters "alpha"
    Then GraphTab for "alpha" is shown
    And the document shows a workspace tab named "ADR"
    And the document does not show a workspace tab named "Specs"

  Scenario: Error — workspace tab without project is Dashboard
    Given the operator opens "?tab=adr"
    When the first paint completes
    Then Dashboard is visible
    And the URL query does not require a project to show the list
```

## Success Metrics
| Metric | Target | Current | Status |
| Enter default tab | Graph 100% | Graph + workspace strip | met |
| Specs tab when sdd_skill_present=false | 0 renders | omitted | met |
| ADR on Dashboard rows | 0 | 0 | met |
| Last indexed in workspace header | 100% when name is in list | time[dateTime] | met |
Primary KPI: first metric plus Specs-hidden-when-absent plus ADR-as-tab.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + existing C HTTP. Prefer GET/POST `/api/adr` and GET `/api/spec-board`. New endpoints only if architect proves these cannot meet AC.
- `indexed_at` is already on `list_projects`. Reuse `formatIndexedAt`. Do not add a freshness field.
- `TabId` today is `dashboard | graph`. Workspace tabs extend routing; `stats`/`control` stay Dashboard aliases.
- GraphTab `colorForLabel` / EdgeLines hex stay byte-identical (grill ADR-011 / SDD-ADR-005).
- Chrome stays spec-001 grayscale tokens.

Business:
- Grill epic 002 only. Path 1:1 and legacy duplicates are spec-003. ADR parse-on-reindex is spec-004.
- Specs Kanban remains read-only (no write-back to `.sdd-skill/`).

Planner defaults (reject a Gherkin scenario to change these):
1. Tab registry = hardcoded list + documented TS type. No plugin/runtime registry.
2. Specs presence = existing spec-board payload. Omit tab until true. No strip hint when absent.
3. Whole `/api/adr` blob is manual in this spec. No generated/manual fence (spec-004).
4. `?tab=specs|adr` without `project=` → Dashboard. `?tab=specs` with project but no skill → Graph.
5. Last-indexed from `useProjects` list cache only.
6. Empty ADR = empty textarea + current placeholder; not pre-saved.
7. No project switcher inside the workspace. Leave → Dashboard only.
8. Tab order: Graph | Specs? | ADR.
9. Unsaved ADR uses `window.confirm` on tab change and on leave. Dismiss stays.
10. Delete-ADR (POST empty) stays on the ADR tab when `has_adr`.
11. AdrButton modal is not mounted anywhere after this spec (editor lives in the tab).

## Out of Scope
- Recolor 3D nodes/edges or redesign GraphTab canvas
- Redesign Specs Kanban columns/cards
- Path↔Project 1:1, duplicate-create redirect, legacy duplicate delete — spec-003
- ADR auto-fill / parse of context_ai.md + TECH_STACK.md + ARCHITECTURE_ADR.md — spec-004
- Generated vs manual ADR region markers
- Writing back to `.sdd-skill/` or source
- In-workspace project switcher
- gamedev / future tabs (extension point only)
- C indexer changes unless architect proves AC cannot be met in graph-ui
- Changing Dashboard list, Control polls, or create-index (spec-001 regressions only)

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% on touched graph-ui (reporter may be absent) [ ] review approved [ ] human docs confirmed [ ] Dashboard still 0 ADR / 0 get_graph_schema [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- Promote `TabId` (or a workspace-tab union) so `graph|specs|adr` are first-class with `project=`
- Workspace chrome: header (name + last-indexed + leave) + tab strip; GraphTab / SpecBoardTab / ADR editor as panes
- Lift AdrButton editor into an ADR tab; delete or stop exporting the modal trigger
- SpecBoardTab without ProjectPicker when hosted in workspace
- Presence fetch on workspace enter (not only when clicking Specs)
- i18n en+zh for tab labels, ADR save/error, last-indexed header
- Document the tab list as the extension point for later epics

## Questions for Architect (answered in plan.md)
- Whether AdrButton.tsx becomes the pane or a new AdrTab.tsx wrapping the same fetch/save
- How to avoid Specs flicker without a disabled placeholder (omit-until-true vs cached presence)
- Whether `TabId` stays one union or splits account vs workspace
- Confirm no C change; if `/api/spec-board` without columns is enough for presence-only

## Questions for @implementer
- [ ] Map every Gherkin scenario to a Vitest in graph-ui
- [ ] Do not mount AdrButton on Dashboard
- [ ] Do not call `get_graph_schema` from the workspace header path
- [ ] Do not write a generated-region marker into ADR content
- [ ] Do not change `colorForLabel` hex

## Related Specs
Depends on: spec-001-w3q-executive-dashboard (Dashboard home, grayscale tokens, Enter → graph)
Blocks: spec-004-adr-parse-on-reindex (needs ADR tab)
Supersedes: spec-001 routing that aliases `?tab=specs` (with project) to Dashboard — now a workspace tab
Companion to: .grill/plans/executive-ui-ia/epics/epic-002-project-workspace.md

## Revision History
| Date | Author | Change |
| 2026-08-29 | @planner | draft from grill epic-002 + ADR-004/007/008/010/011; debt=no (TD-002 is spec-003) |
| 2026-08-29 | @planner | Gherkin approved (user: approved). Status=approved. Handoff @architect. |
| 2026-08-29 | @architect | plan.md + tasks.md (5) + checklist.md. SDD-ADR-009..013. Awaiting plan approval. |
| 2026-08-29 | @planner | CLOSED. Human: "read and understood". 5/5 PASS DEV. KPI met. Constitution: no changes. |

## Approval Sign-off
Spec Owner(@planner): closed 2026-08-29 / Gherkin scenarios(user): approved 2026-08-29 / Product Owner: approved 2026-08-29 / Architecture(@architect): approved 2026-08-29
