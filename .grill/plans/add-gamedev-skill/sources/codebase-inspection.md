plan: add-gamedev-skill
date: 2026-08-30
entry: mixed (phrase + CBM codebase + gamedev-skill docs + bevy-tetris living .gamedev/)

cbm-sdd-surface-today:
- Workspace tabs: graph, specs (conditional), adr. Comment in types.ts: later gamedev tab = add id to WORKSPACE_TABS. No plugin runtime.
- Specs tab iff spec-board 200 AND (sdd_skill_present OR grill_skill_present). gamedev-only → omit Specs (App.test).
- Specs kanban: 3 cols todo/in_progress/done from .sdd-skill/specs/active.json. Grill unconverted epics mixed into Todo only.
- Spec card expand, archive (CBM .db, zero skill writes), last-indexed local TZ.
- ADR generated region: reindex parse of .sdd-skill/{context_ai.md, baseline/TECH_STACK.md, baseline/ARCHITECTURE_ADR.md}. No gamedev paths.
- Constitution I.2: indexer/graph-ui must not write skill cycle files. Same constraint will apply to .gamedev/.
- Path=Project 1:1 (spec-003). One root_path → one chrome identity.

gamedev-hooks-already-in-cbm (unused by UI):
- cbm_spec_board_gamedev_skill_present: true iff root/.gamedev is a dir.
- spec-board read/to_json MUST NOT emit gamedev_skill_present (spec_board.h).
- GET /api/skill-presence?project= → {sdd_skill, gamedev_skill}. Comment says frontend tab visibility; graph-ui never calls it (useSddSkillPresent uses spec-board only).
- ADR-006 of plan add-epics-plans-kanban: gamedev out of Specs; presence/kind additive; dual sdd+gamedev on one path was UNDECIDED.

prior-grill-out-of-scope-now-this-plan:
- add-epics-plans-kanban: do not read .gamedev/, do not paint gamedev cards, do not hide epics via gamedev companion.

sdd-feature-map-candidate-for-gamedev-analog (not yet scoped):
| sdd CBM | gamedev filesystem analog |
| Specs 3-col kanban | NOT a copy — user wants phase-first board |
| spec.md expand | Track A systems/<id>/{spec,tasks,review}.md |
| state.md one role | state.md many parallel agent lines + phase= |
| blocked_note | agent line blocked:"task":"blocked-by" |
| grill epics in Todo | conversion today is sdd Companion-to + active.json.source.grill_epic; gamedev has no that matcher |
| ADR trio parse | .gamedev/{game_context.md, baseline/TECH_STACK.md, baseline/ARCHITECTURE_ADR.md} exist |
| Specs presence strip | new tab; skill-presence already has gamedev_skill bool |

bevy-tetris (indexed project, living example):
- Has .gamedev/, no .sdd-skill/, no .grill/ on disk now.
- phase=02-production, skill_v=1.10.0, parallel-capable state (qa-lead:ready).
- roadmap.md maps grill plan epics → systems/SYS-* Track A + parked Track B epic-005. Proof grill→gamedev conversion already happens in the skill, not in CBM.
