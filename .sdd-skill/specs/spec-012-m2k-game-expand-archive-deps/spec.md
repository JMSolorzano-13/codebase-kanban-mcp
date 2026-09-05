# Spec-012-m2k: Game expand, archive, and deps
Status: completed | Spec ID: spec-012-m2k-game-expand-archive-deps
Ref Ticket: none | KPI: On Game, a card expands in place with kind-appropriate detail; a done artifact archives into CBM and stays hidden on a fresh visit; the board lists live state.md blocked lines | Priority: P0
Created: 2026-08-31 | Tech Debt Ref: none

## Executive Summary
spec-011 paints four Game columns with exist-only artifact cards and unconverted grill Inbox. Cards are still dead: no expand, no archive, no live blocked-by. spec-011 Gherkin forbids POST `/api/game-board` and expand; this spec supersedes those two locks only.

This spec adds Specs-parity detail on the Game map: in-place expand (Track A blurb+tasks+Inputs; Track B/hybrid header; grill summary already on the card), CBM-owned archive of done artifacts, and live deps (blocked-by overlay plus a board strip). CBM still does not write `.gamedev/` or launch the skill. Archive is the sole Game mutation.

Business impact: an operator inspects one artifact without leaving the board, hides finished work, and sees who is blocked — without a second interaction pattern or a skill-file write.

## User Stories

### US-001: Any Game card expands in place
As an operator on Game, I want to activate the title control on an artifact or Inbox card and have that same card grow, So that I do not need a modal, side panel, or a second page.
Acceptance Criteria:
- [ ] Click target is a title control on the card (`aria-expanded` true when open). Not a popover, modal, side panel, or accordion
- [ ] Continue on the card does not toggle expand (existing `stopPropagation` stays)
- [ ] Chrome `/gamedev-skill continue` text is not an expand control
- [ ] Every artifact and Inbox card can expand, including Track A with zero tasks and Track B with empty `open`/`last_decision`
- [ ] All cards start collapsed on first paint of GameBoardTab for a project (no Specs-style auto-expand)
- [ ] Several cards may stay expanded at once; opening one does not collapse another
- [ ] Expanded state is a Set keyed by GET `id` (root-relative path). A later GET refetch must not collapse a card the operator opened
- [ ] Changing `?project=` or remounting GameBoardTab starts all collapsed
- [ ] Cards remain `draggable={false}`. No mark-done. Silent win, four columns, conversion, and clipboard from spec-010/011 stay

### US-002: Expand body follows artifact kind
As an operator, I want the expand body to match the skill artifact (Track A spec, Track B header, level changelog, playtest last round, grill summary), So that I never see a fake task list or a dumped GDD.
Acceptance Criteria:
- [ ] Additive GET fields on every card: `blurb` (string), `tasks` (array, may be `[]`), `inputs` (string), `last_decision` (string), `open` (string), `recent` (string), `blocked_by` (string or JSON null), `archived` (boolean)
- [ ] Track A (`track` `"A"`, SYS-* with `spec.md`): `blurb` is the first 1–2 sentences of `## What it does` until the next `## ` heading. Sentence end = `.` / `?` / `!` followed by space or end of text. Markdown links become link text. Cap 512 bytes. If that heading is missing or the body is empty: use header `open` if non-empty and not `none`; else header `last_decision` if non-empty and not `—`; else `""`
- [ ] When `## What it does` has a non-empty body, header `open` / `last_decision` are not used as `blurb` (they may still appear as `open` / `last_decision` fields)
- [ ] Track A `tasks` come from that system's `tasks.md` checkbox lines `- [ ]` / `- [x]` / `- [X]` only. Number is 1-based list order. `done` is true iff the box is `[x]` or `[X]`. Lines that are `Subagent:` / `Path:` / HTML comments / empty are not tasks. Cap 48; overflow omitted; no `has_more`
- [ ] Track A expand lists every parsed task as `#N name` (all tasks, not pending-only). Zero tasks → English "No tasks planned yet"; expand still works
- [ ] Track A `inputs` is the body of the first heading whose text starts with `Inputs` after `## ` (covers `## Inputs / Outputs` and `## Inputs`) until the next `## `. Cap 512 bytes. Missing heading or empty body → `inputs` `""`; UI omits an Inputs region
- [ ] Track A expand shows an Inputs region only when `inputs` is non-empty. English label: "Inputs"
- [ ] Track B and asset `context.md` (`track` `"B"`): expand shows `last_decision` and `open` when non-empty; no task list; no "No tasks planned yet"; `blurb` `""`; `inputs` `""`; `tasks` `[]`
- [ ] LVL-* (`track` `"H"`): expand shows `last_decision` and `open` from `level.md` plus `recent` = last 8 non-empty non-HTML-comment lines of `changelog.md` joined by newline. Missing changelog → `recent` `""`
- [ ] `playtest-log.md`: `recent` is the last block that starts with a heading `## Round ` through the next `## Round ` or EOF, cap 512 bytes. An earlier Round is not copied. No Round heading → `recent` `""`
- [ ] Inbox (`kind` `"epic"`): collapsed still shows `summary` and `plan_title` as spec-011. Expand does not add blurb/tasks/Inputs/Archive/Unarchive. Empty summary does not show "No tasks planned yet"
- [ ] Missing or unreadable `spec.md` / `tasks.md` / `level.md` / `changelog.md` / `playtest-log.md` degrades that card (empty strings / `[]`) and does not fail GET 200
- [ ] Expand never dumps the full GDD, spec body, or review.md

