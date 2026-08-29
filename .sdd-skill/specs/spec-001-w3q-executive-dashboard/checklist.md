# Feature Checklist — Spec-001: Executive Dashboard
Start Date: 2026-08-29 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — useProjects list-only
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — Chrome tokens + palette lock
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — formatIndexedAt
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #4 — Dashboard page
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #5 — App routing + TabBar delete
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Task #1 verified 2026-08-29 (project constitution I–IX; perf = no N+1 get_graph_schema):
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(no schema N+1) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [ ] auth/JWT [x] sensitive data [x] XSS [ ] access control [ ] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #2 verified 2026-08-29 (project constitution I–IX; III chrome grayscale vs graph categorical):
Constitution: [x] nomenclature [x] API standards [x] security [x] performance [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [ ] auth/JWT [x] sensitive data [x] XSS [ ] access control [ ] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #3 verified 2026-08-29 (project constitution I–IX; D2 UTC en-US/zh-CN):
Constitution: [x] nomenclature [x] API standards [x] security [x] performance [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [ ] auth/JWT [x] sensitive data [x] XSS [ ] access control [ ] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #4 verified 2026-08-29 (project constitution I–IX; D1 stack + D6 path-only POST; no schema on Dashboard path):
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(no get_graph_schema) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [ ] auth/JWT [x] sensitive data [x] XSS [ ] access control [ ] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

## Testing — @tester fills
Task #1 DEV 2026-08-29 (Vitest; Playwright not run — constitution IX.4):
[ ] happy paths (all US) [ ] edge cases (all EC-NNN) [x] error conditions [x] performance targets [ ] cross-module integration
[ ] coverage>80% [ ] critical paths 100% [ ] public APIs E2E [ ] boundary cases (empty/null/max)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| useProjects.test.ts | n/a | PASS | Task #1 Gherkin: indexed_at from list_projects; 0 get_graph_schema; RPC fail → non-empty error + [] |
| StatsTab.test.tsx | n/a | PASS | Extra/regression: Project[] consumers still green (11 tests) |

Task #2 DEV 2026-08-29 (Vitest; Playwright not run — constitution IX.4):
[ ] happy paths (all US) [ ] edge cases (all EC-NNN) [ ] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [ ] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| colors.test.ts | n/a | PASS | Gherkin graph deep link: colorForLabel("Function")==="#06b6d4"; LABEL_COLORS lock; unknown → #94a3b8 |
| chrome-tokens.test.ts | n/a | PASS | Teal chrome tokens gone; 4 distinct surfaces; #22d3ee loader; bg-card panels; EdgeLines CALLS/default hex; gauge gray/amber/red |
| graph-ui full | n/a | PASS | Extra/regression: 12 files, 54/54 |

Task #3 DEV 2026-08-29 (Vitest; Playwright not run — constitution IX.4):
[ ] happy paths (all US) [ ] edge cases (all EC-NNN) [x] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [ ] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| formatIndexedAt.test.ts | n/a | PASS | Gherkin Two projects datetime derived (helper): en/zh contain 2026+29 and ≠ raw ISO; invalid/empty return raw |

Task #4 DEV 2026-08-29 (Vitest; Playwright not run — constitution IX.4):
[x] happy paths (all US) [x] edge cases (all EC-NNN) [x] error conditions [x] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| Dashboard.test.tsx | n/a | PASS | Task #4 Gherkin: two-project identity+freshness + 0 get_graph_schema; create POST {root_path} only + IndexProgress + no onSelectProject; Control polls ≥2 after 3.5s; empty CTA+Control; list RPC fail destructive+Control; index 400 keeps modal; delete cancel no DELETE |
| IndexProgress.test.tsx | n/a | PASS | Extra/regression: poll/done/empty-wait/error dismiss (4) |
| AdrButton.test.tsx | n/a | PASS | Extra/regression: isolated extract; Dashboard does not mount ADR |
| i18n.test.ts | n/a | PASS | Extra/regression: enter+lastIndexed en/zh catalog lock |
| graph-ui full | n/a | PASS | Extra/regression: 15 files, 63/63. Coverage reporter not installed |

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
/sdd-skill hotfix spec-001-w3q-executive-dashboard "Bug: [d]" → creates spec-001-w3q-executive-dashboard-hotfix-1

## Notes & Blockers
Blockers: | Design changes: | Perf issues:
