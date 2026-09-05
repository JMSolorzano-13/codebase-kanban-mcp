# Spec-006-k3n: Spec archive
Status: closed | Spec ID: spec-006-k3n-spec-archive
Ref Ticket: none | KPI: A Done spec can be archived into CBM-owned state, stays in Done, is hidden on a fresh Specs visit, and can be shown then unarchived without writing .sdd-skill/ | Priority: P0
Created: 2026-08-30 | Tech Debt Ref: none

## Executive Summary
The Specs Kanban already expands Done cards (spec-005). Closed work still piles up in Done. sdd-skill has no archived enum; this spec must not write, move, or rename anything under `.sdd-skill/` or other skill trees (grill ADR-001).

Archive is a CBM flag on a listed Done spec. GET `/api/spec-board` stays the board read and adds additive `archived` on every entry. Mutate is POST on the same `/api/spec-board` family. Archived specs remain `column: "done"` (no fourth column). The board starts each visit with archived hidden. Show/hide is session-only (grill ADR-007). Archive is immediate — no confirm dialog (grill ADR-009). Recovery is Show archived then Unarchive.

Business impact: Done stops accumulating history the operator does not need every visit, without changing the skill cycle or `completed_specs`.

## User Stories

### US-001: Archive from a Done expand
As an operator, I want an Archive control on an expanded Done card that hides that spec immediately, So that I can clear history without a confirm dialog or a skill-file change.
Acceptance Criteria:
- [ ] Archive appears only on an expanded Done card whose `archived` is false
- [ ] Todo and In Progress expands do not show Archive or Unarchive
- [ ] Activating Archive sends POST `/api/spec-board` with `project`, `spec_id`, `archived: true` and does not open a dialog, alertdialog, or `window.confirm`
- [ ] POST 200 body includes that `spec_id` and `archived` true
- [ ] After 200, with the session toggle hiding archived, that card is not in the Done column
- [ ] `active.json`, `spec.md` Status, and `completed_specs` are unchanged by the POST

### US-002: Archived stay in Done and start hidden
As an operator, I want archived specs to remain Done entries that a fresh Specs visit hides, So that the Kanban stays three columns and history is not the default view.
Acceptance Criteria:
- [ ] GET `/api/spec-board?project=<name>` still lists an archived spec with `column` "done" and `archived` true
- [ ] No fourth column and no "Archived" column title
- [ ] First paint of SpecBoardTab for a project hides entries where `column` is "done" and `archived` is true
- [ ] Changing `?project=` and returning, or remounting the tab, starts hidden again (no localStorage, no CBM toggle field)
- [ ] Done header count equals the number of Done cards currently shown (hidden archived do not increment it)

### US-003: Session show/hide in the Done header
As an operator, I want a Show archived control on the Done column header, So that I can reveal history for this visit only.
Acceptance Criteria:
- [ ] The control accessible name is "Show archived". It lives in the Done column header only
- [ ] Todo and In Progress headers do not show that control
- [ ] `aria-pressed` is false on first paint. Activating it sets `aria-pressed` true and shows Done cards with `archived` true
- [ ] Activating it again sets `aria-pressed` false and hides those cards again
- [ ] When every Done spec is archived and the toggle is off, Done shows the existing English copy "No specs yet" and count 0; the Show archived control remains

### US-004: Unarchive from the same expand
As an operator, I want Unarchive on an expanded archived Done card, So that a mistaken archive is reversible without a skill write.
Acceptance Criteria:
- [ ] When Show archived is on, an expanded Done card with `archived` true shows Unarchive and does not show Archive
- [ ] Activating Unarchive POSTs `archived: false` for that `spec_id`
- [ ] POST 200 body includes that `spec_id` and `archived` false
- [ ] After 200 the card stays in Done and is visible even if Show archived is off
- [ ] Unarchive does not open a confirm dialog

### US-005: Flag lives in CBM; GET merges; orphans do not invent cards
As an operator, I want archive state to survive daemon restart and to ignore leftover flags for specs the skill no longer lists, So that the board never writes skills and never invents a card from a stale flag.
Acceptance Criteria:
- [ ] Every GET `/api/spec-board` entry includes boolean `archived` (false when no flag)
- [ ] `spec_board.c` remains a skill-file reader. Merge of flags happens in CBM (store/HTTP) after the read
- [ ] Flag key is project name + spec folder id. Persist in that project's CBM store (not a skill sidecar, not localStorage)
- [ ] A flag whose `spec_id` is not on the current board does not add a spec to GET
- [ ] A listed spec in `todo` or `in_progress` with a leftover true flag still has `archived` true in JSON but is not hidden and has no Archive/Unarchive control
- [ ] If that spec later returns to Done, the existing true flag hides it again
- [ ] No new MCP tool. Archive is HTTP-only
- [ ] Graph, Dashboard, ADR tab, Path 1:1, ADR fill, and spec-005 expand rules stay unchanged

### US-006: POST errors do not write and poll cannot resurrect
As an operator, I want a failed archive to leave the board as it was, and a successful one to stay hidden across the next poll, So that a 4s refresh cannot undo Archive.
Acceptance Criteria:
- [ ] POST for a listed spec whose `column` is not "done" returns 409 `{"error":"spec not done"}` and does not persist a flag
- [ ] POST for a `spec_id` not on the current board returns 404 `{"error":"spec not found"}`
- [ ] POST with missing `project` or `spec_id` returns 400. Missing or non-boolean `archived` returns 400 `{"error":"invalid archived"}`
- [ ] Unknown project returns 404 `{"error":"project not found"}` (same string as GET)
- [ ] Repeat POST with the same `archived` value is 200 and idempotent
- [ ] After a 200 archive, a later GET (poll) still has `archived` true for that id; the hidden card does not reappear while the toggle is off

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Spec archive

  # Happy Paths
  Scenario: Archive hides a Done card without a confirm dialog
    Given project "alpha" has root_path "/tmp/alpha" and sdd_skill_present is true
    And GET /api/spec-board?project=alpha includes id "spec-012-ccc-closed" column "done" archived false
    And the Specs tab is showing the Kanban for "alpha"
    And the operator has expanded the card whose id text is "spec-012-ccc-closed"
    When the operator activates the control named "Archive"
    Then no dialog or alertdialog is shown
    And POST /api/spec-board is sent with JSON project "alpha" spec_id "spec-012-ccc-closed" archived true
    And that POST response status is 200
    And that POST response JSON has spec_id "spec-012-ccc-closed" and archived true
    And the Done column does not show a card whose id text is "spec-012-ccc-closed"
    And "/tmp/alpha/.sdd-skill/specs/active.json" is byte-identical to its contents before the POST

  Scenario: Show archived reveals the card and Unarchive restores it
    Given GET /api/spec-board?project=alpha includes id "spec-012-ccc-closed" column "done" archived true
    And the Specs tab is showing the Kanban for "alpha"
    And the control named "Show archived" has aria-pressed false
    And the Done column does not show a card whose id text is "spec-012-ccc-closed"
    When the operator activates the control named "Show archived"
    Then that control has aria-pressed true
    And the Done column shows a card whose id text is "spec-012-ccc-closed"
    When the operator activates the title control on that card
    And the operator activates the control named "Unarchive"
    Then POST /api/spec-board is sent with JSON project "alpha" spec_id "spec-012-ccc-closed" archived false
    And that POST response status is 200
    And that POST response JSON has archived false
    And the Done column still shows a card whose id text is "spec-012-ccc-closed"
    When the operator activates the control named "Show archived"
    Then that control has aria-pressed false
    And the Done column still shows a card whose id text is "spec-012-ccc-closed"

  Scenario: Fresh visit starts with archived hidden
    Given GET /api/spec-board?project=alpha includes id "spec-012-ccc-closed" column "done" archived true
    And the operator had Show archived pressed in a previous mount
    When SpecBoardTab mounts for project "alpha"
    Then the control named "Show archived" has aria-pressed false
    And the Done column does not show a card whose id text is "spec-012-ccc-closed"

  Scenario: GET keeps archived specs in Done with archived true
    Given CBM has a flag for project "alpha" spec_id "spec-012-ccc-closed" archived true
    And that spec is listed in completed_specs
    When GET /api/spec-board?project=alpha is read for id "spec-012-ccc-closed"
    Then that entry has column "done"
    And that entry has archived true
    And the response JSON has no column value other than "todo" or "in_progress" or "done"

  # Limit Cases
  Scenario: Limit Case — Done count ignores hidden archived
    Given GET /api/spec-board?project=alpha includes exactly two Done specs
    And id "spec-012-ccc-closed" has archived true
    And id "spec-017-hhh-visible" has archived false
    And Show archived is not pressed
    When the Specs tab paints the Done column
    Then the Done header count text is "1"
    And the Done column shows a card whose id text is "spec-017-hhh-visible"
    And the Done column does not show a card whose id text is "spec-012-ccc-closed"

  Scenario: Limit Case — leftover flag on a Todo spec does not hide it
    Given GET /api/spec-board?project=alpha includes id "spec-010-aaa-planned" column "todo" archived true
    When the Specs tab paints the Todo column
    Then the Todo column shows a card whose id text is "spec-010-aaa-planned"
    And that card expand does not show a control named "Archive"
    And that card expand does not show a control named "Unarchive"

  Scenario: Limit Case — orphan flag does not invent a card
    Given CBM has a flag for project "alpha" spec_id "spec-099-zzz-gone" archived true
    And GET /api/spec-board?project=alpha specs do not include id "spec-099-zzz-gone"
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And the specs array has no entry whose id is "spec-099-zzz-gone"

  Scenario: Limit Case — all Done archived shows empty copy and keeps the toggle
    Given every spec with column "done" has archived true
    And Show archived is not pressed
    When the Specs tab paints the Done column
    Then the Done column shows the text "No specs yet"
    And the Done header count text is "0"
    And the Done header shows a control named "Show archived"

  Scenario: Limit Case — repeat archive is idempotent
    Given GET /api/spec-board?project=alpha includes id "spec-012-ccc-closed" column "done" archived true
    When POST /api/spec-board is sent with JSON project "alpha" spec_id "spec-012-ccc-closed" archived true
    Then the response status is 200
    And the response JSON has archived true
    And a following GET /api/spec-board?project=alpha entry id "spec-012-ccc-closed" has archived true

  Scenario: Limit Case — poll after archive does not resurrect the card
    Given the operator archived id "spec-012-ccc-closed" and the card is hidden
    And Show archived is not pressed
    When GET /api/spec-board?project=alpha returns that id with column "done" archived true
    Then the Done column does not show a card whose id text is "spec-012-ccc-closed"

  # Error Scenarios
  Scenario: Error — archive a Todo spec is 409 and writes nothing
    Given GET /api/spec-board?project=alpha includes id "spec-010-aaa-planned" column "todo" archived false
    When POST /api/spec-board is sent with JSON project "alpha" spec_id "spec-010-aaa-planned" archived true
    Then the response status is 409
    And the response body is {"error":"spec not done"}
    And a following GET /api/spec-board?project=alpha entry id "spec-010-aaa-planned" has archived false
    And "/tmp/alpha/.sdd-skill/specs/active.json" is byte-identical to its contents before the POST

  Scenario: Error — unknown spec_id is 404
    Given GET /api/spec-board?project=alpha specs do not include id "spec-099-zzz-gone"
    When POST /api/spec-board is sent with JSON project "alpha" spec_id "spec-099-zzz-gone" archived true
    Then the response status is 404
    And the response body is {"error":"spec not found"}

  Scenario: Error — missing project on POST is 400
    When POST /api/spec-board is sent with JSON spec_id "spec-012-ccc-closed" archived true and no project field
    Then the response status is 400
    And the response JSON has an error field

  Scenario: Error — invalid archived on POST is 400
    When POST /api/spec-board is sent with JSON project "alpha" spec_id "spec-012-ccc-closed" archived "yes"
    Then the response status is 400
    And the response body is {"error":"invalid archived"}

  Scenario: Error — unknown project on POST is 404
    When POST /api/spec-board is sent with JSON project "missing-proj" spec_id "spec-012-ccc-closed" archived true
    Then the response status is 404
    And the response body is {"error":"project not found"}
