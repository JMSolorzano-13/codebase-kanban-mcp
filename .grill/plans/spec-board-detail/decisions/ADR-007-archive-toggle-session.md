adr: 007
plan: spec-board-detail
date: 2026-08-29
status: accepted

context: ADR-004 hides archived by default; user needed whether show/hide survives reloads.
decision: Show/hide archived is session-only. Each visit starts hidden. Do not persist the toggle in localStorage or CBM.
why: User 2026-08-29 accepted recommendation. Persisting would turn "hidden by default" into a per-browser ghost. Archive flags themselves stay in CBM (ADR-001).
alternatives: [localStorage per project, CBM-persisted preference]
irreversible-because: Operators will expect a fresh board to hide history; a later persist would surprise that default.
epics: [epic-002]
