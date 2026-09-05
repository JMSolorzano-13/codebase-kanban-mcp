# Feature Checklist — Spec-017: Game debt chrome
Start Date: 2026-09-03 / Target Close Date: 2026-09-03

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — game_board debt parse + additive JSON
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — HTTP GET additive + POST leftover locks
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — GameBoardTab strip + leftover Vitest
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [x] monitoring — Tasks #1–#3
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

## Testing — @tester fills
[x] happy paths (all US) [x] edge cases (all EC-NNN) [x] error conditions [x] performance targets [x] cross-module integration
[x] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [C + Vitest] [ ] E2E [Playwright] [x] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| game_board | reporter absent | PASS | 74; Task #1 parse/JSON |
| httpd | reporter absent | PASS | 131 / 1 skipped; Task #2 leftover |
| graph-ui vitest | reporter absent | PASS | 302 / 25 files; Task #3 strip |

## Human Validation — @human-trainer fills
[x] task summaries done [x] spec summary done [x] architecture diagram updated [x] QUICK-DEBUG updated [x] PROJECT-OVERVIEW updated [x] @sdd-* headers present [x] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status(IN PROGRESS/BLOCKED/COMPLETE) | — DoD, Code Quality, Testing, Human Validation, TOTAL

## Closure Criteria (ALL required)
[ ] all DoD 100% [ ] @review 0 open issues [ ] @tester PASS all levels [ ] human confirmation received [ ] integration tests pass [ ] 0 critical issues [ ] perf targets met [ ] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-017 "Bug: [d]" → creates spec-017-hotfix-1

## Notes & Blockers
Blockers: | Design changes: | Perf issues:
