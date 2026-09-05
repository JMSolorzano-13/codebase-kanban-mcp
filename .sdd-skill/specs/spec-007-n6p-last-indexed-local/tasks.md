# Work Breakdown — Spec-007: Last indexed local
Total Tasks: 2 / Estimated Total Effort: 3h

## Task Dependency Graph
```
#1 helper drop UTC pin + Intl oracles ──► #2 surface Gherkin (dateTime/title/helper text)
```
Critical path: #1 (1h) → #2 (2h) = 3h. No parallel branch (surfaces already call the helper; tests must see the new clock).

## Tasks

### Task #1 — Drop UTC pin in formatIndexedAt + helper oracles
Definition of Done:
- [x] `formatIndexedAt` options keep year/month/day/hour/minute and `timeZoneName: "short"`
- [x] Options do not include `timeZone: "UTC"` or any other explicit IANA zone
- [x] Invalid / empty input still returns the raw string
- [x] `formatIndexedAt.test.ts` uses Intl oracles (`localFmt` without `timeZone`, `utcFmt` with `timeZone:"UTC"`), not a hardcoded wall-clock string
- [x] en: return equals `localFmt`; not identical to the ISO; if `localFmt !== utcFmt`, return is not `utcFmt`
- [x] zh: return equals `Intl.DateTimeFormat("zh-CN", {same parts, no timeZone}).format(instant)`; not identical to the ISO
- [x] `formatIndexedAt("not-a-date", "en")` is `"not-a-date"`; `formatIndexedAt("", "zh")` is `""`
- [x] Host-UTC Limit Case: when `localFmt === utcFmt`, return equals that shared string (no fail)
- [x] No `process.env.TZ` requirement; no second exported formatter
- [x] tests pass locally; breadcrumbs (SDD-ADR-034)
User Stories Addressed: US-001, US-006 (helper half)
Gherkin covered:
- Helper matches runtime-local Intl and not the UTC pin when those differ
- Helper zh locale is the same instant in zh-CN local
- Limit Case — invalid ISO returns the raw string
- Limit Case — runtime timezone is UTC (helper Then; Dashboard `dateTime` is Task #2)
Dependencies: None
Estimated Effort: 1h
Subagent: no
Path: full
Implementation Notes: Rename `UTC_PARTS` if it still says UTC. Do not edit Dashboard / header / AdrTab in this task. Do not add C/HTTP. Do not add i18n. Compact-path fails here: helper Gherkin slice is 4 scenarios (>3).

### Task #2 — Surface Gherkin: local text, raw ISO dateTime/title
Definition of Done:
- [x] Dashboard list row `<time>`: `dateTime` and `title` equal that project's `indexed_at`; `textContent` equals `formatIndexedAt(indexed_at, "en")`; document contains "Last indexed"
- [x] Dashboard conflict: newest Enter target is still name `"alpha"` (ISO compare unchanged); both member `<time>` elements have raw ISO `dateTime` and text `formatIndexedAt(...)`
- [x] Workspace header listed name: `<time dateTime={indexed_at}>` text equals `formatIndexedAt`; ghost name still omits every `time`
- [x] AdrTab with markers + listed name: stamp `<time dateTime>` text equals `formatIndexedAt`; textarea does not include the ISO
- [x] AdrTab without `CBM-GENERATED-START` still omits "Generated at" and has zero `time` elements
- [x] Invalid `indexed_at` `"not-a-date"`: Dashboard, header, and AdrTab-with-markers show `<time dateTime="not-a-date">` whose text is `"not-a-date"`; AdrTab textarea does not invent a formatted datetime
- [x] Limit host UTC: a Dashboard `<time>` for a valid ISO still has `dateTime` equal to that raw ISO (independent of host TZ)
- [x] No new i18n; no C/HTTP; `pathGroups.ts` / `colors.ts` / Specs / fill untouched
- [x] `colorForLabel("Function") === "#06b6d4"` still locked if that test is in the suite run
- [x] tests pass; breadcrumbs
User Stories Addressed: US-002, US-003, US-004, US-005, US-006 (surface half)
Gherkin covered:
- Dashboard row time is local text and raw ISO dateTime
- Dashboard conflict member uses the same helper
- Workspace header last-indexed is local
- AdrTab generated stamp is local
- Limit Case — runtime timezone is UTC (Dashboard `dateTime` Then)
- Limit Case — ghost workspace omits time
- Limit Case — AdrTab without generated markers omits the stamp
- Error — Dashboard invalid indexed_at stays visible as raw
- Error — workspace header invalid indexed_at stays visible as raw
- Error — AdrTab invalid indexed_at with markers stays visible as raw
Dependencies: Task #1
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Surfaces already call the helper — prefer test-only diffs. Tighten existing year/day/`not.toBe(iso)` asserts to helper equality + `title`. Reuse existing ghost / no-marker tests. Do not boot GraphTab/Three. Do not add Playwright unless @tester later requires CERTIFICATION.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | Drop UTC pin + helper oracles | 1h | None | done |
| 2 | Surface Gherkin dateTime/title/helper | 2h | #1 | done |
Total: 3h

## Critical Path
#1 → #2 (3h). No parallelization.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `formatIndexedAt.ts`. Helper owns oracles. Vitest owns all 13 scenarios.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
