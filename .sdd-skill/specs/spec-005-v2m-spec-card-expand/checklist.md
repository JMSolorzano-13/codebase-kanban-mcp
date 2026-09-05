# Feature Checklist — Spec-005: Spec card expand
Start Date: 2026-08-30 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — Blurb extract helper + JSON
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — Enrich all + dual done matcher
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — SpecCard expand + Set + filter
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #4 — Vitest Gherkin + no Archive
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [x] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`
Task #1 review 2026-08-30: APPROVED. C extract/json only — JWT/DB/UI N/A. Coverage ~86% extract/json.
Task #2 review 2026-08-30: APPROVED. Enrich-all + dual matcher. No UI expand. Coverage ~87% enrich/matcher.
Task #3 review 2026-08-30: APPROVED. expandedIds Set; seed active; poll persist; canExpand true; remount useEffect gone; Todo pending filter; blurb text region. Coverage ~88% SpecBoardTab.
Task #4 review 2026-08-30: APPROVED. Vitest UI Gherkin + poll persist + no Archive. Host tests hold. colorForLabel lock. Coverage ~90% SpecBoardTab.

## Testing — @tester fills
[ ] happy paths (all US) [ ] edge cases (all EC-NNN) [ ] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [ ] critical paths 100% [ ] public APIs E2E [ ] boundary cases (empty/null/max)
Tools: [ ] unit [C + Vitest] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
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
/sdd-skill hotfix spec-005-v2m-spec-card-expand "Bug: [d]" → creates spec-005-v2m-hotfix-1

## Notes & Blockers
Blockers: [date — desc — RESOLVED/PENDING] | Design changes: [date — what — why — @architect approval] | Perf issues: [issue — status — resolution]
