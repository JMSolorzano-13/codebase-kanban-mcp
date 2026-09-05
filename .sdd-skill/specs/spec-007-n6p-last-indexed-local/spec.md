# Spec-007-n6p: Last indexed local
Status: closed | Spec ID: spec-007-n6p-last-indexed-local
Ref Ticket: none | KPI: Last indexed on Dashboard, workspace header, and AdrTab generated stamp shows the browser local wall clock of Project.indexed_at; stored ISO and time[dateTime] stay the raw instant | Priority: P0
Created: 2026-08-30 | Tech Debt Ref: none

## Executive Summary
`formatIndexedAt` pins `timeZone: "UTC"` (SDD-ADR-003). The operator reads freshness against UTC, not the machine clock. Grill ADR-005 reverses that display contract on the three surfaces that already call the helper: Dashboard (list rows and conflict members), WorkspaceHeader, and AdrTab generated stamp.

This spec changes display only. `indexed_at` stays ISO Z in `list_projects` / `Project`. `<time dateTime>` and `title` stay the raw ISO. Conflict "newest" still compares ISO strings. No second freshness field, no C/HTTP change.

Business impact: the operator judges index age against the same clock as the laptop, and can see a short timezone name so the stamp is not mistaken for UTC.

## User Stories

### US-001: Helper uses the runtime timezone
As an operator, I want `formatIndexedAt` to format a valid ISO instant in the browser timezone, So that every surface that already calls the helper shows local wall clock without a second formatter.
Acceptance Criteria:
- [ ] A valid ISO is formatted with `Intl.DateTimeFormat` locale `en-US` (lang en) or `zh-CN` (lang zh)
- [ ] Options stay year/month/day/hour/minute plus `timeZoneName: "short"`
- [ ] Options do not include `timeZone: "UTC"` (or any other explicit IANA zone). The runtime default timezone is used
- [ ] Invalid or empty input still returns the raw string (spec-001 fallback unchanged)

### US-002: Dashboard last-indexed is local
As an operator, I want Dashboard list rows and conflict-group members to show the local last-indexed string, So that I pick a project against the machine clock.
Acceptance Criteria:
- [ ] Each list row `<time>` has `dateTime` equal to that project's `indexed_at` ISO
- [ ] That `<time>` visible text equals `formatIndexedAt(indexed_at, lang)` after this spec
- [ ] Conflict-group member rows use the same helper and the same `<time dateTime>` contract
- [ ] i18n labels stay "Last indexed" / existing zh. No new "UTC" or "local" suffix in copy

### US-003: Workspace header last-indexed is local
As an operator, I want the workspace header datetime to match Dashboard, So that freshness is the same clock after Enter.
Acceptance Criteria:
- [ ] When `useProjects` has a matching name, header `<time dateTime={indexed_at}>` visible text equals `formatIndexedAt(indexed_at, lang)`
- [ ] Ghost name (not in the list) still omits `<time>` (spec-002 unchanged)
- [ ] Header still does not call `/api/index-status` or invent a second freshness field

### US-004: AdrTab generated stamp is local
As an operator, I want the AdrTab "Generated at" stamp to use the same local helper, So that generated vs last-indexed do not disagree on timezone.
Acceptance Criteria:
- [ ] When GET content includes `CBM-GENERATED-START` and the project is in the list, the stamp `<time dateTime={indexed_at}>` visible text equals `formatIndexedAt(indexed_at, lang)`
- [ ] Stamp is still omitted when markers are absent or the name is missing from the list
- [ ] The ISO is not written into the ADR textarea content

### US-005: Instant, storage, and newest stay ISO
As an operator, I want the stored instant and conflict newest to stay ISO-based, So that grouping and hover do not depend on the display timezone.
Acceptance Criteria:
- [ ] `list_projects` / `Project.indexed_at` values are unchanged by this spec (still ISO Z from the daemon)
- [ ] `<time title>` remains the raw `indexed_at` string
- [ ] Conflict newest still compares `indexed_at` ISO strings (then name). No UI copy that says "local vs stored"
- [ ] No new HTTP or MCP field. No C change unless architect proves the UI cannot meet AC without it

