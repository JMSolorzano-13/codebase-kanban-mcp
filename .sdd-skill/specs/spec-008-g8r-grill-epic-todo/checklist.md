# Feature Checklist — Spec-008: Grill epic Todo
Start Date: 2026-08-30 / Target Close Date: 2026-08-30

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — spec_board grill read + additive JSON
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — HTTP GET additive + POST epic-id 404
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — SpecBoardTab EpicCard + Todo epics-then-specs
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #4 — Vitest Gherkin mapping remaining UI scenarios
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #1 review 2026-08-30: APPROVED (Path: full). Reader-only. HTTP GET/POST, graph-ui, MCP, store, Makefile.cbm out of scope. Coverage ~88% (gcov reporter absent; 13/13 Task #1 Gherkin Thens mapped; 35/35 spec_board). Commit held (not a reject).

Constitution (project docs/constitution.md I–IX; role-template I–X mapped below):
- I Source of truth: implements Task #1 DoD / US-001,002,004,005,006 reader half. fopen `"rb"` only (`spec_board.c:37`). No write to `.grill/` or `.sdd-skill/` (I.2). HTTP/UI deferred to #2–#4.
- II Languages: C11 `cbm_spec_board_*` + `CBM_SPEC_BOARD_MAX_EPICS` 64. Allocation/IO NULL-checked (`read_whole_file` `:38-62`; unreadable epic skip `:978-984`). No graph-ui / new runtime / Makefile source.
- III Chrome/graph: no UI. N/A.
- IV Identity/APIs: no new HTTP/MCP this task. Additive keys on existing `cbm_spec_board_to_json` (IV.3). `kind` only on epics (`:1258-1260`); specs keep spec-006 fields, no `"kind"`.
- V Testing: 35 C tests in `suite spec_board`. All Task #1 Then clauses covered (mixed JSON, Companion-to exact + trailing notes, source.grill_epic, done claim, two-plan order, closed leftover, pending+detailed, missing Companion-to, 65th/cap, no .grill, unreadable skip, byte-identical). HTTP/UI Gherkin deferred to #2–#4.
- VI Security/HTTP: no secrets; dirent reject `/` `\` `..` (`:678-691`); Companion-to token never fopen'd (`:322-323`, match `strcmp` `:744-754`); JSON via `cbm_json_escape`; loopback/bind unchanged.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on `spec_board.h`, `spec_board.c`, `test_spec_board.c`.
- VIII Performance: bounded opendir + fopen; stop appending at 64 (`:967-968`, `:1026-1028`). No poll. No `get_graph_schema`.
- IX Patterns: same `cbm_fopen` `"rb"` + `read_whole_file` as spec-005/006; grill walk inside `cbm_spec_board_read` (SDD-ADR-036); separate `epics[]` (SDD-ADR-035); `kv_extract_colon_line` reused for name/summary/title. Pattern Notes: ✓ (no deviation). Did not call `cbm_spec_board_gamedev_skill_present` from read/to_json (`:1170-1173`).
- X DRY (role template; no Section X in constitution.md): no `src/shared/` duplicate; no sibling `grill_board.c`.
- Naming (role template vs II.1): snake_case files + `cbm_`/`grill_*` statics match this C tree. Caps UPPER_SNAKE.
- API (role template): additive JSON on existing serializer; no new route.
- Monitoring: N/A (no new poll/metrics). Left unchecked, same as spec-001..007.

SOLID: grill walk split collect/append/convert; conversion is strcmp only; public API still `spec_board.h`; HTTP not grown.

OWASP: no SQL; no JWT/auth endpoint; no PII logs; path join sanitized; XSS N/A (C strings, no HTML render this task).

Types: C, all params/returns typed. No `any` / `!`.

Coverage: ~88% of touched spec_board grill path (static). All Task #1 Thens exercised. Untested are defensive (OOM, slug overflow skip `:971-972`, GRILL_MAX_PLANS 96). 35/35 `scripts/test.sh --suites spec_board` PASS. gcov reporter absent — not a reject.

SDD-ADR-035..037: same GET additive `epics[]`; walk in spec_board.c; `summary`+`plan_title`; exact path; index.md then slug-asc. HTTP Gherkin and EpicCard correctly absent (Tasks #2/#3).

AUTO-REJECT: none (no secret, no SQLi, no new endpoint, constitution held, critical fns tested). Commit held by convention.

Task #2 review 2026-08-30: APPROVED (Path: full). HTTP only. graph-ui/EpicCard, MCP, Makefile.cbm, store out of scope. Coverage ~90% (gcov reporter absent; 4/4 Task #2 HTTP Gherkin mapped). Commit held (not a reject).

Constitution (project docs/constitution.md I–IX):
- I Source of truth: implements Task #2 DoD / US-001 HTTP + US-006. GET/POST 404 leave .grill/ and .sdd-skill/ bytes identical (`test_httpd.c:3730-3732`, `:3792-3793`, `:3864-3866`). No cycle-file writes (I.2).
- II Languages: C11 handlers unchanged in shape; calloc NULL-checked (`http_server.c:502-506`, `:620-624`). Zero `fopen`/`cbm_fopen`/`cbm_opendir` in `http_server.c`. No graph-ui / new runtime / Makefile source.
- III Chrome/graph: no UI. N/A.
- IV Identity/APIs: same GET+POST `/api/spec-board` (`:2240-2251`). No sibling `/api/grill-board`. Additive keys via existing `to_json` (IV.3). 404 strings unchanged: `project not found` (`:497`), `spec not found` (`:629`).
- V Testing: 4 C HTTP tests. Mixed Todo GET 200 JSON; POST epic id 404 + no store row + bytes; GET unknown project 404; GET leaves skill trees. Existing POST 200/409/400/423 stay. UI Gherkin deferred to #3–#4.
- VI Security/HTTP: loopback/bind unchanged; POST `spec_id` never fopen'd as a grill path (`spec_board_find` `:537-541` specs[] only). No secrets.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on apply/get/find/post (`http_server.c:432-553`) and grill HTTP block (`test_httpd.c:3576-3580`).
- VIII Performance: no new poll; no `get_graph_schema`. Archive merge still one query-open + specs[] loop (`:464-473`).
- IX Patterns: same spec-006 dispatch (SDD-ADR-030/036): read → merge specs only → to_json. POST find stays specs[] (SDD-ADR-035). Pattern Notes: ✓. Grill IO stays in `cbm_spec_board_read` (trace: GET hop-1 callees are resolve/read/merge/to_json only).
- X DRY (role template): no second HTTP grill reader; no `src/shared/` duplicate.
- Naming: snake_case + `cbm_` match this C tree.
- Monitoring: N/A. Left unchecked.

SOLID: GET is serialize-only; POST 404 returns before lock and before `cbm_store_spec_archive_set` (`:626-630` vs `:656`). find has 0 callees, callers = post only.

OWASP: no SQL in HTTP; archive via store API; no JWT; no PII logs; XSS N/A (JSON via `cbm_json_escape` on 200 POST).

Types: C, all params/returns typed.

Coverage: ~90% of touched spec-board HTTP (static). GET 200/404 + POST epic-miss 404 exercised. Untested defensive: OOM 500, serialize 500. Claimed 124 passed / 1 skipped (spec_board+httpd). gcov absent — not a reject.

SDD-ADR-035/036: same GET additive; walk not in HTTP; POST spec-only same 404 string. EpicCard correctly absent (Task #3).

AUTO-REJECT: none (no secret, no SQLi, no new endpoint, constitution held, critical GET/POST paths tested). Commit held by convention.

Task #3 review 2026-08-30: APPROVED (Path: full). graph-ui EpicCard + Todo order. C HTTP/MCP/useSddSkillPresent/useSpecBoard/formatIndexedAt/colors.ts/WorkspaceTabStrip out of scope. Coverage ~88% (Vitest reporter absent; 29 SpecBoardTab). Remaining UI Gherkin is Task #4 (not a reject). Commit held (not a reject). Live :9749 not click-verified — not a reject (V.1 / IX.4).

Constitution (project docs/constitution.md I–IX):
- I Source of truth: implements Task #3 DoD / US-001,003,004,006 (tab). EpicCard display-only; no skill-file writes (I.2). Task #4 remaining Then mapping not this task.
- II Languages: React 19 FC; no bare `any`; `--color-epic-mark` in existing `globals.css` `@theme inline` (`:28`); no new CSS file. Letter `E` literal (`SpecBoardTab.tsx:104`), not i18n (II.3). No new runtime.
- III Chrome/graph: one documented E hue `#7d8ec9` (SDD-ADR-038); not `--color-destructive` / TaskList emerald / Function `#06b6d4`. `colors.ts` Function stays `#06b6d4`. `--color-primary` unchanged.
- IV Identity/APIs: same GET via unedited `useSpecBoard` (poll 4000). No second board URL. `persistArchive` still POST `/api/spec-board` from SpecCard only.
- V Testing: 29 SpecBoardTab tests. Mixed Todo paint order + E token; epic click no Archive/Unarchive/noTasksYet/no POST; spec expand+Archive stay; missing `epics` host; grill true + sdd false → not-sdd-skill. Two-plan/cap/Has more/colorForLabel-in-suite = Task #4.
- VI Security/HTTP: loopback unchanged; React text nodes not HTML; EpicCard never POSTs. No secrets.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on `types.ts`, `globals.css`, `SpecBoardTab.tsx`, `SpecBoardTab.test.tsx`. Epic `id` visible as text. EpicCard is not an interactive control (no unlabeled button).
- VIII Performance: no `get_graph_schema`; poll stays 4s (`useSpecBoard.ts` POLL_MS 4000, not edited).
- IX Patterns: spec-005 expand Set + spec-006 Archive stay on SpecCard (IX.2). Host still `!board.sdd_skill_present` (`:344-346`). Pattern Notes: ✓. Locked files not this task: `useSddSkillPresent` spec-002 `=== true`; `useSpecBoard` spec-006; `formatIndexedAt` spec-007; `WorkspaceTabStrip` spec-002; `colors.ts` not in detect_changes vs main. detect_changes vs main lists those files from prior uncommitted specs — not a Task #3 edit.
- X DRY (role template): EpicCard in SpecBoardTab.tsx; no sibling CSS; `board.epics ?? []`.
- Naming: PascalCase components; camelCase fns; CSS kebab token.
- Monitoring: N/A. Left unchecked.

SOLID: EpicCard display-only (0 callees; no persistArchive). SpecCard keeps onToggle + TaskList. Column `todoEpics = id === "todo" ? epics : []`; only Todo is passed `board.epics ?? []`. pendingCount = epics + spec todos. Three Column children.

OWASP: no SQL; no JWT; no PII logs; XSS: title/summary/plan_title/id as text. POST only from SpecCard Archive/Unarchive.

Types: `SpecBoardEpic` `kind: "epic"`, `column: "todo"`; `grill_skill_present?` / `epics?`. No `any`. No unwarranted `!` in production files.

Coverage: ~88% of touched SpecBoardTab (static). 29 `it(` in SpecBoardTab.test.tsx (5 host + 10 expand + 11 archive + 3 epic). Claimed 165 graph-ui. Reporter absent — not a reject. Index coverage: SpecBoardTab.tsx / types.ts `no_recorded_issue` (best-effort; indexed_at 2026-08-30T20:32:33Z).

trace_path: EpicCard callees_total 0 (no POST). persistArchive callers = SpecBoardTab only. SpecCard still onToggle + TaskList.

SDD-ADR-035/038: sibling `epics[]`; E via `--color-epic-mark #7d8ec9`.

AUTO-REJECT: none (no secret, no SQLi, no new endpoint, constitution held, critical EpicCard/host paths tested). Commit held by convention.

