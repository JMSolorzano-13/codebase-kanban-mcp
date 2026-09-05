adr: 004
plan: spec-board-detail
date: 2026-08-29
status: accepted

context: User confirmed how archived specs appear on the Kanban after CBM-owned archive (ADR-001).
decision: Archived specs stay in Done (no 4th column). Hidden by default. Board has a show/hide control. When shown, expand offers Unarchive (inverse of Archive). Archive/Unarchive do not change skill files.
why: User 2026-08-29 accepted recommendation. Show/hide was the ask, not a new lifecycle column. One-way archive without unarchive is a dead end.
alternatives: [4th Archived column, stay visible in Done with badge only, archive with no unarchive]
irreversible-because: Board IA stays 3 columns; operators will treat Done as live+optional-history, not a new swimlane.
epics: [epic-002]
