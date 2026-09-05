epic: 002
plan: add-epics-plans-kanban
name: specs-tab-grill-presence
status: detailed

summary: Specs tab appears when `.grill/` exists even if `.sdd-skill/` does not; presence flags stay additive for a later gamedev plan.
delivery-rationale: Grill-only path can open the Kanban; today omit-until-true hides Specs without sdd so 001 never shows there.

tech:
  facts:
    - shipped spec-009-t4x-specs-tab-grill-presence (closed 2026-08-30)
    - same one-shot GET; present = 200 AND (sdd_skill_present OR grill_skill_present)
    - no C/HTTP change this spec; flags already on spec-008 GET
    - gamedev dir alone does not show Specs; GET still must not emit gamedev_skill_present
  open-qs: []

product:
  facts:
    - Tab visible if sdd OR grill; label Specs; order Graph | Specs | ADR (ADR-001, ADR-008)
    - Grill-only Kanban not notSddSkill; In progress/Done empty → existing noSpecs
    - ?tab=specs stays on grill-only; neither-skill still omits
    - gamedev out this plan; path dual-skill undecided (ADR-006)
  open-qs: []

deps: [epic-001 — grill-only Todo empty without epic cards]
adrs: [ADR-001, ADR-006, ADR-008]
glossary-refs: [Specs tab presence, Gamedev (this plan), Board read]
shipped: spec-009-t4x-specs-tab-grill-presence
