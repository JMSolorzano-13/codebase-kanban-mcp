# Spec-011-q5n: Game phase board
Status: completed | Spec ID: spec-011-q5n-game-phase-board
Ref Ticket: none | KPI: Game tab paints four columns with existing artifact cards and unconverted grill Inbox; converted epics omitted; GET `/api/game-board` fills the four arrays; zero skill writes | Priority: P0
Created: 2026-08-30 | Tech Debt Ref: none

## Executive Summary
spec-010 shipped the Game tab, silent win, and GET `/api/game-board` with empty `inbox` / `preproduction` / `production` / `postproduction`. The operator still cannot see the cycle map: phases, tracks, parallel work, or leftover grill.

This spec fills those four arrays and paints the board. Cards are existing filesystem artifacts (no placeholders) plus unconverted grill epics in Inbox. Converted = Companion-to exact path on a Game artifact, or roadmap slug+NNN. Chrome from spec-010 stays. No expand, archive, drag, mark-done, or skill write.

Business impact: an operator on a live `.gamedev/` tree (e.g. bevy-tetris) sees phase columns, track and work-state on each card, owning agent, and a copyable `/gamedev-skill continue @role`, plus leftover grill in Inbox.

## User Stories

### US-001: Four phase columns and current-phase highlight
As an operator on Game, I want four always-visible columns (Inbox | Pre-production | Production | Post-production & Launch), So that I can read the skill spine without guessing which phase is current.
Acceptance Criteria:
- [ ] When Game pane is shown, all four column headers are visible. English labels (tests assert): "Inbox", "Pre-production", "Production", "Post-production & Launch"
- [ ] Document order of columns is Inbox, then Pre-production, then Production, then Post-production & Launch
- [ ] Empty column: header only. No placeholder card. No copy "no artifacts in this phase"
- [ ] The column that matches GET `phase` has `aria-current="true"`. Mapping: `01-preproduction` → Pre-production; `02-production` → Production; `03-postproduction` → Post-production & Launch
- [ ] Inbox never has `aria-current="true"`
- [ ] When `phase` is null (missing/unreadable state.md), no phase column has `aria-current="true"`. All four headers still show
- [ ] Other columns are not dimmed, hidden, or removed when empty or when they are not the current phase
- [ ] spec-010 chrome (phase label, focus, `/gamedev-skill continue`) remains on the pane

