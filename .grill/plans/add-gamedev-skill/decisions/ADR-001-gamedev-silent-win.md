adr: 001
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: Same-path sdd vs gamedev undecided in prior plan ADR-006; user wants gamedev complete, not mixed into Specs.
decision: Per path, `.gamedev/` present → Game tab; Specs tab omitted even if `.sdd-skill/` and/or `.grill/` exist. No conflict banner. No hybrid chrome. Other paths may still be sdd+grill as today. Product hosts both skills; one path does not.
why: Track A already is SDD inside `.gamedev/`. Mixing Specs would duplicate the cycle and starve the phase/track/parallel board. Silent win keeps the Game surface uncompromised.
alternatives: [visible conflict UI, sdd wins, both tabs, merge into Specs kanban]
irreversible-because: Specs presence becomes "sdd-or-grill AND NOT gamedev". A later dual-tab on one path would fight this rule and the Game-complete design.
epics: [001, 004]
