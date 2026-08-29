# Dev Log
Updated: 2026-08-29T21:26:00Z
Active Spec: spec-003-h7q-path-project-identity / Current Task: #4 create 409 redirect / Total Tasks Completed: 13 / Branch: local working tree

## [2026-08-29] — Task #4 Create modal path_exists redirect + notice
- POST still `{root_path}` only; 409 `path_exists` → `onPathExists` (no `onCreated` / IndexProgress)
- App: `?tab=graph&project=<existing>` + `role=status` notice; clears on next navigate
- 409 `name_exists` stays in modal with body.error; listed `canonical_root` skips POST
- Validation: `npx vitest run src/components/CreateIndexModal.test.tsx src/App.test.tsx` → 25/25; full graph-ui 123/123
- Next: @human-trainer docs for Task #4

## [2026-08-29] — Task #3 Dashboard conflict groups (human-trainer Trigger A)
- Group by `canonical_root` (`root_path` fallback); newest = `indexed_at` then greater `name` (same as C)
- Conflict region: all names + times + Path once; Enter newest; per-name confirm `DELETE /api/project?name=`
- i18n en+zh: `projects.conflict`, `deleteNamed`; no Reindex (Task #5)
- Validation: `pathGroups.test.ts` + Dashboard conflict cases + i18n locks
- Next: @review Task #3

## [2026-08-29] — Task #2 Admit create vs reindex (HTTP + MCP + jobs)
- `cbm_identity_admit`: CREATE / REINDEX / MCP; 409 `path_exists`|`name_exists` before HTTP slot
- HTTP `project`/`project_name` is reindex key only; MCP empty name + 1 owner reindexes that name
- Daemon same Path + different `project_key` → PATH_CONFLICT (`path_exists`)
- Validation: `scripts/test.sh --suites identity` → 18/18; `--suites httpd` → 69/69 (+1 skip)
- Next: Task #3 Dashboard conflict groups (parallel after #1)

## [2026-08-29] — Task #1 Identity catalog + list_projects fields
- `cbm_identity_*`: canonical_root, newest/tie, cache `.db` catalog (no UNIQUE SQL)
- `list_projects` always emits `indexed_at` + `canonical_root`; display `root_path` unchanged
- `Project.canonical_root?: string` in graph-ui
- Validation: `scripts/test.sh --suites identity` → 7/7; `npx vitest run src/hooks/useProjects.test.ts src/lib/i18n.test.ts` → 8/8
- Next: Task #2 admit 409 (depends on #1)

## [2026-08-29] — Close spec-002-p8w-project-workspace
- Workspace: Graph default, Specs omit-until-true, ADR tab, last-indexed in header
- AdrButton deleted; no C change; KPI met; constitution unchanged (still draft)
- Validation: test_results.log Tasks #1–#5 PASS DEV (106 Vitest)
- Next: /sdd-skill feature new <name> (planned: spec-003 path identity)

## [2026-08-29] — Task #5 App compose + Gherkin
- Workspace: header + tablist under header + Graph / Specs? / AdrTab
- fallbackSpecsToGraph replaceState; dirty leave confirm; AdrButton deleted
- Validation: `cd graph-ui && npx vitest run` → 21 files, 106 tests
- Next: @review / @tester Task #5 then closeprep

## [2026-08-29] — Task #3 Specs presence + tab strip
- useSddSkillPresent: one-shot GET /api/spec-board; present only on 200 && sdd_skill_present===true
- WorkspaceTabStrip: tablist Graph | Specs? | ADR; fallbackSpecsToGraph helper (App #5 wires replaceState)
- SpecBoardTab: non-null project never shows picker; mock body {sdd_skill_present,specs}
- Validation: `cd graph-ui && npx vitest run` → 22 files, 98 tests
- Next: Task #5 mounts strip + helper; do not poll strip with useSpecBoard

## [2026-08-29] — Task #4 AdrTab pane
- AdrTab GET/POST /api/adr; res.ok; alert on 500; status on save; onDirtyChange
- Empty textarea + AdrButton placeholder; Delete posts ""; no CBM-GENERATED
- AdrButton still on disk; App not mounted
- Validation: AdrTab.test.tsx (5 cases). App compose is Task #5
- Next: @review/@tester Task #4; then Task #5 when #3 also reviewed

## [2026-08-29] — Task #2 Workspace header
- WorkspaceHeader: route name + optional time[dateTime] from useProjects + formatIndexedAt
- Ghost name omits time; leave calls onLeave (confirm is Task #5)
- Not mounted in App yet
- Validation: `cd graph-ui && npx vitest run src/components/WorkspaceHeader.test.tsx` → 3 passed
- Next: @human-trainer after #3/#4 land; App compose is Task #5

## [2026-08-29] — Task #1 TabId + readRoute + routeUrl
- TabId dashboard|graph|specs|adr; WORKSPACE_TABS is the extension point
- readRoute/routeUrl in lib/route.ts; workspace+project keeps tab; else Dashboard project=null
- Specs-without-skill fallback and workspace chrome not in this task
- Validation: `cd graph-ui && npx vitest run` → 17 files, 76 tests
- Next: @human-trainer docs for Task #1

## [2026-08-29] — Close spec-001-w3q-executive-dashboard
- Dashboard is account home; list_projects only; grayscale chrome; TabBar deleted
- KPI: default URL = Dashboard + Control, last-indexed visible, 0 Nodes/Edges
- TD-001 / TD-003 / TD-004 resolved; constitution unchanged (still draft)
- Validation: test_results.log Tasks #1–#5 PASS DEV (70 Vitest)
- Next: /sdd-skill feature new <name> (planned: spec-002 workspace)

## [2026-08-29] — Task #5 App routing + TabBar delete
- TabId dashboard|graph; readRoute aliases stats/control/specs/missing/unknown; graph needs project
- Header: brand only on Dashboard; Graph chip + backToDashboard; no Specs/Graph/Projects/Control tabs
- TabBar.tsx deleted; SpecBoardTab unrouted; replaceState ?tab=dashboard without project
- Validation: `cd graph-ui && npx vitest run` → 16 files, 70 tests pass
- Next: @human-trainer docs for Task #5 (last spec-001 task)

## [2026-08-29] — Task #4 Dashboard page
- Dashboard: list + Control on one ScrollArea max-w-4xl; rows name/path/time/HealthDot/Enter/Delete
- CreateIndexModal POST {root_path} only; no Project ID; 202 stays + IndexProgress
- ControlTab embedded polls 3s/2s; StatsTab deleted; AdrButton not imported
- Validation: `cd graph-ui && npx vitest run` → 15 files, 63 tests pass
- Next: @human-trainer docs for Task #4; #5 routing pending

## [2026-08-29] — Task #3 formatIndexedAt
- formatIndexedAt(iso, en|zh): Intl en-US/zh-CN, UTC, year/month/day/hour/minute, timeZoneName short
- Invalid ISO returns the raw string; no JSX, Dashboard wiring is Task #4
- Validation: `cd graph-ui && npx vitest run src/lib/formatIndexedAt.test.ts` → 3 passed
- Next: @human-trainer docs for Task #3; #4 can start (#1 #2 #3 done)

## [2026-08-29] — Task #2 grayscale chrome + palette lock
- globals.css primary/accent/ring gray; --color-hover; surfaces distinct
- Gauge healthy #a3a3a3; >80 red >50 amber; GRAPH_EDGE_PALETTE export
- Chrome panels bg-card; colors.ts + graph-loader #22d3ee unchanged
- Validation: `cd graph-ui && npx vitest run` → 12 files, 54 tests pass
- Next: @human-trainer docs for Task #2; #3 still pending

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
1. @human-trainer docs for Task #2 → @review
2. Task #3 formatIndexedAt (parallel-ready)
3. Task #4 Dashboard after #1 #2 #3