### US-003: Archive a done artifact into CBM
As an operator, I want Archive on an expanded done artifact that hides that card immediately, So that finished work leaves the default board without a confirm dialog or a skill-file change.
Acceptance Criteria:
- [ ] Archive appears only on an expanded artifact whose JSON `work_state` is `"done"` and `archived` is false
- [ ] Inbox, pending, in_progress, and blocked expands do not show Archive or Unarchive
- [ ] Activating Archive sends POST `/api/game-board` with JSON `project`, `card_id` (the GET `id`), `archived` true. No dialog, alertdialog, or `window.confirm`
- [ ] POST 200 body is a flag object: `card_id` plus `archived` true. It is not the full board
- [ ] After 200, with Show archived off, that card is not in its phase column
- [ ] Flag persist is a new `game_archive` table in that project's CBM `.db`, keyed by `card_id`. Not `spec_archive`. Not localStorage. Not a skill sidecar
- [ ] GET `/api/game-board` merges `archived` after the skill read. A flag whose `card_id` is not on the current board does not invent a card
- [ ] POST does not write, move, or rename anything under `.gamedev/`, `.sdd-skill/`, or `.grill/`
- [ ] No MCP archive tool. POST `/api/spec-board` is unchanged and must not accept Game `card_id` values as spec ids (404 `{"error":"spec not found"}`)

### US-004: Session Show archived and Unarchive
As an operator, I want a session-only Show archived control and Unarchive on an expanded archived done card, So that a mistaken archive is reversible without persisting the toggle.
Acceptance Criteria:
- [ ] Accessible name is "Show archived". It lives in Game pane chrome (not Inbox header, not a fourth column)
- [ ] `aria-pressed` is false on first paint of GameBoardTab for a project. Activating it sets `aria-pressed` true and shows phase cards with `archived` true. Activating again hides them
- [ ] Changing `?project=` or remounting starts unpressed. No localStorage. A GET refetch after POST does not reset the toggle
- [ ] Inbox is never filtered by `archived`
- [ ] When every artifact in a phase column is archived and the toggle is off, that column is header only (spec-011 empty rule). No "No specs yet" copy
- [ ] When Show archived is on, an expanded done card with `archived` true shows Unarchive and does not show Archive
- [ ] Unarchive POSTs `archived` false for that `card_id`. POST 200 body has `archived` false. After 200 the card stays visible even if Show archived is off
- [ ] Unarchive does not open a confirm dialog
- [ ] Leftover `archived` true on a card whose `work_state` is not `"done"`: JSON may say true; UI does not hide it; no Archive/Unarchive. If that card later becomes done, the existing true flag hides it again

### US-005: Blocked-by overlay and board strip
As an operator, I want live `blocked-by` on that agent's artifact cards and a strip of current blocked lines, So that parallel blocks are visible without a roadmap graph.
Acceptance Criteria:
- [ ] GET `/api/game-board` includes a top-level `blocked` array. Each element: `owner` (`@` + agent-slug), `task` (string), `blocked_by` (string)
- [ ] Source is `.gamedev/state.md` agent lines whose status token is `blocked`. Compact form: `slug:blocked:"task":"blocked-by"`. `needs_review`, `in_progress`, and `done` lines are not strip rows
- [ ] Strip order is document order of those lines. Cap 16. Overflow omitted. No `has_more`
- [ ] Empty `blocked` → the strip is omitted (no heading)
- [ ] Non-empty `blocked` → a strip above the four columns and below spec-010 chrome. English accessible name "Blocked". Each row shows `owner`, `task`, and `blocked_by`. Clicking a row does nothing (no scroll, no highlight, no navigation)
- [ ] Overlay: every artifact card whose `owner` equals that row's `owner` gets JSON `work_state` `"blocked"` (overrides header mapping from spec-011) and `blocked_by` equal to that line's blocked-by text. Inbox cards are never overlaid
- [ ] A card whose owner has no `blocked` line keeps spec-011 header `work_state` and `blocked_by` JSON null
- [ ] Expanded/collapsed card with non-null `blocked_by` shows that text. English prefix "Blocked"
- [ ] Do not parse `roadmap.md` into a graph. Do not paint agents.md Needs as edges

### US-006: Same GET family, POST archive only, zero skill writes
As an operator, I want expand data on the existing GET and archive on POST `/api/game-board`, So that Game does not grow a second read URL or write skill trees.
Acceptance Criteria:
- [ ] GET path, 400, 404, presence, chrome (`phase`, `focus`, `continue`), four arrays, conversion, and spec-011 card keys stay. Additive card fields and top-level `blocked` only
- [ ] After POST 200, UI refetches GET `/api/game-board` (or applies the flag) so a later GET cannot resurrect a hidden card while Show archived is off. Poll interval remains architect; spec-010 one-shot is allowed
- [ ] POST missing `project` or `card_id` → 400. Missing or non-boolean `archived` → 400 `{"error":"invalid archived"}`
- [ ] Unknown project → 404 `{"error":"project not found"}` (same string as GET)
- [ ] `card_id` not on the current board, or an Inbox epic id → 404 `{"error":"card not found"}`. No store write
- [ ] Artifact whose JSON `work_state` is not `"done"` → 409 `{"error":"card not done"}`. No store write
- [ ] Repeat POST with the same `archived` value is 200 and idempotent
- [ ] GET 200 leaves `.gamedev/`, `.sdd-skill/`, and `.grill/` byte-identical. POST 200 leaves those trees byte-identical
- [ ] GET `/api/spec-board` still must not emit `gamedev_skill_present`. graph-ui must not call `/api/skill-presence`. No MCP game-board tool
- [ ] i18n: Archive, Unarchive, Show archived, No tasks planned yet, Inputs, Blocked strip — `en` and `zh`. Tests may assert English. Reuse Specs English strings where they already match

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Game expand, archive, and deps

  # Happy Paths
  Scenario: Track A SYS card expands with blurb, tasks, and Inputs
    Given GET /api/game-board?project=bevy returns HTTP 200
    And gamedev_skill_present is true
    And production includes kind "artifact" id ".gamedev/phases/02-production/systems/SYS-001-movement" track "A" work_state "pending" archived false
    And that entry has blurb "Moves the tetromino left and right. DAS applies after the first tap."
    And that entry has inputs "Grid occupancy from collision. Outputs a new piece position."
    And that entry has tasks [{"number":1,"name":"Parse input","done":true},{"number":2,"name":"Apply DAS","done":false}]
    And the Game tab is showing the board for "bevy"
    When the operator activates the title control on the card whose id text is ".gamedev/phases/02-production/systems/SYS-001-movement"
    Then that card has aria-expanded true
    And that card shows the text "Moves the tetromino left and right. DAS applies after the first tap."
    And that card shows the text "#1 Parse input"
    And that card shows the text "#2 Apply DAS"
    And that card shows the text "Inputs"
    And that card shows the text "Grid occupancy from collision. Outputs a new piece position."
    And that card does not show a control named "Archive"
    And that card does not show a control named "Unarchive"

  Scenario: Track B gdd expand shows header only
    Given GET /api/game-board?project=bevy includes preproduction id ".gamedev/phases/01-preproduction/gdd.md" track "B" work_state "pending"
    And that entry has last_decision "Lock the loop as rotating tetrominoes."
    And that entry has open "Need player fantasy one-liner."
    And that entry has blurb ""
    And that entry has tasks []
    And that entry has inputs ""
    And the Game tab is showing the board for "bevy"
    When the operator activates the title control on the card whose id text is ".gamedev/phases/01-preproduction/gdd.md"
    Then that card has aria-expanded true
    And that card shows the text "Lock the loop as rotating tetrominoes."
    And that card shows the text "Need player fantasy one-liner."
    And that card does not show the text "No tasks planned yet"
    And that card does not show the text "Inputs"
    And that card does not show a control named "Archive"

  Scenario: Archive hides a done artifact without a confirm dialog
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/audio-direction.md" work_state "done" archived false
    And the Game tab is showing the board for "bevy"
    And the operator has expanded the card whose id text is ".gamedev/phases/01-preproduction/audio-direction.md"
    When the operator activates the control named "Archive"
    Then no dialog or alertdialog is shown
    And POST /api/game-board is sent with JSON project "bevy" card_id ".gamedev/phases/01-preproduction/audio-direction.md" archived true
    And that POST response status is 200
    And that POST response JSON has card_id ".gamedev/phases/01-preproduction/audio-direction.md" and archived true
    And the Pre-production column does not show a card whose id text is ".gamedev/phases/01-preproduction/audio-direction.md"
    And "/tmp/bevy/.gamedev/phases/01-preproduction/audio-direction.md" is byte-identical to its contents before the POST

  Scenario: blocked strip and overlay on that owner's cards
    Given project "bevy" has root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev/state.md" contains a line gameplay-engineer:blocked:"Combat system v2":"Waiting on final boss design"
    And GET /api/game-board?project=bevy production includes id ".gamedev/phases/02-production/systems/SYS-001-movement" owner "@gameplay-engineer"
    And GET /api/game-board?project=bevy preproduction includes id ".gamedev/phases/01-preproduction/gdd.md" owner "@game-designer" work_state "pending"
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And the JSON blocked array has one entry with owner "@gameplay-engineer" task "Combat system v2" blocked_by "Waiting on final boss design"
    And the entry id ".gamedev/phases/02-production/systems/SYS-001-movement" has work_state "blocked"
    And that entry has blocked_by "Waiting on final boss design"
    And the entry id ".gamedev/phases/01-preproduction/gdd.md" has work_state "pending"
    And that entry has blocked_by null
    When the operator activates the tab named "Game"
    Then the pane shows a region named "Blocked"
    And that region shows the text "@gameplay-engineer"
    And that region shows the text "Waiting on final boss design"
    And the card whose id text is ".gamedev/phases/02-production/systems/SYS-001-movement" shows the text "Blocked"
    And that card shows the text "Waiting on final boss design"
    And the card whose id text is ".gamedev/phases/01-preproduction/gdd.md" does not show the text "Waiting on final boss design"

  # Limit Cases
  Scenario: Limit Case — Inbox expand has no Archive and POST epic is 404
    Given GET /api/game-board?project=bevy inbox includes id ".grill/plans/bevy-funnel/epics/epic-003-juice.md" kind "epic" summary "Juice pass" plan_title "Bevy funnel"
    And the Game tab is showing the board for "bevy"
    When the operator activates the title control on the card whose id text is ".grill/plans/bevy-funnel/epics/epic-003-juice.md"
    Then that card has aria-expanded true
    And that card shows the text "Juice pass"
    And that card shows the text "Bevy funnel"
    And that card does not show a control named "Archive"
    And that card does not show a control named "Unarchive"
    And that card does not show the text "No tasks planned yet"
    When POST /api/game-board is sent with JSON project "bevy" card_id ".grill/plans/bevy-funnel/epics/epic-003-juice.md" archived true
    Then the response status is 404
    And the response body is {"error":"card not found"}

  Scenario: Limit Case — empty Track A tasks still expands
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/02-production/systems/SYS-002-score" track "A" tasks [] blurb "Adds a score counter."
    And the Game tab is showing the board for "bevy"
    When the operator activates the title control on the card whose id text is ".gamedev/phases/02-production/systems/SYS-002-score"
    Then that card has aria-expanded true
    And that card shows the text "Adds a score counter."
    And that card shows the text "No tasks planned yet"

  Scenario: Limit Case — What it does wins over header open
    Given "/tmp/bevy/.gamedev/phases/02-production/systems/SYS-001-movement/spec.md" contains open: "Ignore this open line."
    And that file contains a heading "## What it does" whose body is "Moves the tetromino left and right. DAS applies after the first tap."
    When GET /api/game-board?project=bevy is read for id ".gamedev/phases/02-production/systems/SYS-001-movement"
    Then that entry has blurb "Moves the tetromino left and right. DAS applies after the first tap."
    And that entry has open "Ignore this open line."

  Scenario: Limit Case — missing Inputs heading omits the region
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/02-production/systems/SYS-001-movement" inputs ""
    And the operator has expanded that card
    Then that card does not show the text "Inputs"

  Scenario: Limit Case — playtest recent is the last Round only
    Given "/tmp/bevy/.gamedev/phases/02-production/qa/playtest-log.md" contains "## Round 1 — 2026-08-01 — build 0.1" then a body line "old finding" then "## Round 2 — 2026-08-20 — build 0.4" then a body line "DAS feels sticky"
    When GET /api/game-board?project=bevy is read for id ".gamedev/phases/02-production/qa/playtest-log.md"
    Then that entry recent contains "Round 2"
    And that entry recent contains "DAS feels sticky"
    And that entry recent does not contain "old finding"

  Scenario: Limit Case — level recent is the last 8 changelog lines
    Given "/tmp/bevy/.gamedev/phases/02-production/levels/LVL-001-well/changelog.md" has 10 non-empty non-comment lines where the first line is "line-one" and the last line is "line-ten"
    When GET /api/game-board?project=bevy is read for id ".gamedev/phases/02-production/levels/LVL-001-well"
    Then that entry recent contains "line-ten"
    And that entry recent does not contain "line-one"

  Scenario: Limit Case — Show archived is session-only and remount starts hidden
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/audio-direction.md" work_state "done" archived true
    And the operator had Show archived pressed in a previous mount
    When GameBoardTab mounts for project "bevy"
    Then the control named "Show archived" has aria-pressed false
    And the Pre-production column does not show a card whose id text is ".gamedev/phases/01-preproduction/audio-direction.md"

  Scenario: Limit Case — Unarchive restores the card while the toggle is off
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/audio-direction.md" work_state "done" archived true
    And the Game tab is showing the board for "bevy"
    And the control named "Show archived" has aria-pressed false
    When the operator activates the control named "Show archived"
    Then that control has aria-pressed true
    And the Pre-production column shows a card whose id text is ".gamedev/phases/01-preproduction/audio-direction.md"
    When the operator activates the title control on that card
    And the operator activates the control named "Unarchive"
    Then POST /api/game-board is sent with JSON project "bevy" card_id ".gamedev/phases/01-preproduction/audio-direction.md" archived false
    And that POST response status is 200
    And that POST response JSON has archived false
    When the operator activates the control named "Show archived"
    Then that control has aria-pressed false
    And the Pre-production column still shows a card whose id text is ".gamedev/phases/01-preproduction/audio-direction.md"

  Scenario: Limit Case — two cards stay expanded and continue does not toggle
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/gdd.md" and id ".gamedev/phases/02-production/systems/SYS-001-movement"
    And the Game tab is showing the board for "bevy"
    And the operator has expanded the card whose id text is ".gamedev/phases/01-preproduction/gdd.md"
    When the operator activates the title control on the card whose id text is ".gamedev/phases/02-production/systems/SYS-001-movement"
    Then the card whose id text is ".gamedev/phases/01-preproduction/gdd.md" has aria-expanded true
    And the card whose id text is ".gamedev/phases/02-production/systems/SYS-001-movement" has aria-expanded true
    When the operator activates the continue control on the card whose id text is ".gamedev/phases/01-preproduction/gdd.md"
    Then that card still has aria-expanded true
    And navigator.clipboard.writeText was called with "/gamedev-skill continue @game-designer"

  Scenario: Limit Case — empty blocked omits the strip
    Given GET /api/game-board?project=bevy JSON blocked is []
    When the operator activates the tab named "Game"
    Then the pane does not show a region named "Blocked"

  Scenario: Limit Case — needs_review is not a strip row
    Given "/tmp/bevy/.gamedev/state.md" contains a line content-artist:needs_review:"Enemy sprites batch 2"
    And that file has no line whose status token is blocked
    When GET /api/game-board?project=bevy is read
    Then the JSON blocked array length is 0

  Scenario: Limit Case — leftover archived on pending does not hide
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/gdd.md" work_state "pending" archived true
    When the Game tab paints Pre-production
    Then the Pre-production column shows a card whose id text is ".gamedev/phases/01-preproduction/gdd.md"
    And that card expand does not show a control named "Archive"
    And that card expand does not show a control named "Unarchive"

  Scenario: Limit Case — orphan game_archive row does not invent a card
    Given CBM has a game_archive flag for project "bevy" card_id ".gamedev/phases/01-preproduction/missing-doc.md" archived true
    And GET /api/game-board?project=bevy arrays do not include that id
    When GET /api/game-board?project=bevy is read
    Then the response status is 200
    And no array entry has id ".gamedev/phases/01-preproduction/missing-doc.md"

  Scenario: Limit Case — all artifacts in a column archived leaves header only
    Given every preproduction card has archived true
    And Show archived is not pressed
    When the Game tab paints Pre-production
    Then the pane shows a column named "Pre-production"
    And that column does not show a card
    And the pane shows a control named "Show archived"

  Scenario: Limit Case — repeat archive is idempotent
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/audio-direction.md" work_state "done" archived true
    When POST /api/game-board is sent with JSON project "bevy" card_id ".gamedev/phases/01-preproduction/audio-direction.md" archived true
    Then the response status is 200
    And the response JSON has archived true
    And a following GET /api/game-board?project=bevy entry id ".gamedev/phases/01-preproduction/audio-direction.md" has archived true

  Scenario: Limit Case — 48 task cap omits the 49th
    Given "/tmp/bevy/.gamedev/phases/02-production/systems/SYS-001-movement/tasks.md" contains 49 checkbox task lines
    When GET /api/game-board?project=bevy is read for id ".gamedev/phases/02-production/systems/SYS-001-movement"
    Then that entry tasks length is 48
    And that entry has no has_more field

  # Error Scenarios
  Scenario: Error — archive a pending artifact is 409 and writes nothing
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/01-preproduction/gdd.md" work_state "pending" archived false
    When POST /api/game-board is sent with JSON project "bevy" card_id ".gamedev/phases/01-preproduction/gdd.md" archived true
    Then the response status is 409
    And the response body is {"error":"card not done"}
    And a following GET /api/game-board?project=bevy entry id ".gamedev/phases/01-preproduction/gdd.md" has archived false
    And "/tmp/bevy/.gamedev/phases/01-preproduction/gdd.md" is byte-identical to its contents before the POST

  Scenario: Error — archive a blocked overlay card is 409
    Given GET /api/game-board?project=bevy includes id ".gamedev/phases/02-production/systems/SYS-001-movement" work_state "blocked" archived false
    When POST /api/game-board is sent with JSON project "bevy" card_id ".gamedev/phases/02-production/systems/SYS-001-movement" archived true
    Then the response status is 409
    And the response body is {"error":"card not done"}

  Scenario: Error — unknown card_id is 404
    Given GET /api/game-board?project=bevy arrays do not include id ".gamedev/phases/01-preproduction/nope.md"
    When POST /api/game-board is sent with JSON project "bevy" card_id ".gamedev/phases/01-preproduction/nope.md" archived true
    Then the response status is 404
    And the response body is {"error":"card not found"}

  Scenario: Error — missing project on POST is 400
    When POST /api/game-board is sent with JSON card_id ".gamedev/phases/01-preproduction/audio-direction.md" archived true and no project field
    Then the response status is 400
    And the response JSON has an error field

  Scenario: Error — invalid archived on POST is 400
    When POST /api/game-board is sent with JSON project "bevy" card_id ".gamedev/phases/01-preproduction/audio-direction.md" archived "yes"
    Then the response status is 400
    And the response body is {"error":"invalid archived"}

  Scenario: Error — unknown project on POST is 404
    When POST /api/game-board is sent with JSON project "missing-proj" card_id ".gamedev/phases/01-preproduction/audio-direction.md" archived true
    Then the response status is 404
    And the response body is {"error":"project not found"}

  Scenario: Error — POST spec-board with a Game card_id does not archive
    Given GET /api/spec-board?project=bevy specs do not include id ".gamedev/phases/01-preproduction/audio-direction.md"
    When POST /api/spec-board is sent with JSON project "bevy" spec_id ".gamedev/phases/01-preproduction/audio-direction.md" archived true
    Then the response status is 404
    And the response body is {"error":"spec not found"}
    And a following GET /api/game-board?project=bevy entry id ".gamedev/phases/01-preproduction/audio-direction.md" does not gain archived true from that POST
```

## Success Metrics
| Metric | Target | Current | Status |
| Game cards expand in place | 100% of listed artifact+inbox ids | no expand | not met |
| Done artifact Archive hides on fresh visit | 100% of archived done ids | no archive | not met |
| Skill-file writes from Game GET/POST | 0 | 0 | met (must stay 0) |
| Roadmap graph / Needs edges | 0 | 0 | met (must stay 0) |
| Confirm dialog on Archive | 0 | 0 | met (must stay 0) |
Primary KPI: expand + archive-hide + blocked strip all observable on Game.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + C HTTP + per-project SQLite. GET `/api/game-board` stays the read. POST on that same path is archive-only.
- Constitution I.2: indexer/graph-ui do not write skill cycle files. Archive is CBM `.db` only.
- spec-011 card `id` is the archive key (root-relative path; dirs have no trailing slash).
- Caps: 64 cards/column (unchanged), 48 tasks, 16 blocked rows, 512-byte blurb/inputs/recent.
- Chrome grayscale. GraphTab `colorForLabel` hex stays. Tests may assert English.
- Path = Project 1:1. Unknown project 404 string matches spec-board.
- useGameBoard may stay one-shot; POST 200 must refetch or patch so hide survives.

Business:
- Grill plan add-gamedev-skill epic-003. ADR-004 archive done only. ADR-007 expand gesture = Specs. ADR-010 blocked-by + strip, no roadmap graph.
- Epic 004 ADR trio is out of scope.
- Skill confirmation of done stays in gamedev-skill. CBM archives already-done cards.

Planner defaults A (reject a Gherkin scenario to change these):
1. Ref ticket none. No TD.
2. New `game_archive` table. Do not namespace into `spec_archive`. POST `/api/game-board` `{project, card_id, archived}`. 200 = flag object.
3. Archive eligible = JSON `work_state` `"done"` after blocked overlay. Blocked/inbox/pending/in_progress → 409 or 404 as specified. No confirm.
4. Show archived: pane chrome, session-only, remount unpressed. Empty archived column = header only.
5. Expand Set keyed by `id`. All start collapsed. Continue does not toggle. Multi-open.
6. Track A blurb = `## What it does` first; else `open`; else `last_decision`. tasks.md checkbox dialect, all tasks, cap 48. Inputs = first `## Inputs*` heading.
7. Track B header only. LVL last 8 changelog lines. playtest last `## Round ` block. Inbox no Archive.
8. Blocked strip = `blocked` lines only, above columns, no click behavior. Overlay all artifact cards of that `@owner`.
9. Zero skill writes. No MCP. spec-board POST stays specs-only.

