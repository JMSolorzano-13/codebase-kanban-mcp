# Feature Checklist — Spec-010: Game tab silent win
Start Date: 2026-08-31 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — C/HTTP GET /api/game-board contract
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
  (commit held — user did not ask; 9/9 game_board + 5/5 httpd Gherkin; 103 passed / 1 skipped; coverage ~88% reporter absent; @review APPROVED 2026-08-31)

### Task #2 — TabId, route kernels, strip, i18n
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — GameBoardTab chrome only
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #4 — App silent-win, dual fetch, deep-links
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #5 — Remaining Vitest Gherkin mapping
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #1 review 2026-08-31: APPROVED (Path: full). C/HTTP GET /api/game-board only. graph-ui TabId/strip/GameBoardTab/App, POST, MCP out of scope. Coverage ~88% (gcov reporter absent; public fns 2/2 tested; 5/5 Task #1 Gherkin Thens mapped). Commit held (not a reject).

Graph (mcp_idx=yes, project Users-jmsolorzano-SWE-tools-codebase-memory-mcp):
- detect_changes vs main: 88 files (prior uncommitted specs). This-task seeds: game_board.c/h, http_server.c handler+dispatch, Makefile.cbm UI_SRCS/TEST_UI_SRCS, test_game_board.c, test_httpd.c game-board block.
- check_index_coverage: game_board.c/h no_recorded_issue metadata_match. http_server.c parse_partial 2057 (pre-existing json_unique_member; not this task).
- trace_path: handle_game_board_get caller = dispatch_request; callees = resolve_project_root_path, cbm_game_board_read, cbm_game_board_to_json. cbm_game_board_read callers = handle_game_board_get + 9 test_game_board tests; hop-1 callee cbm_spec_board_gamedev_skill_present. cbm_game_board_to_json callers = handle_game_board_get + 4 tests; callee cbm_json_escape. dispatch_request outbound includes handle_game_board_get; no handle_game_board_post. cbm_spec_board_gamedev_skill_present inbound = cbm_game_board_read + handle_skill_presence + spec_board_gamedev_presence — not cbm_spec_board_read / to_json.

DoD locks: spec_board.c read/to_json and mcp.c not edited this task (vs HEAD they carry prior specs; no game-board symbol in mcp.c; to_json keys stay sdd/grill/specs/epics). discover.c ALWAYS_SKIP not given .gamedev. fopen `"rb"` only (`game_board.c:32`). GET 200 leaves skill-tree bytes (`test_game_board.c:270`, `test_httpd.c:3999`).