### US-006: Host UTC and bad ISO stay honest
As an operator, I want a UTC-hosted browser and a bad ISO to stay readable, So that CI and corrupt list rows do not invent a clock.
Acceptance Criteria:
- [ ] If the runtime timezone is UTC, the visible string may equal the old UTC-pinned format; `dateTime` is still the raw ISO
- [ ] `formatIndexedAt("not-a-date", "en")` is `"not-a-date"`
- [ ] `formatIndexedAt("", "zh")` is `""`
- [ ] Graph hex, Specs expand/archive, Path 1:1, and ADR fill stay unchanged

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: Last indexed local

  # Happy Paths
  Scenario: Helper matches runtime-local Intl and not the UTC pin when those differ
    Given iso is "2026-08-29T10:00:00Z"
    And localFmt is Intl.DateTimeFormat("en-US", {year,month,day,hour,minute,timeZoneName:"short"}).format(that instant)
    And utcFmt is Intl.DateTimeFormat("en-US", {year,month,day,hour,minute,timeZone:"UTC",timeZoneName:"short"}).format(that instant)
    When formatIndexedAt(iso, "en") is called
    Then the return value equals localFmt
    And the return value is not identical to iso
    And if localFmt is not identical to utcFmt, the return value is not identical to utcFmt

  Scenario: Helper zh locale is the same instant in zh-CN local
    Given iso is "2026-08-29T10:00:00Z"
    When formatIndexedAt(iso, "zh") is called
    Then the return value equals Intl.DateTimeFormat("zh-CN", {year,month,day,hour,minute,timeZoneName:"short"}).format(that instant)
    And the return value is not identical to iso

  Scenario: Dashboard row time is local text and raw ISO dateTime
    Given list_projects returns name "alpha" root_path "/tmp/alpha" indexed_at "2026-08-29T10:00:00Z"
    When the operator opens Dashboard
    Then a time element has dateTime "2026-08-29T10:00:00Z"
    And that time title is "2026-08-29T10:00:00Z"
    And that time text content equals formatIndexedAt("2026-08-29T10:00:00Z", "en")
    And the document contains "Last indexed"

  Scenario: Dashboard conflict member uses the same helper
    Given list_projects returns two projects with the same canonical_root "/tmp/alpha"
      | name       | indexed_at           |
      | alpha-old  | 2026-08-28T10:00:00Z |
      | alpha      | 2026-08-29T10:00:00Z |
    When the operator opens Dashboard
    Then newest for that group is still name "alpha" (ISO compare unchanged)
    And a time element has dateTime "2026-08-29T10:00:00Z" and text formatIndexedAt("2026-08-29T10:00:00Z", "en")
    And a time element has dateTime "2026-08-28T10:00:00Z" and text formatIndexedAt("2026-08-28T10:00:00Z", "en")

  Scenario: Workspace header last-indexed is local
    Given list_projects returns name "alpha" indexed_at "2026-08-29T10:00:00Z"
    When the operator opens "?tab=graph&project=alpha"
    Then the workspace header contains a time with dateTime "2026-08-29T10:00:00Z"
    And that time text content equals formatIndexedAt("2026-08-29T10:00:00Z", "en")

  Scenario: AdrTab generated stamp is local
    Given list_projects returns name "alpha" indexed_at "2026-08-30T12:00:00Z"
    And GET /api/adr?project=alpha content includes "CBM-GENERATED-START"
    When the operator opens "?tab=adr&project=alpha"
    Then a time element has dateTime "2026-08-30T12:00:00Z"
    And that time text content equals formatIndexedAt("2026-08-30T12:00:00Z", "en")
    And the textarea value does not include "2026-08-30T12:00:00Z"

  # Limit Cases
  Scenario: Limit Case — invalid ISO returns the raw string
    When formatIndexedAt("not-a-date", "en") is called
    Then the return value is "not-a-date"
    When formatIndexedAt("", "zh") is called
    Then the return value is ""

  Scenario: Limit Case — runtime timezone is UTC
    Given the runtime timezone formats "2026-08-29T10:00:00Z" identically with and without timeZone "UTC"
    When formatIndexedAt("2026-08-29T10:00:00Z", "en") is called
    Then the return value equals that shared Intl string
    And a Dashboard time for that project still has dateTime "2026-08-29T10:00:00Z"

  Scenario: Limit Case — ghost workspace omits time
    Given list_projects does not include name "ghost"
    When the operator opens "?tab=graph&project=ghost"
    Then the workspace header shows "ghost"
    And the header contains zero time elements

  Scenario: Limit Case — AdrTab without generated markers omits the stamp
    Given list_projects returns name "alpha" indexed_at "2026-08-30T12:00:00Z"
    And GET /api/adr?project=alpha content does not include "CBM-GENERATED-START"
    When the operator opens "?tab=adr&project=alpha"
    Then the document does not contain "Generated at"
    And the ADR tab contains zero time elements

  # Error Scenarios
  Scenario: Error — Dashboard invalid indexed_at stays visible as raw
    Given list_projects returns name "alpha" root_path "/tmp/alpha" indexed_at "not-a-date"
    When the operator opens Dashboard
    Then a time element has dateTime "not-a-date"
    And that time text content is "not-a-date"

  Scenario: Error — workspace header invalid indexed_at stays visible as raw
    Given list_projects returns name "alpha" indexed_at "not-a-date"
    When the operator opens "?tab=graph&project=alpha"
    Then the workspace header contains a time with dateTime "not-a-date"
    And that time text content is "not-a-date"

  Scenario: Error — AdrTab invalid indexed_at with markers stays visible as raw
    Given list_projects returns name "alpha" indexed_at "not-a-date"
    And GET /api/adr?project=alpha content includes "CBM-GENERATED-START"
    When the operator opens "?tab=adr&project=alpha"
    Then a time element has dateTime "not-a-date"
    And that time text content is "not-a-date"
    And the textarea value does not invent a formatted datetime
