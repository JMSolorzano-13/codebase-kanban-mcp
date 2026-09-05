# Spec-016-d9v: Game Inbox registry
Status: completed | Spec ID: spec-016-d9v-game-inbox-registry
Ref Ticket: none | KPI: When `.gamedev/epics_registry.md` exists, Game Inbox omits grill epics whose matching registry Status is in_progress, closed, or parked, and does not also apply Companion-to/roadmap hide; when the file is absent, prior hide stays; Inbox epic id wraps; zero skill writes; same GET | Priority: P0
Created: 2026-09-02 | Tech Debt Ref: none

## Executive Summary
Game Inbox today hides a grill epic when a Game artifact has an exact Companion-to path or when roadmap slug+NNN matches (spec-011). Operators who adopted gamedev Epic Tracking still see leftovers that `@director` already marked in_progress, closed, or parked, and they lose the filename because InboxCard truncates `id`.

This spec changes only the Inbox hide predicate and the Inbox id chrome. If `.gamedev/epics_registry.md` exists as a regular file (even with zero parsed rows), hide is solely Plan=grill slug AND Epic=NNN with Status in {in_progress, closed, parked}. Companion-to and roadmap must not also hide. If the file is absent, spec-011 hide is unchanged. Inbox cards keep the short `name` as title and wrap the full `id` path. Hide is server-side omit from `inbox[]`. Specs conversion and debt chrome stay locked.

Business impact: on a gamedev path that has a registry, Inbox means “faltan” in the skill’s own tracking file; older games without the file keep the prior funnel.

## User Stories

### US-001: Registry hide-set when the file exists
As an operator on Game, I want Inbox to drop grill epics that `epics_registry.md` already tracks as in_progress, closed, or parked, So that leftover cards match what `@director` considers still pending.
Acceptance Criteria:
- [ ] GET `/api/game-board?project=<name>` remains the only Game board HTTP read
- [ ] Hide is applied server-side: omitted cards are absent from JSON `inbox[]` (not a client filter)
- [ ] A grill Inbox candidate matches a registry row iff Plan cell equals that epic’s grill plan directory slug AND Epic cell normalizes to the same integer NNN as the filename `epic-NNN-*.md`
- [ ] Matching Status `in_progress`, `closed`, or `parked` → that epic is omitted from `inbox[]`
- [ ] Matching Status `not_started` → epic stays
- [ ] No matching row → epic stays
- [ ] Origin column is unused for hide
- [ ] Epic 0 / Status `evergreen` never hides a grill Inbox card
- [ ] Plan cell `native` never matches a grill slug

### US-002: File present is sole hide source
As an operator, I want Companion-to and roadmap to stop hiding Inbox once the registry file exists, So that an unlisted leftover still shows as “falta”.
Acceptance Criteria:
- [ ] Regular file exists at `.gamedev/epics_registry.md` (empty, header-only, or zero matching rows) → spec-011 Companion-to exact and roadmap slug+NNN must not omit any Inbox epic
- [ ] CBM never creates `.gamedev/epics_registry.md`
- [ ] Directory or other non-regular path at that name is not “present” → US-003 applies

### US-003: File absent keeps prior hide
As an operator on an older game without a registry, I want Inbox hide unchanged from spec-011, So that converted work does not flood Inbox.
Acceptance Criteria:
- [ ] Path `.gamedev/epics_registry.md` does not exist as a regular file → omit iff Companion-to exact path OR (slug token in roadmap.md|game_context.md AND roadmap table cell NNN / epic-NNN)
- [ ] `active.json` `source.grill_epic` still does not convert Game Inbox
- [ ] No kebab/name fuzzy match (unchanged)

### US-004: Parse handoff (last wins, NNN, exact slug)
As an operator, I want table parse to follow the skill’s integer Epic and slug Plan, So that `001`, `1`, and `epic-001` hide the same card and a leftover path in Plan does not.
Acceptance Criteria:
- [ ] Duplicate Plan+NNN rows: the last data row in file order wins. Earlier Status is ignored
- [ ] Epic cell: trim; optional leading `epic-` (case-sensitive exact prefix `epic-`); then parse as base-10 integer (leading zeros allowed). `001`, `1`, and `epic-001` all equal NNN 1. Other text → row skipped
- [ ] Plan cell: trim; exact string equals the grill plan directory slug. A path leftover (`tech-debt-and-epics-registry/epics` or `.grill/plans/inbox-plan`) does not match slug `inbox-plan`
- [ ] Status cell: trim; hide tokens are exact `in_progress`, `closed`, `parked`. Typo or unknown token is not hide (row matches Plan+NNN but does not hide)
- [ ] Header row, separator row, and unparseable data rows are skipped. GET stays 200
- [ ] Unreadable regular file: treat as present with zero parsed rows (US-002). GET 200

### US-005: Inbox path wraps; walk and cap stay
As an operator on Inbox, I want the full grill epic path under the short name, wrapping not truncated, So that I can tell which epic file I am looking at.
Acceptance Criteria:
- [ ] InboxCard title stays `card.title` (`name:`) and may still truncate
- [ ] InboxCard id line shows the full `card.id` (`.grill/plans/<slug>/epics/epic-NNN-<name>.md`) and wraps. No CSS `truncate` / `text-overflow: ellipsis` on that line
- [ ] Artifact card id lines, Spec cards, WorkspaceHeader path, and Specs EpicCard (already wrap from spec-015) are unchanged this spec
- [ ] Walk remains `game_grill_fill_inbox`: all `.grill/plans/*/epics/` files, all plan statuses. Only the hide predicate changes
- [ ] Inbox cap 64 (`CBM_GAME_BOARD_MAX_CARDS`) stays. Overflow omitted. No `has_more`
- [ ] Empty Inbox after hide: column header only. No placeholder card. No copy “all tracked” / “no artifacts in this phase”
- [ ] spec-014: Inbox stays unfiltered by Show Dones and Track A/B/All

