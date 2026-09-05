# Spec-005-v2m: Spec card expand
Status: closed | Spec ID: spec-005-v2m-spec-card-expand
Ref Ticket: none | KPI: On the Specs Kanban, a click on a Todo or Done card (and In Progress) expands that same card in place and shows a 1-2 sentence Executive Summary blurb when present plus the column-appropriate task list | Priority: P0
Created: 2026-08-30 | Tech Debt Ref: none

## Executive Summary
The workspace Specs tab already hosts the three-column Kanban (spec-002). Only the active spec can expand, and only when it has tasks. Todo and Done cards are dead: no title from spec.md, no objective, no task list. `spec_board.c` parses title/tasks/checklist/agent only for `active.json:active_spec`.

This spec reuses the existing in-card expand (grill ADR-002) on all three columns. GET `/api/spec-board` stays the one endpoint: each entry gains an additive `blurb` and the same title/tasks enrichment for every listed spec. Todo shows pending tasks only; In Progress shows all plus status; Done shows the full list. Archive is the next spec (grill epic-002). Last-indexed timezone is a later spec (grill epic-003).

Business impact: the operator can open any card and see why the spec exists and which tasks it has, without a new overlay or a write into `.sdd-skill/`.

## User Stories

### US-001: Any spec card expands in place
As an operator, I want to click the title control on a Todo, In Progress, or Done card and have that same card grow, So that I do not need an overlay or a separate spec page.
Acceptance Criteria:
- [ ] Click target is the existing title button on the card (not a new floating popover, modal, side panel, or accordion)
- [ ] A card expands when `task_count` is 0 (blurb-only or the existing no-tasks copy)
- [ ] The active spec still starts expanded on first paint; non-active cards start collapsed
- [ ] Several cards may stay expanded at once; opening one does not collapse another (grill ADR-008)
- [ ] A later GET `/api/spec-board` poll must not collapse a card the operator opened (expanded state is per spec id)
- [ ] Active-spec chrome (current agent, N/M tasks, checklist percent, blocked note) stays outside the task list and remains as today

### US-002: Blurb is 1-2 sentences from Executive Summary
As an operator, I want the expand body to start with a short objective taken from `spec.md` `## Executive Summary`, So that I see why the spec exists without reading KPI or invented copy.
Acceptance Criteria:
- [ ] `blurb` is an additive string field on each GET `/api/spec-board` entry (same object as today; no second endpoint)
- [ ] Source is the body of `## Executive Summary` until the next `## ` heading. Not the KPI line. Not the H1
- [ ] Value is the first 1-2 sentences of that body. Sentence end = `.` / `?` / `!` followed by space or end of text. Markdown links become their link text
- [ ] Missing header, empty body, or whitespace-only body → `blurb` is `""`. UI omits the blurb region. Card still expands (grill ADR-006)
- [ ] A `## KPI` (or any other) section is never copied into `blurb`

