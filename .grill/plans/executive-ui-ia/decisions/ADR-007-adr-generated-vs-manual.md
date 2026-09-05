adr: 007
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: CBM ADR is dual-write: Phase 2 parse + human editor. Specs and graph nodes are read-only derived.
decision: ADR document has two regions. Generated region: overwritten on every manual reindex parse. Manual region: never touched by parse. Specs tab and graph nodes stay read-only (no write-back to .sdd-skill/ or source).
why: User Q2 exact wording. Prevents reindex from deleting hand edits; keeps Specs/nodes as adapters not editors.
alternatives: [full overwrite, skip parse if dirty, prompt merge UI]
irreversible-because: Locks ADR storage shape (must distinguish generated vs manual) and reindex write rules. Marker/format is spec-level.
epics: []