### US-002: Artifact cards for files and folders that exist
As an operator, I want one card per existing owned artifact, So that I see parallel work and which agent to continue as.
Acceptance Criteria:
- [ ] A card is emitted only if the file or directory exists under `.gamedev/`. No placeholder for a missing name
- [ ] Pre-production files (if present): `phases/01-preproduction/gdd.md`, `narrative-bible.md`, `style-guide.md`, `tech-architecture.md`, `audio-direction.md`, `production-plan.md`
- [ ] Production: each immediate child directory of `phases/02-production/systems/` whose name starts with `SYS-`; each immediate child directory of `levels/` whose name starts with `LVL-`; each immediate child directory of `art/`, `animation/`, `audio/`, `ui/`; file `phases/02-production/qa/playtest-log.md` if present. Loose `.md` files directly in `systems/` are not cards
- [ ] Post-production files (if present): `phases/03-postproduction/optimization.md`, `platform-integration.md`, `release-plan.md`, `marketing-plan.md`, `postmortem.md`
- [ ] SYS-* / LVL-* / asset directory without `spec.md` / `level.md` / `context.md` is still a card. Title is the directory name. File card title is the filename (`gdd.md`)
- [ ] Card `id` is the root-relative path starting with `.gamedev/` (directory id has no trailing slash)
- [ ] JSON `kind` is `"artifact"`. Track badge is the text `A` | `B` | `H` (not a second letter-E mark). No A/B/H kind letters besides that badge
- [ ] JSON `work_state` is one of `pending` | `in_progress` | `done` | `blocked`. UI English: Pending / In progress / Done / Blocked
- [ ] Header `status:` first token maps: `draft` or missing → pending; `in_review` | `needs_review` | `wip` | `ready` → in_progress; `approved` | `done` → done; `blocked` → blocked. Unknown token → pending. `ready` is In progress (bevy qa-lead)
- [ ] Directory with no header file → pending
- [ ] Default owner/track when `.gamedev/docs/agents.md` is absent (filesystem.md table): gdd.md @game-designer B; narrative-bible.md @narrative-designer B; style-guide.md @art-director B; tech-architecture.md @tech-architect A; audio-direction.md @audio-director B; production-plan.md @producer B; SYS-* @gameplay-engineer A; LVL-* @level-designer H; art/* @content-artist B; animation/* @animator B; audio/* @sound-designer B; ui/* @ui-designer B; playtest-log.md @qa-lead H; optimization.md @performance-engineer A; platform-integration.md @platform-integrator A; release-plan.md @release-engineer A; marketing-plan.md @marketing-strategist B; postmortem.md @analyst B
- [ ] JSON `owner` is `@role`. JSON `continue` is `/gamedev-skill continue @role` with that same role
- [ ] Many cards in one phase may be `in_progress` at once
- [ ] Column order: Pre-production fixed list gdd, narrative-bible, style-guide, tech-architecture, audio-direction, production-plan (present only). Production: SYS-* by NNN then name, then LVL-* by NNN then name, then art/* alpha, animation/* alpha, audio/* alpha, ui/* alpha, then playtest-log.md. Post-production: optimization, platform-integration, release-plan, marketing-plan, postmortem (present only)
- [ ] Cap 64 per column (`CBM_GAME_BOARD_MAX_CARDS`). Overflow omitted. No `has_more` field or control
- [ ] Not cards: `state.md`, `game_context.md`, `roadmap.md`, `docs/`, `baseline/`, `history/`, `prompts/`, `backlog.md`, `assets_registry.md`

### US-003: Game Inbox lists unconverted grill epics
As an operator on a `.gamedev/` path that also has `.grill/`, I want leftover grill epics in Game Inbox, So that silent win does not hide the funnel.
Acceptance Criteria:
- [ ] Inbox cards come only from GET `/api/game-board` `inbox` (not spec-board, not a second poll URL)
- [ ] Eligible = every epic file under `.grill/plans/*/epics/`, all plans (draft and closed), pending and detailed. Converted (US-004) omitted
- [ ] No `.grill/` directory → `inbox` is `[]`
- [ ] Each inbox entry: `kind` `"epic"`, `id` `.grill/plans/<slug>/epics/epic-NNN-<name>.md`, `title` (epic.md `name`), `summary`, `plan_title` (index.md title for that slug; else plan.md `title`), `track` null, `work_state` null, `owner` `""`, `continue` exactly `/gamedev-skill continue` (no `@role`)
- [ ] UI: letter E (reuse spec-008 `--color-epic-mark`). Title, summary, plan_title visible. No track A/B/H badge. No work-state label
- [ ] Inbox order: `.grill/index.md` table row order; within a plan, `epic-NNN` numeric. Plan dir not in index.md after indexed plans, slug ascending
- [ ] Inbox cap 64, independent of the three phase caps. Overflow omitted. No `has_more`

### US-004: Converted epics leave Inbox
As an operator, I want an epic to leave Inbox when gamedev has claimed it, So that I do not see a duplicate funnel item after conversion.
Acceptance Criteria:
- [ ] Omit iff (1) some Game artifact file contains `Companion to:` whose path token equals that epic `id`, OR (2) roadmap epic-map matches that epic’s plan slug and NNN
- [ ] Companion-to token rule = spec-008: first `.grill/plans/`…`.md` on that line; trailing notes after `.md` still match. Scan fopen rb of existing artifact files: the six pre-prod docs, five post-prod docs, playtest-log.md, and if present `spec.md` / `context.md` / `level.md` inside a listed folder
- [ ] Roadmap match requires both: plan slug as a contiguous token in `.gamedev/roadmap.md` and/or `.gamedev/game_context.md`; AND a markdown table row in `roadmap.md` with a cell that is exactly the 3-digit NNN or `epic-NNN`. Citing the plan directory or slug alone does not convert every epic
- [ ] No kebab/name fuzzy match. `active.json` `source.grill_epic` does not convert Game Inbox
- [ ] Missing both links → epic stays in Inbox (may sit beside a SYS/doc card)
- [ ] Epic file on disk is unchanged

### US-005: Same GET, filled card objects
As the UI, I want GET `/api/game-board` to keep the spec-010 path and keys and emit card objects in the four arrays, So that epic 002 does not add a second HTTP family.
Acceptance Criteria:
- [ ] Path, 400, 404, presence, chrome fields (`phase`, `focus`, `continue`) unchanged from spec-010
- [ ] When `gamedev_skill_present` is false: four arrays are `[]`
- [ ] Card object keys: `kind`, `id`, `title`, `track` (string or null), `work_state` (string or null), `owner` (string), `continue` (string), `summary` (string; `""` on artifacts), `plan_title` (string; `""` on artifacts)
- [ ] Artifact `track` is `"A"` | `"B"` | `"H"`. Epic `track` and `work_state` are JSON null
- [ ] GET `/api/spec-board` still must not emit `gamedev_skill_present`. graph-ui must not call `/api/skill-presence`
- [ ] No new HTTP path. No POST `/api/game-board`. No MCP game-board tool
- [ ] Poll: no second URL. Interval is architect (one-shot from spec-010 is allowed)

### US-006: Map only; zero skill writes
As an operator, I want cards to copy continue and never mutate the skill trees, So that gamedev-skill remains the writer.
Acceptance Criteria:
- [ ] Activating an artifact card’s continue control copies its `continue` string (`/gamedev-skill continue @role`). No toast. No launcher button
- [ ] Activating an inbox card’s continue control copies `/gamedev-skill continue`
- [ ] Activating a card does not expand, Archive, Unarchive, or send POST
- [ ] Cards cannot be dragged between columns. No mark-done control
- [ ] GET 200 leaves `.gamedev/`, `.sdd-skill/`, and `.grill/` byte-identical (and does not create them)
- [ ] Silent win, Enter→Graph, deep-links, spec-008/009 Specs behavior on non-gamedev paths stay unchanged
- [ ] i18n: column headers and work-state labels in `en` and `zh`; tests may assert English

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Game phase board

  # Happy Paths
  Scenario: four column headers and current-phase highlight
    Given GET /api/game-board?project=bevy returns HTTP 200
    And gamedev_skill_present is true
    And phase is "02-production"
    When the operator activates the tab named "Game"
    Then the pane shows a column named "Inbox"
    And the pane shows a column named "Pre-production"
    And the pane shows a column named "Production"
    And the pane shows a column named "Post-production & Launch"
    And in document order those columns appear Inbox then Pre-production then Production then Post-production & Launch
    And the column named "Production" has aria-current "true"
    And the column named "Inbox" does not have aria-current "true"
    And the column named "Pre-production" does not have aria-current "true"
    And the column named "Post-production & Launch" does not have aria-current "true"
    And the pane shows the text "Production"
    And the pane shows the text "/gamedev-skill continue"

  Scenario: existing gdd.md paints a Pre-production artifact card
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    And "/tmp/bevy/.gamedev/phases/01-preproduction/gdd.md" exists
    And that file contains "status: draft"
    And "/tmp/bevy/.gamedev/docs/agents.md" does not exist
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And preproduction has an entry whose id is ".gamedev/phases/01-preproduction/gdd.md"
    And that entry has kind "artifact"
    And that entry has title "gdd.md"
    And that entry has track "B"
    And that entry has work_state "pending"
    And that entry has owner "@game-designer"
    And that entry has continue "/gamedev-skill continue @game-designer"
    And that entry has summary ""
    And that entry has plan_title ""
    When the operator activates the tab named "Game"
    Then the column named "Pre-production" shows a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"
    And that card shows the text "gdd.md"
    And that card shows the text "B"
    And that card shows the text "Pending"
    And that card shows the text "@game-designer"
    And that card shows the text "/gamedev-skill continue @game-designer"
    And that card does not show the text "E"

  Scenario: SYS directory without spec.md is a Production card
    Given "/tmp/bevy/.gamedev/phases/02-production/systems/SYS-001-movement/" exists as a directory
    And "/tmp/bevy/.gamedev/phases/02-production/systems/SYS-001-movement/spec.md" does not exist
    When GET /api/game-board?project=bevy is read
    Then production has an entry whose id is ".gamedev/phases/02-production/systems/SYS-001-movement"
    And that entry has title "SYS-001-movement"
    And that entry has track "A"
    And that entry has work_state "pending"
    And that entry has owner "@gameplay-engineer"
    And that entry has continue "/gamedev-skill continue @gameplay-engineer"
    When the operator activates the tab named "Game"
    Then the column named "Production" shows a card whose title text is "SYS-001-movement"
    And that card shows the text "A"
    And that card shows the text "Pending"

  Scenario: two in_progress cards stay in Production
    Given "/tmp/bevy/.gamedev/phases/02-production/systems/SYS-001-movement/spec.md" contains "status: in_review"
    And "/tmp/bevy/.gamedev/phases/02-production/systems/SYS-002-score/spec.md" contains "status: wip"
    When GET /api/game-board?project=bevy is read
    Then production has an entry id ".gamedev/phases/02-production/systems/SYS-001-movement" with work_state "in_progress"
    And production has an entry id ".gamedev/phases/02-production/systems/SYS-002-score" with work_state "in_progress"
    When the operator activates the tab named "Game"
    Then the column named "Production" shows a card "SYS-001-movement" with the text "In progress"
    And the column named "Production" shows a card "SYS-002-score" with the text "In progress"

  Scenario: unconverted grill epic paints in Inbox
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    And "/tmp/bevy/.grill/index.md" lists plan slug "inbox-plan" with title "Inbox Plan"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" has name "inbox" and summary "Filter unread first."
    And no Game artifact Companion-to path equals ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And "/tmp/bevy/.gamedev/roadmap.md" does not contain a table cell "001" or "epic-001"
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And that entry has kind "epic"
    And that entry has title "inbox"
    And that entry has summary "Filter unread first."
    And that entry has plan_title "Inbox Plan"
    And that entry has track null
    And that entry has work_state null
    And that entry has owner ""
    And that entry has continue "/gamedev-skill continue"
    When the operator activates the tab named "Game"
    Then the column named "Inbox" shows a card whose id text is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And that card shows the text "E"
    And that card shows the text "inbox"
    And that card shows the text "Filter unread first."
    And that card shows the text "Inbox Plan"
    And that card shows the text "/gamedev-skill continue"
    And that card does not show the text "@director"
    And that card does not show the text "Pending"
    And that card does not show the text "A"

  Scenario: Companion-to exact path omits the epic from Inbox
    Given "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.gamedev/phases/01-preproduction/gdd.md" contains "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md"
    When GET /api/game-board?project=bevy is read
    Then inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" is byte-identical to its contents before the GET

  Scenario: roadmap slug plus table NNN omits the epic
    Given "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And no Game artifact contains a Companion-to path equal to that id
    And "/tmp/bevy/.gamedev/roadmap.md" contains the token "inbox-plan"
    And "/tmp/bevy/.gamedev/roadmap.md" contains a markdown table row with a cell "001"
    When GET /api/game-board?project=bevy is read
    Then inbox has no entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: click artifact copies continue at-role
    Given the column named "Pre-production" shows a card whose continue text is "/gamedev-skill continue @game-designer"
    When the operator activates that card's continue control
    Then the clipboard text is "/gamedev-skill continue @game-designer"
    And the pane does not show a toast
    And POST /api/game-board is not sent

  Scenario: click inbox copies continue without at-role
    Given the column named "Inbox" shows a card whose continue text is "/gamedev-skill continue"
    When the operator activates that card's continue control
    Then the clipboard text is "/gamedev-skill continue"
    And the pane does not show a toast

  # Limit Cases
  Scenario: Limit Case — empty column has header and no placeholder
    Given GET /api/game-board?project=bevy returns HTTP 200
    And gamedev_skill_present is true
    And postproduction is []
    When the operator activates the tab named "Game"
    Then the column named "Post-production & Launch" is shown
    And that column shows 0 cards
    And that column does not show the text "no artifacts in this phase"

  Scenario: Limit Case — missing narrative-bible is not a placeholder card
    Given "/tmp/bevy/.gamedev/phases/01-preproduction/gdd.md" exists
    And "/tmp/bevy/.gamedev/phases/01-preproduction/narrative-bible.md" does not exist
    When GET /api/game-board?project=bevy is read
    Then preproduction has no entry whose id is ".gamedev/phases/01-preproduction/narrative-bible.md"
    And preproduction has no entry whose title is "narrative-bible.md"

  Scenario: Limit Case — phase null highlights no phase column
    Given GET /api/game-board?project=bevy returns HTTP 200
    And gamedev_skill_present is true
    And phase is null
    When the operator activates the tab named "Game"
    Then the pane shows a column named "Inbox"
    And the pane shows a column named "Pre-production"
    And the pane shows a column named "Production"
    And the pane shows a column named "Post-production & Launch"
    And no column has aria-current "true"
    And the pane shows the text "state.md missing"

  Scenario: Limit Case — 65th production card omitted
    Given 65 SYS-* directories exist under "/tmp/bevy/.gamedev/phases/02-production/systems/"
    When GET /api/game-board?project=bevy is read
    Then production length is 64
    And the response JSON has no field whose name is "has_more"
    When the operator activates the tab named "Game"
    Then the column named "Production" does not show a control named "Has more"

  Scenario: Limit Case — closed grill plan leftover stays in Inbox
    Given "/tmp/bevy/.grill/index.md" lists slug "old-plan" with status closed
    And "/tmp/bevy/.grill/plans/old-plan/epics/epic-009-leftover.md" has status pending
    And that epic is not converted
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/old-plan/epics/epic-009-leftover.md"

  Scenario: Limit Case — missing conversion link sits beside a SYS card
    Given "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.gamedev/phases/02-production/systems/SYS-001-inbox/" exists as a directory
    And no Companion-to path equals ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And roadmap.md has no table cell "001" or "epic-001"
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
    And production has an entry whose id is ".gamedev/phases/02-production/systems/SYS-001-inbox"

  Scenario: Limit Case — status ready maps to In progress
    Given "/tmp/bevy/.gamedev/phases/02-production/qa/playtest-log.md" contains "status: ready"
    When GET /api/game-board?project=bevy is read
    Then production has an entry whose id is ".gamedev/phases/02-production/qa/playtest-log.md"
    And that entry has work_state "in_progress"
    And that entry has track "H"
    And that entry has owner "@qa-lead"
    When the operator activates the tab named "Game"
    Then that card shows the text "In progress"

  Scenario: Limit Case — no grill directory yields empty Inbox
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    And "/tmp/bevy/.grill/" does not exist
    When GET /api/game-board?project=bevy is read
    Then inbox is []
    When the operator activates the tab named "Game"
    Then the column named "Inbox" shows 0 cards

  Scenario: Limit Case — Inbox order is index.md then epic-NNN
    Given "/tmp/bevy/.grill/index.md" lists slug "plan-a" then slug "plan-b"
    And plan-a has epics epic-002-second.md then epic-001-first.md on disk
    And plan-b has epic-001-other.md
    And none of those epics are converted
    When GET /api/game-board?project=bevy is read
    Then inbox id order is
      ".grill/plans/plan-a/epics/epic-001-first.md"
      then ".grill/plans/plan-a/epics/epic-002-second.md"
      then ".grill/plans/plan-b/epics/epic-001-other.md"

  Scenario: Limit Case — present false still emits empty arrays
    Given GET /api/game-board?project=alpha returns HTTP 200
    And gamedev_skill_present is false
    Then inbox is []
    And preproduction is []
    And production is []
    And postproduction is []

  Scenario: Limit Case — spec-board still has no gamedev field
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    When GET /api/spec-board?project=bevy is read
    Then the response status is 200
    And the response JSON has no field whose name is "gamedev_skill_present"

  # Error Scenarios
  Scenario: Error — GET does not write skill trees
    Given project "bevy" root_path is "/tmp/bevy"
    And "/tmp/bevy/.gamedev/" exists as a directory
    And "/tmp/bevy/.grill/" exists as a directory
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And "/tmp/bevy/.gamedev/" files are byte-identical to their contents before the GET
    And "/tmp/bevy/.grill/" files are byte-identical to their contents before the GET
    And "/tmp/bevy/.sdd-skill/" does not get created
    And graph-ui issues no request whose path contains "/api/skill-presence"

  Scenario: Error — unknown project is still 404
    Given no catalog project named "missing"
    When GET /api/game-board?project=missing is read
    Then the response status is 404
    And the response body is {"error":"project not found"}

  Scenario: Error — missing project query is still 400
    When GET /api/game-board is read
    Then the response status is 400
    And the response body is {"error":"missing project parameter"}

  Scenario: Error — card activate does not expand or archive
    Given the column named "Pre-production" shows a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"
    When the operator activates that card
    Then that card does not show a control named "Archive"
    And that card does not show a control named "Unarchive"
    And that card does not show the text "No tasks planned yet"
    And POST /api/game-board is not sent
    And POST /api/spec-board is not sent

  Scenario: Error — cards are not draggable
    Given the column named "Production" shows a card whose title text is "SYS-001-movement"
    When the operator attempts to drop that card on the column named "Pre-production"
    Then preproduction length is unchanged
    And that card remains in the column named "Production"

  Scenario: Error — plan-folder cite without NNN does not convert
    Given "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.gamedev/roadmap.md" contains the text ".grill/plans/inbox-plan/"
    And "/tmp/bevy/.gamedev/roadmap.md" has no table cell "001" or "epic-001"
    And no Companion-to path equals that epic id
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Error — kebab name does not convert
    Given "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.gamedev/phases/02-production/systems/SYS-001-inbox/" exists as a directory
    And no Companion-to path equals that epic id
    And roadmap.md has no table cell "001" or "epic-001"
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"

  Scenario: Error — table NNN without slug cite does not convert
    Given "/tmp/bevy/.grill/plans/inbox-plan/epics/epic-001-inbox.md" exists
    And "/tmp/bevy/.gamedev/roadmap.md" contains a markdown table row with a cell "001"
    And neither roadmap.md nor game_context.md contains the token "inbox-plan"
    And no Companion-to path equals that epic id
    When GET /api/game-board?project=bevy is read
    Then inbox has an entry whose id is ".grill/plans/inbox-plan/epics/epic-001-inbox.md"
```

