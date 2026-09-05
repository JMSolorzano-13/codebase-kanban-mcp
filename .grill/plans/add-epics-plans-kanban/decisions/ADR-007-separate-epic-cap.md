adr: 007
plan: add-epics-plans-kanban
date: 2026-08-30
status: accepted

context: ADR-003 wants all unconverted epics; board already caps specs at 64. Unbounded list rejected.
decision: Epics have their own cap, same magnitude as specs (64). Epic slots do not steal spec slots. Exact N is spec-level. Overflow omitted this read (same best-effort as extra specs today). No "has more" chrome in this plan.
why: User accepted separate cap + omit overflow. Promising unbounded fights spec_board reader. Mixing one 64 pool would drop specs when many epics exist.
alternatives: [single 64 pool, unbounded, has-more UI]
irreversible-because: A shared pool makes ADR-003 and spec Todo compete; raising later is easy, shrinking a shared pool hides specs.
epics: [epic-001]
