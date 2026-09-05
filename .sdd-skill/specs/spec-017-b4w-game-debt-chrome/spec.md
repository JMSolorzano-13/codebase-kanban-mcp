# Spec-017-b4w: Game debt chrome
Status: completed | Spec ID: spec-017-b4w-game-debt-chrome
Ref Ticket: none | KPI: On Game, open backlog.md debt:* entries (no resolved-by in the entry) appear in a chrome strip after BlockedStrip and before the 4 columns; zero skill writes; same GET | Priority: P0
Created: 2026-09-02 | Tech Debt Ref: none

## Executive Summary
Game already maps phase artifacts, Inbox hide (spec-011 / spec-016), and a Blocked strip. The operator cannot see leftover gamedev debt without opening `.gamedev/backlog.md`. gamedev has no TECH_DEBT.md; debt lives as `debt:gate-*` / `debt:adopt-gap-*` / `debt:<slug>` lines. Mixing those into Inbox would collide with grill cards and registry hide.

This spec adds an additive `debt` array on GET `/api/game-board` (open debt:* only) and paints those rows in a Game-only chrome strip after BlockedStrip, before the four columns. Parse stays in `game_board.c` (not the spec-015 TECH_DEBT.md helper). Inbox hide, Specs debt chrome, and conversion stay locked.

Business impact: an operator already on Game sees leftover backlog debt the same way Specs shows open TD-NNN, without a fifth column or Inbox cards.

## User Stories

### US-001: Open gamedev debt in Game chrome
As an operator on Game, I want open backlog.md `debt:*` entries listed after BlockedStrip, So that leftover debt is visible without opening the file or adding a column.
Acceptance Criteria:
- [ ] GET `/api/game-board?project=<name>` remains the only Game board HTTP read
- [ ] Response includes additive `debt` (array). Each item has `id` (full tag including `debt:` prefix) and `title` (description only)
- [ ] A row is included iff the entry has a `debt:<tag>` start and the entry body does not contain `resolved-by`
- [ ] When `debt.length` > 0, GameBoardTab paints a region with accessible name "Open tech debt" after the BlockedStrip slot and before the four columns. Each row text is the tag then the title
- [ ] The strip is not in WorkspaceHeader, not on the Show archived | Show Dones | Track row, and not above the phase line
- [ ] Graph, ADR, and Specs do not show Game `debt` rows. Specs may still show its own spec-board strip from TECH_DEBT.md
- [ ] No owner, target, severity, or color on the row. Chrome stays grayscale
- [ ] Rows are dead text: activating a row does not POST, does not expand, does not write the clipboard
- [ ] Show Dones, Track A/B/All, and Show archived do not hide or filter debt rows

### US-002: Omit strip when nothing is open
As an operator, I want the debt strip gone when the file is missing or every debt:* entry is closed, So that chrome does not show an empty header.
Acceptance Criteria:
- [ ] File missing, unreadable, directory-at-path, or `debt` empty → JSON `debt` is `[]` and the Game "Open tech debt" region is not shown
- [ ] Empty `.gamedev/` (Game still shown, `state.md missing`): strip omitted
- [ ] File with only `design` / `tech` tags and no `debt:` → `debt` is `[]`; strip omitted
- [ ] CBM never creates `.gamedev/backlog.md`

