adr: 003
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: User wants a new kanban split by phase (not Specs todo/doing/done) and still to see pending / in-progress / done. Work inside a phase is parallel. Track is independent of phase.
decision: Game board = four columns L→R: Inbox | Pre-production | Production | Post-production & Launch. Track A/B/H is a badge on the card, not a swimlane. Current phase (`state.md phase=`) is highlighted; earlier phases stay visible. Work-state (pending / in_progress / done, plus blocked if present) lives on the card. Many cards in one phase may be in_progress at once. Agent status does not become columns.
why: Skill spine is linear phases; agents run parallel inside (cycle.md). Specs 3-col is status-of-one-spec; copying it would hide parallelism the user asked to see. Inbox left = ADR-002 funnel without implying a grill epic already entered a phase. Swimlanes 3×3 leave Inbox with no track and split @producer/@qa-lead poorly.
alternatives: [status columns like Specs, track swimlanes, current-phase-only view]
irreversible-because: Operators learn phase = column. Putting status back on the axis later would look like cards jumped and would fight the parallel read.
epics: [002]
