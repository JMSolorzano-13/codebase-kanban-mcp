adr: 008
plan: add-epics-plans-kanban
date: 2026-08-30
status: accepted

context: Specs tab already polls GET /api/spec-board ~4s. Constitution IV.3: prefer existing HTTP. spec-005/006 additive fields on the same GET.
decision: Same GET /api/spec-board. Additive: grill_skill_present, epic entries (or kind on mixed list). No second poll, no MCP board tool. Tab label stays "Specs".
why: A second endpoint doubles the 4s poll and splits the mixed column. Tab rename reopens workspace IA (executive-ui-ia).
alternatives: [GET /api/grill-board, rename tab, MCP tool]
irreversible-because: Two board reads would be a second source of truth for one Kanban.
epics: [epic-001, epic-002]
