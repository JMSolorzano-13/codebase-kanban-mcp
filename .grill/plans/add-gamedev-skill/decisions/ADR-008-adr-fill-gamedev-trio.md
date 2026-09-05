adr: 008
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: ADR tab always exists. Fill today is sdd trio only (`adr_fill.c`). ADR-001 omits Specs on `.gamedev/` paths; bevy-tetris keeps `.sdd-skill/` as MVP1 archive while `.gamedev/` is the live cycle. User wants sdd-feature parity for ADR parse.
decision: `.gamedev/` present → generated ADR region fills from `.gamedev/game_context.md` + `.gamedev/baseline/TECH_STACK.md` + `.gamedev/baseline/ARCHITECTURE_ADR.md` only. Do not merge or fall back to the sdd trio. Missing/unreadable file → omit that H1; reindex still succeeds. Trigger stays user-triggered reindex; generated vs manual regions unchanged. Paths without `.gamedev/` still use the sdd trio. Not graph nodes. No LLM. Ignore DEV_LOG, constitution, GDD, playtest-log for this fill.
why: Silent win: Game chrome + ADR generated must describe the same cycle. Merging trios duplicates Purpose/Stack (bevy MVP1 vs MVP2). game_context.md is the Purpose analog of context_ai.md; stack/decisions paths match. Same omit-heading contract as spec-004.
alternatives: [always sdd trio, merge both, fallback sdd if gamedev file missing, watcher fill]
irreversible-because: Parse contract + which missing files are empty vs error. A later merge would fight silent win and dual-write markers.
epics: [004]