```

## Success Metrics
| Metric | Target | Current | Status |
| Done Archive hides card on fresh visit | 100% of archived Done ids | no archive | not met |
| Skill-file writes from archive | 0 | 0 | met (must stay 0) |
| Fourth column | 0 | 0 | met (must stay 0) |
| Confirm dialog on Archive | 0 | 0 | met (must stay 0) |
| MCP archive tool | 0 | 0 | met (must stay 0) |
Primary KPI: first metric plus zero skill writes.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + C HTTP + per-project SQLite. `spec_board.c` stays zero-write.
- GET `/api/spec-board?project=` remains the only board read. Additive `archived` boolean on every entry.
- POST `/api/spec-board` is the mutate. New path only if architect proves this family cannot meet AC.
- Caps stay 64 specs / 48 tasks. `useSpecBoard` poll may stay ~4s.
- Chrome grayscale tokens and GraphTab `colorForLabel` hex stay unchanged.
- i18n en+zh for Archive, Unarchive, Show archived. Tests may assert English.
- Constitution VI.3 (confirm on delete) does not apply: archive is not delete; recovery is Unarchive.

Business:
- Grill epic-002 only. Last indexed local TZ = epic-003 / a later spec.
- spec-005 expand (blurb, Ver tareas, multi-open) stays.

Planner defaults (reject a Gherkin scenario to change these):
1. Persist flags in the project's CBM store keyed by spec folder id. Not daemon-global. Not `.sdd-skill/`.
2. Merge `archived` onto GET. POST `/api/spec-board` `{project, spec_id, archived}`. HTTP-only; no MCP tool.
3. Orphan flags: keep the store row; ignore on read if the spec is not listed; do not invent a card.
4. Leftover true flag on todo/in_progress: JSON may say true; UI does not hide; no Archive/Unarchive. Return to Done → hide again.
5. After POST 200, UI refetches or applies the flag so the next poll cannot resurrect a hidden card.
6. Toggle: Done header only. Accessible name "Show archived". Session-only. Fresh mount starts unpressed.
7. Done count = visible Done cards only.
8. Unarchive while hide → card reappears immediately (it is no longer archived).
9. No confirm. No fourth column. No skill write.

## Out of Scope
- Last indexed in browser local TZ — grill epic-003
- Recolor or redesign Graph 3D
- Fourth "Archived" column, overlay, accordion, or a separate spec page
- Writing, moving, or renaming `.sdd-skill/` files; changing `active.json` or spec.md Status
- Changing the sdd-skill agent cycle
- MCP `manage_adr` / new archive MCP tool
- Raising 64/48 caps
- Confirm dialog, undo toast, or persisted show/hide preference
- Auto-clear of flags when a spec leaves `completed_specs` (keep row; ignore if unlisted)

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% on touched C + graph-ui (reporter may be absent) [ ] review approved [ ] human docs confirmed [ ] zero skill writes [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- Store table or equivalent in the project `.db` (project name + spec id → archived)
- HTTP: GET merge after `cbm_spec_board_read`; POST validate listed+done, persist, 200
- C tests for merge, 409/404/400, idempotent POST, orphan ignore, active.json unchanged
- SpecBoardTab: session `showArchived` boolean; Done filter; header toggle; Archive/Unarchive on Done expand only
- After POST, refetch GET or patch the matching entry so poll cannot resurrect
- i18n en+zh; Vitest on SpecBoardTab + C HTTP tests

## Questions for Architect (answered in plan.md)
- Exact SQLite shape (new table vs reuse an existing project-db relation)
- Whether POST stays `/api/spec-board` or must be a sibling path
- Where merge lives (http_server vs a store helper vs a thin wrapper around spec_board)
- Whether 200 POST should return the single flag object (planner default) or the full board JSON
- How UI avoids poll-resurrect: mandatory refetch vs merge POST body into board state

## Questions for @implementer
- [ ] Map every Gherkin scenario to a C test and/or graph-ui Vitest
- [ ] Do not write `.sdd-skill/`
- [ ] Do not add a fourth column
- [ ] Do not persist Show archived
- [ ] Do not add an MCP archive tool
- [ ] Do not change `colorForLabel` hex
- [ ] Do not change `formatIndexedAt` (later spec)

## Related Specs
Depends on: spec-005-v2m-spec-card-expand (Done expand exists)
Companion to: .grill/plans/spec-board-detail/epics/epic-002-spec-archive.md
Does not include: grill epic-003 last-indexed-local

## Revision History
| Date | Author | Change |
| 2026-08-30 | @planner | draft from grill epic-002 + ADR-001/004/007/009; debt=no |
| 2026-08-30 | @planner | Gherkin approved; status approved; handoff @architect |
| 2026-08-30 | @architect | plan.md/tasks.md/checklist.md delivered; SDD-ADR-029..033; awaiting user |
| 2026-08-30 | @planner | CLOSED. KPI met. Constitution: 1 MODIFIED (IX.2). |

## Approval Sign-off
Spec Owner(@planner): closed 2026-08-30 / Gherkin scenarios(user): approved / Product Owner: closeprep + constitution confirmed / Architecture(@architect): plan approved