Constitution (project docs/constitution.md I–IX; role-template I–X mapped below):
- I Source of truth: implements Task #1 DoD / US-005 + US-006 C/HTTP half. fopen `"rb"` only. No write to `.gamedev/` / `.sdd-skill/` / `.grill/` (I.2). UI deferred to #2–#5.
- II Languages: C11 `cbm_game_board_*` + `CBM_GAME_BOARD_MAX_CARDS` 64. calloc/malloc NULL-checked (`http_server.c:545-548`, `game_board.c:49-53`, `:231-234`). No graph-ui / new runtime this task.
- III Chrome/graph: no UI. C JSON never emits English phase labels (`game_board.c:235-238`; tests `:71-72`, `:163`).
- IV Identity/APIs: new GET `/api/game-board` required — spec-board stays gamedev-free (IV.3, SDD-ADR-045). Same 400/404 strings as spec-board (`:535` / `:541` vs `:492` / `:498`). 500 class `out of memory` / `board serialization failed`. No POST. No MCP tool.
- V Testing: 9 C unit + 5 HTTP Gherkin. Thens: 200 present true empty arrays; spec-board no gamedev field; 404 `project not found`; 400 `missing project parameter`; GET bytes identical + missing trees not created. present-false JSON + compact/alias/unknown-token/unreadable covered in test_game_board. UI Gherkin deferred to #2–#5.
- VI Security/HTTP: no secrets; path is catalog root + fixed suffix `.gamedev/state.md` (`:195`); query/body never fopen'd; JSON via `cbm_json_escape`; loopback/bind unchanged.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on `game_board.h`, `game_board.c`, `test_game_board.c`, handler (`http_server.c:522-530`), test_httpd game-board block (`:3880-3885`). Makefile.cbm two list lines, no header (build file; same habit as prior specs).
- VIII Performance: one dir stat + optional one state.md fopen; arrays stay 0. No poll. No `get_graph_schema`.
- IX Patterns: same heap calloc + 400/404/500 class as `handle_spec_board_get` (`:488-518`). Dir check reuses `cbm_spec_board_gamedev_skill_present` (no second `.gamedev` stat). No archive merge. Pattern Notes: ✓. IX.2 spec-009 Specs predicate unchanged this task (AND NOT gamedev is App #4).
- X DRY (role template; no Section X in constitution.md): dedicated `game_board.c` so spec_board read/to_json stay gamedev-free. Static `kv_extract` mirrors spec_board (intentional isolation; not `src/shared/`).
- Naming (role template vs II.1): snake_case files + `cbm_` exports match this C tree. Caps UPPER_SNAKE.
- Monitoring: N/A (no new poll/metrics). Left unchecked, same as spec-001..009.

SOLID: read vs to_json vs HTTP resolve/reply split; struct slots empty for epic 002 (OCP); public API is `game_board.h`; dispatch GET-only.

OWASP: no SQL; no JWT/auth widen; no PII logs; path not from query string; XSS N/A (C JSON, no HTML this task).

Types: C, all params/returns typed. No `any` / `!`.

Coverage: ~88% of touched game_board + httpd Gherkin (static). Public `cbm_game_board_read` / `cbm_game_board_to_json` both tested. Untested defensive: read_whole_file fseek/oversize/OOM, to_json OOM/overflow, HTTP 500. `scripts/test.sh --suites game_board,httpd` 103 passed / 1 skipped. gcov reporter absent — not a reject.

SDD-ADR-044: compact `phase=` / `focus=` prefer; `active_phase` / `director_focus` `=` or line-start `:`; unknown token → JSON null. SDD-ADR-045: dedicated heap struct + new GET; arrays `[]`; no POST/MCP; spec-board gamedev-free.

AUTO-REJECT: none (no secret, no skill-tree write, spec_board read/to_json and mcp.c not this task, no POST, no gamedev key on spec-board, 400/404/empty arrays/no-write tested, I.2 / II.1 / IV.3 / V.3 / VII.2 held). Commit held by convention.

## Testing — @tester fills
[ ] happy paths (all US) [ ] edge cases (all EC-NNN) [ ] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [ ] critical paths 100% [ ] public APIs E2E [ ] boundary cases (empty/null/max)
Tools: [ ] unit [Vitest + C] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |

## Human Validation — @human-trainer fills
[ ] task summaries done [ ] spec summary done [ ] architecture diagram updated [ ] QUICK-DEBUG updated [ ] PROJECT-OVERVIEW updated [ ] @sdd-* headers present [ ] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status(IN PROGRESS/BLOCKED/COMPLETE) |
| DoD | 0 | IN PROGRESS |
| Code Quality | 0 | IN PROGRESS |
| Testing | 0 | IN PROGRESS |
| Human Validation | 0 | IN PROGRESS |
| TOTAL | 0 | IN PROGRESS |

## Closure Criteria (ALL required)
[ ] all DoD 100% [ ] @review 0 open issues [ ] @tester PASS all levels [ ] human confirmation received [ ] integration tests pass [ ] 0 critical issues [ ] perf targets met [ ] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-010-c4h-game-tab-silent-win "Bug: [d]" → creates spec-010-c4h-hotfix-1

## Notes & Blockers
Blockers: [date — desc — RESOLVED/PENDING] | Design changes: [date — what — why — @architect approval] | Perf issues: [issue — status — resolution]
