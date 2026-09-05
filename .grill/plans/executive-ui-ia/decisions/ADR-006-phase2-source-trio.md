adr: 006
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: Phase 2 must parse existing sdd-skill docs, not invent architecture via LLM. User confirmed a trio.
decision: Only these three files under the indexed project's .sdd-skill/: context_ai.md, baseline/TECH_STACK.md, baseline/ARCHITECTURE_ADR.md. Ignore DEV_LOG, TECH_DEBT, constitution, human/*, specs/*, history/*.
why: User Q1=yes. Those three map to PURPOSE/STACK/decisions. Other skill files are chronology, debt, or already owned by Specs tab.
alternatives: [baseline three-file rule incl. DEV_LOG, also constitution.md, all of .sdd-skill/]
irreversible-because: Defines the parse contract and which missing files are "empty generated block" vs error.
epics: []
