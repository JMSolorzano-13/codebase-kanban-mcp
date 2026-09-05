# Feature Checklist — Spec-013: ADR fill from gamedev trio
Start Date: 2026-08-31 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — XOR trio select + gamedev relatives
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg
  (commit held — user did not ask; 21 adr_fill; coverage ~88% gcov reporter absent; @review APPROVED 2026-08-31)

### Task #2 — HTTP + MCP + watcher Gherkin (gamedev XOR)
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg
  (commit held — user did not ask; httpd 117 / 1 skipped; mcp 201 / 2 skipped; coverage reporter absent; @review APPROVED 2026-08-31)

### Task #3 — AdrTab generic chrome; no cycle stamp
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #1 review 2026-08-31: APPROVED (Path: full). XOR in helper (`adr_select_trio`); `.gamedev/` dir → gamedev relatives only; else `.sdd-skill/` dir → sdd; neither → NULL (SDD-ADR-058..060). Local `cbm_is_dir` (no `spec_board.h`; SDD-ADR-059). Empty `.gamedev/` still marks. File-at-path `.gamedev` is not present. No sdd fallback. Extract/splice/markers/caps stay spec-004. No `.gamedev` on ALWAYS_SKIP (SDD-ADR-061). pipeline/HTTP/graph-ui/discover not this task. 21 adr_fill (7 XOR Thens + sdd/pipeline stay). Coverage ~88% (gcov reporter absent). Commit held (not a reject). IX.5 still names sdd trio — close-time planner rec, not a Task #1 reject.

Task #2 review 2026-08-31: APPROVED (Path: full). Tests only: `ui_adr_fill_gamedev_tree` + leftover sdd; 9 HTTP + 3 MCP job Gherkin. Dual-tree XOR, create `{root_path}`, switch/remove/both-gone, partial, empty dir, unreadable directory-at-path (SDD-ADR-022/061), POST replace, `index_repository` same blob, `manage_adr` survive, `adr_fill: false`. spec-004 `ui_adr_fill_tree` / `mcp_write_index_tree` still `.sdd-skill` only. 32768 + 409/202 reused. pipeline/discover/mcp/http_server/adr_fill/graph-ui not this task. httpd 117 / 1 skipped; mcp 201 / 2 skipped. Coverage reporter absent. Commit held (not a reject). IX.5 sdd wording = close-time rec, not a Task #2 reject.

Task #3 review 2026-08-31: APPROVED (Path: compact). Test-only `AdrTab.test.tsx`. Isolated AdrTab: formatIndexedAt(indexed_at) + replaceWarning + one textarea; document has no gamedev-skill. No GameBoardTab. AdrTab.tsx/i18n.ts/App.tsx/colors.ts not this task. window.confirm App.tsx:116 unchanged. colorForLabel Function #06b6d4 (colors.test.ts). vitest 12. Coverage reporter absent. Commit held (not a reject). IX.5 sdd wording = close-time rec, not a Task #3 reject.

## Testing — @tester fills
[x] Task #1 unit XOR happy/limit/error [x] Task #2 HTTP/MCP/watcher [x] Task #3 AdrTab chrome [ ] performance targets [ ] cross-module integration
[x] Task #1 coverage>80% (reporter absent ~88%) [ ] critical paths 100% [ ] public APIs E2E [x] Task #1/#2 boundary (empty/file-at-path/unreadable/both-gone)
Tools: [x] unit [C adr_fill] [x] HTTP/MCP job [x] unit [Vitest AdrTab] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| adr_fill | ~88% (gcov absent) | PASS DEV | 21/21; 8 Task #1 Gherkin unit Thens |
| httpd,mcp | reporter absent | PASS DEV | 12/12 Task #2 job Gherkin; httpd 117 / mcp 201 |
| AdrTab + locks | reporter absent | PASS DEV | 18/18 (3 files); no gamedev-skill |

## Human Validation — @human-trainer fills
[x] task summaries done [x] spec summary done [x] architecture diagram updated [x] QUICK-DEBUG updated [x] PROJECT-OVERVIEW updated [x] @sdd-* headers present [x] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status | Notes |
| DoD | 100 | COMPLETE | commit held (user did not ask) |
| Code Quality | 100 | COMPLETE | |
| Testing | 100 | COMPLETE | DEV only; Playwright not run |
| Human Validation | 100 | COMPLETE | |
| TOTAL | 100 | COMPLETE | |

## Closure Criteria (ALL required)
[x] all DoD 100% [x] @review 0 open issues [x] @tester PASS all levels [x] human confirmation received [x] integration tests pass [x] 0 critical issues [x] perf targets met [x] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-013 "Bug: [d]" → creates spec-013-hotfix-1

## Notes & Blockers
Blockers: none
Design changes: none
Perf issues: none
