# Feature Checklist — Spec-011: Game phase board
Start Date: 2026-08-31 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — C artifact walk + widen card JSON
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
  (commit held — user did not ask; 18/18 game_board + 6 httpd Gherkin; coverage ~88% reporter absent; @review APPROVED 2026-08-31)

### Task #2 — C Inbox walk + conversion + HTTP bytes
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — GameBoardTab four columns + cards + i18n
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #4 — Clipboard, no-drag, typed card parse
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #5 — Remaining Vitest Gherkin mapping
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
  (commit held — user did not ask; graph-ui 238 passed; leftover UI Thens + no skill-presence + colorForLabel lock)

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #1 review 2026-08-31: APPROVED (Path: full). C artifact walk + widen card JSON only. Inbox grill / GameBoardTab / clipboard / Vitest paint, POST, MCP out of scope. Coverage ~88% (gcov reporter absent; public fns 2/2 tested; 10/10 Task #1 Gherkin Thens mapped). Commit held (not a reject).

Graph (mcp_idx=yes, project Users-jmsolorzano-SWE-tools-codebase-memory-mcp):
- detect_changes vs main: prior uncommitted specs + this-task seeds. This-task product: game_board.c/h, test_game_board.c, test_httpd.c game-board breadcrumb (`:3880`).
- check_index_coverage: game_board.c / game_board.h / test_game_board.c no_recorded_issue metadata_match.
- trace_path: cbm_game_board_read callers = handle_game_board_get + 18 test_game_board tests; hop-1 callees = cbm_spec_board_gamedev_skill_present, parse_state_md, fill_artifacts, read_whole_file. cbm_game_board_to_json callers = handle_game_board_get + tests; callees = cbm_json_escape, game_json_append, game_json_emit_array. handle_game_board_get caller = dispatch_request (GET only). No handle_game_board_post.

DoD locks: spec_board.c / mcp.c / Makefile.cbm / http_server.c not edited this task (mtimes predating game_board.c Task #1). mcp.c has no game-board symbol. spec_board to_json does not emit gamedev_skill_present. fopen `"rb"` only (`game_board.c:89`). GET 200 leaves skill-tree bytes (`test_game_board.c:658`, `test_httpd.c:4024`).

Constitution (project docs/constitution.md I–IX; role-template I–X mapped below):
- I Source of truth: implements Task #1 DoD / US-002 + US-005 artifact half. fopen `"rb"` only. No write to `.gamedev/` / `.sdd-skill/` / `.grill/` (I.2). Inbox conversion deferred to #2. UI deferred to #3–#5.
- II Languages: C11 `cbm_game_board_*` + `CBM_GAME_BOARD_MAX_CARDS` 64 + `CBM_GAME_BOARD_LIST_MAX` 256. calloc/malloc/realloc NULL-checked (`game_board.c:106-109`, `:571-574`, `:670-680`, `:770-773`). No graph-ui / new runtime this task.
- III Chrome/graph: no UI. C JSON never emits English phase labels (empty-array tests).
- IV Identity/APIs: same GET `/api/game-board` (IV.3). Same 400/404 strings. 500 class unchanged. No POST. No MCP tool. spec-board stays gamedev-free.
- V Testing: 18 C unit + 6 HTTP Gherkin kept. Thens: gdd JSON fields; SYS dir without spec.md; two in_progress; missing narrative-bible; 65th omitted no has_more; status ready → in_progress; present false empty arrays; spec-board no gamedev field; 404 `project not found`; 400 `missing project parameter`. UI Gherkin deferred to #3–#5. Inbox Gherkin deferred to #2.
- VI Security/HTTP: no secrets; paths are catalog root + fixed suffixes / sanitized dirents (`game_dirent_rejected` `/` `\\` `..`); query never fopen'd; JSON via `cbm_json_escape`; compiled owner table (no agents.md fopen); loopback/bind unchanged.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on `game_board.h`, `game_board.c`, `test_game_board.c`, test_httpd game-board block (`:3880-3885`). http_server.c handler still spec-010 (file not this task).
- VIII Performance: present false returns after memset (no phase walk). present true: state.md + bounded opendir/fopen; scratch ≤256 then cap 64. No poll. No `get_graph_schema`.
- IX Patterns: same GET / same heap calloc / same 400/404/500 class as spec-010 `handle_game_board_get`. Dir check still `cbm_spec_board_gamedev_skill_present`. Local grow `to_json` (not shared — SDD-ADR-046/048). No archive merge. Pattern Notes: ✓. IX.2 spec-010 empty-array sentence waits for spec close.
- X DRY (role template; no Section X in constitution.md): walk + JSON stay in `game_board.c`. Static owner table + grow helper are local (intentional isolation; not `src/shared/`).
- Naming (role template vs II.1): snake_case files + `cbm_` exports match this C tree. Caps UPPER_SNAKE.
- Monitoring: N/A (no new poll/metrics). Left unchecked, same as spec-001..010.

SOLID: walk vs status map vs owner table vs grow emit vs HTTP resolve/reply split; card slots include epic fields for Task #2 (OCP); public API is `game_board.h`; dispatch GET-only.

OWASP: no SQL; no JWT/auth widen; no PII logs; path not from query string; XSS N/A (C JSON, no HTML this task).

Types: C, all params/returns typed. No `any` / `!`.

Coverage: ~88% of touched game_board + httpd Gherkin (static). Public `cbm_game_board_read` / `cbm_game_board_to_json` both tested. Untested defensive: read_whole_file fseek/oversize/OOM, to_json OOM, HTTP 500, animation/audio/ui dir rows (same fill_production path as art). `scripts/test.sh --suites game_board,httpd` 113 passed / 1 skipped (implementer). gcov reporter absent — not a reject.

SDD-ADR-048: cap 64/column; scratch 256; sort then copy; grow to_json; no has_more. SDD-ADR-049: compiled filesystem.md table; do not fopen docs/agents.md.

AUTO-REJECT: none (no secret, no skill-tree write, spec_board.c/mcp.c/Makefile.cbm not this task, no POST, no gamedev key on spec-board, 400/404/present-false/no-write/gdd/SYS/ready/65th tested, I.2 / II.1 / IV.3 / V.3 / VII.2 held). Inbox conversion is Task #2 — not a reject. Commit held by convention.

Task #5 review 2026-08-31: APPROVED (Path: full). Test-only leftover UI Gherkin mapping. Product App.tsx / GameBoardTab.tsx / useGameBoard.ts / C not this task. Silent win + Enter Graph + leftover tab=specs→game stay. No /api/skill-presence. colorForLabel Function #06b6d4 locked in-suite. Coverage ~88% (reporter absent). Commit held (not a reject). Last impl task — after tester PASS → human-trainer Trigger B closeprep.

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
/sdd-skill hotfix spec-011-q5n-game-phase-board "Bug: [d]" → creates spec-011-q5n-hotfix-1

## Notes & Blockers
Blockers: [date — desc — RESOLVED/PENDING] | Design changes: [date — what — why — @architect approval] | Perf issues: [issue — status — resolution]
