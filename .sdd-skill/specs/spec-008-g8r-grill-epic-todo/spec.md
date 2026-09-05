# Spec-008-g8r: Grill epic Todo
Status: closed | Spec ID: spec-008-g8r-grill-epic-todo
Ref Ticket: none | KPI: On a project that already shows Specs, Todo lists every unconverted grill epic (letter E, title, summary, plan) before planned/draft specs; converted epics are omitted; In progress and Done stay specs-only; GET /api/spec-board is unchanged in path and remains zero-write | Priority: P0
Created: 2026-08-30 | Tech Debt Ref: none

## Executive Summary
The Specs Kanban already lists sdd-skill specs from `.sdd-skill/` (spec-002, spec-005, spec-006). grill-skill writes plans and epics under `.grill/` that later become specs. Today the board never reads `.grill/`, so the operator cannot see that backlog in Todo.

This spec paints unconverted grill epics in Todo on the same GET `/api/spec-board`, mixed with planned/draft specs. Conversion is a read-side match: a listed spec.md `Companion to:` grill path, or `active.json.source.grill_epic`, equal to that epic file path. CBM does not write, move, or rename `.grill/` or `.sdd-skill/` to hide a card. In progress and Done stay specs-only. Specs-tab visibility without `.sdd-skill/` is the next spec (grill epic-002).

Business impact: an operator already on Specs sees the grill funnel in Todo without a second board, a drag action, or a skill-file write.

## User Stories

### US-001: Mixed Todo shows unconverted grill epics
As an operator on a project that already has Specs, I want Todo to show grill epics that are not yet specs, So that I see the backlog that will become sdd work.
Acceptance Criteria:
- [ ] GET `/api/spec-board?project=<name>` remains the only board HTTP read. Poll may stay ~4s
- [ ] Response includes additive `grill_skill_present` (true when `root_path/.grill/` exists as a directory)
- [ ] Response includes an additive `epics` array. Specs array, archive merge, and POST `/api/spec-board` stay spec-only
- [ ] Each epic entry has `kind` "epic", `column` "todo", `id` equal to the relative path `.grill/plans/<slug>/epics/epic-NNN-<name>.md`, `title` (epic.md `name`), `summary` (epic.md `summary`), `plan_title` (index.md title for that slug; else plan.md frontmatter `title`)
- [ ] Todo paints those epic cards. In progress and Done paint zero epic cards
- [ ] Eligible = every epic file under `.grill/plans/*/epics/`, all plans (draft and closed), pending and detailed. Converted epics (US-002) are omitted

### US-002: Converted epics disappear from Todo
As an operator, I want an epic to leave Todo when sdd-skill has created a spec that claims that epic path, So that I do not see a duplicate funnel item after conversion.
Acceptance Criteria:
- [ ] An epic is converted (omitted from `epics` and from Todo) iff at least one listed board spec has a `Companion to:` grill path token equal to that epic `id`, OR `active.json.source.grill_epic` equals that `id`
- [ ] Match is exact path string. No kebab/name fuzzy match. No CBM epic↔spec table
- [ ] `Companion to:` may have trailing notes after the `.md` path; the path token is the first `.grill/plans/`…`.md` on that line
- [ ] Missing both links → the epic stays in Todo even if a similarly named spec exists (may sit beside that spec)
- [ ] The epic file on disk is unchanged. The matching spec still appears in its own column
- [ ] A spec in in_progress or done that claims the path still omits the epic from Todo

### US-003: Epic card is E + title + summary + plan, no expand
As an operator, I want an epic card to show kind, name, summary, and owning plan at a glance, So that I can scan Mixed Todo without opening a spec-style expand.
Acceptance Criteria:
- [ ] Kind mark is the single letter "E". No word "Epic". No filled pill or badge
- [ ] "E" uses one discreet chromatic hue. Not health red/amber/green. Not GraphTab `colorForLabel` / EdgeLines hex. Exact token is architect
- [ ] Title, 1-line summary, and plan title are always visible (not hidden behind a click)
- [ ] The epic card has no title control that expands tasks, blurb, Archive, or Unarchive
- [ ] Activating the epic card does not send POST `/api/spec-board`
- [ ] Spec cards keep spec-005 expand and spec-006 Archive/Unarchive unchanged

