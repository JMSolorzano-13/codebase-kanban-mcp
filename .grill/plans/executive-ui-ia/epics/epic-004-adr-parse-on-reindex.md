epic: 004
plan: executive-ui-ia
name: adr-parse-on-reindex
status: detailed

summary: Phase 2 — on manual reindex only, parse .sdd-skill context_ai.md + baseline/TECH_STACK.md + baseline/ARCHITECTURE_ADR.md into the ADR generated region; never touch the manual region; no LLM.
delivery-rationale: Reindex an sdd-skill repo and the ADR tab shows updated generated architecture notes, same family as Specs parse and node extraction.

tech:
  facts:
    - CBM ADR = SQLite project_summaries markdown; manage_adr get/update/store/sections; UI /api/adr
    - Specs adapter spec_board.c is read-only parse of .sdd-skill/, best-effort, no write to skill files
    - Nodes come from tree-sitter index pipeline, not an LLM
    - sdd-skill codebase-memory.md: reindex is user-triggered only; graph must not index .sdd-skill/ docs
    - Watcher / auto_watch incremental must not run this fill
    - Trio paths relative to project root_path
    - Phase 1 ADR tab may already have a whole-document blob with no regions
  open-qs:
    1. Region markers: exact syntax so parse and human editor share one blob (handoff — do not invent in grill)?
    2. Where parse runs: index pipeline C pass vs post-index UI/server hook — must be zero-token either way?
    3. How to distinguish manual reindex from watcher/daemon incremental/supervised worker?
    4. Missing/partial trio: empty generated, skip write, or partial fill?
    5. manage_adr(update) replacing the whole document — protect manual region or reject whole-doc writes after Phase 2?
    6. Existing Phase-1 blobs with no markers: first reindex treats all as manual, all as generated, or one-time migrate?
    7. Parse must not feed .md into the code graph (skill forbids graph-on-docs) — keep file reads out-of-graph?

product:
  facts:
    - Separate later epic; ships after 001–003
    - Manual edit remains
    - Specs and graph nodes stay read-only (no write-back to .sdd-skill/ or source)
    - Only the three confirmed files; ignore DEV_LOG/TECH_DEBT/constitution/human/specs/history
    - Projects without the skill: no auto-fill (ADR tab remains manual-only)
  open-qs:
    1. Show "generated at <indexed_at>" vs raw markdown only?
    2. Warn that edits inside the generated region die on next reindex?
    3. Generated headings language: English only (skill files are English) vs UI i18n?
    4. Reindex from Dashboard vs only from MCP/CLI — both trigger fill?
    5. Failure to parse one file: surface on ADR tab, or silent best-effort like Specs?

deps: [epic-002 ADR tab + blob, epic-003 Path identity so parse hits the one store for that repo]
adrs: [ADR-005, ADR-006, ADR-007]
glossary-refs: [ADR auto-fill (Phase 2), Phase 2 source trio, ADR generated region, ADR manual region, Manual reindex]
