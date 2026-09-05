epic: 002
plan: spec-board-detail
name: spec-archive
status: detailed

summary: Operator can archive a Done spec into CBM-owned state; archived stay in Done, hidden by default, session toggle to show, Unarchive from the same expand.
delivery-rationale: Done stops accumulating history without touching sdd-skill files.

tech:
  facts:
    - ADR-001: never write .sdd-skill/ or other skill trees; completed_specs unchanged
    - spec_board.c remains a reader; archive merge must happen in CBM (store/HTTP), not by editing active.json
    - No archived field on SpecBoardEntry today; SpecColumn is todo|in_progress|done only
    - Per-project SQLite already holds ADR/summaries; identity is project name + spec folder id
    - Board poll ~4s; a POST must be visible on next fetch or optimistic UI
    - If skill later drops a spec from completed_specs, CBM flags may orphan
  open-qs:
    1. Where to persist flags (existing project db vs daemon-global) and what key (project+spec id)?
    2. Merge archived:bool onto GET /api/spec-board vs separate GET/POST archive API?
    3. MCP surface for archive, or HTTP-only for the operator UI?
    4. Orphan flags when spec id leaves completed_specs — drop, keep, ignore on read?
    5. Race: poll overwrites optimistic hide; confirm vs POST-then-refetch?

product:
  facts:
    - ADR-004: no 4th column; hidden default; Unarchive in expand
    - ADR-007: show/hide session-only; archive flags persist in CBM
    - Archive button only on Done expand (ADR-003)
    - Accidental archive + default-hide looks like delete until toggle is on
    - ADR-009: no confirm dialog; recovery = show archived + Unarchive
  open-qs:
    1. Toggle placement: board-level chrome vs Done column header only?
    2. Done count (column badge): visible specs only, or include hidden archived?
    3. Unarchive while toggle is Hide — spec reappears immediately in Done?
    4. Spec archived in CBM then skill moves it out of completed_specs (re-opened) — auto-clear flag or keep hidden if it returns to Done?

deps: [epic-001 — Archive/Unarchive live in the Done expand]
adrs: [ADR-001, ADR-004, ADR-007, ADR-009]
glossary-refs: [Archive (spec), Spec board, Zero-write-to-skills]
