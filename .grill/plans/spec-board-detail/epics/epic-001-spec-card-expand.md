epic: 001
plan: spec-board-detail
name: spec-card-expand
status: detailed

summary: Any spec card on the Kanban expands in-place with a 1-2 sentence Executive Summary blurb and a column-appropriate task list (todo=pending, in_progress=all+status, done=all).
delivery-rationale: Operator clicks a Todo or Done card and sees why it exists and which tasks it has — today those cards are dead.

tech:
  facts:
    - spec_board.c zero-write; title/tasks/checklist/agent/blocker parsed ONLY for active_spec
    - planned/draft/done entries: id+column only from active.json
    - GET /api/spec-board JSON: SpecBoardEntry has no blurb field today
    - SpecCard canExpand = active && task_count>0; TaskList already renders #N name + current/done/pending
    - useSpecBoard polls ~4s; caps CBM_SPEC_BOARD_MAX_SPECS=64 MAX_TASKS=48
    - test_results.log is one shared log; done-status match by Task #N can leak across specs (documented)
    - spec.md has ## Executive Summary; KPI is a different header line
  open-qs:
    1. Enrich every spec in cbm_spec_board_read (title+summary+tasks.md) vs lazy-read on expand — 4s poll * N files?
    2. How to take "1-2 sentences" from markdown (split, max chars, strip links) without inventing copy (ADR-006)?
    3. Additive JSON field for blurb on existing /api/spec-board vs a second endpoint?
    4. Apply shared test_results.log caveat to non-active specs' done flags, or tasks.md headings only until a better signal?
    5. Payload size: 64 specs * 48 tasks on every poll — trim fields for collapsed cards?

product:
  facts:
    - ADR-002 in-card expand all columns, no overlay
    - ADR-003 blurb + per-column task rules; Done list+Archive is epic 002's button
    - ADR-006 omit empty blurb; still expand
    - Active card already shows agent, N/M tasks, checklist, blocked outside the task list — keep
    - Todo/Done must become clickable even with zero tasks (blurb-only)
    - ADR-008: several cards may stay expanded at once (no accordion)
  open-qs:
    1. Keep auto-expand of the active spec on load, or all start collapsed?
    2. Click target stays the title button, or whole card including chrome?
    3. Done column in this epic: full task list visible, Archive button deferred to 002 — OK gap on Done cards?
    4. Long task names already truncate — blurb truncate with ellipsis or hard 2-sentence cut only?

deps: [workspace Specs tab already hosts SpecBoardTab]
adrs: [ADR-002, ADR-003, ADR-006, ADR-008]
glossary-refs: [Spec board, Spec card, Spec objective blurb, Zero-write-to-skills]
