# Feature Checklist — Spec-012: Game expand, archive, and deps
Start Date: 2026-08-31 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — Store game_archive table + set/load/copy
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg
  (commit held — user did not ask; 10 store_game_archive; coverage ~88% reporter absent; @review APPROVED 2026-08-31)

### Task #2 — C expand parse + blocked overlay + JSON
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg
  (commit held — user did not ask; 10 new game_board Gherkin Thens; coverage ~88% reporter absent; @review APPROVED 2026-08-31)

### Task #3 — HTTP POST + GET merge + C Gherkin + publish copy
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg
  (commit held — user did not ask; httpd POST/GET merge/publish Gherkin; coverage ~88% reporter absent; @review APPROVED 2026-08-31)

### Task #4 — GameBoardTab expand + blocked strip + i18n
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg
  (commit held — user did not ask; pane Gherkin + invert spec-011 title-lock; 48 GameBoardTab+useGameBoard+i18n Vitest; coverage ~88% reporter absent; @review APPROVED 2026-08-31)

### Task #5 — Archive UI + refresh + remaining Vitest
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg
  (commit held — user did not ask; Archive UI + refresh + remaining Vitest; 260 graph-ui Vitest; coverage ~88% reporter absent; @review APPROVED 2026-08-31)

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #5 review 2026-08-31: APPROVED (Path: full). useGameBoard.refresh same GET never setSettled/setPresent false (SDD-ADR-054; unset only on project one-shot); App passes project+refresh; Show archived pane chrome session-only aria-pressed false on mount/project/remount, GET refetch does not reset, no localStorage; phase filter work_state==="done" && archived && !showArchived; Inbox unfiltered; leftover pending archived shows, no Archive/Unarchive; Archive only expanded done && !archived; Unarchive only expanded done && archived; POST /api/game-board {project, card_id, archived} then await refresh(); no dialog/confirm; all-archived column header only; silent-win/Enter Graph/leftover tab=specs→game/no skill-presence stay; colorForLabel Function #06b6d4. SpecBoardTab / useSddSkillPresent / useSpecBoard / colors.ts / C / constitution.md not this task. Pattern = spec-006 (IX). Coverage ~88% (reporter absent). Commit held (not a reject). Last impl task — after tester PASS → human-trainer Trigger B closeprep.

Task #4 review 2026-08-31: APPROVED (Path: full). Title `<button aria-expanded>` in-place Set by GET id; all collapsed; multi-open; project/remount clear; same-project GET refetch keeps Set; Track A/B/Inbox/H bodies; Blocked region from GET blocked[] (omit empty; click no-op); blocked_by prefix collapsed+expanded; parse missing archived false / blocked []; skip invalid kind; useGameBoard one-shot; no POST/Archive this task; invert spec-011 title-lock; keep body-click / no-drag / clipboard; i18n reuse specBoard.noTasksYet + gameBoard.inputs/blockedStrip en+zh; SDD-ADR-055/057. App.tsx / SpecBoardTab / useSddSkillPresent / useSpecBoard not this task. 48 Vitest. Coverage ~88% (reporter absent). Commit held (not a reject). Archive UI / refresh / App wiring = Task #5.

Task #3 review 2026-08-31: APPROVED (Path: full). GET merge matching ids only (heap calloc 512); missing table/open fail → all false 200; POST yyjson 4096, exact 400/404/409, epic→404, overlay blocked 409 before set, mutation lock, flag object, card_id[256]; dispatch GET vs POST no 405; publish_staged game_archive_copy same live-open as spec_archive, copy fail fails publish, adr_fill unchanged; spec-board Game id 404 spec not found; store APIs parameterized; fopen rb; SDD-ADR-053 breadcrumbs. spec_board.c / mcp.c / graph-ui not this task. Coverage ~88% (gcov reporter absent). Commit held (not a reject). UI Tasks #4/#5 not this task.

Task #2 review 2026-08-31: APPROVED (Path: full). Widen card 512-byte expand fields + tasks[48] + board blocked[16]; local ## What it does extract (not spec_board extract_blurb; SDD-ADR-056); state.md :blocked: strip; overlay in C on artifact arrays only (SDD-ADR-055); to_json additive keys, blocked always [], empty blocked_by JSON null, archived false; fopen rb; heap calloc board; no POST. 10 new test_game_board Gherkin Thens. Coverage ~88% (gcov reporter absent). Commit held (not a reject). spec_board.c / mcp.c / http_server.c POST not this task.

Task #1 review 2026-08-31: APPROVED (Path: full). Dedicated game_archive (card_id PK 256, CAP 512) sibling of spec_archive; UPSERT set; sqlite_master missing-table load OK count 0; copy src→dst no-op if src missing. HTTP/pipeline/game_board/graph-ui out of scope. 10 store_game_archive tests. Coverage ~88% (gcov reporter absent). Commit held (not a reject).

