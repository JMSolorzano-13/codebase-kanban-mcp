adr: 002
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: ADR-001 omits Specs on a `.gamedev/` path. Grill unconverted epics today paint only on Specs Todo (prior plan). User confirmed Game-tab inbox so "use grill" is not blind.
decision: Path with `.gamedev/` and `.grill/` → unconverted grill epics list on the Game tab as an inbox. Specs stays omitted. Paths without `.gamedev/` unchanged (Specs+grill as today). Conversion match rule is a later ADR (gamedev has no Companion-to; living example maps grill epics in roadmap.md).
why: Only chrome on that path is Game. Reopening Specs for grill would fight silent win. Hiding until converted leaves leftover epics invisible (same reason prior ADR-003 lists all unconverted).
alternatives: [hide until converted, reopen Specs for grill only, mixed into a phase column]
irreversible-because: Operators learn grill lives in Game inbox on gamedev paths. Reopening Specs later contradicts ADR-001. Mixing into Pre-prod would imply the epic already entered the cycle.
epics: [002]
