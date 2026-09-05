# Spec-015-s5k: Specs debt and path
Status: completed | Spec ID: spec-015-s5k-specs-debt-and-path
Ref Ticket: none | KPI: On Specs, open TECH_DEBT.md items (Status ≠ resolved) appear in a chrome strip above the 3 columns; Todo epic cards show the full grill path wrapping under the short name; zero skill writes; same GET | Priority: P0
Created: 2026-09-01 | Tech Debt Ref: none

## Executive Summary
Specs already hosts the Kanban and Mixed Todo (spec-008/009). The operator cannot see leftover SDD debt without opening `.sdd-skill/baseline/TECH_DEBT.md`, and Todo epic cards truncate the grill path so two epics in the same plan look alike.

This spec adds an additive `debt` array on GET `/api/spec-board` (open TD-NNN only) and paints those rows in a Specs-only chrome strip above the three columns. Todo epic cards keep the short `name` as title (truncate stays) and wrap the full `id` path. Conversion (Companion-to / `active.json.source.grill_epic`) does not change. Game, registry, and backlog.md are the next epics.

Business impact: an operator already on Specs sees open debt and which grill epic is which without leaving the tab or opening Game.

## User Stories

### US-001: Open SDD debt in Specs chrome
As an operator on Specs, I want open TECH_DEBT.md items listed above the three columns, So that leftover debt is visible without opening the file or adding a column.
Acceptance Criteria:
- [ ] GET `/api/spec-board?project=<name>` remains the only board HTTP read. Poll may stay ~4s
- [ ] Response includes additive `debt` (array). Each item has `id` (TD-NNN) and `title` (heading title after `## TD-NNN:`)
- [ ] A row is included iff its Status ≠ `resolved` (identified, deferred, in_progress, unknown, typo)
- [ ] When `debt.length` > 0, SpecBoardTab paints a region with accessible name "Open tech debt" above the three columns. Each row text is `TD-NNN` then the title
- [ ] The strip is not in WorkspaceHeader. Graph, ADR, and Game do not show it
- [ ] No severity, category, status word, or color on the row. Chrome stays grayscale
- [ ] Rows are dead text: activating a row does not POST, does not expand, does not write the clipboard

### US-002: Omit strip when nothing is open
As an operator, I want the debt strip gone when the file is missing or every item is resolved, So that chrome does not show an empty header.
Acceptance Criteria:
- [ ] File missing, unreadable, or `debt` empty → JSON `debt` is `[]` and the "Open tech debt" region is not shown
- [ ] Grill-only path (no `.sdd-skill/`) with no TECH_DEBT.md: Specs tab still shown; strip omitted
- [ ] This-repo all-resolved file: `debt` is `[]`; strip omitted

### US-003: Heading Status wins; table is fallback
As an operator, I want Status to come from the `## TD-NNN` block first, So that a stale summary table cannot hide an open item or invent one.
Acceptance Criteria:
- [ ] If the heading block has a `Status:` token, that token wins even when Debt Summary disagrees
- [ ] If the heading block has no `Status:` token, use the Debt Summary table row for that TD-NNN
- [ ] Unknown or typo Status (anything other than the exact token `resolved`) counts as open
- [ ] A block with no parseable TD-NNN id is skipped. GET stays 200
- [ ] Order is heading/file order (first `## TD-NNN` in the file first). Not severity sort

### US-004: Full path wraps on Todo epic cards
As an operator on Todo, I want the full grill epic path under the short name, wrapping not truncated, So that I can tell which epic file I am looking at.
Acceptance Criteria:
- [ ] EpicCard title stays `epic.title` (`name:`) and may still truncate
- [ ] EpicCard id line shows the full `epic.id` (`.grill/plans/<slug>/epics/epic-NNN-<name>.md`) and wraps. No CSS `truncate` / `text-overflow: ellipsis` on that line
- [ ] Spec cards, Artifact/Inbox cards, and WorkspaceHeader path are unchanged this spec
- [ ] Conversion keys stay the same string. No second identifier field

### US-005: Conversion and Game stay locked
As an operator, I want Specs hide rules and Game untouched, So that this spec does not leak registry or backlog into Specs.
Acceptance Criteria:
- [ ] Companion-to exact path and `active.json.source.grill_epic` still omit that epic from `epics[]` / Todo
- [ ] Do not read `.gamedev/epics_registry.md` or `.gamedev/backlog.md` for Specs
- [ ] GET `/api/game-board` JSON and GameBoardTab chrome/cards unchanged
- [ ] Spec expand, archive, Show archived, last-indexed, Enter→Graph stay

### US-006: Cap 16; same GET; zero skill writes
As an operator, I want overflow omitted and no new route, So that chrome cannot grow without bound and the board stays one fetch.
Acceptance Criteria:
- [ ] Open-debt cap is 16 (`CBM_SPEC_BOARD_MAX_DEBT` or equivalent). Independent of specs 64 / epics 64
- [ ] The 17th open item (heading order) is omitted. No `has_more`. No overflow copy
- [ ] No new HTTP path. No MCP debt tool. `debt` is read sync in `cbm_spec_board_read` (or the existing board reader) on each GET
- [ ] GET 200 and POST leave `.sdd-skill/`, `.grill/`, and `.gamedev/` byte-identical
- [ ] POST `/api/spec-board` with an epic id is still 404 `{"error":"spec not found"}`

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Specs debt and path

  # Happy Paths
  Scenario: Open heading item appears on GET and in the Specs strip
    Given project "alpha" has root_path "/tmp/alpha" and sdd_skill_present is true
    And "/tmp/alpha/.sdd-skill/baseline/TECH_DEBT.md" contains a heading "## TD-005: leftover cache"
    And that heading block has Status: identified
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And debt has length 1
    And debt[0].id is "TD-005"
    And debt[0].title is "leftover cache"
    And the response JSON has no field whose name is "has_more"
    When the operator activates the tab named "Specs"
    Then a region named "Open tech debt" is shown above the three columns
    And that region shows the text "TD-005"
    And that region shows the text "leftover cache"
    And WorkspaceHeader does not show the text "TD-005"
    And the Todo column is still shown

  Scenario: Todo epic id wraps the full path under the short name
    Given GET /api/spec-board?project=alpha epics includes id ".grill/plans/inbox-plan/epics/epic-001-inbox.md" title "inbox"
    When the Specs tab paints the Todo column
    Then the Todo column shows a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And that card shows the text "inbox"
    And that card's id line computed style text-overflow is not "ellipsis"
    And that card's id line does not have class "truncate"
    And that card's title line may still truncate

  Scenario: Companion-to still omits the converted epic
    Given a listed spec.md contains "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/spec-board?project=alpha is read
    Then epics has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And the Todo column does not show a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: All-resolved TECH_DEBT.md omits the strip
    Given "/tmp/alpha/.sdd-skill/baseline/TECH_DEBT.md" has four ## TD-NNN blocks each with Status: resolved
    When GET /api/spec-board?project=alpha is read
    Then debt is []
    When the operator activates the tab named "Specs"
    Then a region named "Open tech debt" is not shown

  # Limit Cases
  Scenario: Limit Case — missing TECH_DEBT.md omits the strip
    Given project "alpha" has sdd_skill_present true
    And "/tmp/alpha/.sdd-skill/baseline/TECH_DEBT.md" does not exist
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And debt is []
    When the operator activates the tab named "Specs"
    Then a region named "Open tech debt" is not shown

  Scenario: Limit Case — grill-only without TECH_DEBT.md still shows Specs
    Given GET /api/spec-board?project=alpha returns HTTP 200
    And sdd_skill_present is false
    And grill_skill_present is true
    And "/tmp/alpha/.sdd-skill/baseline/TECH_DEBT.md" does not exist
    When the operator activates the tab named "Specs"
    Then the tab named "Specs" is shown
    And a region named "Open tech debt" is not shown
    And the pane does not show the text "This project doesn't use sdd-skill — nothing to show here"

  Scenario: Limit Case — heading Status wins when the summary table disagrees
    Given "/tmp/alpha/.sdd-skill/baseline/TECH_DEBT.md" heading "## TD-005: leftover cache" has Status: identified
    And the Debt Summary table row for TD-005 has Status resolved
    When GET /api/spec-board?project=alpha is read
    Then debt has an entry whose id is "TD-005"
    And that entry has title "leftover cache"

  Scenario: Limit Case — table Status is used when the heading has no Status token
    Given heading "## TD-006: no status line" has no Status: token
    And the Debt Summary table row for TD-006 has Status in_progress
    When GET /api/spec-board?project=alpha is read
    Then debt has an entry whose id is "TD-006"

  Scenario: Limit Case — unknown Status token is open
    Given heading "## TD-007: typo status" has Status: resolvd
    When GET /api/spec-board?project=alpha is read
    Then debt has an entry whose id is "TD-007"

  Scenario: Limit Case — 17th open item is omitted
    Given TECH_DEBT.md has 17 heading blocks TD-001 through TD-017 each with Status: identified
    When GET /api/spec-board?project=alpha is read
    Then debt length is 16
    And debt has no entry whose id is "TD-017"
    And the response JSON has no field whose name is "has_more"
    And the Specs tab does not show a control named "Has more"

  Scenario: Limit Case — spec card id line still truncates
    Given GET /api/spec-board?project=alpha includes spec id "spec-010-aaa-planned" column "todo"
    When the Specs tab paints the Todo column
    Then the spec card whose id text is "spec-010-aaa-planned" has class "truncate" on its id line

  Scenario: Limit Case — long debt title wraps in the strip
    Given debt[0].id is "TD-005"
    And debt[0].title is "a very long leftover cache title that exceeds one chrome line"
    When the operator activates the tab named "Specs"
    Then the region named "Open tech debt" shows the full title text
    And that title line computed style text-overflow is not "ellipsis"

  Scenario: Limit Case — Graph ADR and Game do not show the strip
    Given GET /api/spec-board?project=alpha debt has an entry id "TD-005"
    When the operator activates the tab named "Graph"
    Then a region named "Open tech debt" is not shown
    When the operator activates the tab named "ADR"
    Then a region named "Open tech debt" is not shown

  Scenario: Limit Case — activating a debt row does nothing
    Given the region named "Open tech debt" shows the text "TD-005"
    When the operator activates that row
    Then POST /api/spec-board is not sent
    And navigator.clipboard writeText is not called
    And no card expand region is shown

  # Error Scenarios
  Scenario: Error — unreadable TECH_DEBT.md still 200 with empty debt
    Given "/tmp/alpha/.sdd-skill/baseline/TECH_DEBT.md" exists and cannot be read
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And debt is []

  Scenario: Error — heading block without a TD-NNN id is skipped
    Given TECH_DEBT.md contains "## leftover with no id" and a well-formed "## TD-005: leftover cache" with Status: identified
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And debt has an entry whose id is "TD-005"
    And debt has no entry whose title is "leftover with no id"

  Scenario: Error — POST archive with an epic id is 404 and writes nothing
    Given GET /api/spec-board?project=alpha epics includes id ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And specs do not include that id
    When POST /api/spec-board is sent with JSON project "alpha" spec_id ".grill/plans/inbox-plan/epics/epic-001-inbox.md" archived true
    Then the response status is 404
    And the response body is {"error":"spec not found"}
    And "/tmp/alpha/.grill/plans/inbox-plan/epics/epic-001-inbox.md" is byte-identical to its contents before the POST

  Scenario: Error — unknown project on GET is still 404
    When GET /api/spec-board?project=missing-proj is read
    Then the response status is 404
    And the response body is {"error":"project not found"}

  Scenario: Error — GET does not write skill trees or game files
    Given project "alpha" root_path is "/tmp/alpha"
    When GET /api/spec-board?project=alpha is read
    Then the response status is 200
    And "/tmp/alpha/.sdd-skill/baseline/TECH_DEBT.md" is byte-identical to its contents before the GET
    And "/tmp/alpha/.sdd-skill/specs/active.json" is byte-identical to its contents before the GET
    And "/tmp/alpha/.grill/index.md" is byte-identical to its contents before the GET
    And GET /api/game-board is not required to change
```

## Success Metrics
| Metric | Target | Current | Status |
| Open TD items (Status ≠ resolved) visible in Specs strip | 100% up to cap 16 | 0 TECH_DEBT.md reads | not met |
| Strip shown when debt is [] or file missing | 0 | n/a | met (must stay 0) |
| Todo epic id fully visible (no truncate) | 100% of painted epic cards | CSS truncate | not met |
| Conversion mismatches vs spec-008 | 0 | 0 | met (must stay 0) |
| Skill-file writes from GET or debt UI | 0 | 0 | met (must stay 0) |
| New HTTP path | 0 | 0 | met (must stay 0) |
Primary KPI: open debt in chrome + full epic path wrap + zero skill writes.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + C HTTP + per-project SQLite. `spec_board.c` stays zero-write.
- GET `/api/spec-board?project=` remains the only Specs board read. Additive `debt: [{id, title}]`.
- Caps: specs 64, epics 64, tasks 48, open debt 16. Overflow omit. No has-more.
- Chrome grayscale stays (SDD-ADR-005 / constitution III.1). No severity color.
- i18n: region name "Open tech debt" in `en` and `zh`. Tests may assert English. Letter E unchanged.
- Constitution I.2: indexer/graph-ui do not write skill cycle files.
- Constitution IV.3: no new endpoint.
- Sync fopen of TECH_DEBT.md inside the existing spec-board read (not a cache layer).

Business:
- Grill plan tech-debt-and-epics-registry epic-001 only. Game Inbox registry = epic-002. Game debt chrome = epic-003.
- This-repo TECH_DEBT.md is all resolved today; omit is the live default until a new TD is opened.

Planner defaults (reject a Gherkin scenario to change these):
1. Ref ticket none. Tech Debt Ref none.
2. JSON key `debt` always present (empty array when omit). Items `{id, title}` only.
3. Open = Status token ≠ exact `resolved`. Heading `Status:` wins; Debt Summary table is fallback only.
4. Unknown/typo Status = open. Block without TD-NNN id = skip.
5. Cap 16, heading/file order, omit overflow, no has_more.
6. Strip region accessible name "Open tech debt". No visible section heading required. Above 3 columns. Not WorkspaceHeader.
7. Rows wrap; dead text (no copy, no expand, no POST).
8. EpicCard: title truncate stays; id line drops truncate and wraps. Spec/Artifact/Inbox id lines unchanged.
9. Conversion matcher unchanged. No epics_registry / backlog.md read on Specs.
10. Same GET. Sync read. Zero skill writes. POST epic id still 404.

## Out of Scope
- Game Inbox hide via epics_registry (grill epic-002)
- Game debt chrome / backlog.md `debt:*` (grill epic-003)
- New Kanban column, debt cards in Todo, kind E for debt, expand/archive on debt
- Changing Companion-to / active.json conversion
- Writing `.grill/`, `.sdd-skill/`, or `.gamedev/`
- has_more chrome, severity/category/status/color on rows
- Clipboard copy of TD-NNN
- New HTTP path or MCP debt tool
- Changing spec-005 expand, spec-006 archive, spec-007 formatIndexedAt, spec-014 Game filters
- Reading `.gamedev/` from spec_board.c

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% [ ] review approved [ ] human docs confirmed [ ] zero skill writes [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- Extend `cbm_spec_board_t` with a debt array (cap 16); parse TECH_DEBT.md in spec_board.c (zero-write)
- JSON: additive `debt` on the existing GET serializer; never emit has_more
- SpecBoardTab: strip above Column row; EpicCard id class change only
- i18n en+zh for the region name
- C tests: parse heading vs table, unknown token, cap, missing/unreadable file
- Vitest: strip paint/omit, wrap vs spec-card truncate, conversion regression, dead row, Graph/ADR absence

## Questions for Architect (answered in plan.md)
- Always-emit `debt: []` vs omit key when empty (planner default: always emit)
- Status line regex (pipe `ID: … | Status: x |` vs standalone `Status:`)
- Title extract: text after `## TD-NNN:` vs a Title: field
- Wrap CSS: `break-all` vs `break-words` on the epic id and debt title
- Visible "Open tech debt" heading vs visually-hidden / aria-only (planner default: accessible name, no required visible heading)

## Questions for @implementer
- [ ] Map every Gherkin scenario to a C test and/or graph-ui Vitest
- [ ] Do not write `.sdd-skill/`, `.grill/`, or `.gamedev/`
- [ ] Do not read backlog.md or epics_registry.md
- [ ] Do not put debt in Todo / In progress / Done
- [ ] Do not add a second GET or an MCP tool
- [ ] Do not change GameBoardTab
- [ ] Do not add has-more chrome
- [ ] Do not drop truncate from spec card id lines

## Related Specs
Depends on: spec-008-g8r-grill-epic-todo (epics[] + EpicCard), spec-009-t4x-specs-tab-grill-presence (Specs sdd OR grill)
Companion to: .grill/plans/tech-debt-and-epics-registry/epics/epic-001-specs-debt-and-path.md
Does not include: grill epic-002 game-inbox-registry, grill epic-003 game-debt-chrome

## Revision History
| Date | Author | Change |
| 2026-09-01 | @planner | draft from grill epic-001 + ADR-001,002,005,006,007,008; debt=no; ticket=none; heading wins; cap 16; debt[{id,title}] sync; wrap + dead text |
| 2026-09-01 | @planner | Gherkin approved (user: scenarios approved). Status: approved. Handoff @architect |
| 2026-09-02 | @planner | Closed. Constitution IX.2 append confirmed. KPI met. 4/4 PASS DEV |

## Approval Sign-off
Spec Owner(@planner): approved / Gherkin scenarios(user): approved 2026-09-01 / Product Owner: pending / Architecture(@architect): approved 2026-09-01