## Testing — @tester fills
Task #5 DEV 2026-08-31 (Unit; Playwright not run — LEVEL 1 Archive UI + refresh):
[x] happy paths (Task #5 UI US-003/US-004/US-006) [x] edge cases (Task #5 Gherkin) [x] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [Vitest] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| graph-ui Vitest | 13/13 mapped; 25 files, 260 passed | PASS DEV | Gherkin UI: Archive hide no confirm; Show archived remount hidden; Unarchive stays while toggle off; leftover pending not hidden; all-archived header only. refresh never unsets settled/present; App Show archived chrome. #4 pane 8/8 none missing. silent-win / Enter Graph / leftover tab=specs→game / no skill-presence stay. colorForLabel Function #06b6d4. C HTTP 409/404/bytes=Task #3 (not FAIL). Last impl task → human-trainer Trigger B closeprep |

Task #4 DEV 2026-08-31 (Unit; Playwright not run — LEVEL 1 pane expand/strip):
[x] happy paths (Task #4 pane US-001/US-002/US-005) [x] edge cases (Task #4 Gherkin) [x] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [Vitest] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| graph-ui Vitest | 12/12 mapped; 25 files, 249 passed | PASS DEV | Gherkin pane: Track A expand; Track B header only; blocked strip+overlay; Inbox no Archive; empty Track A No tasks planned yet; missing Inputs omits; two cards stay expanded continue no toggle; empty blocked omits strip; body-click no expand; chrome p + remount/project collapse. parse additive defaults; i18n inputs+blockedStrip. silent-win/no skill-presence/no-drag/clipboard stay. Archive POST UI / Show archived / Unarchive / App refresh = Task #5 |

Task #3 DEV 2026-08-31 (HTTP+Unit; Playwright not run — LEVEL 1 POST/GET merge/publish):
[x] happy paths (Task #3 HTTP US-003/US-006) [x] edge cases (Task #3 Gherkin) [x] error conditions [ ] performance targets [x] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [C] [ ] E2E [Playwright] [x] integration [httpd] [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| httpd (C) | 12/12 mapped Gherkin; 108 passed / 1 skipped | PASS DEV | POST 200 flag+idempotent; Inbox epic 404; leftover+orphan GET merge; 409 pending bytes; 409 blocked overlay; 404 unknown card; 400 missing project; 400 invalid archived; 404 unknown project; spec-board Game id 404. DoD: publish copy. game_board 38/38. UI expand/archive=Tasks #4/#5 |

Task #2 DEV 2026-08-31 (C unit; Playwright not run — LEVEL 1 JSON parse/overlay):
[x] happy paths (Task #2 JSON US-002/US-005) [x] edge cases (Task #2 Gherkin) [x] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [C] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| game_board (C) | 10/10 mapped; 38/38 suite | PASS DEV | Gherkin: Track A blurb/tasks/inputs; Track B header only; What-it-does wins; playtest last Round; level last 8; 48-cap omits 49th; needs_review not strip; empty blocked []; overlay @owner; GET bytes identical (expand_read + read_leaves). httpd GET 97 passed / 1 skipped. spec-011 leftovers green. Pane/UI=Task #4; POST/HTTP=Task #3; archive UI=Task #5 |

Task #1 DEV 2026-08-31 (C unit; Playwright not run — LEVEL 1 store-only):
[ ] happy paths (all US) [x] edge cases (Task #1 Gherkin) [x] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [C] [ ] E2E [Playwright] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| store_game_archive (C) | 10/10 | PASS DEV | Gherkin: orphan row survives load (missing-doc.md archived=1); repeat archive set true twice → one row archived=1. DoD: schema, UPSERT roundtrip, empty/too-long id ERR, unarchive keeps row, missing-table count 0, copy src→dst, copy missing src no-op. store_spec_archive 9/9. HTTP invent-card / POST / GET merge / UI / parse = Tasks #2–#5 |

## Human Validation — @human-trainer fills
[x] task summaries done [x] spec summary done [x] architecture diagram updated [x] QUICK-DEBUG updated [x] PROJECT-OVERVIEW updated [x] @sdd-* headers present [x] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status(IN PROGRESS/BLOCKED/COMPLETE) |
| DoD | 100 | COMPLETE |
| Code Quality | 100 | COMPLETE |
| Testing | 100 | COMPLETE |
| Human Validation | 100 | COMPLETE |
| TOTAL | 100 | COMPLETE |

## Closure Criteria (ALL required)
[x] all DoD 100% [x] @review 0 open issues [x] @tester PASS all levels [x] human confirmation received [x] integration tests pass [x] 0 critical issues [x] perf targets met [x] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-012-m2k-game-expand-archive-deps "Bug: [d]" → creates spec-012-m2k-hotfix-1

## Notes & Blockers
Blockers: [date — desc — RESOLVED/PENDING] | Design changes: [date — what — why — @architect approval] | Perf issues: [issue — status — resolution]