### US-006: Specs lock; same GET; zero skill writes
As an operator, I want Specs conversion and Game chrome besides Inbox hide/wrap untouched, So that this spec does not leak registry into Specs or add a route.
Acceptance Criteria:
- [ ] GET `/api/spec-board` must not read `epics_registry.md`. Companion-to / `active.json.source.grill_epic` omit rules unchanged
- [ ] No new HTTP path. No new JSON key required for hide (omit from `inbox[]`). No MCP registry tool
- [ ] GET 200 and POST `/api/game-board` leave `.gamedev/`, `.sdd-skill/`, and `.grill/` byte-identical (and do not create `epics_registry.md`)
- [ ] Silent win, four columns, expand, archive, Blocked strip, Show Dones, Track filters, Enter→Graph stay
- [ ] Game debt chrome / `backlog.md` is out (grill epic-003)

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Game Inbox registry

  # Happy Paths
  Scenario: Registry in_progress omits the epic from Inbox
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And no Game artifact Companion-to path equals ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And "/tmp/bevy/.gamedev/roadmap.md" does not contain a table cell "001" or "epic-001"
    And "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has a data row whose Epic cell is "001" Plan cell is "inbox-plan" Status cell is "in_progress"
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And "/tmp/bevy/.gamedev/epics_registry.md" is byte-identical to its contents before the GET
    When the operator activates the tab named "Game"
    Then the column named "Inbox" does not show a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Registry not_started keeps the epic in Inbox
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has a data row Epic "001" Plan "inbox-plan" Status "not_started"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    When the operator activates the tab named "Game"
    Then the column named "Inbox" shows a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Registry closed and parked omit; no-row stays
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has a data row Epic "002" Plan "inbox-plan" Status "closed"
    And that file has a data row Epic "003" Plan "inbox-plan" Status "parked"
    And that file has no data row whose Plan is "inbox-plan" and Epic normalizes to 4
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-002-done.md" exists
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-003-paused.md" exists
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-004-leftover.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-002-done.md"
    And inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-003-paused.md"
    And inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-004-leftover.md"

  Scenario: File present and Companion-to does not hide an unlisted epic
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has no data row whose Plan is "inbox-plan" and Epic normalizes to 1
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.gamedev/phases/01-preproduction/gdd.md" contains "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md"
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: File absent still hides via Companion-to
    Given "/tmp/bevy/.gamedev/epics_registry.md" does not exist
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.gamedev/phases/01-preproduction/gdd.md" contains "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md"
    When GET /api/game-board?project=bevy is read
    Then inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Inbox id wraps the full path under the short name
    Given GET /api/game-board?project=bevy inbox includes id ".grill/plans/inbox-plan/epics/epic-001-inbox.md" title "inbox"
    When the Game tab paints the Inbox column
    Then the Inbox column shows a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And that card shows the text "inbox"
    And that card's id line computed style text-overflow is not "ellipsis"
    And that card's id line does not have class "truncate"
    And that card's title line may still truncate

  # Limit Cases
  Scenario: Limit Case — empty registry file is present and does not apply prior hide
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file with only the template header and separator
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.gamedev/roadmap.md" contains the token "inbox-plan"
    And "/tmp/bevy/.gamedev/roadmap.md" contains a markdown table row with a cell "001"
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — last duplicate Plan+NNN row wins
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has an earlier data row Epic "001" Plan "inbox-plan" Status "not_started"
    And that file has a later data row Epic "001" Plan "inbox-plan" Status "closed"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — later not_started unhides after an earlier closed
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has an earlier data row Epic "001" Plan "inbox-plan" Status "closed"
    And that file has a later data row Epic "001" Plan "inbox-plan" Status "not_started"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — Epic cell 1 and epic-001 match NNN 1
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has a data row Epic "1" Plan "inbox-plan" Status "in_progress"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — Plan path leftover does not match the slug
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has a data row Epic "001" Plan ".grill/plans/inbox-plan" Status "closed"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — file absent still hides via roadmap slug plus table NNN
    Given "/tmp/bevy/.gamedev/epics_registry.md" does not exist
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And no Game artifact Companion-to path equals that id
    And "/tmp/bevy/.gamedev/roadmap.md" contains the token "inbox-plan"
    And "/tmp/bevy/.gamedev/roadmap.md" contains a markdown table row with a cell "001"
    When GET /api/game-board?project=bevy is read
    Then inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — empty Inbox after hide keeps header only
    Given GET /api/game-board?project=bevy returns HTTP 200
    And gamedev_skill_present is true
    And inbox is []
    When the operator activates the tab named "Game"
    Then the column named "Inbox" is shown
    And that column shows 0 cards
    And that column does not show the text "all tracked"
    And that column does not show the text "no artifacts in this phase"

  Scenario: Limit Case — Inbox cap 64 still omits the 65th
    Given "/tmp/bevy/.gamedev/epics_registry.md" does not exist
    And 65 unconverted grill epic files exist under "/tmp/bevy/.grill/plans/"
    When GET /api/game-board?project=bevy is read
    Then inbox length is 64
    And the response JSON has no field whose name is "has_more"

  Scenario: Limit Case — artifact id line still truncates
    Given GET /api/game-board?project=bevy preproduction includes id ".gamedev/phases/01-preproduction/gdd.md"
    When the Game tab paints the Pre-production column
    Then the artifact card whose id text is ".gamedev/phases/01-preproduction/gdd.md" has class "truncate" on its id line

  Scenario: Limit Case — Specs Todo does not read the registry
    Given "/tmp/bevy/.gamedev/epics_registry.md" has a data row Epic "001" Plan "inbox-plan" Status "closed"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And no spec.md Companion-to path equals that id
    And active.json has no source.grill_epic equal to that id
    When GET /api/spec-board?project=bevy is read
    Then epics has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — Show Dones off still shows a visible Inbox card
    Given GET /api/game-board?project=bevy inbox includes id ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And the Game pane Show Dones control is not pressed
    When the operator activates the tab named "Game"
    Then the column named "Inbox" shows a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — evergreen Epic 0 does not hide a grill card
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has a data row Epic "0" Plan "inbox-plan" Status "evergreen"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — unknown Status token does not hide
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has a data row Epic "001" Plan "inbox-plan" Status "closd"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Limit Case — Plan native never matches a grill slug
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has a data row Epic "001" Plan "native" Status "closed"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  # Error Scenarios
  Scenario: Error — unreadable registry file is present with zero parsed rows
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file and cannot be read
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.gamedev/phases/01-preproduction/gdd.md" contains "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md"
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Error — malformed table row is skipped
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file contains a data row whose Epic cell is "n/a" Plan cell is "inbox-plan" Status cell is "closed"
    And that file contains a well-formed data row Epic "002" Plan "inbox-plan" Status "parked"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-002-done.md" exists
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-002-done.md"

  Scenario: Error — unknown project on GET is still 404
    When GET /api/game-board?project=missing-proj is read
    Then the response status is 404
    And the response body is {"error":"project not found"}

  Scenario: Error — GET does not write skill trees or create the registry
    Given project "bevy" root_path is "/tmp/bevy"
    And "/tmp/bevy/.gamedev/epics_registry.md" does not exist
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And "/tmp/bevy/.gamedev/epics_registry.md" does not exist
    And "/tmp/bevy/.gamedev/state.md" is byte-identical to its contents before the GET
    And "/tmp/bevy/.grill/index.md" is byte-identical to its contents before the GET
    And "/tmp/bevy/.sdd-skill/specs/active.json" is byte-identical to its contents before the GET
```

## Success Metrics
| Metric | Target | Current | Status |
| Inbox omit when registry Status in hide-set | 100% of matching Plan+NNN | met | met |
| Inbox omit via Companion-to/roadmap when registry file present | 0 | met | met |
| Inbox omit via Companion-to/roadmap when registry file absent | 100% of spec-011 cases | met | met (must stay) |
| Inbox epic id fully visible (no truncate) | 100% of painted Inbox cards | met | met |
| Specs conversion mismatches vs spec-008 | 0 | 0 | met (must stay 0) |
| Skill-file writes from GET or Inbox UI | 0 | 0 | met (must stay 0) |
| New HTTP path | 0 | 0 | met (must stay 0) |
Primary KPI: registry sole hide when file present + prior hide when absent + Inbox path wrap + zero skill writes.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + C HTTP + per-project SQLite. `game_board.c` stays zero-write.
- GET `/api/game-board?project=` remains the only Game board read. Hide = omit from `inbox[]`.
- Caps: 64 per Game column including Inbox. Overflow omit. No has-more.
- Chrome grayscale stays. Letter E unchanged.
- i18n: no new Inbox empty-state string. Tests may assert English.
- Constitution I.2: indexer/graph-ui do not write skill cycle files.
- Constitution IV.3: no new endpoint.
- Sync fopen of `epics_registry.md` inside the existing game-board read (not a cache layer).
- “Present” = regular file at `.gamedev/epics_registry.md`. CBM never creates it.

Business:
- Grill plan tech-debt-and-epics-registry epic-002 only. Game debt chrome = epic-003.
- gamedev-skill v1.12.0 table: Epic | Plan | Name | Origin | Status | …

Planner defaults (reject a Gherkin scenario to change these):
1. Ref ticket none. Tech Debt Ref none.
2. Duplicate Plan+NNN: last data row in file order wins.
3. Epic cell normalizes to integer NNN (`001` / `1` / `epic-001`). Unparseable Epic → skip row.
4. Plan cell exact slug only. No basename / path leftover.
5. Hide-set exact tokens: in_progress, closed, parked. not_started and unknown stay. evergreen / Epic 0 ignored.
6. File present (regular file, including empty/unreadable/0 parsed rows) → registry hide only. File absent → spec-011 hide.
7. Empty Inbox: existing empty-column chrome. No “all tracked”.
8. Reuse `game_grill_fill_inbox` walk; swap hide predicate only.
9. InboxCard: title truncate stays; id line drops truncate and wraps (`break-all` like EpicCard). Artifact id truncate locked.
10. Same GET. Server-side omit. Zero skill writes. Specs does not read the registry.

## Out of Scope
- Game debt chrome / backlog.md `debt:*` (grill epic-003)
- Reading `epics_registry.md` from spec_board / Specs Todo
- Changing Companion-to / active.json conversion on Specs
- Writing `.grill/`, `.sdd-skill/`, or `.gamedev/` (including creating the registry)
- New Inbox column, debt cards, or “all tracked” copy
- Fuzzy Plan/Epic match, kebab, or path basename
- New HTTP path or MCP registry tool
- Changing spec-012 expand/archive, spec-014 filters, spec-015 Specs debt strip
- Cards for backlog.md / assets_registry.md / roadmap / game_context / state.md

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% [ ] review approved [ ] human docs confirmed [ ] zero skill writes [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- Parse `.gamedev/epics_registry.md` in `game_board.c` (zero-write fopen rb); do not import spec_board
- Swap hide predicate in `game_grill_epic_converted` / `game_grill_fill_inbox`: if registry regular file present, use registry match only; else keep Companion-to + roadmap
- graph-ui: InboxCard id class change only (drop `truncate`, add wrap). ArtifactCard id locked
- C tests: hide-set, file-present vs absent, last-wins, NNN normalize, exact slug, unreadable, malformed row, cap 64, 404, zero writes / no create
- Vitest: Inbox wrap vs artifact truncate, empty column copy, Show Dones still shows Inbox, Specs regression optional if mocked

## Questions for Architect (answered in plan.md)
- Detect “present”: `stat` regular file vs successful fopen (planner default: regular file exists; unreadable still present)
- Skip loading Companion-to/roadmap tokens when registry file is present (planner default: allowed; must not affect omit when present)
- Wrap CSS: `break-all` vs `break-words` on Inbox id (planner default: same as EpicCard `whitespace-normal break-all`)
- Table column index if a row has fewer cells than the template (planner default: skip row)

## Questions for @implementer
- [ ] Map every Gherkin scenario to a C test and/or graph-ui Vitest
- [ ] Do not write `.gamedev/`, `.sdd-skill/`, or `.grill/`
- [ ] Do not create `epics_registry.md`
- [ ] Do not read the registry from spec_board.c
- [ ] Do not apply Companion-to/roadmap hide when the registry regular file exists
- [ ] Do not add a second GET or an MCP tool
- [ ] Do not drop truncate from artifact card id lines
- [ ] Do not add “all tracked” copy
- [ ] Do not filter Inbox with Show Dones / Track

## Related Specs
Depends on: spec-011-q5n-game-phase-board (Inbox walk + prior hide), spec-012-m2k-game-expand-archive-deps (InboxCard expand), spec-014-x7m-filters (Inbox unfiltered), spec-015-s5k-specs-debt-and-path (EpicCard wrap pattern; Specs lock)
Companion to: .grill/plans/tech-debt-and-epics-registry/epics/epic-002-game-inbox-registry.md
Does not include: grill epic-003 game-debt-chrome

## Revision History
| Date | Author | Change |
| 2026-09-02 | @planner | draft from grill epic-002 + ADR-003,004,005,006,007; ticket=none; debt=no; last-wins; NNN normalize; exact slug |
| 2026-09-02 | @planner | Gherkin approved (user: scenarios approved). Status: approved. Handoff @architect. |
| 2026-09-02 | @architect | plan.md + tasks.md + checklist.md. SDD-ADR-069..071. Awaiting plan approval. |

## Approval Sign-off
Spec Owner(@planner): approved / Gherkin scenarios(user): approved / Product Owner: pending / Architecture(@architect): pending