## Success Metrics
| Metric | Target | Current | Status |
| Four headers always visible on Game | 100% | 0 columns painted | not met |
| Current-phase column aria-current | 100% when phase is 01/02/03 | n/a | not met |
| Existing artifacts appear; missing names do not | 100% | arrays [] | not met |
| Unconverted grill in Inbox; converted omitted | 100% | inbox [] | not met |
| Continue copy @role / inbox without @ | 100% | chrome continue only | not met |
| Skill-file writes from GET or Game UI | 0 | 0 | met (must stay 0) |
| New HTTP path or POST/MCP | 0 | 0 | met (must stay 0) |
Primary KPI: first four metrics plus zero skill writes.

## Constraints & Assumptions
Technical:
- Same GET `/api/game-board`. spec-010 chrome and presence rules stay. `cbm_game_board_card_t` today is id+title; this spec widens the emitted JSON.
- `CBM_GAME_BOARD_MAX_CARDS` is already 64. Use it per column.
- Constitution I.2: read-only on skill trees. IV.3: no new endpoint unless the current GET cannot meet AC (it can).
- Chrome grayscale. Letter E reuses spec-008 token. Track badge is text, not a new chromatic kind mark.
- Path = Project 1:1. 400/404 strings unchanged.

Business:
- Grill plan add-gamedev-skill epic-002. ADR-002, ADR-003, ADR-005, ADR-006, ADR-009.
- Epic 003 expand/archive/deps. Epic 004 ADR trio.

