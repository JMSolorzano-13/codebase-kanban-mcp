# Feature Checklist — Spec-014: Game visibility filters
Start Date: 2026-09-01 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — i18n + chrome + predicate + invert first-paint
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — Track / AND leftover + Specs lock + no-persist
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #1 review 2026-09-01: APPROVED (Path: full). visiblePhaseCards(cards, showArchived, showDones, track) per plan; Inbox = board.inbox; chrome Show archived | Show Dones | Track A/B/All; five aria-pressed; first paint dones/archived false, All true; keep t.specBoard.showArchived; no gameBoard.showArchived; i18n en+zh locked; lastProjectRef reset clears showDones+trackFilter; invert SYS-003-done + Archive (Show Dones first); no GET/POST/query/localStorage this task; no radiogroup; text-[10px] tokens; breadcrumbs. SpecBoardTab.tsx / useGameBoard.ts / C / types.ts not this task. Track/AND leftover Gherkin = Task #2. Coverage reporter absent (53 tests, 2 files). Commit held (not a reject). SDD-ADR-062..064.

Task #2 review 2026-09-01: APPROVED (Path: full). Tests only. Track A only A; Track B only B; All keeps A/B/H/other/null then dones/archived. Archived done iff Show Dones AND Show archived. Invert remount (Show archived alone hidden; both reveal; remount both unpressed); Unarchive stays while Show Dones on; project change resets three, GET refetch keeps. Inbox = board.inbox; all-filtered column header only, no "No specs yet". Filter click no GET/POST /api/game-board; no query track/show_dones/show_archived; localStorage no showDones/gameTrack. SpecBoardTab mount lock no Show Dones/Track A/All; SpecBoardTab.tsx unedited (spec-009 breadcrumbs). GameBoardTab.tsx / useGameBoard.ts / C / types.ts not this task. silent-win / Enter Graph / leftover tab=specs→game / no /api/skill-presence stay. colorForLabel Function #06b6d4 locked. Breadcrumbs on both test files. Coverage reporter absent (94 tests, 2 files). Commit held (not a reject). SDD-ADR-062..064.

## Testing — @tester fills
[x] happy paths (Task #1 first-paint + Show Dones; Task #2 Track A + both toggles) [x] edge cases (Task #1 leftover/remount; Task #2 Inbox/Track B→All/project vs refetch/header-only/chrome+Specs) [x] error conditions (Task #1 Show Dones alone; Task #2 Show archived alone / Track A+Show Dones / no query) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% (reporter absent) [x] Task #1+#2 critical paths 100% [ ] public APIs E2E [x] Task #1+#2 boundary
Tools: [x] unit [Vitest] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| graph-ui Vitest | 10/10 mapped Task #2 + 5/5 Task #1 stay; 25 files, 276 passed | PASS DEV | Track A hides B/H; both toggles reveal archived done; Inbox unfiltered; Track B then All restores H; project vs refetch; header-only; chrome | + Specs lock (GameBoardTab Inbox + SpecBoardTab document); Show archived alone hidden; Track A+Show Dones hides B done; no GET/POST/query/localStorage (C fopen N/A). Task #1 5/5 stay. silent-win / Enter Graph / leftover tab=specs→game / no /api/skill-presence / Function #06b6d4 stay. Playwright not run (LEVEL 1). |

Task #1 DEV 2026-09-01 (Unit; Playwright not run — LEVEL 1):
[x] happy paths (Task #1 Gherkin) [x] edge cases (Task #1 Gherkin) [x] error conditions (Task #1 Gherkin) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] Task #1 critical paths 100% [ ] public APIs E2E [x] boundary cases (leftover/remount)
Tools: [x] unit [Vitest] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| GameBoardTab.test.tsx | 5/5 Gherkin Thens | PASS DEV | names match spec Task #1 covered list; re-run 2026-09-01 stay green |

Task #2 DEV 2026-09-01 (Unit; Playwright not run — LEVEL 1):
[x] happy paths (Task #2 Gherkin) [x] edge cases (Task #2 Gherkin) [x] error conditions (Task #2 Gherkin) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] Task #2 critical paths 100% [ ] public APIs E2E [x] boundary cases (Inbox/Track B→All/project vs refetch/header-only/chrome+Specs)
Tools: [x] unit [Vitest] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| GameBoardTab.test.tsx + SpecBoardTab.test.tsx | 10/10 Gherkin Thens | PASS DEV | chrome Specs lock split: Inbox chrome in GameBoardTab; Specs document lock in SpecBoardTab |

## Human Validation — @human-trainer fills
[x] task summaries done [x] spec summary done [x] architecture diagram updated [x] QUICK-DEBUG updated [x] PROJECT-OVERVIEW updated [x] @sdd-* headers present [ ] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status | Notes |
| DoD | 0 | IN PROGRESS | |
| Code Quality | 0 | IN PROGRESS | |
| Testing | 0 | IN PROGRESS | |
| Human Validation | 0 | IN PROGRESS | |
| TOTAL | 0 | IN PROGRESS | |

## Closure Criteria (ALL required)
[ ] all DoD 100% [ ] @review 0 open issues [ ] @tester PASS all levels [ ] human confirmation received [ ] integration tests pass [ ] 0 critical issues [ ] perf targets met [ ] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-014 "Bug: [d]" → creates spec-014-hotfix-1

## Notes & Blockers
Blockers: none
Design changes: none
Perf issues: none
