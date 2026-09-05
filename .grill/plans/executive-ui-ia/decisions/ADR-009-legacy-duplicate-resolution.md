adr: 009
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: ADR-001 blocks new duplicates. Existing stores may already have two indexes for one Path (or colliding names).
decision: Do not silent-delete. Surface a conflict. Open the most recently updated/reindexed project. Ask authorization to delete the older. Yes → delete older. No → older remains listed until the user decides later.
why: User Q2. 1:1 is forward-looking; history needs an explicit human gate.
alternatives: [auto-delete older, keep both forever, merge DBs]
irreversible-because: Defines data-loss path (delete_project on the older) and which row "enter" picks (newest indexed_at).
epics: []