### US-004: Todo order is epics then specs
As an operator, I want unconverted epics first, grouped by plan, then planned/draft specs, So that the grill funnel sits above sdd Todo.
Acceptance Criteria:
- [ ] Todo document order: epic cards, then spec cards with column "todo"
- [ ] Epic groups follow `.grill/index.md` table row order. Within a plan, `epic-NNN` numeric order
- [ ] A plan directory not listed in index.md comes after indexed plans, slug ascending
- [ ] Specs after epics keep current `active.json` planned/draft array order
- [ ] In progress and Done order unchanged (specs only)

### US-005: Own epic cap; overflow omitted
As an operator, I want epics to have their own slot pool so a large grill tree cannot drop spec cards, So that spec Todo does not compete with epic Todo.
Acceptance Criteria:
- [ ] Epic cap is 64 (`CBM_SPEC_BOARD_MAX_EPICS` or equivalent). Independent of `CBM_SPEC_BOARD_MAX_SPECS` 64
- [ ] Eligible epics beyond the cap are omitted from this GET. No error. No "has more" control or copy
- [ ] Spec cap 64 is unchanged. Filling the epic cap does not reduce spec slots
- [ ] Task cap 48 is unchanged

### US-006: Board read stays zero-write; tab presence unchanged
As an operator, I want this board to only read skill trees, and Specs to still require `.sdd-skill/` this spec, So that conversion stays a skill action and grill-only paths wait for the next spec.
Acceptance Criteria:
- [ ] `spec_board.c` (or its HTTP wrapper) may read `.grill/`. It does not create, write, move, or rename files under `.grill/` or `.sdd-skill/`
- [ ] GET 200 leaves epic.md, index.md, active.json, and spec.md byte-identical to before the request
- [ ] No new MCP board tool. No second poll URL
- [ ] Tab label stays "Specs". `useSddSkillPresent` still shows Specs only when `sdd_skill_present === true`
- [ ] `grill_skill_present` is emitted but does not by itself show the tab (grill epic-002)
- [ ] Graph, Dashboard, ADR tab, Path 1:1, ADR fill, spec-005 expand, spec-006 archive, and spec-007 `formatIndexedAt` stay unchanged
- [ ] Do not read `.gamedev/`. Do not paint gamedev cards. Presence/kind stay additive for a later plan

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Grill epic Todo

  # Happy Paths
  Scenario: Mixed Todo paints an unconverted epic then a planned spec
    Given project "alpha" has root_path "/tmp/alpha" and sdd_skill_present is true
    And "/tmp/alpha/.grill/" exists as a directory
    And "/tmp/alpha/.grill/index.md" lists plan slug "inbox-plan" with title "Inbox Plan"
    And "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" has name "inbox" and summary "Filter unread first."
    And no listed spec.md Companion-to path equals ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And active.json source.grill_epic is not that path
    And GET /api/spec-board?project=alpha includes a spec with id "spec-010-aaa-planned" column "todo"
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And grill_skill_present is true
    And epics has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And that epic entry has kind "epic" and column "todo"
    And that epic entry has title "inbox"
    And that epic entry has summary "Filter unread first."
    And that epic entry has plan_title "Inbox Plan"
    And the Specs tab Todo column shows a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And that card shows the text "E"
    And that card shows the text "inbox"
    And that card shows the text "Filter unread first."
    And that card shows the text "Inbox Plan"
    And the Todo column shows a card whose id text is "spec-010-aaa-planned"
    And in document order the epic card appears before that spec card

  Scenario: Companion-to exact path omits the epic from Todo
    Given GET /api/spec-board?project=alpha would list spec id "spec-010-aaa-planned" column "todo"
    And that spec.md contains a line Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md
    And "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/spec-board?project=alpha is read
    Then epics has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And the Todo column does not show a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And the Todo column shows a card whose id text is "spec-010-aaa-planned"
    And "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" is byte-identical to its contents before the GET

  Scenario: active.json source.grill_epic omits the epic
    Given "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-002-later.md" exists
    And no listed spec.md Companion-to path equals ".grill/plans/inbox-plan/epics/epic-002-later.md"
    And active.json source.grill_epic is ".grill/plans/inbox-plan/epics/epic-002-later.md"
    When GET /api/spec-board?project=alpha is read
    Then epics has no entry whose id is ".grill/plans/inbox-plan/epics/epic-002-later.md"
    And the Todo column does not show a card whose id text is ".grill/plans/inbox-plan/epics/epic-002-later.md"

  Scenario: Companion-to trailing notes still match
    Given a listed spec.md contains "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md (ADR-001)"
    And "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/spec-board?project=alpha is read
    Then epics has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Done spec claiming an epic still omits it from Todo
    Given GET /api/spec-board?project=alpha includes spec id "spec-012-ccc-closed" column "done"
    And that spec.md Companion-to path equals ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/spec-board?project=alpha is read
    Then epics has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And the Done column shows a card whose id text is "spec-012-ccc-closed"
    And the Done column does not show a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And the In progress column does not show a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  # Limit Cases
  Scenario: Limit Case — two plans follow index.md then epic-NNN then specs
    Given "/tmp/alpha/.grill/index.md" lists slug "plan-a" then slug "plan-b"
    And plan-a has epics epic-002-second.md then epic-001-first.md on disk
    And plan-b has epic-001-other.md
    And none of those epics are converted
    And GET includes spec id "spec-010-aaa-planned" column "todo"
    When the Specs tab paints the Todo column
    Then document order of card id texts is
      ".grill/plans/plan-a/epics/epic-001-first.md"
      then ".grill/plans/plan-a/epics/epic-002-second.md"
      then ".grill/plans/plan-b/epics/epic-001-other.md"
      then "spec-010-aaa-planned"

  Scenario: Limit Case — closed plan leftover unconverted epic stays in Todo
    Given "/tmp/alpha/.grill/index.md" lists slug "old-plan" with status closed
    And "/tmp/alpha/.grill/plans/old-plan/epics/epic-009-leftover.md" has status pending
    And that epic is not converted
    When GET /api/spec-board?project=alpha is read
    Then epics has an entry whose id is ".grill/plans/old-plan/epics/epic-009-leftover.md"
    And the Todo column shows a card whose id text is ".grill/plans/old-plan/epics/epic-009-leftover.md"

  Scenario: Limit Case — pending and detailed both listed
    Given epic-001-inbox.md has status pending and is not converted
    And epic-003-ready.md has status detailed and is not converted
    When GET /api/spec-board?project=alpha is read
    Then epics includes both ids
      ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
      and ".grill/plans/inbox-plan/epics/epic-003-ready.md"

  Scenario: Limit Case — missing Companion-to keeps the epic beside a similarly named spec
    Given "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And GET includes spec id "spec-010-aaa-inbox" column "todo"
    And that spec.md has no Companion-to grill path
    And active.json source.grill_epic is not ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    When GET /api/spec-board?project=alpha is read
    Then epics has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And the Todo column shows both that epic card and the spec card "spec-010-aaa-inbox"

  Scenario: Limit Case — 65th epic is omitted and spec slots stay 64
    Given 65 unconverted epic files exist under .grill/plans
    And 64 specs are listed on the board
    When GET /api/spec-board?project=alpha is read
    Then epics length is 64
    And specs length is 64
    And the response JSON has no field whose name is "has_more"
    And the Specs tab does not show a control named "Has more"

  Scenario: Limit Case — epic card does not expand or archive
    Given the Todo column shows an epic card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    When the operator activates that card
    Then that card does not show a control named "Archive"
    And that card does not show a control named "Unarchive"
    And that card does not show the text "No tasks planned yet"
    And POST /api/spec-board is not sent

  Scenario: Limit Case — no .grill directory
    Given project "alpha" has sdd_skill_present true
    And "/tmp/alpha/.grill/" does not exist
    When GET /api/spec-board?project=alpha is read
    Then grill_skill_present is false
    And epics is []
    And specs still includes planned/draft/active/completed entries as today

  Scenario: Limit Case — Specs tab still requires sdd_skill_present
    Given GET /api/spec-board?project=alpha has sdd_skill_present false and grill_skill_present true
    When the workspace chrome is painted
    Then the tab named "Specs" is not shown

  # Error Scenarios
  Scenario: Error — unreadable epic file is skipped and GET is still 200
    Given "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists and is well-formed
    And "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-002-broken.md" cannot be read
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And epics includes id ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And epics does not include id ".grill/plans/inbox-plan/epics/epic-002-broken.md"

  Scenario: Error — POST archive with an epic id is 404 and writes nothing
    Given GET /api/spec-board?project=alpha epics includes id ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And specs do not include that id
    When POST /api/spec-board is sent with JSON project "alpha" spec_id ".grill/plans/inbox-plan/epics/epic-001-inbox.md" archived true
    Then the response status is 404
    And the response body is {"error":"spec not found"}
    And "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" is byte-identical to its contents before the POST
    And "/tmp/alpha/.sdd-skill/specs/active.json" is byte-identical to its contents before the POST

  Scenario: Error — unknown project on GET is still 404
    When GET /api/spec-board?project=missing-proj is read
    Then the response status is 404
    And the response body is {"error":"project not found"}

  Scenario: Error — GET does not write skill trees
    Given project "alpha" root_path is "/tmp/alpha"
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And "/tmp/alpha/.grill/index.md" is byte-identical to its contents before the GET
    And "/tmp/alpha/.sdd-skill/specs/active.json" is byte-identical to its contents before the GET
