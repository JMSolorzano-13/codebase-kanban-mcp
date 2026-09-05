epic: 004
plan: add-gamedev-skill
name: adr-fill-gamedev-trio
status: detailed

summary: User-triggered reindex fills ADR generated region from `.gamedev/game_context.md` + baseline TECH_STACK + ARCHITECTURE_ADR; no merge with sdd trio; same generated/manual split.
delivery-rationale: Operator on a gamedev path reindexes and ADR Purpose/Stack/Decisions match the Game cycle (MVP2 on bevy), not archived `.sdd-skill/`.

tech:
  facts:
    - adr_fill.c opens only three sdd relatives; H1 Purpose/Stack/Decisions; 8K gen buf; missing/unreadable = omit H1; job still succeeds
    - Trigger = user index/reindex/MCP index_repository; watcher does not fill (spec-004)
    - Generated vs manual markers (executive ADR-007); manage_adr whole-document
    - `.sdd-skill` ALWAYS_SKIP; fill is out-of-graph
    - ADR-008: if `.gamedev/` dir present, those three gamedev paths only; no sdd fallback/merge
    - game_context.md is Purpose analog; bevy also has leftover `.sdd-skill/` MVP1 archive
    - Skill may condense game_context ~6KB; ARCHITECTURE_ADR on bevy is a large table
  open-qs:
    1. Presence test: same cbm_spec_board_gamedev_skill_present or independent is-dir?
    2. Truncation: 8K buf vs bevy ARCHITECTURE_ADR size — omit tail, raise cap, or bounded extract like spec-004 splice?
    3. H1 titles stay Purpose/Stack/Decisions when source is game_context.md?
    4. Empty generated file: still write empty headings or skip fill?
    5. Reindex after `.gamedev/` added to a former sdd project: overwrite generated (drop sdd extracts) in one job?
    6. `.gamedev/` removed later: next reindex restore sdd trio or leave last gamedev generated?
    7. Unreadable game_context vs unreadable ARCHITECTURE_ADR: same omit-H1 as sdd?
    8. Tests: bevy-like fixture with both trees — assert sdd trio strings absent from generated?
    9. MCP index_repository vs HTTP reindex vs create-index: all three must pick gamedev trio?

product:
  facts:
    - ADR tab always exists; Specs omitted on gamedev paths (ADR-001) so generated must not stay on MVP1
    - User: no LLM; no write-back into `.gamedev/`
    - Manual region never touched by parse
  open-qs:
    1. Visible stamp: “from gamedev-skill” vs same generic generated warning as sdd?
    2. Operator who still wants MVP1 ADRs: only via leftover `.sdd-skill/` file on disk / Graph, not ADR generated — acceptable?
    3. Partial trio (only TECH_STACK): show Purpose empty or hide the heading in the editor?
    4. First create-index of a gamedev repo: fill immediately (spec-004 yes) or wait for explicit Reindex?
    5. Dirty manual region + reindex: same window.confirm as AdrTab today?

deps: [epic-001 silent win implies which cycle ADR describes; fill can ship without 002/003]
adrs: [ADR-008, ADR-001]
glossary-refs: [Gamedev ADR trio, Silent win]
