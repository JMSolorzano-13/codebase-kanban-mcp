adr: 001
plan: spec-board-detail
date: 2026-08-29
status: accepted

context: User asked where Done-spec archive lives; also forbade this feature from moving skill files.
decision: Archive flag is CBM-owned. This plan never writes/moves/renames anything under .sdd-skill/ or other skill trees. sdd-skill cycle files stay as-is (active.json, spec.md Status, completed_specs unchanged).
why: User 2026-08-29: "debe vivir en el CBM, definitivo, no queremos que esta funcionalidad mueva nada de las skills." Adapter is already zero-write; skill has no archived enum.
alternatives: [mutate active.json archived_specs, set spec.md Status=archived, sidecar file inside .sdd-skill/]
irreversible-because: Splits board hygiene from skill lifecycle; later skill-aware archive would be a second source of truth.
epics: [epic-002]
