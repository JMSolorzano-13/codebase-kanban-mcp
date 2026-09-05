# Task #3 — AdrTab generic chrome; no cycle stamp
Last updated: 2026-08-31 — spec-013-r9w-adr-fill-gamedev-trio | Path: compact | Patterns: ✓

Isolated AdrTab with markers still shows `formatIndexedAt(indexed_at)`, the existing replace warning, and one textarea. The render must not contain `gamedev-skill`. File: `graph-ui/src/components/AdrTab.test.tsx` only (`AdrTab.tsx` / `i18n.ts` / App / colors untouched). Why: US-006 chrome stays generic after gamedev XOR fill — no cycle stamp.

Quick ref: spec US-006; test `shows generated-at from list indexed_at and a replace warning when markers are present` (`AdrTab.test.tsx:187`). If `gamedev-skill` appears → AdrTab or i18n grew a cycle stamp.
