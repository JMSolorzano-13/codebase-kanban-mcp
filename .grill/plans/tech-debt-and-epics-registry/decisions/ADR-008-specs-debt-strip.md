adr: 008
plan: tech-debt-and-epics-registry
date: 2026-09-01
status: accepted
context: Specs has no board-level strip (last-indexed is WorkspaceHeader on all tabs; Show archived is Done-column only). User confirmed chrome list. Header placement would leak onto Graph/ADR. Per-Todo would imply debt is column-scoped. Severity/color would fight grayscale chrome (executive-ui / SDD-ADR-005).
decision: Specs open-debt list is a board strip above the 3 columns. Each row = TD-NNN + title, heading/file order. No severity, category, status, or color. Not WorkspaceHeader. Not inside a column header.
why: Debt is project-wide, same “chrome not axis” as Game BlockedStrip. Register file keeps the rest. Grayscale stays locked.
alternatives: [WorkspaceHeader, Todo header, +severity, full summary-table row, sort by severity]
irreversible-because: Operators will look above the columns for debt; adding fields later re-densifies chrome and invites sort-by-severity scope.
epics: [001]
