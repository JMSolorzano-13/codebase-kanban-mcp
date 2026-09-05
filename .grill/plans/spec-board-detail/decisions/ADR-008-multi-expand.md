adr: 008
plan: spec-board-detail
date: 2026-08-29
status: accepted

context: SpecCard already has independent expanded state; user chose multi-open vs accordion.
decision: Multiple spec cards may be expanded at once. No accordion (opening one does not collapse others).
why: User 2026-08-29 accepted recommendation. Comparing cards across columns is the useful case; matches current per-card useState.
alternatives: [accordion one-open, only one per column]
irreversible-because: Operators will learn to leave several cards open; an accordion later would feel like a regression.
epics: [epic-001]
