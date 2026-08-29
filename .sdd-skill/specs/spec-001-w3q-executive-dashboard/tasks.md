# Work Breakdown — Spec-001: Executive Dashboard
Total Tasks: 5 / Estimated Total Effort: 13h

## Task Dependency Graph
```
#1 useProjects list-only ──┐
#2 chrome tokens + color lock ──┼──► #4 Dashboard page ──► #5 App routing + TabBar delete
#3 formatIndexedAt ──────────┘
```
#1, #2, #3 are parallel. Critical path: #2 (2h) → #4 (5h) → #5 (3h) = 10h if #1/#3 finish in the #2 window.

## Tasks

### Task #1 — useProjects list-only (TD-001)
Definition of Done:
- [x] `useProjects` calls `list_projects` only; no `get_graph_schema` in the hook
- [x] Return type is `{ projects: Project[], loading, error, refresh }`
- [x] `SpecBoardTab` ProjectPicker lists name + path without reading schema
- [x] `useProjects.test.ts`: fetch spy records zero RPC tool names `get_graph_schema`; list still hydrates `indexed_at`
- [x] tests pass locally; breadcrumbs on touched files
User Stories Addressed: US-002 (schema-free list)
Gherkin covered: Two projects — spy clause; supports Error — list_projects RPC fails
Dependencies: None
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Do not keep `schema: null` placeholders if that invites a later fetch. Update any `p.project` destructure. `ProjectCard.tsx` is unused — leave it; do not wire it.

