# Feature Checklist — Spec-015: Specs debt and path
Start Date: 2026-09-02 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — spec_board debt parse + additive JSON
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — HTTP GET additive + POST leftover locks
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — SpecBoardTab strip + EpicCard wrap + i18n
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #4 — leftover Vitest locks
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [ ] nomenclature [ ] API standards [ ] security [ ] performance(API<200ms,DB<50ms) [ ] testing [ ] documentation [ ] error handling [ ] monitoring
SOLID: [ ] SRP [ ] OCP [ ] LSP [ ] ISP [ ] DIP
OWASP: [ ] injection [ ] auth/JWT [ ] sensitive data [ ] XSS [ ] access control [ ] dependency vulns
Type safety: [ ] no bare `any` [ ] params typed [ ] returns typed [ ] no unwarranted `!`

## Testing — @tester fills
[x] happy paths (Task #1 parse + Task #2 GET 200 + Task #3 strip + Task #4 header/Companion-to) [x] edge cases (cap 16, grill-only, Graph/ADR/Game omit, wrap) [x] error conditions (unreadable, POST epic 404, dead row) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% (reporter absent) [x] Task #1–#4 critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/missing/17th/64-epic)
Tools: [x] unit [C + Vitest] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| spec_board | reporter absent | PASS DEV | 11/11 Task #1 Gherkin unit Thens |
| httpd | reporter absent | PASS DEV | 5/5 Task #2 HTTP Gherkin; 119 passed / 1 skipped |
| SpecBoardTab + i18n | reporter absent | PASS DEV | 6/6 Task #3 UI Gherkin |
| leftover Vitest | reporter absent | PASS DEV | 6/6 Task #4 leftover UI; graph-ui 25 files, 290 passed |

Task #4 DEV 2026-09-02 (Unit; Playwright not run — LEVEL 1):
[x] happy paths (Task #4 Gherkin) [x] edge cases (Task #4 Gherkin) [x] error conditions (dead row) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] Task #4 critical paths 100% [ ] public APIs E2E [x] boundary (grill-only / 17th / Graph-ADR-Game)
Tools: [x] unit [Vitest] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| WorkspaceHeader + SpecBoardTab + App + GameBoardTab | 6/6 leftover Thens | PASS DEV | names match Task #4 covered list; Function #06b6d4 stay |

## Human Validation — @human-trainer fills
[ ] task summaries done [ ] spec summary done [ ] architecture diagram updated [ ] QUICK-DEBUG updated [ ] PROJECT-OVERVIEW updated [ ] @sdd-* headers present [ ] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status(IN PROGRESS/BLOCKED/COMPLETE) | — DoD, Code Quality, Testing, Human Validation, TOTAL

## Closure Criteria (ALL required)
[ ] all DoD 100% [ ] @review 0 open issues [ ] @tester PASS all levels [ ] human confirmation received [ ] integration tests pass [ ] 0 critical issues [ ] perf targets met [ ] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-015 "Bug: [d]" → creates spec-015-hotfix-1

## Notes & Blockers
Blockers: | Design changes: | Perf issues:
