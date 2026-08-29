# Dev Log
Updated: 2026-08-29T19:00:00Z
Active Spec: spec-001-w3q-executive-dashboard / Current Task: Task #1 done → @human-trainer / Total Tasks Completed: 1 / Branch: local working tree

## [2026-08-29] — Task #1 useProjects list-only (TD-001)
- useProjects: list_projects only; return { projects: Project[], loading, error, refresh }
- Dropped ProjectInfo + per-project get_graph_schema; no schema: null placeholder
- SpecBoardTab ProjectPicker maps Project name + path; StatsTab p.project → p.name
- Validation: `cd graph-ui && npx vitest run` → 10 files, 40 tests pass
- Next: @human-trainer docs for Task #1; #2/#3 still pending

## [2026-08-29] — Architect plan spec-001
- plan.md: stack Dashboard+Control, UTC `<time>`, delete TabBar, no C change
- tasks.md: 5 tasks (#1 useProjects, #2 chrome lock, #3 formatIndexedAt, #4 Dashboard, #5 routing)
- SDD-ADR-002..008 in ARCHITECTURE_ADR.md
- Validation: `cd graph-ui && npm test` after implement (not run this step)
- Gate: user must approve before @implementer

## Recently Completed
Adopt init. Architect plan written; waiting approval.

## Known Issues
- UI home is StatsTab: aggregate nodes/edges + per-card schema chips + ADR modal. Planned removal on Dashboard (this spec).
- `useProjects` is list_projects only (Task #1 done). Dashboard home still StatsTab until #4/#5.
- Project identity is name, not `root_path` (PENDING — grill epic 003).
- Optional UI `project_name` creates aliases (PENDING — remove in this spec's create flow).
- Global tabs default to Specs (PENDING — Dashboard becomes default).

## Performance Metrics
| Metric | Target | Current | Status |
| Dashboard list | no per-project schema RPC | 0 schema calls in useProjects | OK (Task #1) |
| UI port | 9749 | 9749 | OK |

## Dependencies Updated
none this init

## Next Steps
1. @human-trainer docs for Task #1 → @review
2. Tasks #2 chrome tokens and #3 formatIndexedAt (parallel)
3. Task #4 Dashboard after #1 #2 #3
