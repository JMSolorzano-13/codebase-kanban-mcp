# Spec-001-w3q: Executive Dashboard
Status: completed | Spec ID: spec-001-w3q-executive-dashboard
Ref Ticket: none | KPI: Opening localhost:9749 shows Dashboard (project list + Control) with last-indexed visible and zero Nodes/Edges sections, without entering a project | Priority: P0
Created: 2026-08-29 | Tech Debt Ref: TD-001, TD-004

## Executive Summary
The graph UI home is a lab console: global tabs default to Specs, Projects (StatsTab) dumps node/edge totals and label chips, Control is a sibling tab, chrome is teal-green.

This spec replaces account home with an executive Dashboard: indexed projects and Control on one screen, grayscale chrome, last index datetime on each row, create-index without a Project ID field. Enter still opens the existing Graph tab until spec-002 builds the workspace shell.

Business impact: the operator can judge index freshness and daemon health without reading graph cardinality.

## User Stories

### US-001: Dashboard is the account home
As an operator, I want localhost:9749 to open on Dashboard, So that I see projects and Control without picking a project or a lab tab.
Acceptance Criteria:
- [ ] With no `?tab=` query (or unknown tab), the main view is Dashboard
- [ ] Header has no sibling tabs labeled Specs, Graph, Projects, or Control as the account IA
- [ ] `?tab=stats` and `?tab=control` render Dashboard (bookmark alias)
- [ ] `?tab=graph&project=<name>` still opens the existing GraphTab for that name
- [ ] From GraphTab, a control returns to Dashboard (replaces today's × → stats)

### US-002: Project row is identity + freshness, not graph dumps
As an operator, I want each project row to show name, path, last indexed, and health, So that I can choose what to enter without counting nodes.
Acceptance Criteria:
- [ ] Each row shows `Project.name`, `Project.root_path`, and `Project.indexed_at` (visible datetime, not hidden)
- [ ] HealthDot remains (healthy green / missing amber / corrupt red / loading gray) with the existing tooltip labels
- [ ] Row actions: Enter (current copy may change from "View Graph" to Enter) and Delete-with-confirm
- [ ] No ADR button or ADR modal on Dashboard rows
- [ ] No Nodes/Edges aggregate cards
- [ ] No per-row node count, edge count, or label chips
- [ ] Rendering the list does not call `get_graph_schema`

### US-003: Control lives on Dashboard
As an operator, I want daemon CPU/RAM, process list, and logs on the same screen as the project list, So that I do not open a second account tab.
Acceptance Criteria:
- [ ] Control Panel heading and the current gauges (Total CPU, Total RAM, Processes, Self RAM) are visible on Dashboard
- [ ] Active Processes and Process Logs remain (full current Control, not a compact summary)
- [ ] Existing polls stay: `/api/processes` about 3s, `/api/logs` about 2s
- [ ] Control remains visible when the project list is empty

### US-004: Grayscale chrome, semantic health only
As an operator, I want dark gray chrome with contrast on buttons and links, So that the product reads executive and the 3D graph stays colorful.
Acceptance Criteria:
- [ ] Dashboard, header, buttons, links, Control, and create-index modal do not use `#1DA27E` / `#1C8585` as the primary chrome accent
- [ ] Surfaces use distinct gray levels (background vs card vs hover vs border) so primary actions remain findable
- [ ] HealthDot and Control error/warning colors stay semantic (green/amber/red)
- [ ] `colorForLabel` output is unchanged; GraphTab node/edge hues match pre-spec screenshots/tests

### US-005: Create index has no Project ID field
As an operator, I want to browse a folder and index it, So that the project name is always derived from the path.
Acceptance Criteria:
- [ ] Create-index modal has no Project ID / project name input
- [ ] POST `/api/index` body is `{ "root_path": "<selected path>" }` only (no `project_name` key)
- [ ] Folder browse, Windows breadcrumbs, filter, and "Index This Folder" behavior stay
- [ ] Successful start still shows IndexProgress on Dashboard and does not auto-enter Graph

### US-006: Empty, indexing, and list errors
As an operator, I want a usable home when there are zero projects or a job is running, So that I can index without a blank operational panel.
Acceptance Criteria:
- [ ] Zero projects: empty copy + CTA to open create-index, and Control still shown
- [ ] While a job is indexing, IndexProgress banner is on Dashboard
- [ ] If `list_projects` / `/rpc` fails, a destructive-styled error is shown and Control still shown
- [ ] Delete confirm cancel leaves the row in the list

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Executive Dashboard

  # Happy Paths
  Scenario: Default URL opens Dashboard
    Given the UI is served with no query string
    When the operator loads the app
    Then the document shows the text "Indexed Projects" or the empty-state CTA "Index your first repository"
    And the document shows the text "Control Panel"
    And the document does not show a header tab named "Specs"
    And the document does not show a header tab named "Graph"
    And the document does not show a header tab named "Projects"
    And the document does not show a header tab named "Control"

  Scenario: Two projects render identity and freshness only
    Given list_projects returns
      | name   | root_path        | indexed_at           |
      | alpha  | /tmp/alpha       | 2026-08-29T10:00:00Z |
      | beta   | /tmp/beta        | 2026-08-29T11:30:00Z |
    And get_graph_schema is never required to paint the list
    When the operator opens Dashboard
    Then the document contains "alpha" and "/tmp/alpha" and a visible datetime derived from "2026-08-29T10:00:00Z"
    And the document contains "beta" and "/tmp/beta" and a visible datetime derived from "2026-08-29T11:30:00Z"
    And the document does not contain the aggregate labels "nodes" and "edges" as StatsTab does today
    And the document does not contain label chips "Function" or "Class" sourced from schema
    And the document does not contain an ADR control on either row
    And the test spy records zero RPC calls whose tool name is "get_graph_schema"

  Scenario: Enter opens existing Graph, back returns to Dashboard
    Given list_projects returns one project named "alpha" with root_path "/tmp/alpha"
    When the operator activates Enter on that row
    Then the URL query includes "tab=graph" and "project=alpha"
    When the operator activates the control that returns home
    Then the main view is Dashboard again
    And the URL does not require "project=alpha" to show the list

  Scenario: Create index posts path only
    Given the create-index modal is open on a POSIX browse listing for "/home/dev" with dirs ["alpha"]
    When the operator activates "Index This Folder" for "/home/dev"
    Then the POST body to "/api/index" equals {"root_path":"/home/dev"}
    And the request JSON has no "project_name" property
    And the document does not contain the accessible name "Project ID (optional — permanent, cannot be renamed)"
    And after HTTP 202 the Dashboard shows IndexProgress
    And the URL query does not become "tab=graph"

  Scenario: Control polls remain on the same screen
    Given Dashboard is visible
    When 3.5 seconds elapse
    Then the client has requested "/api/processes" at least twice
    And the client has requested "/api/logs" at least twice

  # Limit Cases
  Scenario: Limit Case — zero projects still shows Control
    Given list_projects returns {"projects":[]}
    When the operator opens Dashboard
    Then the document contains "No indexed projects"
    And the document contains "Index your first repository"
    And the document contains "Control Panel"

  Scenario: Limit Case — bookmark tab=stats aliases Dashboard
    Given the operator opens the app with query "?tab=stats"
    When the first paint completes
    Then Dashboard is visible (project list or empty CTA)
    And "Control Panel" is visible

  Scenario: Limit Case — bookmark tab=control aliases Dashboard
    Given the operator opens the app with query "?tab=control"
    When the first paint completes
    Then Dashboard is visible
    And "Control Panel" is visible

  Scenario: Limit Case — graph deep link still works
    Given a project named "alpha" exists
    When the operator opens "?tab=graph&project=alpha"
    Then GraphTab for "alpha" is shown
    And GraphTab node colors still come from colorForLabel (unchanged hex for label "Function")

  # Error Scenarios
  Scenario: Error — list_projects RPC fails
    Given /rpc list_projects returns a non-OK or thrown error
    When the operator opens Dashboard
    Then a visible error region uses the destructive color token
    And the error text is non-empty
    And "Control Panel" is still visible

  Scenario: Error — index POST fails
    Given the create-index modal is open
    And POST /api/index returns HTTP 400 with {"error":"not a directory"}
    When the operator submits the current path
    Then the modal stays open
    And the document contains "not a directory"

  Scenario: Error — delete cancelled
    Given Dashboard lists project "alpha"
    When the operator activates Delete and dismisses the confirm dialog
    Then "alpha" remains in the list
    And no DELETE request to "/api/project?name=alpha" is sent
```

## Success Metrics
| Metric | Target | Current | Status |
| Default view is Dashboard | 100% of no-query loads | Dashboard | met |
| get_graph_schema on Dashboard list | 0 calls | 0 | met |
| Nodes/Edges sections on home | 0 | 0 | met |
| Last indexed visible per row | 100% | formatIndexedAt + time[dateTime] | met |
Primary KPI: first metric plus zero Nodes/Edges sections.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + existing C HTTP. No new backend field for last indexed.
- POST `/api/index` without `project_name` already derives the name in C (`cbm_project_name_from_path`). Spec-001 does not add Path 1:1 reject/redirect (epic 003).
- SpecBoard ProjectPicker must keep working if something still mounts it; it must not depend on schema.

Business:
- Grill epic 001 only. Workspace tabs, ADR tab, Path 1:1, ADR parse are later specs.

Planner defaults (reject a Gherkin scenario to change these):
1. Empty state shows Control + CTA (does not hide Control).
2. IndexProgress stays on Dashboard.
3. New Index is a page-level button, not a per-row action. Rows: Enter + Delete only.
4. Control is the full current panel, stacked on the same scroll surface as the list (not a compact teaser).
5. After a successful first index, stay on Dashboard.
6. HealthDot stays with semantic colors.
7. `?tab=stats` and `?tab=control` alias Dashboard; default URL is Dashboard.

## Out of Scope
- Recolor 3D nodes/edges or redesign GraphTab canvas
- Project workspace tab strip (Graph / Specs? / ADR) — spec-002
- Path↔Project uniqueness, redirect on duplicate create, legacy duplicate delete — spec-003
- ADR auto-fill from sdd-skill files — spec-004
- Writing back to `.sdd-skill/` or source
- Renaming existing custom project names
- Compact Control or hiding Control until the first project
- Auto-enter Graph after create
- C indexer / spec_board.c changes unless architect proves AC cannot be met in graph-ui

## Acceptance Checklist
- [x] all US implemented [x] all AC met [x] ALL Gherkin scenarios pass [ ] tests>80% on touched graph-ui (reporter not installed) [x] review approved [x] human docs confirmed [x] Dashboard schema RPC = 0 [x] security passed [ ] manual test by owner done

## Architecture Considerations
- Likely split `useProjects` into list-only vs schema-enriched
- Replace StatsTab as home; keep IndexProgress + CreateIndexModal (field removed)
- Embed ControlTab on Dashboard
- Chrome tokens in globals.css scoped vs graph
- i18n en+zh for last-indexed, Enter, Dashboard
- App.tsx default route and tab IA

## Questions for Architect (answered in plan.md)
- Stack vs two-pane layout for list + Control on one scroll surface
- Exact datetime format for `indexed_at` (locale vs ISO); must be visible and derived from the field
- Whether to delete or reuse TabBar.tsx (TD-003)
- How to prove GraphTab colors unchanged (unit assertion on `colorForLabel("Function")`)

## Questions for @implementer
- [ ] Map every Gherkin scenario to a Vitest in graph-ui
- [ ] Do not send `project_name` even if leftover state exists
- [ ] Do not call `get_graph_schema` from the Dashboard data path

## Related Specs
Depends on: none
Blocks: spec-002 (workspace assumes Dashboard home + grayscale tokens)
Supersedes: none
Companion to: .grill/plans/executive-ui-ia/epics/epic-001-executive-dashboard.md

## Revision History
| Date | Author | Change |
| 2026-08-29 | @planner | draft from grill epic-001 + ADR-002/003/010/011/012 |
| 2026-08-29 | @planner | Gherkin approved by user; status → approved |
| 2026-08-29 | @planner | Closed after human confirmation; KPI met; constitution unchanged |

## Approval Sign-off
Spec Owner(@planner): approved 2026-08-29 / Gherkin scenarios(user): approved 2026-08-29 / Product Owner: approved via Gherkin / Architecture(@architect): plan approved 2026-08-29 / Closed: 2026-08-29
