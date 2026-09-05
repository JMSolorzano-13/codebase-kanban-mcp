epic: 003
plan: spec-board-detail
name: last-indexed-local
status: detailed

summary: Last indexed datetime displays in the browser local timezone on Dashboard, workspace header, and AdrTab generated stamp.
delivery-rationale: Operator reads freshness against the machine clock instead of UTC.

tech:
  facts:
    - formatIndexedAt pins Intl timeZone:"UTC" + timeZoneName:short (SDD-ADR-003)
    - Call sites: Dashboard.tsx, WorkspaceHeader.tsx, AdrTab.tsx; tests in formatIndexedAt.test.ts do not assert UTC vs local wall clock
    - indexed_at stored ISO Z; <time dateTime={iso}> already the raw instant
    - i18n lang selects en-US vs zh-CN locale, not timezone
  open-qs:
    1. Drop timeZone:UTC only, or also drop/keep timeZoneName so local abbreviation shows?
    2. Tests currently check year/day presence — need a TZ-stable assertion strategy (mock TZ vs check not-UTC-suffix)?
    3. Any copy that says UTC / "Z" beside the helper that must change?

product:
  facts:
    - ADR-005: all three surfaces, one helper, storage unchanged
    - User: machine where the UI runs = browser TZ
  open-qs:
    1. Show a short TZ name (CST, CDMX) so operators can tell it is not UTC, or time only?
    2. Conflict-group "newest" still compares ISO strings (unchanged) — any UI copy that must say local vs stored?

deps: [none — independent of 001/002]
adrs: [ADR-005]
glossary-refs: [Last indexed]
