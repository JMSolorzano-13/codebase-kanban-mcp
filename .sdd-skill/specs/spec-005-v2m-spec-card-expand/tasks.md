# Work Breakdown — Spec-005: Spec card expand
Total Tasks: 4 / Estimated Total Effort: 12h

## Task Dependency Graph
```
#1 blurb extract helper ──► #2 enrich all + dual done matcher ──► #3 SpecCard expand + Set + filter
                                                              └──► #4 Vitest Gherkin + no Archive
```
#4 is sequential after #3 (needs expand body + filter). Critical path: #1 (3h) → #2 (4h) → #3 (3h) → #4 (2h) = 12h.

## Tasks

### Task #1 — Blurb extract helper + entry.blurb + JSON
Definition of Done:
- [x] `cbm_spec_board_entry_t` has `char blurb[512]` (`CBM_SPEC_BOARD_BLURB_MAX`)
- [x] Public `cbm_spec_board_extract_blurb(const char *spec_md, char *out, size_t outsz)` in `spec_board.h` — NULL/empty in → `out[0] = 0`
- [x] Body is `## Executive Summary` until next `\n## `; not H1; not `###`; `## KPI` never copied
- [x] First 1–2 sentences (`.?!` + isspace or EOS); third sentence dropped; then byte-truncate to `outsz-1` with last-space walkback
- [x] `[text](url)` → `text`; newlines in kept text collapse to a single space
- [x] `cbm_spec_board_to_json` emits `"blurb"` (escaped) on every spec; missing extract is `""`
- [x] C tests cover the Then clauses below without HTTP
- [x] tests pass locally; breadcrumbs
User Stories Addressed: US-002
Gherkin covered (unit-level Then):
- Limit — KPI section is not used as blurb (extract string + optional fixture)
- Limit — empty Executive Summary → `""`
- Error — unreadable/missing md → extract not called / empty (full-tree unreadable is Task #2)
Dependencies: None
Estimated Effort: 3h
Subagent: yes
Path: full
Implementation Notes: Do not loop all specs yet. Existing fixtures may emit `"blurb":""` — keep them green. Do not raise caps. Do not write skill files. Pattern is the same best-effort parse as `read_spec_title`.

### Task #2 — Enrich every listed spec + dual done matcher
Definition of Done:
- [x] After `read_active_json` + active append, every listed entry gets title + blurb from one `spec.md` open and tasks from `tasks.md`
- [x] Active still gets `state.md`, checklist, `current` task, agent, blocked (unchanged fields)
- [x] Non-active: no agent/checklist/`current` fill; `title`/`blurb`/`tasks` filled when files exist
- [x] `history/test_results.log` opened once per `cbm_spec_board_read` and applied to all entries
- [x] Active done matcher stays bare `Task #N` last-line-wins (existing compact fixture still PASS/FAIL)
- [x] Non-active done: line must contain spec id + `Task #N`; last such line wins; `PASS` required for `done==true`
- [x] Missing/unreadable `spec.md` → that entry `title` `""` `blurb` `""`; siblings unchanged; `sdd_skill_present` stays true
- [x] Missing `tasks.md` → `task_count` 0 and empty `tasks[]`
- [x] Zero writes (no `fopen` write, no skill mutation)
- [x] Idle test comment updated (non-active without files still `task_count==0`)
- [x] C tests cover the Then clauses below
- [x] tests pass; breadcrumbs
User Stories Addressed: US-003 (done flags), US-005 (full task list data), US-006
Gherkin covered:
- Limit — KPI section is not used as blurb (full `cbm_spec_board_read`)
- Limit — non-active done requires spec id in the log line
- Error — missing spec.md degrades one entry (200 / present true)
- Error — missing tasks.md is empty task list
- Error — unreadable spec.md omits blurb (`chmod 0` or directory-at-path)
Dependencies: Task #1
Estimated Effort: 4h
Subagent: no
Path: full
Implementation Notes: `handle_spec_board` needs no edit if it already calls read+to_json. Do not add HTTP tests unless a C JSON assert is insufficient. Do not raise 64/48. Do not cache mtime. Do not parse checklist for non-active.

### Task #3 — SpecCard expand Set + blurb region + TaskList filter
Definition of Done:
- [x] `SpecBoardEntry.blurb: string` in `types.ts` (missing treated as `""`)
- [x] `SpecBoardTab` holds `expandedIds: Set<string>`; `project` change clears + reseeds; first board seeds active ids
- [x] Poll / new `board` object does not clear the Set
- [x] Title `<button>` is the only expand control; `canExpand` true for every listed card including `task_count === 0`
- [x] Deleted `useEffect(() => setExpanded(entry.active), [entry.active])`
- [x] Expand body: blurb text region only when `blurb` is non-empty; then TaskList
- [x] Todo TaskList: `done === false` only; zero pending (incl. `task_count` 0) → existing `t.specBoard.noTasksYet`
- [x] In Progress / Done: all tasks + existing current/done/pending chrome; `#N` + name
- [x] Active chrome (agent, N/M, checklist, blocked) stays outside the task list, collapsed or expanded
- [x] No overlay/modal/new page; no Archive/Unarchive control
- [x] No new i18n key unless a string is truly new (prefer `noTasksYet`)
- [x] Component tests for expand + filter + empty blurb + zero tasks (full Gherkin table is Task #4)
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002 (UI), US-003, US-004, US-005
Gherkin covered (implementation + partial tests):
- Todo pending-only
- In Progress first paint expanded + chrome
- Done full list
- empty ES omits blurb region
- zero tasks + no-tasks copy
Dependencies: Task #2
Estimated Effort: 3h
Subagent: no
Path: full
Implementation Notes: Mock `useSpecBoard`. Do not change `useSpecBoard` interval. Do not touch `formatIndexedAt` or Graph colors. Do not put workspace tabs or last-indexed in this task.

### Task #4 — Vitest Gherkin + poll persist + no Archive
Definition of Done:
- [x] `SpecBoardTab.test.tsx` (or sibling) maps every UI Gherkin Then below
- [x] Existing host tests: no `selectProject` when `project` is set; loading / not-sdd-skill still hold
- [x] Poll: rerender with a new `board` object (same ids); opened non-active card still shows its blurb
- [x] Multi-expand: two cards show blurbs at once
- [x] Document has no control named Archive or Unarchive
- [x] In Progress first paint: expanded, blurb, both tasks, agent text, `1/2 tasks` (use existing `tasksDone` i18n)
- [x] `colorForLabel("Function") === "#06b6d4"` still locked if that test is in the suite run
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-005 (no Archive)
Gherkin covered:
- Todo card expands with blurb and pending tasks only
- In Progress card shows blurb and every task with status chrome
- Done card expands with full task list and no Archive
- Two cards stay expanded at once
- Limit — empty Executive Summary omits blurb and still expands
- Limit — zero tasks still expands with no-tasks copy
- Limit — poll does not collapse an opened non-active card
Dependencies: Task #3
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: English assertions. Click the title control that contains the spec id text. Do not boot GraphTab/Three. Do not add Playwright unless @tester later requires CERTIFICATION.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | Blurb extract helper + JSON | 3h | None | done |
| 2 | Enrich all + dual done matcher | 4h | #1 | done |
| 3 | SpecCard expand + Set + filter | 3h | #2 | done |
| 4 | Vitest Gherkin + no Archive | 2h | #3 | done |
Total: 12h

## Critical Path
#1 → #2 → #3 → #4 (12h). No parallel branch after #2 (UI needs the JSON field and done flags).

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `spec_board.c` + `SpecBoardTab.tsx`. C owns extract/matcher/degrade. Vitest owns expand/filter/multi-open/poll/Archive-absent.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
