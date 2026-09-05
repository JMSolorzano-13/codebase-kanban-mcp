# Work Breakdown — Spec-013: ADR fill from gamedev trio
Total Tasks: 3 / Estimated Total Effort: 8h

## Task Dependency Graph
```
#1 XOR trio in adr_fill ──► #2 HTTP/MCP/watcher Gherkin
                       └──► #3 AdrTab no-cycle-stamp
```
#3 is parallel after #1 (needs no C change if warning already exists). Critical path: #1 (3h) → #2 (4h) = 7h if #3 finishes in the #2 window.

## Tasks

### Task #1 — XOR trio select + gamedev relatives
Definition of Done:
- [x] `cbm_adr_fill_document` selects trio by directory: `.gamedev/` dir → gamedev relatives only; else `.sdd-skill/` dir → existing sdd relatives; neither → NULL
- [x] Gamedev relatives are exactly `.gamedev/game_context.md`, `.gamedev/baseline/TECH_STACK.md`, `.gamedev/baseline/ARCHITECTURE_ADR.md`
- [x] When gamedev dir: do not fopen any `.sdd-skill/` trio path
- [x] Empty `.gamedev/` dir (trio all missing): markers written; generated has no extracts; leftover sdd strings absent
- [x] File-at-path `.gamedev` is not present; sdd fill applies if `.sdd-skill/` is a dir
- [x] Missing/empty/unreadable gamedev file omits that H1 only; no sdd fallback
- [x] Extract window, H1 titles, splice, markers, DEV_LOG/GDD-never-opened stay spec-004
- [x] No `#include` of `spec_board.h`. Local `cbm_is_dir` only
- [x] Header comment: NULL = neither skill dir
- [x] Existing sdd-only `test_adr_fill.c` cases stay green
- [x] New C unit Thens below pass without a full index
- [x] tests pass; breadcrumbs; `@sdd-spec` / `@sdd-decision` updated on `adr_fill.*`
User Stories Addressed: US-001, US-002, US-004, US-005
Gherkin covered (unit-level Then):
- dual-tree: PURPOSE/STACK/DECISION-GAME present; PURPOSE/STACK/DECISION-SDD and SECRET-GDD/DEVLOG absent; unmarked existing becomes manual
- only TECH_STACK: `# Stack` present; `# Purpose` / `# Decisions` / PURPOSE-SDD-FALLBACK absent
- empty `.gamedev/` dir: markers; PURPOSE-SDD-EMPTYDIR absent
- file-at-path `.gamedev` + sdd trio: sdd extract present
- neither dir: NULL / no markers
- extract tail: DECISION-HEAD present; DECISION-TAIL after 2000 bytes absent
- unreadable `game_context.md`: STACK/DECISION readable present; `# Purpose` and PURPOSE-SDD-UNREADABLE absent
- sdd-only regression (no `.gamedev/`): existing PURPOSE-ALPHA-GRAPH style still fills
Dependencies: None
Estimated Effort: 3h
Subagent: yes
Path: full
Implementation Notes: Keep `adr_read_extract` / `adr_splice` / caps. Add `af_write_gamedev_trio`. Unreadable = `af_make_unreadable`. Do not touch pipeline, discover, HTTP, or graph-ui. Do not add `.gamedev` to ALWAYS_SKIP.

### Task #2 — HTTP + MCP + watcher Gherkin (gamedev XOR)
Definition of Done:
- [x] Successful POST `/api/index` `{root_path, project}` Reindex on a dual-tree fills gamedev extracts and omits leftover sdd unique strings; GDD/DEV_LOG absent; manual preserved; trio files byte-identical
- [x] Successful `index_repository` fills the same store GET `/api/adr` / `manage_adr` get read
- [x] First create-index bare `{root_path}` of a new Path with `.gamedev/` writes generated + empty/whitespace manual
- [x] Former sdd blob + add `.gamedev/` + one Reindex overwrites generated; manual `# Keep notes` stays
- [x] No `.gamedev/` still fills sdd (existing spec-004 HTTP stays green)
- [x] `.gamedev/` removed + sdd remains: next Reindex restores sdd
- [x] Both skill dirs gone: last ADR blob unchanged (no new CBM-GENERATED if unmarked leftover)
- [x] Watcher / `adr_fill: false` does not insert gamedev strings or markers
- [x] `manage_adr` manual update survives the next user-triggered index on a gamedev tree
- [x] POST `/api/adr` generated-region edit is replaced on next Reindex (`PURPOSE-CANONICAL-GAME`)
- [x] Unreadable `game_context.md`: job success; Purpose omitted; no sdd fallback
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-003, US-005, US-006
Gherkin covered:
- Dashboard Reindex dual-tree
- MCP index_repository same blob
- First create-index .gamedev/
- Adding .gamedev/ overwrites in one job
- Limit — no .gamedev/ still sdd
- Limit — only TECH_STACK + no sdd fallback (HTTP if not fully proven in #1)
- Limit — empty .gamedev/ no fallback
- Limit — remove .gamedev/ restores sdd
- Limit — both dirs gone leaves blob
- Limit — watcher incremental does not fill
- Error — unreadable game_context; job success
- Error — manage_adr survives
- Error — generated-region edits replaced
Dependencies: Task #1
Estimated Effort: 4h
Subagent: no
Path: full
Implementation Notes: Add `ui_adr_fill_gamedev_tree` (and leftover sdd writers) next to `ui_adr_fill_tree`. Reuse `ui_adr_fill_server_start` / `http_wait_index_done` / `http_seed_adr`. Do not change `adr_fill` flag, watcher args, admission 409/202, or POST `/api/adr` 32768. Do not edit `discover.c`. Pipeline/mcp/application stay unless a proven hook bug (not expected).

### Task #3 — AdrTab generic chrome; no cycle stamp
Definition of Done:
- [x] Existing generated-at + replace warning stay (en+zh copy unchanged)
- [x] Isolated AdrTab render with markers: visible datetime from `indexed_at`; warning present; one textarea
- [x] That render's document does not contain the substring `gamedev-skill`
- [x] `colorForLabel("Function")` still `#06b6d4` if that test is in the suite run
- [x] Dirty-leave `window.confirm` unchanged
- [x] `AdrTab.tsx` / `i18n.ts` not edited unless a test cannot be added without a no-op (prefer test-only)
- [x] tests pass; breadcrumbs on any edited file
User Stories Addressed: US-006
Gherkin covered:
- ADR tab keeps the generic generated-at and replace warning
Dependencies: Task #1
Estimated Effort: 1h
Subagent: no
Path: compact
Implementation Notes: Extend the existing stamp/warning test. Do not mount GameBoardTab. Do not add "from gamedev-skill" / "from sdd-skill" chrome.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | XOR trio select + gamedev relatives | 3h | None | done |
| 2 | HTTP/MCP/watcher Gherkin | 4h | #1 | done |
| 3 | AdrTab no-cycle-stamp | 1h | #1 | done |
Total: 8h

## Critical Path
#1 → #2 (7h). #3 parallel after #1.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD
Blocked: document in state.md Notes, switch task, inform @architect
Commit: only if the user asks.

## Test Coverage Requirements
Every Gherkin scenario mapped above. Target >80% on touched C fill. Keep spec-004 sdd fixtures green.

## Success Criteria for All Tasks
- [ ] all DoD complete [ ] tests>80% [ ] passes @review [ ] @tester approves [ ] human docs complete