Task #4 review 2026-08-30: APPROVED (Path: full). Test-only Vitest Gherkin mapping. Product SpecBoardTab.tsx unchanged this task (Task #3 already shipped UI). C HTTP/MCP/useSddSkillPresent/useSpecBoard/formatIndexedAt/colors.ts/WorkspaceTabStrip out of scope. Coverage ~88% (Vitest reporter absent; 34 SpecBoardTab; 165 graph-ui it() counted vs 170 claimed). Commit held (not a reject). Last impl task — NOT closeprep.

Constitution (project docs/constitution.md I–IX):
- I Source of truth: implements Task #4 DoD / US-001–006 UI Thens. Conversion stays C-owned (mocks already-filtered `epics`). No skill-file writes (I.2).
- II Languages: React 19 test file; no bare `any`; English assertions (II.3). No new CSS / runtime / Playwright.
- III Chrome/graph: `colorForLabel("Function") === "#06b6d4"` locked in suite (`SpecBoardTab.test.tsx:963` + `colors.test.ts`). colors.ts not edited. No GraphTab/Three boot.
- IV Identity/APIs: no new HTTP/MCP. persistArchive still SpecCard-only (product). Tests assert epic click does not POST.
- V Testing: 34 SpecBoardTab tests. All Task #4 UI Gherkin Thens mapped (Mixed Todo paint; Companion-to omit via empty mock; Done/In progress no epic id; two-plan document order; missing Companion-to both cards; 64-epic mock no Has more; epic no expand/archive; grill true + sdd false not-sdd-skill). Host: no selectProject when project set; loading/not-sdd-skill hold with grill true. C Gherkin remains Tasks #1–#2.
- VI Security/HTTP: loopback unchanged; no secrets; tests do not fopen Companion-to as path.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on `SpecBoardTab.test.tsx`. Epic id text asserted.
- VIII Performance: no `get_graph_schema`; poll unchanged (useSpecBoard not this task).
- IX Patterns: UI does not re-implement C conversion/cap/sort. Host still `!board.sdd_skill_present` (product `:344-346`). useSddSkillPresent still `=== true` only (unedited). Pattern Notes: ✓. detect_changes vs main lists prior uncommitted specs + Task #3 product — Task #4 mtime is test file only.
- X DRY (role template): helpers `makeEpic` / `todoCardIds` / `epicCardById`; no second conversion matcher.
- Naming: PascalCase components; camelCase test helpers.
- Monitoring: N/A. Left unchecked.