### US-003: Todo expand lists pending tasks only
As an operator, I want a Todo card expand to list only unfinished tasks, So that planned work is not mixed with already-passed items.
Acceptance Criteria:
- [ ] Todo expand renders tasks where `done` is false
- [ ] Zero pending tasks (including `task_count` 0) → existing English copy "No tasks planned yet"
- [ ] Non-active specs: `done` is true only when `history/test_results.log` has a line that contains both that spec's id and `Task #N` and `PASS` (last matching line wins). A bare `Task #N` without the spec id does not mark a non-active task
- [ ] Title for a Todo card is the spec.md first-line H1 (same strip-`#` parse as today's active title). Empty/missing spec.md → visible text is the spec id

### US-004: In Progress expand lists all tasks plus status
As an operator, I want the In Progress expand to show every task with current/done/pending, So that the active card stays as informative as today plus the blurb.
Acceptance Criteria:
- [ ] In Progress expand shows every task from `tasks.md` (capped at `CBM_SPEC_BOARD_MAX_TASKS` 48)
- [ ] Each visible row still includes `#N` and the heading name
- [ ] Active spec keeps today's `current` (from `state.md` task number) and `done` (from the shared log, last `Task #N` line wins — existing active matcher)
- [ ] Agent, N/M, checklist, and blocked remain on the card outside the task list (collapsed or expanded)

### US-005: Done expand lists every task and does not archive
As an operator, I want a Done card to open with its full task list, So that I can recall what shipped before archive exists.
Acceptance Criteria:
- [ ] Done expand shows every task from that spec's `tasks.md` (same 48 cap)
- [ ] The document does not show a control named "Archive" or "Unarchive"
- [ ] Done cards do not leave the Done column in this spec
- [ ] Zero tasks → "No tasks planned yet"; expand still works

### US-006: Board read stays zero-write and one GET
As an operator, I want titles, blurbs, and tasks for all listed specs on the existing poll, So that expand does not require a second fetch or a skill-file write.
Acceptance Criteria:
- [ ] GET `/api/spec-board?project=<name>` remains the only spec-board HTTP read. Caps stay 64 specs / 48 tasks. Poll interval may stay ~4s
- [ ] Planned, draft, active, and completed entries each include `title`, `blurb`, `task_count`, and `tasks` when those files exist
- [ ] No write to `.sdd-skill/` (not `active.json`, not `spec.md`, not `tasks.md`, not Status). `spec_board.c` stays a reader
- [ ] Missing or unreadable `spec.md` / `tasks.md` for one spec degrades that entry (empty title/blurb/tasks) and does not fail the whole board
- [ ] Graph, Dashboard, ADR tab, Path 1:1, and ADR parse-on-reindex behavior are unchanged

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Spec card expand

  # Happy Paths
  Scenario: Todo card expands with blurb and pending tasks only
    Given project "alpha" has root_path "/tmp/alpha" and sdd_skill_present is true
    And GET /api/spec-board?project=alpha includes a spec with id "spec-010-aaa-planned" column "todo" active false
    And that entry has title "Spec-010-aaa: Planned Work"
    And that entry has blurb "Planned work ships the inbox filter. Operators see unread first."
    And that entry has tasks [{"number":1,"name":"Write parser","done":true,"current":false},{"number":2,"name":"Write tests","done":false,"current":false}]
    And the Specs tab is showing the Kanban for "alpha"
    When the operator activates the title control on the card whose id text is "spec-010-aaa-planned"
    Then that card shows the text "Planned work ships the inbox filter. Operators see unread first."
    And that card shows the text "#2 Write tests"
    And that card does not show the text "#1 Write parser"

  Scenario: In Progress card shows blurb and every task with status chrome
    Given GET /api/spec-board?project=alpha includes a spec with id "spec-011-bbb-active" column "in_progress" active true
    And that entry has blurb "Active work fills the card."
    And that entry has current_agent "implementer" and task_count 2 and tasks_done 1
    And that entry has tasks [{"number":1,"name":"Write parser","done":true,"current":false},{"number":2,"name":"Write tests","done":false,"current":true}]
    And the Specs tab is showing the Kanban for "alpha"
    When the first paint of that card completes
    Then that card is expanded
    And that card shows the text "Active work fills the card."
    And that card shows the text "#1 Write parser"
    And that card shows the text "#2 Write tests"
    And that card shows the text "implementer"
    And that card shows the text "1/2 tasks"

  Scenario: Done card expands with full task list and no Archive control
    Given GET /api/spec-board?project=alpha includes a spec with id "spec-012-ccc-closed" column "done" active false
    And that entry has blurb "Closed work is still readable."
    And that entry has tasks [{"number":1,"name":"Write parser","done":true,"current":false},{"number":2,"name":"Write tests","done":true,"current":false}]
    And the Specs tab is showing the Kanban for "alpha"
    When the operator activates the title control on the card whose id text is "spec-012-ccc-closed"
    Then that card shows the text "Closed work is still readable."
    And that card shows the text "#1 Write parser"
    And that card shows the text "#2 Write tests"
    And the document does not show a control named "Archive"
    And the document does not show a control named "Unarchive"

  Scenario: Two cards stay expanded at once
    Given the Todo card "spec-010-aaa-planned" is expanded
    And the Done card "spec-012-ccc-closed" is collapsed
    When the operator activates the title control on the card whose id text is "spec-012-ccc-closed"
    Then the card "spec-010-aaa-planned" still shows its blurb
    And the card "spec-012-ccc-closed" shows its blurb

  # Limit Cases
  Scenario: Limit Case — empty Executive Summary omits blurb and still expands
    Given GET /api/spec-board?project=alpha includes id "spec-013-ddd-empty" column "todo" blurb ""
    And that entry has tasks [{"number":1,"name":"Write parser","done":false,"current":false}]
    When the operator activates the title control on the card whose id text is "spec-013-ddd-empty"
    Then that card shows the text "#1 Write parser"
    And that card does not contain a blurb region
    And that card does not show the text "KPI"

  Scenario: Limit Case — zero tasks still expands with no-tasks copy
    Given GET /api/spec-board?project=alpha includes id "spec-014-eee-bare" column "todo" blurb "Bare spec still opens." task_count 0 tasks []
    When the operator activates the title control on the card whose id text is "spec-014-eee-bare"
    Then that card shows the text "Bare spec still opens."
    And that card shows the text "No tasks planned yet"

  Scenario: Limit Case — KPI section is not used as blurb
    Given "/tmp/alpha/.sdd-skill/specs/spec-015-fff-kpi/spec.md" has a "## KPI" line containing "KPI-MUST-NOT-BLURB" and has no "## Executive Summary" heading
    When GET /api/spec-board?project=alpha is read for id "spec-015-fff-kpi"
    Then that entry has blurb ""
    And that entry title or blurb does not contain "KPI-MUST-NOT-BLURB"

  Scenario: Limit Case — poll does not collapse an opened non-active card
    Given the operator has expanded the Todo card "spec-010-aaa-planned"
    When GET /api/spec-board?project=alpha returns the same specs again (poll)
    Then that card still shows the text "Planned work ships the inbox filter. Operators see unread first."

  Scenario: Limit Case — non-active done requires spec id in the log line
    Given "/tmp/alpha/.sdd-skill/specs/spec-010-aaa-planned/tasks.md" contains "### Task #1 — Write parser" and "### Task #2 — Write tests"
    And "/tmp/alpha/.sdd-skill/history/test_results.log" contains a line "Task #1 — PASS" with no "spec-010-aaa-planned"
    And that log also contains a line "spec-010-aaa-planned Task #2 — PASS"
    When GET /api/spec-board?project=alpha is read for id "spec-010-aaa-planned"
    Then that entry tasks include number 1 with done false
    And that entry tasks include number 2 with done true

  # Error Scenarios
  Scenario: Error — missing spec.md degrades one entry
    Given id "spec-016-ggg-ghost" is listed in planned_specs
    And "/tmp/alpha/.sdd-skill/specs/spec-016-ggg-ghost/spec.md" does not exist
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And the entry id "spec-016-ggg-ghost" has title "" and blurb ""
    And specs other than "spec-016-ggg-ghost" still include their title or blurb when those files exist

  Scenario: Error — missing tasks.md is empty task list
    Given id "spec-010-aaa-planned" has a readable spec.md
    And "/tmp/alpha/.sdd-skill/specs/spec-010-aaa-planned/tasks.md" does not exist
    When GET /api/spec-board?project=alpha is read for that id
    Then that entry has task_count 0 and tasks []

  Scenario: Error — unreadable spec.md omits blurb
    Given "/tmp/alpha/.sdd-skill/specs/spec-010-aaa-planned/spec.md" cannot be read
    When GET /api/spec-board?project=alpha is read for that id
    Then that entry has title "" and blurb ""
    And the response status is 200
    And sdd_skill_present is true
```

## Success Metrics
| Metric | Target | Current | Status |
| Todo/Done cards can expand | 100% of listed cards | only active && task_count>0 | not met |
| Empty Executive Summary | blurb omitted, expand still works | n/a | not met |
| Archive control in this spec | 0 | 0 | met (must stay 0) |
| Skill-file writes from spec_board | 0 | 0 | met (must stay 0) |
Primary KPI: first metric plus blurb-from-Executive-Summary-only.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + C HTTP. Prefer additive fields on GET `/api/spec-board`. New endpoints only if architect proves this GET cannot meet AC.
- `spec_board.c` is zero-write. Enrichment is more reads of files the skill already writes.
- Caps: `CBM_SPEC_BOARD_MAX_SPECS` 64, `CBM_SPEC_BOARD_MAX_TASKS` 48. Do not raise in this spec.
- `useSpecBoard` ~4s poll stays unless architect proves a cheaper path still meets AC.
- Chrome grayscale tokens and GraphTab `colorForLabel` hex stay unchanged.
- i18n en+zh for any new chrome string. Tests may assert English. Reuse "No tasks planned yet" for empty pending/empty tasks.

Business:
- Grill epic-001 only. Archive/unarchive + session toggle = epic-002 / a later spec. Last indexed local TZ = epic-003 / a later spec.
- Active card chrome (agent / N/M / checklist / blocked) is not removed.

Planner defaults (reject a Gherkin scenario to change these):
1. One GET `/api/spec-board`; additive `blurb` (empty string when omitted). No lazy expand endpoint.
2. Enrich every listed spec: H1 title + Executive Summary blurb + `tasks.md` headings.
3. Blurb = first 1-2 sentences after `## Executive Summary` until the next `## ` heading. Links → link text. No invented fallback. KPI never used.
4. Click target = existing title button. Active starts expanded; others collapsed. Multi-open. Poll must not collapse an opened card.
5. Todo = `done==false` only. In Progress = all + status. Done = all. No Archive UI.
6. Non-active `done`: log line must contain spec id + `Task #N` + PASS. Bare `Task #N` does not mark non-active tasks. Active keeps today's bare `Task #N` matcher.
7. Zero tasks or zero pending → "No tasks planned yet". Card still expands.
8. Missing/unreadable spec.md or tasks.md degrades that entry only.
9. No write to `.sdd-skill/`. No fourth column. No overlay.

## Out of Scope
- Archive / Unarchive, session show-archived toggle, confirm dialog — grill epic-002
- Last indexed in browser local TZ — grill epic-003
- Recolor or redesign Graph 3D
- Redesign Kanban columns or add an Archivados column
- Overlay, popover, accordion, or a separate spec page
- Writing, moving, or renaming `.sdd-skill/` files; changing `active.json` or spec.md Status
- Changing the sdd-skill agent cycle
- Raising 64/48 caps
- New HTTP endpoint unless architect proves GET `/api/spec-board` cannot meet AC

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% on touched C + graph-ui (reporter may be absent) [ ] review approved [ ] human docs confirmed [ ] zero skill writes [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- Extend `cbm_spec_board_entry_t` + JSON with `blurb`; run title/summary/tasks reads for every listed spec, not only active
- C tests for blurb extract, non-active done matching, missing files
- SpecCard: `canExpand` for all columns; persist expanded Set by spec id across poll
- Todo TaskList filters `done`; In Progress/Done show all
- i18n only if a new string is required; prefer existing no-tasks copy
- Vitest on SpecBoardTab with mocked board payloads for expand/filter/multi-open/poll persistence

## Questions for Architect (answered in plan.md)
- Blurb C buffer size and whether a third sentence is dropped only or also byte-truncated
- Whether active `done` matcher stays bare `Task #N` (planner default) or switches to spec-id-qualified for one code path
- How SpecCard holds expanded ids so a 4s poll does not remount-collapse
- Whether title+blurb+tasks for 64 specs on every poll needs a read budget or stays naive fopen (grill open-q 1/5)

## Questions for @implementer
- [ ] Map every Gherkin scenario to a C test and/or graph-ui Vitest
- [ ] Do not render Archive/Unarchive
- [ ] Do not write `.sdd-skill/`
- [ ] Do not change `colorForLabel` hex
- [ ] Do not add a second spec-board HTTP route unless plan.md says so

## Related Specs
Depends on: spec-002-p8w-project-workspace (Specs tab hosts SpecBoardTab)
Blocks: grill epic-002 spec-archive (needs Done expand)
Companion to: .grill/plans/spec-board-detail/epics/epic-001-spec-card-expand.md
Does not include: grill epic-003 last-indexed-local

## Revision History
| Date | Author | Change |
| 2026-08-30 | @planner | draft from grill epic-001 + ADR-002/003/006/008; debt=no |
| 2026-08-30 | @planner | CLOSED. KPI met. Constitution: 1 MODIFIED (IX.2). |

## Approval Sign-off
Spec Owner(@planner): closed 2026-08-30 / Gherkin scenarios(user): approved / Product Owner: closeprep confirmed / Architecture(@architect): plan approved
