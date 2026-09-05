adr: 005
plan: add-epics-plans-kanban
date: 2026-08-30
status: accepted

context: Mixed Todo (ADR-001) needs a stable order for epics vs specs.
decision: Todo paints unconverted epics first, grouped by plan in `.grill/index.md` row order, epics within a plan by epic-NNN. Then sdd planned/draft specs in current active.json array order. In progress/Done unchanged (specs only).
why: Funnel: grill epic is before an sdd planned spec. Group-by-plan matches the card's plan field. No shared timestamp to interleave.
alternatives: [specs first, interleave by mtime, flat epic list ignoring plan]
irreversible-because: Operators learn a scan order; flipping later looks like cards jumped columns.
epics: [epic-001]
