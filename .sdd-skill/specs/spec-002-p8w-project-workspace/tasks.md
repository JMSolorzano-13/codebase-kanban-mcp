# Work Breakdown — Spec-002: Project Workspace
Total Tasks: 5 / Estimated Total Effort: 13h

## Task Dependency Graph
```
#1 route + TabId ──┬──► #2 workspace header ──┐
                   ├──► #3 Specs presence ─────┼──► #5 App compose + Gherkin + delete AdrButton
                   └──► #4 AdrTab ─────────────┘
```
#2, #3, #4 are parallel after #1. Critical path: #1 (2h) → #3 (3h) → #5 (3h) = 8h if #2/#4 finish in the #3 window.

## Tasks

### Task #1 — TabId + readRoute + routeUrl
Definition of Done:
- [x] `TabId` = `"dashboard" | "graph" | "specs" | "adr"`
- [x] `WORKSPACE_TABS` exported as `["graph","specs","adr"]` with a comment that later tabs add an id here (no plugin runtime)
- [x] `graph-ui/src/lib/route.ts` owns `readRoute` / `routeUrl`
- [x] Workspace tab + non-empty `project` → that tab; `stats`/`control`/`dashboard`/unknown/missing/`graph|specs|adr` without project → Dashboard and `project: null`
- [x] `route.test.ts` covers the rows below
- [x] App uses `route.ts` (no duplicated parse). First-load `replaceState` still writes the canonical query
- [x] tests pass locally; breadcrumbs on touched files
User Stories Addressed: US-002 (ids), US-006 (aliases)
Gherkin covered: Error — workspace tab without project is Dashboard (route unit); supports Limit — Dashboard aliases (full assert in #5)
Dependencies: None
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Do not implement Specs fallback-to-Graph here (needs presence, Task #3). Do not build the tab strip. Keep `navigate` in App.

### Task #2 — Workspace header: name + last-indexed + leave
Definition of Done:
- [x] `WorkspaceHeader` shows project name from the route (not only from the list)
- [x] If `useProjects` has a matching `name`, render `<time dateTime={indexed_at} title={indexed_at}>` via `formatIndexedAt` + `useUiLanguage` (same as Dashboard)
- [x] If the name is missing from the list, omit every `time` in the header
- [x] Leave control `aria-label={t.graph.backToDashboard}` calls the provided `onLeave` (App wires confirm in #5)
- [x] Header uses spec-001 grayscale tokens; no `get_graph_schema`, no `/api/index-status`
- [x] `WorkspaceHeader.test.tsx` covers ghost vs listed `indexed_at`
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001 (leave control), US-005
Gherkin covered: Limit Case — indexed_at missing from list omits datetime (component); Enter datetime clause completed in #5
Dependencies: Task #1
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Do not add a project picker. Brand mark stays in App header; this component is the identity cluster (name + time + ×), still inside `<header>` beside the brand.

### Task #3 — Specs presence + tab strip + SpecBoard host
Definition of Done:
- [x] `useSddSkillPresent(project)` one-shot GET `/api/spec-board?project=`; `present === true` only on HTTP 200 and `sdd_skill_present === true`
- [x] Loading, HTTP error, network error, false → `present === false` (omit)
- [x] Does not start a 4s interval (do not reuse `useSpecBoard` for the strip)
- [x] `WorkspaceTabStrip`: `role="tablist"` sibling under `<header>` (not inside it). Tabs `role="tab"` labeled Graph, optional Specs, ADR. Selected: `aria-selected="true"` and `aria-current="page"`
- [x] Order: Graph | Specs? | ADR. No disabled Specs placeholder. No `notSddSkill` in the strip
- [x] `SpecBoardTab` with a non-null `project` never renders `t.specBoard.selectProject`
- [x] App-or-helper: if route tab is `specs` and `present` is not true → `replaceState`/`navigate` to `graph` + same project immediately
- [x] `useSddSkillPresent.test.ts` + `WorkspaceTabStrip.test.tsx` + SpecBoard host test
- [x] tests pass; breadcrumbs
User Stories Addressed: US-002, US-003
Gherkin covered:
- Specs tab appears when sdd-skill is present (strip + no picker; URL click in #5)
- Limit Case — spec-board still loading omits Specs
- Limit Case — specs deep link without sdd-skill falls back to Graph (URL rewrite can be unit-tested here; App in #5)
- Error — spec-board request fails omits Specs
Dependencies: Task #1
Estimated Effort: 3h
Subagent: yes
Path: full
Implementation Notes: Mock `{ sdd_skill_present, specs: [] }` never `columns`. `useSpecBoard` poll stays for the Kanban pane only. Hung fetch: do not resolve the mock Promise. Graph+ADR remain clickable while the presence request is in flight.

### Task #4 — AdrTab pane (replace AdrButton)
Definition of Done:
- [x] `AdrTab` GET `/api/adr?project=` into a textarea; empty/`has_adr:false` → value `""`; placeholder is the current AdrButton headings (`# Architecture Decision Record` / Context / Decision / Consequences)
- [x] No POST until Save. Save POST `{ project, content }`. Treat `!res.ok` as error: visible `role="alert"` non-empty; textarea unchanged
- [x] Success: visible `role="status"`; dirty baseline updates to saved content; success clears on next edit
- [x] Delete visible when `has_adr`; POST `{ project, content: "" }`
- [x] `onDirtyChange(content !== lastClean)` so App can confirm
- [x] No modal overlay. No `CBM-GENERATED` / generated-region marker in markup or saved body
- [x] `AdrTab.test.tsx` covers load/save/empty/500/delete; migrate `AdrButton.test.tsx` delete case
- [x] i18n en+zh: `tabs.adr`, `adr.saveSuccess`, `adr.saveError` (unsaved confirm string can land here or #5)
- [x] tests pass; breadcrumbs
User Stories Addressed: US-004
Gherkin covered:
- ADR tab loads and saves the existing blob (component POST)
- Limit Case — empty ADR shows placeholder and does not persist it
- Error — ADR save fails keeps the draft
Dependencies: Task #1
Estimated Effort: 3h
Subagent: yes
Path: full
Implementation Notes: Check HTTP status (AdrButton did not). Do not import AdrButton. App mount + dirty leave is Task #5. Placeholder stays the English heading string even in zh (Gherkin asserts that text). Delete test asserts POST `{ project, content: "" }` only — do not assert a following GET `has_adr:false` (C upserts empty; row remains).

### Task #5 — App compose, dirty confirm, delete AdrButton, Gherkin
Definition of Done:
- [x] Workspace = header identity (#2) + tab strip (#3) + pane GraphTab | SpecBoardTab | AdrTab
- [x] Enter → `?tab=graph&project=`; default pane Graph
- [x] `requestNavigate` / leave: if ADR dirty, `window.confirm(t.adr.unsavedConfirm)`; dismiss keeps `tab=adr` and draft
- [x] `AdrButton.tsx` and `AdrButton.test.tsx` deleted; no remaining imports; Dashboard still has no ADR control
- [x] spec-001 aliases remain: `stats`/`control`/unknown → Dashboard + Control Panel; no workspace ADR tab
- [x] `App.test.tsx` covers Gherkin rows below; mock GraphTab; mock hung/false/true/500 spec-board as needed
- [x] `colorForLabel` lock remains (existing `colors.test.ts` — do not edit hex)
- [x] i18n en+zh: `adr.unsavedConfirm` if not in #4; `i18n.test.ts` locks new keys
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-006; integration of US-002..005
Gherkin covered:
- Enter opens workspace Graph with last-indexed and Graph+ADR tabs
- Specs tab appears when sdd-skill is present (click + URL + Kanban + no picker)
- ADR tab loads and saves (URL `tab=adr`)
- Leave project returns to Dashboard
- Limit Case — spec-board still loading omits Specs (App)
- Limit Case — specs deep link without sdd-skill falls back to Graph (App)
- Limit Case — Dashboard aliases still home
- Error — unsaved ADR leave cancelled stays on ADR
- Error — workspace tab without project is Dashboard (`?tab=adr`)
- Error — spec-board 500 omits Specs (App)
Dependencies: Task #2, Task #3, Task #4
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: Keep `assertNoAccountHeaderTabs` for Projects/Control (and Specs/Graph as header buttons). Workspace tabs are in the tablist, not header buttons named Graph. Stub `confirm` for dirty-leave. First-load replaceState must not strip a valid workspace query.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Subagent | Path | Status |
| 1 | TabId + readRoute | 2h | None | no | full | done |
| 2 | Workspace header | 2h | #1 | no | full | done |
| 3 | Specs presence + strip | 3h | #1 | yes | full | done |
| 4 | AdrTab pane | 3h | #1 | yes | full | done |
| 5 | App compose + Gherkin | 3h | #2 #3 #4 | no | full | done |
Total: 13h

## Critical Path
#1 (2h) → #3 (3h) → #5 (3h) = 8h wall if #2 and #4 run in parallel with #3.

## Implementation Guidance for @implementer
Read order: spec.md (Gherkin is the contract) → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → constitution → tests for that task's Gherkin → clear commit
Blocked: document in state.md Notes, switch task, inform @architect
Do not expand to spec-003/004. Do not edit C. Do not call `index_repository`.

## Test Coverage Requirements
Happy path + limit + error Gherkin above. Target >80% on touched graph-ui files. Every scenario maps to at least one task test (see per-task Gherkin covered).

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