### US-003: Entry parse (line + body; resolved-by anywhere in entry)
As an operator, I want a backlog entry to match how gamedev-skill writes comments and then appends `resolved-by`, So that a closure on the next line hides the row and a `design` tag never appears.
Acceptance Criteria:
- [ ] Start line: first `debt:` immediately followed by a tag body `[A-Za-z0-9][A-Za-z0-9_-]*`. `id` is that full token (example `debt:gate-preproduction`)
- [ ] HTML comments, list items (`-` / `*` / `+` / ordered), and ATX headings all count if they contain that token
- [ ] `debt:` with no tag body (space, punctuation, or end after the colon) is skipped
- [ ] `design` and `tech` without a `debt:` prefix are not entries
- [ ] Entry body: from the start line through (not including) the next start line, the next line-start `##` heading, or EOF. Blank lines do not end the entry
- [ ] Closed iff the entry body contains the exact token `resolved-by` (hyphen, lowercase), on the start line or any later line of that body
- [ ] `title` is the start-line text after the tag, trimmed; strip a wrapping `]`; strip trailing `-->`; cut at the first ` — ` (em dash) or ` -- ` (space double-hyphen space). Owner/target after that cut are omitted. Empty title is allowed
- [ ] Order is file/start-line order. Not owner sort. Duplicate ids: first start-line wins; later same-id starts are skipped
- [ ] A start line without a parseable tag is skipped. GET stays 200

### US-004: Cap 16; same GET; zero skill writes
As an operator, I want overflow omitted and no new route, So that chrome cannot grow without bound and the board stays one fetch.
Acceptance Criteria:
- [ ] Open-debt cap is 16 (`CBM_GAME_BOARD_MAX_DEBT` or equivalent). Independent of cards 64 / blocked 16 / spec-board debt 16
- [ ] The 17th open entry (file order) is omitted. No `has_more`. No overflow copy
- [ ] No new HTTP path. No MCP debt tool. `debt` is read sync in `cbm_game_board_read` on each GET
- [ ] Parse lives in `game_board.c`. Do not call `cbm_spec_board_parse_tech_debt`. Do not fopen backlog.md from HTTP or spec_board.c
- [ ] GET 200 and POST `/api/game-board` leave `.gamedev/`, `.sdd-skill/`, and `.grill/` byte-identical (and do not create `backlog.md`)
- [ ] `backlog.md` stays a non-card (existing non-card lock)

### US-005: Specs and Inbox stay locked
As an operator, I want Specs debt chrome and Game Inbox hide untouched, So that this spec only adds Game chrome.
Acceptance Criteria:
- [ ] GET `/api/spec-board` must not read `backlog.md`. Specs `debt[]` still comes only from TECH_DEBT.md
- [ ] Inbox hide (registry regular file vs spec-011 fallback) unchanged
- [ ] InboxCard wrap, Artifact truncate, four columns, expand, archive, Blocked strip, Show Dones, Track, silent win, Enter→Graph stay
- [ ] POST `/api/game-board` body stays `{project, spec_id, archived}` (or the current flag object). Debt ids are not archive targets

### US-006: Long title wraps; density matches Specs
As an operator, I want a long description to wrap in the strip, So that chrome density matches Specs ADR-008 and I do not lose the rest of the sentence.
Acceptance Criteria:
- [ ] Debt title line wraps. No CSS `truncate` / `text-overflow: ellipsis` on that line
- [ ] Path-wrap remains Inbox/Epic cards only. Artifact id truncate stays
- [ ] Region accessible name is "Open tech debt" (same English as Specs). No visible section heading required

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Game debt chrome

  # Happy Paths
  Scenario: Open comment entry appears on GET and in the Game strip
    Given project "bevy" has root_path "/tmp/bevy" and gamedev_skill_present is true
    And "/tmp/bevy/.gamedev/backlog.md" contains "<!-- debt:gate-preproduction missing GDD lock — director — M1 -->"
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And debt has length 1
    And debt[0].id is "debt:gate-preproduction"
    And debt[0].title is "missing GDD lock"
    And the response JSON has no field whose name is "has_more"
    When the operator activates the tab named "Game"
    Then a region named "Open tech debt" is shown after the Blocked strip slot and before the four columns
    And that region shows the text "debt:gate-preproduction"
    And that region shows the text "missing GDD lock"
    And that region does not show the text "director"
    And that region does not show the text "M1"
    And WorkspaceHeader does not show the text "debt:gate-preproduction"
    And the Inbox column is still shown

  Scenario: List-item entry without resolved-by is open
    Given "/tmp/bevy/.gamedev/backlog.md" contains "- debt:save-slot no checkpoint -- gameplay -- M2"
    When GET /api/game-board?project=bevy is read
    Then debt has an entry whose id is "debt:save-slot"
    And that entry has title "no checkpoint"

  Scenario: resolved-by on a following line closes the entry
    Given "/tmp/bevy/.gamedev/backlog.md" contains
      """
      <!-- debt:gate-preproduction missing GDD lock — director — M1 -->
      resolved-by: SYS-0.1
      """
    When GET /api/game-board?project=bevy is read
    Then debt is []
    When the operator activates the tab named "Game"
    Then a region named "Open tech debt" is not shown

  Scenario: design and tech tags without debt prefix are omitted
    Given "/tmp/bevy/.gamedev/backlog.md" contains "<!-- design camera shake — gameplay — M1 -->"
    And the same file contains "<!-- tech nav mesh bake — tech-architect — M1 -->"
    And the same file contains "<!-- debt:adopt-gap-audio no sfx list — audio — M2 -->"
    When GET /api/game-board?project=bevy is read
    Then debt has length 1
    And debt[0].id is "debt:adopt-gap-audio"
    And debt[0].title is "no sfx list"

  # Limit Cases
  Scenario: Limit Case — missing backlog.md omits the strip
    Given project "bevy" has gamedev_skill_present true
    And "/tmp/bevy/.gamedev/backlog.md" does not exist
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And debt is []
    When the operator activates the tab named "Game"
    Then a region named "Open tech debt" is not shown
    And the tab named "Game" is still shown

  Scenario: Limit Case — empty gamedev dir still shows Game and omits the strip
    Given GET /api/game-board?project=bevy returns HTTP 200
    And gamedev_skill_present is true
    And "/tmp/bevy/.gamedev/backlog.md" does not exist
    And "/tmp/bevy/.gamedev/state.md" does not exist
    When the operator activates the tab named "Game"
    Then the tab named "Game" is shown
    And a region named "Open tech debt" is not shown

  Scenario: Limit Case — same-line resolved-by closes the entry
    Given "/tmp/bevy/.gamedev/backlog.md" contains "<!-- debt:gate-preproduction missing GDD lock resolved-by: SYS-0.1 — director — M1 -->"
    When GET /api/game-board?project=bevy is read
    Then debt is []

  Scenario: Limit Case — blank line does not end the entry
    Given "/tmp/bevy/.gamedev/backlog.md" contains
      """
      <!-- debt:gate-preproduction missing GDD lock — director — M1 -->

      resolved-by: SYS-0.1
      """
    When GET /api/game-board?project=bevy is read
    Then debt is []

  Scenario: Limit Case — next debt start ends the previous entry
    Given "/tmp/bevy/.gamedev/backlog.md" contains
      """
      <!-- debt:gate-preproduction missing GDD lock — director — M1 -->
      <!-- debt:adopt-gap-audio no sfx list — audio — M2 -->
      resolved-by: SYS-0.2
      """
    When GET /api/game-board?project=bevy is read
    Then debt has length 1
    And debt[0].id is "debt:gate-preproduction"
    And debt has no entry whose id is "debt:adopt-gap-audio"

  Scenario: Limit Case — 17th open item is omitted
    Given backlog.md has 17 start lines debt:d01 through debt:d17 each with no resolved-by
    When GET /api/game-board?project=bevy is read
    Then debt length is 16
    And debt has no entry whose id is "debt:d17"
    And the response JSON has no field whose name is "has_more"
    And the Game tab does not show a control named "Has more"

  Scenario: Limit Case — long debt title wraps in the strip
    Given debt[0].id is "debt:gate-preproduction"
    And debt[0].title is "a very long missing GDD lock title that exceeds one chrome line"
    When the operator activates the tab named "Game"
    Then the region named "Open tech debt" shows the full title text
    And that title line computed style text-overflow is not "ellipsis"

  Scenario: Limit Case — Graph ADR and Specs do not show Game debt rows
    Given GET /api/game-board?project=bevy debt has an entry id "debt:gate-preproduction"
    When the operator activates the tab named "Graph"
    Then a region named "Open tech debt" is not shown
    When the operator activates the tab named "ADR"
    Then a region named "Open tech debt" is not shown

  Scenario: Limit Case — Specs strip still uses spec-board debt only
    Given GET /api/spec-board?project=other debt is []
    And GET /api/game-board is not used for that project
    When the operator activates the tab named "Specs"
    Then a region named "Open tech debt" is not shown

  Scenario: Limit Case — activating a debt row does nothing
    Given the region named "Open tech debt" shows the text "debt:gate-preproduction"
    When the operator activates that row
    Then POST /api/game-board is not sent
    And navigator.clipboard writeText is not called
    And no card expand region is shown

  Scenario: Limit Case — Show Dones off still shows the debt row
    Given GET /api/game-board?project=bevy debt has an entry id "debt:gate-preproduction"
    And Show Dones is not pressed
    When the operator activates the tab named "Game"
    Then the region named "Open tech debt" shows the text "debt:gate-preproduction"

  Scenario: Limit Case — Blocked strip stays above the debt strip
    Given GET /api/game-board?project=bevy blocked has length 1
    And debt has an entry id "debt:gate-preproduction"
    When the operator activates the tab named "Game"
    Then the region named "Blocked" is shown
    And the region named "Open tech debt" is below that Blocked region
    And the four columns are below the region named "Open tech debt"

  Scenario: Limit Case — heading with debt tag counts
    Given "/tmp/bevy/.gamedev/backlog.md" contains "## debt:adopt-gap-audio no sfx list"
    When GET /api/game-board?project=bevy is read
    Then debt has an entry whose id is "debt:adopt-gap-audio"
    And that entry has title "no sfx list"

  Scenario: Limit Case — debt colon with no tag body is skipped
    Given "/tmp/bevy/.gamedev/backlog.md" contains "<!-- debt: missing bare tag — director — M1 -->"
    And the same file contains "<!-- debt:gate-preproduction missing GDD lock — director — M1 -->"
    When GET /api/game-board?project=bevy is read
    Then debt has length 1
    And debt[0].id is "debt:gate-preproduction"

  Scenario: Limit Case — backlog.md is still not a card
    Given "/tmp/bevy/.gamedev/backlog.md" contains "<!-- debt:gate-preproduction missing GDD lock — director — M1 -->"
    When GET /api/game-board?project=bevy is read
    Then inbox, preproduction, production, and postproduction have no card whose id contains "backlog.md"

  Scenario: Limit Case — Inbox registry hide is unchanged
    Given "/tmp/bevy/.gamedev/epics_registry.md" exists as a regular file
    And that file has a row Plan inbox-plan Epic 1 Status in_progress
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    When GET /api/game-board?project=bevy is read
    Then inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  # Error Scenarios
  Scenario: Error — unreadable backlog.md still 200 with empty debt
    Given "/tmp/bevy/.gamedev/backlog.md" exists and cannot be read
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And debt is []

  Scenario: Error — directory at backlog.md path still 200 with empty debt
    Given "/tmp/bevy/.gamedev/backlog.md" exists as a directory
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And debt is []

  Scenario: Error — line without a debt tag is skipped
    Given backlog.md contains "notes about the camera"
    And the same file contains "<!-- debt:gate-preproduction missing GDD lock — director — M1 -->"
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And debt has an entry whose id is "debt:gate-preproduction"
    And debt has no entry whose title is "notes about the camera"

  Scenario: Error — unknown project on GET is still 404
    When GET /api/game-board?project=missing-proj is read
    Then the response status is 404
    And the response body is {"error":"project not found"}

  Scenario: Error — GET does not write skill trees or create backlog.md
    Given project "bevy" root_path is "/tmp/bevy"
    And "/tmp/bevy/.gamedev/backlog.md" does not exist
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And "/tmp/bevy/.gamedev/backlog.md" still does not exist
    And "/tmp/bevy/.gamedev/state.md" is byte-identical to its contents before the GET if it existed
    And "/tmp/bevy/.gamedev/epics_registry.md" is byte-identical to its contents before the GET if it existed
    And "/tmp/bevy/.grill/index.md" is byte-identical to its contents before the GET if it existed

  Scenario: Error — POST archive does not write backlog.md
    Given GET /api/game-board?project=bevy includes a done artifact id "phases/02-production/systems/SYS-001-move"
    And "/tmp/bevy/.gamedev/backlog.md" contains "<!-- debt:gate-preproduction missing GDD lock — director — M1 -->"
    When POST /api/game-board is sent with JSON project "bevy" spec_id "phases/02-production/systems/SYS-001-move" archived true
    Then "/tmp/bevy/.gamedev/backlog.md" is byte-identical to its contents before the POST
```

## Success Metrics
| Metric | Target | Current | Status |
| Open debt:* entries visible in Game strip | 100% up to cap 16 | 0 backlog.md debt reads | not met |
| Strip shown when debt is [] or file missing | 0 | n/a | met (must stay 0) |
| design/tech without debt: in the strip | 0 | n/a | met (must stay 0) |
| Inbox hide mismatches vs spec-016 | 0 | 0 | met (must stay 0) |
| Skill-file writes from GET or debt UI | 0 | 0 | met (must stay 0) |
| New HTTP path | 0 | 0 | met (must stay 0) |
Primary KPI: open debt:* in Game chrome after BlockedStrip + zero skill writes.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + C HTTP + per-project SQLite. `game_board.c` stays zero-write.
- GET `/api/game-board?project=` remains the only Game board read. Additive `debt: [{id, title}]`.
- Caps: cards 64, blocked 16, open game debt 16. Overflow omit. No has-more.
- Chrome grayscale stays (SDD-ADR-005 / constitution III.1). No severity color.
- i18n: region name "Open tech debt" in `en` and `zh`. Tests may assert English.
- Constitution I.2: indexer/graph-ui do not write skill cycle files. Never create backlog.md.
- Constitution IV.3: no new endpoint.
- Sync fopen of `{root}/.gamedev/backlog.md` inside the existing game-board read (not a cache layer).
- spec-015 `cbm_spec_board_parse_tech_debt` is locked to TECH_DEBT.md.

Business:
- Grill plan tech-debt-and-epics-registry epic-003 only. Epic-001 = spec-015. Epic-002 = spec-016 (already closed; 003 does not wait on 002).
- This-repo has no `.gamedev/`; omit is the live default until a gamedev path is opened.

Planner defaults (reject a Gherkin scenario to change these):
1. Ref ticket none. Tech Debt Ref none (TD-001..004 all resolved).
2. JSON key `debt` always present (empty array when omit). Items `{id, title}` only. `id` = full `debt:<tag>` token.
3. Open = start line has `debt:<tag>` and entry body has no exact `resolved-by`.
4. Entry body ends at next debt start, next `##` heading, or EOF. Blank lines do not end it.
5. HTML comment, list item, and heading start lines all count. `design`/`tech` without `debt:` are out. Bare `debt:` skipped.
6. Title cut at ` — ` or ` -- `. Owner/target omitted. Empty title allowed. First duplicate id wins.
7. Cap 16, file order, omit overflow, no has_more.
8. Strip region accessible name "Open tech debt". After BlockedStrip slot, before 4 columns. Not WorkspaceHeader. Not filter row.
9. Rows wrap; dead text (no copy, no expand, no POST). Show Dones / Track / Show archived do not filter debt.
10. Parse in game_board.c only. Same GET. Sync read. Zero skill writes. Never create backlog.md.
11. Specs GET and Inbox hide locked.

