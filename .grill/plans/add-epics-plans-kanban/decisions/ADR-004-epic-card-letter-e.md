adr: 004
plan: add-epics-plans-kanban
date: 2026-08-30
status: accepted

context: User confirmed always-visible title+summary+plan, no task expand; overrode word-label "Epic" with a minimal kind mark.
decision: Epic card always shows title (epic.md name), 1-line summary, owning plan title. No tasks.md expand. Kind mark = single letter "E", no word Epic, no filled pill/badge. One discreet chromatic hue (chrome exception). Not health red/amber/green. Not graph colorForLabel/EdgeLines hex. Exact token/hex is spec-level.
why: User 2026-08-30: title/summary/plan always on card; identifier = letter E, minimal, discreet color. Specs keep title+click-expand blurb/tasks. Epics have no tasks.md.
alternatives: [word label Epic, gray-only E, filled badge, same expand as specs]
irreversible-because: Mixed Todo scannability depends on a stable kind mark; a later word-label or health-colored E fights chrome ADRs.
epics: [epic-001]
