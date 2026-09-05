adr: 005
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: User wants CBM ADR filled from sdd-skill documents without an LLM, same family as Specs parse and node extraction.
decision: Phase 2 is a separate later epic. Trigger = manual reindex only (not watcher). Process = deterministic parse of the project's .sdd-skill/ docs (skill templates at _SKILLs/sdd-skill/source/sdd-skill define shape; runtime input is the indexed repo). Output writes/updates CBM ADR store. Human can still edit by hand. No/min tokens.
why: Specs already parse .sdd-skill/ with zero write to the skill; nodes come from the index pipeline. User forbade LLM fill. sdd-skill codebase-memory.md already treats reindex as user-only.
alternatives: [LLM summarization on open, watcher-triggered fill, parse skill source repo]
irreversible-because: New index-pipeline concern + dual-write with manual ADR. File set = ADR-006. Write policy = ADR-007.
epics: []
