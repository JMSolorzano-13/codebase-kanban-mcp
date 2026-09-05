# Task #3 — formatIndexedAt
Last updated: 2026-08-29 — spec-001-w3q-executive-dashboard | Patterns: ✓

`formatIndexedAt(iso, en|zh)` in `graph-ui/src/lib/formatIndexedAt.ts` turns `Project.indexed_at` into a UTC locale string (`en-US` / `zh-CN`, year/month/day/hour/minute + short TZ). Invalid ISO returns the raw string so freshness stays visible. Dashboard `<time dateTime>` wiring is Task #4 (SDD-ADR-003 / US-002). Tests: `formatIndexedAt.test.ts`.