```

## Success Metrics
| Metric | Target | Current | Status |
| Unconverted eligible epics visible in Todo | 100% up to cap 64 | 0 grill reads | not met |
| Converted epics in Todo | 0 when Companion-to or source.grill_epic matches | n/a | not met |
| Skill-file writes from GET or epic UI | 0 | 0 | met (must stay 0) |
| Epic cards in In progress or Done | 0 | 0 | met (must stay 0) |
| Second board HTTP read | 0 | 0 | met (must stay 0) |
Primary KPI: first two metrics plus zero skill writes.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + C HTTP + per-project SQLite. `spec_board.c` stays zero-write.
- GET `/api/spec-board?project=` remains the only board read. Additive `grill_skill_present` + `epics[]`.
- Specs array, `archived` merge, and POST `/api/spec-board` stay spec-only (spec-006).
- Caps: specs 64, epics 64, tasks 48. Overflow omit. No has-more chrome.
- Chrome grayscale stays. Epic "E" is the one allowed discreet chromatic exception (grill ADR-004). GraphTab hex locked.
- i18n: letter "E" is the same in en and zh. New copy (if any) in both. Tests may assert English.
- Constitution I.2: indexer/graph-ui do not write skill cycle files.

Business:
- Grill epic-001 only. Tab Specs when `.grill/` exists without `.sdd-skill/` = epic-002 / a later spec.
- gamedev out (grill ADR-006).

Planner defaults (reject a Gherkin scenario to change these):
1. Separate `epics` array. Do not put epic objects inside `specs[]` (own cap; SpecCard/archive stay spec-only).
2. Epic `id` = relative path from project root `.grill/plans/<slug>/epics/epic-NNN-<name>.md`.
3. Conversion match: exact path on Companion-to grill token OR `active.json.source.grill_epic`. First `.grill/plans/`…`.md` token on the Companion-to line; trailing notes ignored.
4. Title = epic.md `name`. Summary = epic.md `summary`. Plan title = index.md title, else plan.md `title`.
5. Order: index.md row order, then NNN, then planned/draft specs.
6. Cap 64 epics, omit overflow, no has-more.
7. Tab visibility unchanged this spec. Still emit `grill_skill_present`.
8. Best-effort: unreadable epic skipped; missing `.grill/` → flag false and `epics: []`; board still 200 when sdd is present.
9. POST with an epic id → existing 404 `spec not found`.
10. No MCP tool. No `.gamedev/` read.

## Out of Scope
- Specs tab visible when `.grill/` exists and `.sdd-skill/` does not — grill epic-002
- Drag, button, or CBM write that creates a spec or deletes/moves an epic
- Fuzzy name/kebab match or a CBM-owned epic↔spec table
- Redesign of the three-column Kanban or Graph 3D
- Reading `.gamedev/`, painting or hiding via gamedev
- Deciding whether a path may hold sdd and gamedev at once
- "Has more" chrome when epic cap overflows
- Renaming the tab away from "Specs"
- Changing spec-005 expand or spec-006 archive rules for spec cards
- Changing `formatIndexedAt` (spec-007)

## Acceptance Checklist
- [x] all US implemented [x] all AC met [x] ALL Gherkin scenarios pass [x] tests>80% on touched C + graph-ui (reporter may be absent) [x] review approved [x] human docs confirmed [x] zero skill writes [x] security passed [ ] manual test by owner done

## Architecture Considerations
- Extend `cbm_spec_board_t` with `grill_skill_present` and an epic array (cap 64)
- Reader walks `.grill/index.md` + `plans/*/epics/*.md`; still zero-write
- Conversion scan: listed spec.md Companion-to tokens + active.json `source.grill_epic`
- JSON: additive keys on the existing GET serializer
- SpecBoardTab: Todo concatenates epic cards then spec cards; new EpicCard (no expand)
- i18n en+zh; Vitest on SpecBoardTab + C tests for match/order/cap/skip
- Presence endpoint / `useSddSkillPresent` unchanged this spec

## Questions for Architect (answered in plan.md)
- Exact epic JSON field names (`summary` vs reuse `blurb`; `plan_title` vs `plan`)
- Whether `kind` is also added onto spec entries as additive `"spec"` or only on epics
- Where `.grill/` parse lives (spec_board.c vs a sibling reader called from HTTP)
- Exact "E" token/hex (chrome exception, not health, not graph palette)
- How index.md is parsed (table rows) vs fallback when the catalog is missing

## Questions for @implementer
- [ ] Map every Gherkin scenario to a C test and/or graph-ui Vitest
- [ ] Do not write `.grill/` or `.sdd-skill/`
- [ ] Do not put epics in In progress or Done
- [ ] Do not change Specs tab omit-until-true (still sdd only)
- [ ] Do not add a second GET or an MCP board tool
- [ ] Do not change `colorForLabel` hex
- [ ] Do not add has-more chrome

## Related Specs
Depends on: spec-002-p8w-project-workspace (Specs tab), spec-005-v2m-spec-card-expand (spec cards), spec-006-k3n-spec-archive (archive stays spec-only)
Companion to: .grill/plans/add-epics-plans-kanban/epics/epic-001-grill-epic-todo.md
Does not include: grill epic-002 specs-tab-grill-presence

## Revision History
| Date | Author | Change |
| 2026-08-30 | @planner | Gherkin approved; status approved; handoff @architect |
| 2026-08-30 | @planner | draft from grill epic-001 + ADR-001..005,007,008; debt=no |
| 2026-08-30 | @planner | CLOSED. KPI met. Constitution IX.2 MODIFIED confirmed. |

## Approval Sign-off
Spec Owner(@planner): ✓ 2026-08-30 / Gherkin scenarios(user): ✓ 2026-08-30 / Product Owner: ✓ 2026-08-30 / Architecture(@architect): ✓ 2026-08-30