## Out of Scope
- ADR fill from the gamedev trio (epic 004)
- Writing `.gamedev/` or invoking gamedev-skill from CBM
- Drag / mark-done / launcher / toast
- Roadmap.md graph, agents.md Needs as edges, click-strip scroll/highlight
- Reusing `spec_archive` or POST `/api/spec-board` for Game ids
- MCP game-board / archive tool / GET `/api/skill-presence` from graph-ui
- Emitting `gamedev_skill_present` on spec-board
- Changing silent win, Enter→Graph, conversion, four-column layout, or Specs on non-gamedev paths
- Persisted show-archived preference, confirm dialog, fourth "Archived" column
- Dumping full GDD/spec/review into expand
- Overlay of `needs_review` / `in_progress` / `done` agent lines onto header work-state (blocked only)

## Acceptance Checklist
- [ ] all US implemented [ ] all AC met [ ] ALL Gherkin scenarios pass [ ] tests>80% [ ] review approved [ ] human docs confirmed [ ] zero skill writes [ ] security passed [ ] manual test by owner done

## Architecture Considerations
- Widen `cbm_game_board_card_t` + `cbm_game_board_to_json` for expand fields + `archived`; board-level `blocked[]`
- `game_archive` in the project `.db` with set/load/copy (pipeline copy like `spec_archive`)
- HTTP: GET merge after `cbm_game_board_read`; POST validate listed+done, persist, 200 flag object
- Parse: Track A spec.md / tasks.md; level changelog; playtest last Round; state.md blocked lines (fopen rb)
- graph-ui: title control + expanded Set; Archive/Unarchive; Show archived; blocked strip; refetch after POST
- i18n en+zh; Vitest expand/archive/strip; C tests GET fields, overlay, POST 200/400/404/409, zero skill writes
- spec-011 "no POST / no expand" tests invert

