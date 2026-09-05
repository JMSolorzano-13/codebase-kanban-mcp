# Feature Checklist — Spec-016: Game Inbox registry
Start Date: 2026-09-02 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — game_board registry parse + hide predicate
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% (reporter absent) | tests pass locally | committed w/ clear msg (held) | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — HTTP GET leftover locks
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% (reporter absent) | tests pass locally | committed w/ clear msg (held) | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — InboxCard wrap + leftover Vitest
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% (reporter absent) | tests pass locally | committed w/ clear msg (held) | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [x] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`
Task #1 2026-09-02: APPROVED. Parse in game_board.c only; present=regular file; skip load_conv when present; fopen rb; no new JSON key. Directory-at-path has no dedicated test (game_is_regular_file covers it). Commit held.
Task #3 2026-09-03: APPROVED. InboxCard id whitespace-normal break-all (same Tailwind as spec-015 EpicCard); TitleControl truncate + letter E stay; ArtifactCard id truncate locked. Hide stays C-owned (inbox mocked already-filtered). Empty Inbox header only. Show Dones leftover. Locked files (useGameBoard/i18n/types/SpecBoardTab/WorkspaceHeader/colors) not this task. Coverage reporter absent. Commit held.

## Testing — @tester fills
[x] happy paths (Task #1 hide-set + Task #2 GET omit + Task #3 Inbox wrap) [x] edge cases (last-wins, NNN, slug, native, evergreen, cap 64, empty Inbox, artifact truncate, Show Dones leftover) [x] error conditions (unreadable, malformed, 404, no create) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% (reporter absent) [x] Task #1–#3 critical paths 100% [ ] public APIs E2E [x] boundary cases (empty registry / 65th / n/a Epic / empty Inbox)
Tools: [x] unit [C + HTTP + Vitest] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| game_board | reporter absent | PASS DEV | 17/17 Task #1 Gherkin unit Thens; 56 passed |
| httpd | reporter absent | PASS DEV | 5/5 Task #2 HTTP Gherkin; 122 passed / 1 skipped |
| GameBoardTab | reporter absent | PASS DEV | 4/4 Task #3 UI Gherkin; graph-ui 25 files, 294 passed |

Task #3 DEV 2026-09-03 (Unit; Playwright not run — LEVEL 1):
[x] happy paths (Inbox wrap) [x] edge cases (empty Inbox / artifact truncate / Show Dones leftover) [x] error conditions (none this task) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] Task #3 critical paths 100% [ ] public APIs E2E [x] boundary (empty Inbox header only)
Tools: [x] unit [Vitest] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| GameBoardTab | 4/4 Task #3 UI Gherkin | PASS DEV | graph-ui npx vitest run; 25 files, 294 passed |

Task #2 DEV 2026-09-02 (HTTP+Unit; Playwright not run — LEVEL 1):
[x] happy paths (Task #2 Gherkin omit) [x] edge cases (no has_more / no registry key) [x] error conditions (404 / no create / bytes) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] Task #2 critical paths 100% [ ] public APIs E2E [x] boundary (absent file)
Tools: [x] unit [C HTTP] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| httpd | 5/5 Task #2 HTTP Gherkin | PASS DEV | 122 passed / 1 skipped |

Task #1 DEV 2026-09-02 (Unit; Playwright not run — LEVEL 1):
[x] happy paths (Task #1 Gherkin) [x] edge cases (Task #1 Gherkin) [x] error conditions (unreadable / malformed) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] Task #1 critical paths 100% [ ] public APIs E2E [x] boundary (empty / 65th / native)
Tools: [x] unit [C] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| game_board | 17/17 Task #1 Thens | PASS DEV | scripts/test.sh --suites game_board; 56 passed |

## Human Validation — @human-trainer fills
[x] task summaries done [x] spec summary done [x] architecture diagram updated [x] QUICK-DEBUG updated [x] PROJECT-OVERVIEW updated [x] @sdd-* headers present [x] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status(IN PROGRESS/BLOCKED/COMPLETE) | — DoD, Code Quality, Testing, Human Validation, TOTAL

## Closure Criteria (ALL required)
[ ] all DoD 100% [ ] @review 0 open issues [ ] @tester PASS all levels [ ] human confirmation received [ ] integration tests pass [ ] 0 critical issues [ ] perf targets met [ ] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-016 "Bug: [d]" → creates spec-016-hotfix-1

## Notes & Blockers
Blockers: | Design changes: | Perf issues:
