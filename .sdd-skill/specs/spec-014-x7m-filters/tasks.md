# Work Breakdown — Spec-014: Game visibility filters
Total Tasks: 2 / Estimated Total Effort: 5.5h

## Task Dependency Graph
```
#1 i18n + chrome + predicate + invert first-paint/archive
  └──► #2 Track / AND leftover Gherkin + Specs lock + no-persist
```
Critical path: #1 (3h) → #2 (2.5h) = 5.5h. No parallel branch.

## Tasks

### Task #1 — i18n, chrome cluster, predicate, invert first-paint dones
Definition of Done:
- [x] `messages.en.gameBoard.showDones` === `"Show Dones"`; `trackA` === `"Track A"`; `trackB` === `"Track B"`; `trackAll` === `"All"`
- [x] zh: 显示已完成 / 轨道 A / 轨道 B / 全部. `i18n.test.ts` locks en+zh
- [x] Keep `t.specBoard.showArchived` for the existing Game toggle. Do not add `gameBoard.showArchived`
- [x] `visiblePhaseCards(cards, showArchived, showDones, track)` per plan.md predicate. Inbox still `board.inbox`
- [x] Chrome row: Show archived, `|`, Show Dones, `|`, Track A, Track B, All. Separators are `<span aria-hidden="true">|</span>`. Not Inbox header. Not a fifth column
- [x] All five controls are buttons with `aria-pressed`. First paint: Show Dones false, Show archived false, All true, Track A/B false
- [x] Track clicks set exclusive filter (`"A"` / `"B"` / `"all"`). Show Dones toggles
- [x] `lastProjectRef` reset also clears `showDones` and `trackFilter` to defaults (with existing expand + showArchived)
- [x] Invert (or the file is red): `paints Done and Blocked work-state labels` must click Show Dones before asserting SYS-003-done
- [x] Invert: `Archive hides a done artifact without a confirm dialog` must press Show Dones so the unarchived done card exists, then Archive; card hides with Show archived off (AND)
- [x] Invert: any other Task #1-touched assertion that expects an unarchived done card on first paint
- [x] Gherkin rows below have Vitest Thens
- [x] Do not edit SpecBoardTab.tsx, useGameBoard.ts, C, types.ts. Track/AND leftover Gherkin may wait for Task #2 if listed there
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-003 (chrome + default All), US-004 (reset half)
Gherkin covered:
- First paint hides done artifacts and selects All
- Show Dones reveals an unarchived done card
- Limit Case — leftover archived pending stays visible with Show Dones off
- Limit Case — remount resets all three controls (Show Dones / archived / All)
- Error — Show Dones alone does not reveal an archived done card
Dependencies: None
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-062, SDD-ADR-063, SDD-ADR-064. Same `text-[10px]` pressed/unpressed tokens as Show archived. Do not introduce radiogroup. `App.test.tsx` filledBevyBoard gdd is pending — should stay green without invert. Commit held unless user asks.

### Task #2 — Track filter, AND leftover, Specs lock, no-persist
Definition of Done:
- [x] Track A shows only `track==="A"`. Track B only `"B"`. All shows A, B, H, other, null (then dones/archived)
- [x] Archived done shown iff Show Dones AND Show archived (then Track)
- [x] Invert: `Limit — Show archived is session-only and remount starts hidden` — Show archived alone must NOT reveal the archived done card; reveal requires both toggles; remount still starts both unpressed
- [x] Invert: `Limit — Unarchive restores the card while the toggle is off` — open the card with both toggles; after Unarchive, done+unarchived stays visible only while Show Dones is on (spec-012 "toggle off still shows" is now Show Dones on + Show archived off)
- [x] Invert: `project change resets Show archived; GET refetch does not` — refetch keeps Show Dones + Track + Show archived; project change resets all three; archived done not shown after reset
- [x] Inbox never hidden by Show Dones / Show archived / Track
- [x] All-done (or all-filtered) phase column = header only; Show Dones remains in chrome; no "No specs yet"
- [x] Filter activation does not send GET or POST `/api/game-board`. No query params `track` / `show_dones` / `show_archived`
- [x] `localStorage` has no key whose name contains `showDones` or `gameTrack` after activations
- [x] SpecBoardTab mount: document has no control named "Show Dones", "Track A", or "All". Do not edit SpecBoardTab.tsx
- [x] Inbox column has none of those controls
- [x] Every remaining spec-014 Gherkin Then has a Vitest
- [x] spec-010/011 silent-win / Enter Graph / leftover `tab=specs`→game / no `/api/skill-presence` stay green
- [x] `colorForLabel("Function") === "#06b6d4"` still locked if that test is in the suite run
- [x] tests pass; breadcrumbs
User Stories Addressed: US-002, US-003, US-004
Gherkin covered:
- Track A hides B and H
- Both toggles on reveal an archived done card
- Limit Case — Inbox stays visible under every filter
- Limit Case — Track B then All restores H
- Limit Case — project change resets filters; GET refetch does not
- Limit Case — all phase cards hidden leaves header only
- Limit Case — chrome has separators and Specs does not copy the controls
- Error — Show archived alone does not reveal an archived done card
- Error — Track A plus Show Dones still hides a Track B done card
- Error — filters do not add query params or write skill trees (UI: fetch spy + localStorage; no C fopen)
Dependencies: Task #1
Estimated Effort: 2.5h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-062, SDD-ADR-063. Skill-file byte Then is N/A in Vitest (no C this spec) — fetch + localStorage satisfy the UI half. Do not add Playwright unless @tester later requires CERTIFICATION. Do not weaken silent win.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | i18n + chrome + predicate + invert first-paint | 3h | None | done |
| 2 | Track / AND leftover + Specs lock + no-persist | 2.5h | #1 | done |
Total: 5.5h

## Critical Path
#1 → #2 (5.5h). No parallelization.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

Do not add GET query params or C changes. Do not filter Inbox. Do not add Show Dones / Track to SpecBoardTab.

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched GameBoardTab chrome/filter. Vitest owns all scenarios. C owns nothing this spec.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