## Out of Scope
- Changing Inbox hide (registry or Companion-to/roadmap)
- Specs TECH_DEBT.md parse or Specs strip placement
- New Kanban column, debt cards in Inbox, kind E for debt, expand/archive on debt
- Writing `.grill/`, `.sdd-skill/`, or `.gamedev/` (including creating backlog.md)
- has_more chrome, owner/target/severity/color on rows
- Clipboard copy of the debt tag
- New HTTP path or MCP debt tool
- Sharing `cbm_spec_board_parse_tech_debt` with backlog.md
- Changing spec-012 expand/archive, spec-014 filters, spec-016 registry hide
- Lines tagged only `design` or `tech`

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% [ ] review approved [ ] human docs confirmed [ ] zero skill writes [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- Extend `cbm_game_board_t` with a debt array (cap 16); parse backlog.md in game_board.c (zero-write)
- JSON: additive `debt` on the existing GET serializer; never emit has_more
- GameBoardTab: strip after `<BlockedStrip />`, before the 4-col grid
- i18n en+zh for the region name (same English as Specs)
- C tests: comment/list/heading, resolved-by same-line and next-line, blank-line continuation, design/tech skip, cap, missing/unreadable/dir, non-card lock, byte-identical
- Vitest: strip paint/omit, placement vs Blocked, wrap, dead row, Show Dones lock, Graph/ADR absence

## Questions for Architect (answered in plan.md)
- Always-emit `debt: []` vs omit key when empty (planner default: always emit)
- Tag charset: allow `.` inside tag (`debt:gate-01-preproduction`) or hyphen/alnum only (planner default: `[A-Za-z0-9][A-Za-z0-9_-]*`)
- Wrap CSS: `break-all` vs `break-words` on the debt title
- Visible "Open tech debt" heading vs visually-hidden / aria-only (planner default: accessible name, no required visible heading)
- Reuse `t.specBoard.openTechDebt` vs a new `t.gameBoard` key with the same English string

## Questions for @implementer
- [ ] Map every Gherkin scenario to a C test and/or graph-ui Vitest
- [ ] Do not write `.sdd-skill/`, `.grill/`, or `.gamedev/`
- [ ] Do not create backlog.md
- [ ] Do not put debt in Inbox / pre / prod / post
- [ ] Do not add a second GET or an MCP tool
- [ ] Do not change SpecBoardTab debt parse
- [ ] Do not change Inbox hide
- [ ] Do not add has-more chrome
- [ ] Do not call `cbm_spec_board_parse_tech_debt`

## Related Specs
Depends on: spec-010-c4h-game-tab-silent-win, spec-011-q5n-game-phase-board, spec-012-m2k-game-expand-archive-deps (BlockedStrip)
Companion to: .grill/plans/tech-debt-and-epics-registry/epics/epic-003-game-debt-chrome.md
Mirrors: spec-015-s5k-specs-debt-and-path (chrome density, cap 16, dead text, always-emit debt[])
Does not include: grill epic-001 (done), grill epic-002 (done; Inbox hide locked)

## Revision History
| Date | Author | Change |
| 2026-09-02 | @planner | draft from grill epic-003 + ADR-001,002,007,009; debt=no; ticket=none; entry=debt: line + body until next debt/##; resolved-by anywhere in entry; cap 16; debt[{id,title}] sync; wrap + dead text; parse in game_board.c |
| 2026-09-03 | @planner | Gherkin approved; status approved; hand to @architect |
| 2026-09-03 | @planner | CLOSED. IX.2 append confirmed. KPI met. |

## Approval Sign-off
Spec Owner(@planner): closed / Gherkin scenarios(user): approved / Product Owner: pending / Architecture(@architect): approved
