adr: 001
plan: add-epics-plans-kanban
date: 2026-08-30
status: accepted

context: User asked grill epics on Specs Kanban Todo, compatible with sdd specs; doubted how conversion removes epic from Todo.
decision: Board is a read of both trees. Mixed Todo. In progress/Done = specs only. CBM never writes/moves/renames `.grill/` or `.sdd-skill/` to convert. Specs tab visible if sdd OR grill present.
why: Kanban has no drag; constitution I.2 zero-write; sdd-skill already creates the spec and (in this repo) records origin. Conversion is filesystem, not a board action.
alternatives: [drag-to-create-spec from CBM, CBM writes active.json, hide Specs unless sdd]
irreversible-because: Splits board display from skill lifecycle; a later write-back converter would be a second product.
epics: [epic-001, epic-002]
