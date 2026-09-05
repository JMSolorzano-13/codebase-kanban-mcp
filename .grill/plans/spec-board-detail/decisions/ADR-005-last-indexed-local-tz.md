adr: 005
plan: spec-board-detail
date: 2026-08-29
status: accepted

context: User confirmed Last indexed was showing UTC; formatIndexedAt pins timeZone UTC (SDD-ADR-003) on Dashboard, workspace header, AdrTab stamp.
decision: Same helper, all three surfaces: drop UTC pin; display in the browser's local timezone. Stored indexed_at ISO (Z) unchanged. <time dateTime> keeps the raw ISO.
why: User 2026-08-29 accepted recommendation. One helper — splitting surfaces would mix UTC and local. UI runs on the operator machine.
alternatives: [Dashboard-only, daemon TZ, keep UTC and add a suffix]
irreversible-because: Reverses SDD-ADR-003 display contract; operators will read local wall clock as the freshness signal.
epics: [epic-003]
