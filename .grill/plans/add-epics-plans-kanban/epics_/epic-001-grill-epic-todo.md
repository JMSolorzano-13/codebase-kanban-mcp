epic: 001
plan: add-epics-plans-kanban
name: grill-epic-todo
status: detailed

summary: Specs tab Todo paints unconverted grill epics (E, title, summary, plan) mixed with sdd planned/draft specs; converted epics omitted; own cap; same GET.
delivery-rationale: Operator on a project that already has Specs sees grill backlog in Todo end-to-end without a grill-only path.

tech:
  facts:
    - shipped spec-008-g8r-grill-epic-todo (closed 2026-08-30)
    - same GET; additive grill_skill_present + epics[] cap 64 independent of specs 64
    - conversion: listed spec.md Companion-to first .grill/plans/…md token OR active.json source.grill_epic; exact path
    - zero-write; POST archive with epic id → 404 spec not found
    - kind only on epic entries; specs array unchanged
  open-qs: []

product:
  facts:
    - Mixed Todo; In progress/Done specs-only (ADR-001)
    - Card: letter E + title + summary + plan title; no expand/archive (ADR-004)
    - All unconverted epics all plans; hide via exact match (ADR-002, ADR-003)
    - Order epics then specs (ADR-005); own epic cap overflow omit (ADR-007)
    - Tab still sdd-gated in this epic; grill-only is epic-002
  open-qs: []

deps: [Specs tab + SpecBoardTab already host the Kanban]
adrs: [ADR-001, ADR-002, ADR-003, ADR-004, ADR-005, ADR-007, ADR-008]
glossary-refs: [Mixed Todo, Grill epic card, Converted epic, Eligible epic, Epic card, Todo order, Epic cap, Board read]
shipped: spec-008-g8r-grill-epic-todo
