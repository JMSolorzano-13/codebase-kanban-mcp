adr: 003
plan: add-epics-plans-kanban
date: 2026-08-30
status: accepted

context: Which grill epics appear in Todo after mixed-column (ADR-001) and conversion-hide (ADR-002).
decision: Todo lists every unconverted epic under `.grill/plans/*/epics/`, all plans (draft and closed), both pending and detailed. Closed ≠ shipped. No status filter. Converted (ADR-002) still omitted.
why: grill closed = interview done, leftover epics may never have been handed to sdd and would vanish if filtered. pending is still a plan slice. ADR-002 is the only hide rule.
alternatives: [draft-plans only, detailed-only, hide closed leftovers]
irreversible-because: Filtering closed/pending later silently drops backlog the operator already expects on Todo.
epics: [epic-001]