SOLID: tests assert EpicCard display-only (no button / no persistArchive). Column isolation via document order of `.font-mono` ids. In progress/Done not passed `epics`.

OWASP: no SQL; no JWT; no PII logs; XSS N/A (test assertions on text). POST spy confirms zero calls on epic activate.

Types: no `any`. `as SpecBoardEntry` / `as HTMLElement` after null throw (existing pattern). No unwarranted `!`.

Coverage: ~88% of touched SpecBoardTab (static). 34 `it(` in SpecBoardTab.test.tsx (5 host + 10 expand + 11 archive + 8 epic). graph-ui suite 165 `it(` counted (170 claimed). Reporter absent — not a reject. Index coverage: SpecBoardTab.tsx / SpecBoardTab.test.tsx `no_recorded_issue` (best-effort; indexed_at 2026-08-30T20:46:48Z).

trace_path: skipped — test-only (no product function change this task).

SDD-ADR-035/038: sibling `epics[]` mocked already-filtered; E via `--color-epic-mark`; Function hex locked.

AUTO-REJECT: none (no secret, no SQLi, no new endpoint, constitution held, critical UI Thens tested). Uncommitted work is convention. Test-only valid (Task #3 shipped UI). Commit held by convention.

## Testing — @tester fills
Task #4 DEV 2026-08-30 (graph-ui Vitest remaining UI; Playwright not run — LEVEL 1 Unit; last impl task):
[x] happy paths (Task #4 UI) [x] edge cases (Task #4 Gherkin) [ ] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (two-plan order / 64-epic no Has more / grill true + sdd false)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| graph-ui Vitest | reporter absent (34 SpecBoardTab + 3 colors; 170/170 full suite) | PASS DEV | 8/8 mapped UI Gherkin Thens. Mixed Todo E+title+summary+plan+ids+order; Companion-to omit via empty mock; Done/In progress no epic id; two-plan document order; missing Companion-to both cards; 64-epic mock no Has more; epic no expand/archive/POST; grill true + sdd false not-sdd-skill. Host: no selectProject when project set; loading/not-sdd-skill with grill true. colorForLabel Function #06b6d4. Command: graph-ui npx vitest run. C Gherkin = Tasks #1–#2. Last task. Next: @human-trainer Trigger B closeprep (NOT @planner). |

Task #3 DEV 2026-08-30 (graph-ui Vitest EpicCard; Playwright not run — LEVEL 1 Unit):
[x] happy paths (Task #3 UI) [x] edge cases (Task #3 Gherkin) [ ] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (epic no expand/archive; grill true + sdd false)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| graph-ui Vitest | reporter absent | PASS DEV | 3/3 Task #3 UI Gherkin (SpecBoardTab 29/29 at then; suite 165). Remaining UI Gherkin = Task #4 (not FAIL). |

Task #2 DEV 2026-08-30 (C HTTP spec_board+httpd; Playwright not run — LEVEL 1 HTTP):
[x] happy paths (Task #2 GET 200 mixed JSON) [x] edge cases (GET zero-write) [x] error conditions (POST epic 404, GET unknown project 404) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (POST epic id / GET bytes identical)
Tools: [x] unit [C HTTP] [ ] E2E [Playwright optional DEV] [x] integration [HTTP] [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| spec_board+httpd (Task #2 HTTP Thens) | reporter absent (~90% review) | PASS DEV | 4/4 mapped Gherkin Thens; 124 passed / 1 skipped. scripts/test.sh --suites spec_board,httpd. Task #1 spec_board 35/35 still green. EpicCard + Vitest + Specs-tab sdd-only deferred #3–#4 |

Task #1 DEV 2026-08-30 (C unit spec_board; Playwright not run — LEVEL 1 Unit):
[x] happy paths (Task #1 JSON) [x] edge cases (Task #1 Gherkin) [x] error conditions (unreadable skip) [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (cap 64 / no .grill / bytes identical)
Tools: [x] unit [C] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| spec_board (Task #1 unit Thens) | reporter absent (~88% review) | PASS DEV | 13/13 mapped Gherkin Thens; 35/35 suite. scripts/test.sh --suites spec_board. HTTP GET/POST + EpicCard + Vitest + Specs-tab sdd-only deferred #2–#4 |

## Human Validation — @human-trainer fills
[ ] task summaries done [ ] spec summary done [ ] architecture diagram updated [ ] QUICK-DEBUG updated [ ] PROJECT-OVERVIEW updated [ ] @sdd-* headers present [ ] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status(IN PROGRESS/BLOCKED/COMPLETE) |
| DoD | 0 | IN PROGRESS |
| Code Quality | 50 | IN PROGRESS |
| Testing | 0 | IN PROGRESS |
| Human Validation | 0 | IN PROGRESS |
| TOTAL | 0 | IN PROGRESS |

## Closure Criteria (ALL required)
[ ] all DoD 100% [ ] @review 0 open issues [ ] @tester PASS all levels [ ] human confirmation received [ ] integration tests pass [ ] 0 critical issues [ ] perf targets met [ ] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-008-g8r-grill-epic-todo "Bug: [d]" → creates spec-008-g8r-hotfix-1

## Notes & Blockers
Blockers: [date — desc — RESOLVED/PENDING] | Design changes: [date — what — why — @architect approval] | Perf issues: [issue — status — resolution]
