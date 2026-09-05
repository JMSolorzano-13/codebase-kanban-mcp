# source: sdd-skill filesystem — Phase 2 ADR inputs
date: 2026-08-29
path: /Users/jmsolorzano/SWE/_SKILLs/sdd-skill/source/sdd-skill

runtime-root: <project>/.sdd-skill/   # not the skill source tree
skill-source: templates + filesystem.md define shapes only

architecture-bearing:
  - context_ai.md — PURPOSE-like synthesis, stack, modules, key decisions table
  - baseline/TECH_STACK.md — STACK (versions, env, schema)
  - baseline/ARCHITECTURE_ADR.md — decision log ADR-NNN (Status/Problem/Decision/Consequences)
  - docs/constitution.md — immutable rules (patterns/philosophy-ish)
  - human/PROJECT-OVERVIEW.md — human prose, not AI-primary

chronology-not-architecture:
  - baseline/DEV_LOG.md — recency log, condenses
  - baseline/TECH_DEBT.md — TD register, not decisions
  - history/* — audit, append-only

already-parsed-by-cbm-ui:
  - specs/active.json, specs/*/spec.md, tasks.md, checklist.md, state.md, history/test_results.log
  - adapter: spec_board.c — read-only, best-effort, hides tab if folder absent

cbm-adr-today:
  - sections hint: PURPOSE, STACK, ARCHITECTURE, PATTERNS, TRADEOFFS, PHILOSOPHY
  - store: project_summaries markdown blob
  - no parse of .sdd-skill/ into that blob yet

user-confirmed-trio: context_ai.md + baseline/TECH_STACK.md + baseline/ARCHITECTURE_ADR.md
ignored: DEV_LOG, TECH_DEBT, constitution, human/*, specs/*, history/*
reindex-protocol-in-skill: /sdd-skill reindex is the only user command that indexes; agents must not self-bootstrap
