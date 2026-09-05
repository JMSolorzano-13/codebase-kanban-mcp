# Work Breakdown — Spec-009: Specs tab grill presence
Total Tasks: 3 / Estimated Total Effort: 6h

## Task Dependency Graph
```
#1 hook predicate (sdd OR grill) ──► #2 SpecBoardTab host gate
                                  └──► #3 App strip / deep-link / Enter Vitest
```
#2 and #3 may start in parallel after #1. Critical path if sequential: #1 (1.5h) → #2 (2h) → #3 (2.5h) = 6h. If parallel after #1: #1 + max(#2,#3) = 4h.

## Tasks

### Task #1 — Presence predicate sdd OR grill
Definition of Done:
- [x] `useSddSkillPresent` export, file path, and `UseSddSkillPresentResult` unchanged
- [x] `bodyHasSkill`: `sdd_skill_present === true || grill_skill_present === true`; missing `grill_skill_present` is false; non-boolean truthy is false
- [x] Still one-shot GET `/api/spec-board?project=`; `setPresent(false)` on project change / hang / non-200 / throw
- [x] Zero `setInterval(4000)`; zero `/api/skill-presence`
- [x] `useSddSkillPresent.test.ts`: invert or replace "present true only when sdd === true" — not left green against the old Then
- [x] Hook tests: 200 grill-only → present true; 200 sdd-only → present true; 200 both true → present true; 200 both false → present false; 200 sdd false + missing grill → present false
- [x] Hook tests: in flight / 404 / 500 / network throw stay present false (existing coverage kept or extended)
- [x] Do not edit `useSpecBoard`, `route.ts`, C, HTTP, MCP
- [x] tests pass locally; breadcrumbs
User Stories Addressed: US-001 (predicate), US-004 (neither / sdd-only kernel), US-005 (same GET)
Gherkin covered:
- Limit — sdd-only without grill still shows Specs (present true)
- Error — GET 404 omits Specs (present false)
- Error — request still in flight omits Specs (present false)
Dependencies: None
Estimated Effort: 1.5h
Subagent: no
Path: compact
Implementation Notes: SDD-ADR-039. Comments / `@human-debug` must say sdd OR grill. Do not rename. App wiring already consumes `present` — do not change App this task unless a type forces it (it should not).

### Task #2 — SpecBoardTab host Kanban on grill-only
Definition of Done:
- [ ] Host paints Kanban when `board.sdd_skill_present === true || board.grill_skill_present === true`
- [ ] Grill-only (`grill` true, `sdd` false) does not show `notSddSkill` copy
- [ ] Last-resort: `!board` after load OR both flags false → existing `notSddSkill` copy (no new i18n key)
- [ ] Loading (`loading && !board`) still shows `t.common.loading`, not `notSddSkill`
- [ ] Grill-only + epic in `epics[]` + `specs: []`: Todo shows E + title + id; In progress / Done show `noSpecs`; those columns have no epic id
- [ ] Grill-only + `epics: []` + `specs: []`: all three columns `noSpecs`; pane not `notSddSkill`
- [ ] Invert or remove spec-008 its: "no sdd-skill" with grill true → notSddSkill; "sdd false even if grill true" — not left green against the old Then
- [ ] EpicCard / expand / archive / Todo order / cap / `useSpecBoard` / `formatIndexedAt` / `colors.ts` not edited
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-002
Gherkin covered:
- Grill-only Kanban paints an epic in Todo and empty spec columns
- Limit — grill directory with no epics still shows Specs (pane Thens; strip Then owned by Task #3 grill-only tab)
Dependencies: Task #1
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-040. Keep EpicCard in SpecBoardTab.tsx. Do not add a host poll. Direct `useSpecBoard` mocks; missing `grill_skill_present` on a sdd-true board still paints (sdd OR). Both-false last-resort test required (stale mount).

### Task #3 — App strip, deep-link, Enter, omit Gherkin
Definition of Done:
- [x] `mockAppFetch` can return independent `sdd_skill_present` / `grill_skill_present` and optional `epics` (default neither still omits Specs)
- [x] Grill-only workspace: tab "Specs" shown; accessible name "Specs"; order Graph then Specs then ADR
- [x] `?tab=specs&project=alpha` + grill-only 200 → URL keeps `tab=specs`; SpecBoardTab shown; not `tab=graph`
- [x] sdd+grill 200 → Specs shown
- [x] neither-skill 200 → Specs omitted; order Graph then ADR; `?tab=specs` rewrites to `tab=graph`; GraphTab shown
- [x] gamedev-only UI: mock 200 with both flags false and no `gamedev_skill_present` key → Specs omitted (do not add C)
- [x] Enter on grill-only: `tab=graph` + GraphTab + Specs tab still shown
- [x] GET 500 omits Specs; GraphTab usable (existing test keep green)
- [x] Did not add C/HTTP/MCP; existing spec-008 GET byte-identical test left in place (Gherkin zero-write owner)
- [x] Do not change Enter default, tab label key, WorkspaceTabStrip product, `fallbackSpecsToGraph` signature
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-003, US-004, US-005
Gherkin covered:
- Grill-only project shows the Specs tab
- Deep-link tab=specs stays on Specs when grill-only
- sdd plus grill still shows Specs
- Limit — neither skill omits Specs and deep-link falls back to Graph
- Limit — gamedev directory alone does not show Specs
- Limit — Enter still opens Graph on a grill-only project
- Error — GET 500 omits Specs
- Error — GET does not write skill trees (existing C; no new C)
Dependencies: Task #1
Estimated Effort: 2.5h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-041. App.tsx product path likely unchanged (`present` already drives strip + fallback). Extend mocks; add Gherkin its. Keep "rewrites specs deep link without sdd-skill" as neither-skill (both false). Do not boot GraphTab/Three (keep GraphTab mock). Do not add Playwright unless @tester later requires CERTIFICATION.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | Presence predicate sdd OR grill | 1.5h | None | done |
| 2 | SpecBoardTab host Kanban on grill-only | 2h | #1 | pending |
| 3 | App strip, deep-link, Enter, omit Gherkin | 2.5h | #1 | done |
Total: 6h

## Critical Path
#1 → #3 (4h if #2 parallel; 6h sequential). #2 is the Kanban gate; #3 is the strip/URL contract.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `useSddSkillPresent` + `SpecBoardTab` host + `App` strip. Vitest owns all UI Thens. C owns GET zero-write / no `gamedev_skill_present` already (spec-008); do not add C.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