## Questions for Architect (answered in plan.md)
- `game_archive` schema (card_id TEXT PK) vs extra columns; copy on pipeline/index
- Whether blocked overlay mutates `work_state` in C before JSON (planner default) or UI-only
- useGameBoard: keep one-shot plus explicit refetch after POST vs introduce a poll
- Reuse spec-005 blurb sentence helper vs game_board-local extract (no shared JSON with spec_board)
- Buffer sizes for `last_decision` / `open` / `recent` if 512 is tight on playtest Round bodies
- i18n: reuse `specBoard.archive` strings vs `gameBoard.*` copies

## Questions for @implementer
- [ ] Map every Gherkin scenario to a C test and/or graph-ui Vitest
- [ ] Do not write `.gamedev/`, `.sdd-skill/`, or `.grill/`
- [ ] Do not POST `/api/spec-board` for Game ids; do not reuse `spec_archive`
- [ ] Do not parse roadmap.md into a graph or paint Needs edges
- [ ] Do not auto-expand any card on first paint
- [ ] Invert spec-011 tests that assert no expand / no Archive / no POST
- [ ] Keep silent win, four columns, conversion, clipboard, no drag, Enter→Graph

## Related Specs
Depends on: spec-011-q5n-game-phase-board (cards, ids, columns, GET)
Companion to: .grill/plans/add-gamedev-skill/epics/epic-003-expand-archive-deps.md
Reuses: spec-005 in-card expand + 512 blurb; spec-006 CBM archive + session toggle + no confirm
Supersedes: spec-011 "no expand / no archive / no POST `/api/game-board`"
Does not include: epic 004 ADR trio

## Revision History
| Date | Author | Change |
| 2026-08-31 | @planner | draft from grill epic-003 + ADR-004/007/010; defaults A; debt=no; ticket=none |
| 2026-08-31 | @planner | Gherkin approved by user; status approved; handoff @architect |
| 2026-08-31 | @architect | Plan approved by user; start development |
| 2026-08-31 | @planner | Constitution changes confirmed. Status completed. Spec closed. |

## Approval Sign-off
Spec Owner(@planner): closed / Gherkin scenarios(user): approved / Product Owner: constitution confirmed 2026-08-31 / Architecture(@architect): approved 2026-08-31
