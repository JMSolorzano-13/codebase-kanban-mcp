# source: codebase inspection — add-epics-plans-kanban
date: 2026-08-30
entry: mixed (phrase + spec board + grill-skill source + this repo .grill)

prior-plans: executive-ui-ia (closed), spec-board-detail (closed)
this-repo-sdd: .sdd-skill/ present; active.json status=completed; 7 specs in completed_specs; planned/draft empty
linked_gamedev: none

kanban:
  host: graph-ui SpecBoardTab.tsx — 3 cols todo / in_progress / done
  poll: useSpecBoard GET /api/spec-board ~4s
  no drag, no column move
  Specs tab omit-until-true: useSddSkillPresent requires sdd_skill_present===true
  empty board: !sdd_skill_present → notSddSkill copy
  caps: CBM_SPEC_BOARD_MAX_SPECS=64

spec-board-c:
  file: src/ui/spec_board.c + .h — zero-write, best-effort
  columns from active.json: planned+draft→todo; active_spec→in_progress; completed→done
  no .grill read; no grill_skill_present flag
  gamedev present stub always false

constitution: I.2 indexer/graph-ui must not write skill cycle files

sdd-origin-link (this repo, not skill-wide template):
  active.json.source.grill_plan + grill_epic — current cycle only (now epic-003 last-indexed)
  every spec-001..007 spec.md Related Specs: `Companion to: .grill/plans/<plan>/epics/epic-NNN-<name>.md`
  sdd-skill templates/spec.md Companion to: [spec-NNN] — grill path is local convention
  sdd-skill templates/active.json has no source key