Planner defaults A (reject a Gherkin scenario to change these):
1. Ref ticket none. No TD.
2. Always four headers. Empty = no placeholder. Highlight = aria-current on the matching phase column only. Do not dim or hide.
3. Inbox = all plans (draft and closed), index.md then epic-NNN. Sit-beside if missing link.
4. Inbox letter E. Artifacts: track text A/B/H, no extra letters. Work-state English Pending / In progress / Done / Blocked. Click copies; no toast.
5. Directory existing is a card. Title = folder name or filename. Cap 64 per column. Overflow omit. No has_more. `ready` → in_progress.
6. Conversion: Companion-to exact OR (slug token + table NNN). Not active.json. Not kebab. Not plan-folder-only.
7. Zero writes. No drag / mark-done / expand / archive.

## Out of Scope
- Expand, archive, blocked-by strip, Track A Inputs (epic 003)
- ADR fill from the gamedev trio (epic 004)
- Overlay of state.md agent lines vs header (architect may add later; this spec is header-first)
- Writing `.gamedev/` or invoking gamedev-skill from CBM
- Drag / mark-done / launcher / toast
- POST `/api/game-board` / MCP game-board tool / GET `/api/skill-presence` from graph-ui
- Emitting `gamedev_skill_present` on spec-board
- Changing silent win, Enter→Graph, or Specs on non-gamedev paths
- Cards for state.md, roadmap.md, game_context.md, docs, baseline, history, prompts, backlog, assets_registry
- Grouping all art/* as one card

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% [ ] review approved [ ] human docs confirmed [ ] zero skill writes [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- Widen `cbm_game_board_card_t` + `cbm_game_board_to_json` to emit real arrays (today hardcoded `[]`)
- Walk `.gamedev/phases/**` fopen rb; grill Inbox may reuse spec_board walk or duplicate under game_board.c
- Roadmap table parse is new C; contract is slug token + cell NNN
- graph-ui: GameBoardTab paints four columns + cards; useGameBoard types `unknown[]` → card type
- i18n: column headers, work-state labels
- Vitest: columns, highlight, cards, Inbox, copy, no toast/drag/expand. C tests: GET objects, conversion, cap 64, 400/404, zero writes

## Questions for Architect (answered in plan.md)
- New walk in game_board.c vs shared grill helpers with spec_board.c (no shared JSON)
- Roadmap heading/table if source: line and table disagree beyond US-004 (this spec: both required)
- Heap bound for bevy-sized art+systems listing (64/column already)
- Project `.gamedev/docs/agents.md` override vs filesystem.md default table
- Poll interval vs keep spec-010 one-shot
- Clipboard API vs select-text fallback when clipboard is denied

## Questions for @implementer
- [ ] Map every Gherkin scenario to a Vitest and every GET field/status scenario to a C test
- [ ] Do not write `.gamedev/`, `.sdd-skill/`, or `.grill/`
- [ ] Do not add a new HTTP path, POST, or MCP tool
- [ ] Do not paint expand, Archive, toast, or drag
- [ ] Do not convert Inbox via kebab name or active.json
- [ ] Do not emit `gamedev_skill_present` on spec-board
- [ ] Keep spec-010 chrome and silent win

## Related Specs
Depends on: spec-010-c4h-game-tab-silent-win (tab, GET, empty arrays, silent win)
Companion to: .grill/plans/add-gamedev-skill/epics/epic-002-game-phase-board.md
Reuses: spec-008 Companion-to token + Inbox order + letter E
Does not include: epic 003 expand/archive/deps, epic 004 ADR trio

## Revision History
| Date | Author | Change |
| 2026-08-30 | @planner | draft from grill epic-002 + ADR-002/003/005/006/009; defaults A; debt=no; ticket=none |
| 2026-08-30 | @planner | Gherkin approved by user; status approved; handoff @architect |
| 2026-08-31 | @architect | User: continue approved. Architecture approved. Handoff @implementer Task #1. |
| 2026-08-31 | @planner | Constitution changes confirmed. Status completed. Spec closed. |

## Approval Sign-off
Spec Owner(@planner): closed / Gherkin scenarios(user): approved / Product Owner: constitution confirmed 2026-08-31 / Architecture(@architect): approved 2026-08-31