```

## Success Metrics
| Metric | Target | Current | Status |
| Surfaces using helper show local Intl (no UTC pin) | 3 of 3 (Dashboard, header, AdrTab stamp) | UTC pin | not met |
| Stored indexed_at / dateTime remain raw ISO | 100% | raw ISO | met (must stay) |
| Newest still ISO-compared | 100% | ISO strcmp | met (must stay) |
| New freshness field or C API | 0 | 0 | met (must stay 0) |
Primary KPI: first metric plus unchanged ISO storage.

## Constraints & Assumptions
Technical:
- Stack stays graph-ui React 19 + existing `formatIndexedAt` + `<time dateTime>`. Constitution IV.4: display `indexed_at`; do not invent a second field.
- Locale still follows `useUiLanguage` (en-US / zh-CN). Timezone is not selected by lang.
- Tests may assert English copy. Helper assertions compare to `Intl.DateTimeFormat` oracles (local vs UTC pin), not a hardcoded wall-clock string.
- Chrome grayscale and GraphTab `colorForLabel` hex stay unchanged.
- No C/HTTP/MCP change unless architect proves AC cannot be met in UI.

Business:
- Grill epic-003 only. Expand (spec-005) and archive (spec-006) stay.
- Reverses SDD-ADR-003 display TZ only. Instant contract (`dateTime`, title, storage) stays.

Planner defaults (reject a Gherkin scenario to change these):
1. Drop `timeZone: "UTC"` only. Keep `timeZoneName: "short"` so a local abbreviation is visible.
2. Do not add i18n that says "UTC" or "local". Existing "Last indexed" / "Generated at" stay.
3. `title` stays raw ISO (may end in Z). That is the instant, not a display-TZ label.
4. Conflict newest stays ISO string compare. No new copy about local vs stored.
5. One helper. Do not split Dashboard / header / AdrTab onto different clocks.
6. TZ-stable tests: oracle = Intl without `timeZone` key. Fail the UTC pin when localFmt !== utcFmt. Do not require `process.env.TZ` unless architect chooses it as extra.
7. Host TZ UTC is allowed: localFmt may equal utcFmt.
8. No daemon timezone. Browser runtime only.

## Out of Scope
- Changing stored `indexed_at`, `list_projects`, or adding `canonical` display TZ on the daemon
- Recolor or redesign Graph 3D
- Specs expand / archive behavior
- Path 1:1, ADR fill, watcher
- New HTTP/MCP endpoints
- Persisted timezone preference or a timezone picker
- Replacing `<time dateTime>` with a relative "5 minutes ago" string

## Acceptance Checklist
- [x] all US implemented [x] all AC met [x] ALL Gherkin scenarios pass [x] tests>80% on touched graph-ui (reporter may be absent) [x] review approved [x] human docs confirmed [x] ISO storage unchanged [x] security passed [ ] manual test by owner done

## Architecture Considerations
- Likely UI-only: `formatIndexedAt.ts` + tests; Dashboard / WorkspaceHeader / AdrTab already call the helper
- SDD-ADR-003 display half is superseded; new SDD-ADR for local TZ + keep dateTime/title ISO
- QUICK-DEBUG today tells humans to force UTC — human-trainer must flip that after implementation
- Vitest: helper oracles + existing surface tests that already compare `textContent` to `formatIndexedAt`

## Questions for Architect (answered in plan.md)
- Whether any surface bypasses the helper and must be rewired (expected: no)
- Whether to add an explicit `TZ=` in CI as a second assertion, or rely only on localFmt vs utcFmt oracles
- ADR id for superseding SDD-ADR-003 display TZ
- Confirm zero C/HTTP files

## Questions for @implementer
- [ ] Map every Gherkin scenario to a Vitest
- [ ] Do not pin `timeZone: "UTC"` in the helper
- [ ] Do not change `dateTime` / `title` off the raw ISO
- [ ] Do not change conflict newest compare
- [ ] Do not change `colorForLabel` hex
- [ ] Do not add C/HTTP unless plan.md says so

## Related Specs
Supersedes display TZ of: spec-001-w3q-executive-dashboard (SDD-ADR-003)
Companion to: spec-002-p8w-project-workspace (header), spec-003-h7q-path-project-identity (conflict rows), spec-004-j8k-adr-parse-on-reindex (AdrTab stamp)
Companion to: .grill/plans/spec-board-detail/epics/epic-003-last-indexed-local.md
Does not include: grill epic-001 expand, epic-002 archive

## Revision History
| Date | Author | Change |
| 2026-08-30 | @planner | draft from grill epic-003 + ADR-005; debt=no |
| 2026-08-30 | @planner | Gherkin approved; status approved; handoff @architect |
| 2026-08-30 | @architect | plan.md + tasks.md + checklist.md; SDD-ADR-034; Architecture plan ready |
| 2026-08-30 | @planner | CLOSED. KPI met. Constitution: 1 MODIFIED (IX.2). |

## Approval Sign-off
Spec Owner(@planner): closed 2026-08-30 / Gherkin scenarios(user): approved / Product Owner: closeprep + constitution confirmed / Architecture(@architect): plan approved