### Task #2 — Grayscale chrome tokens + palette lock (TD-004, US-004)
Definition of Done:
- [ ] `globals.css` `--color-primary` / `--color-accent` / `--color-ring` are not `#1DA27E` / `#1C8585`
- [ ] Surfaces have distinct gray levels (background / card / hover / border) per plan D8
- [ ] Control `Gauge` healthy fill is not `#1DA27E`; >80 red and >50 amber stay
- [ ] App header / modal / HealthDot tooltip / IndexProgress chrome drop hardcoded `#0b1920` / `#0e2028` (or whatever this task can reach; Dashboard files in #4 must use tokens from the start)
- [ ] GraphTab / NodeDetailPanel / DisplaySettingsMenu chrome panels use `bg-card` (not canvas)
- [ ] `colors.ts` unchanged; `colors.test.ts` locks `colorForLabel("Function") === "#06b6d4"` and the rest of `LABEL_COLORS`
- [ ] EdgeLines `CALLS` / default hex still `#1DA27E` / `#1C8585` (export for test; do not change values)
- [ ] graph-loader `#22d3ee` untouched
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-004
Gherkin covered: Graph deep link — `colorForLabel("Function")` hex lock
Dependencies: None
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: HealthDot semantic hex stays. Do not import CSS vars into `colorForLabel`. A file-level test that `globals.css` no longer sets teal primary is enough for chrome; do not screenshot GraphTab in this task.

### Task #3 — formatIndexedAt
Definition of Done:
- [ ] `graph-ui/src/lib/formatIndexedAt.ts` implements plan D2 (UTC locale, fallback raw ISO)
- [ ] `formatIndexedAt.test.ts`: `"2026-08-29T10:00:00Z"` + `en` → visible derived string containing `2026` and `29`, not identical to the raw ISO
- [ ] invalid input returns the raw string
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-002 (freshness display helper)
Gherkin covered: Two projects — datetime derived from `indexed_at` (helper)
Dependencies: None
Estimated Effort: 1h
Subagent: no
Path: compact
Implementation Notes: Do not put JSX here. Lang argument `en` | `zh`. #4 wires `<time dateTime>`.

### Task #4 — Dashboard page: list + Control + create-index
Definition of Done:
- [ ] `Dashboard.tsx` stacks Indexed Projects then full Control on one `ScrollArea` (`max-w-4xl`)
- [ ] Rows: name, `root_path`, `<time dateTime={indexed_at}>` via `formatIndexedAt`, HealthDot, Enter, Delete-with-confirm
- [ ] No ADR control, no aggregate Nodes/Edges cards, no per-row counts, no label chips
- [ ] Page-level New Index + Refresh; empty: "No indexed projects" + "Index your first repository" + Control still visible
- [ ] list error: destructive region, non-empty text, Control still visible
- [ ] IndexProgress on Dashboard while indexing; 202 does not navigate to Graph
- [ ] `CreateIndexModal` extracted; no Project ID field/state; POST body `{ "root_path": path }` only
- [ ] Folder browse, Windows breadcrumbs, filter, Index This Folder behavior preserved (existing tests migrated)
- [ ] `ControlTab` `embedded` — polls 3s/2s remain; gauges Total CPU / Total RAM / Processes / Self RAM; Active Processes; Process Logs
- [ ] `HealthDot` / `IndexProgress` extracted; `AdrButton` extracted and not imported by Dashboard
- [ ] `StatsTab.tsx` removed after extract; tests moved to `Dashboard.test.tsx` / `IndexProgress` / `AdrButton.test.tsx`
- [ ] i18n en+zh: `enter`, `lastIndexed`, keep existing Control/empty copy
- [ ] `Dashboard.test.tsx` covers Gherkin rows below
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-002, US-003, US-005, US-006
Gherkin covered:
- Two projects render identity and freshness only
- Create index posts path only
- Control polls remain on the same screen
- Limit Case — zero projects still shows Control
- Error — list_projects RPC fails
- Error — index POST fails
- Error — delete cancelled
Dependencies: Task #1, Task #2, Task #3
Estimated Effort: 5h
Subagent: yes
Path: full
Implementation Notes: Render `Dashboard` directly in tests (pass `onSelectProject`). App wiring is Task #5. `confirm` stub for delete-cancel. Fake timers for polls. Do not mount `ProjectCard`. After 202, `indexing=true` and URL must not become `tab=graph` (Task #5 asserts URL; this task asserts no `onSelectProject` call on create).

### Task #5 — App routing, header IA, TabBar delete
Definition of Done:
- [ ] `TabId` = `"dashboard" | "graph"`; `readRoute` aliases `stats`/`control`/`specs`/missing/unknown → Dashboard; `graph` without project → Dashboard
- [ ] No header tabs labeled Specs, Graph, Projects, or Control
- [ ] Default / no query shows Dashboard (Indexed Projects or empty CTA + Control Panel)
- [ ] Enter on a row → `?tab=graph&project=<name>` mounts existing `GraphTab`
- [ ] Graph header back control (`aria-label` `t.graph.backToDashboard`) → Dashboard; URL does not require `project=`
- [ ] `?tab=graph&project=alpha` still shows GraphTab for alpha
- [ ] `TabBar.tsx` deleted; no leftover import
- [ ] `SpecBoardTab` not routed
- [ ] `App.test.tsx` covers Gherkin rows below; mock `GraphTab` to avoid Three
- [ ] i18n en+zh: `backToDashboard`
- [ ] tests pass; breadcrumbs
User Stories Addressed: US-001
Gherkin covered:
- Default URL opens Dashboard
- Enter opens existing Graph, back returns to Dashboard
- Limit Case — bookmark tab=stats aliases Dashboard
- Limit Case — bookmark tab=control aliases Dashboard
- Limit Case — graph deep link still works (plus Task #2 color lock)
Dependencies: Task #4
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: `replaceState` on first load writes `?tab=dashboard` without project. Do not build workspace tab strip. Keep `onSelectProject → graph`.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Subagent | Path | Status |
| 1 | useProjects list-only | 2h | None | no | full | done |
| 2 | Chrome tokens + palette lock | 2h | None | no | full | pending |
| 3 | formatIndexedAt | 1h | None | no | compact | pending |
| 4 | Dashboard page | 5h | #1 #2 #3 | yes | full | pending |
| 5 | App routing + TabBar delete | 3h | #4 | no | full | pending |
Total: 13h

## Critical Path
#2 (2h) → #4 (5h) → #5 (3h) = 10h wall if #1 and #3 run in parallel with #2.
Parallelization: start #1, #2, #3 together.

## Implementation Guidance for @implementer
Read order: spec.md (Gherkin is the contract) → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → constitution → tests for that task's Gherkin → clear commit
Blocked: document in state.md Notes, switch task, inform @architect
Do not expand to spec-002/003/004. Do not edit C. Do not call `index_repository`.

## Test Coverage Requirements
Happy path + limit + error Gherkin above. Target >80% on touched graph-ui files. Every scenario maps to at least one task test (see per-task Gherkin covered).

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
