plan: tech-debt-and-epics-registry
date: 2026-09-01
skill: /Users/jmsolorzano/SWE/_SKILLs/gamedev-skill/source/gamedev-skill (v1.12.0 Epic Tracking)
inspected: README.md Epic Tracking, CHANGELOG 1.12.0, references/epics.md, references/filesystem.md, references/instructions.md §5+§8, templates/epics_registry.md, templates/backlog.md

no-TECH_DEBT-md:
- gamedev baseline = TECH_STACK, ARCHITECTURE_ADR, DEV_LOG, DEBUG_TOOLS(conditional)
- debt register is .gamedev/backlog.md — not a TD-NNN file

backlog.md:
- owner: any agent adds; @director triages
- tags: debt:gate-<phase> | debt:adopt-gap-<area> | debt:<slug> | design | tech
- comment template: [tag] description — owner — target
- pickup: append `resolved-by: SYS-0.<n>` on the same entry
- gate gaps: debt:gate-* mandatory, never silent (instructions.md §5)

epics_registry.md (conditional, lazy, @director sole writer):
- created first time an epic starts OR Epic 0 needed; never empty scaffold at init
- cols: Epic | Plan | Name | Origin | Status | Systems | Milestone | Opened | Closed | Notes-ref
- Epic: upstream NNN; 0 = permanent no-epic bucket
- Plan: grill slug or `native`
- Origin: epic|bug|debt|feature|infra
- Status: not_started|in_progress|closed|parked|evergreen (evergreen = Epic 0 only)
- Systems: derived from phases/02-production/systems/ SYS-<N>* names; not self-reported
- zero-dep: READ-ONLY toward .grill/; never writes/renames grill; grill `status: detailed` is planning-status, not completion
- completion lives only on gamedev side (registry Status + SYS folder status)
- naming: SYS-<epic>[.<sub>]-<slug>; sub = same-epic follow-up, not a new row
- start: `continue start epic <N>` → read grill epic-N RO → upsert row Status=in_progress
- close: human declares done → Status=closed, Closed=today
- /map: one line per row neither closed nor evergreen

no-epic debt route (not inbox):
- inside open epic → SYS-<that>.<next>
- mechanical → ADR + decisions.log only
- real Track A no epic → SYS-0.<n> (Epic 0 evergreen)
- known debt no work → backlog.md debt:<slug>
