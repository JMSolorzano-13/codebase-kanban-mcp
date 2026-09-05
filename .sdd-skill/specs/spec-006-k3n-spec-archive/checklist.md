# Feature Checklist — Spec-006: Spec archive
Start Date: 2026-08-30 / Target Close Date: 2026-08-30

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — Store spec_archive table + set/load/copy
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — HTTP POST/GET merge + C Gherkin + publish copy
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — SpecBoardTab filter + session toggle + Archive/Unarchive
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #4 — Vitest Gherkin mapping remaining UI scenarios
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #4 review 2026-08-30: APPROVED (Path: full). SpecBoardTab.test.tsx maps every UI Gherkin Then; poll rerender GET archived:true stays hidden while toggle off; spec-005 document-wide no Archive/Unarchive replaced (names only on expanded Done). Host selectProject/loading/not-sdd-skill hold. colorForLabel Function #06b6d4. No GraphTab/Three/Playwright. 27 Vitest. Coverage ~90%.

Task #3 review 2026-08-30: APPROVED (Path: full). SpecBoardTab session showArchived + Done filter + Archive/Unarchive POST + await refresh. formatIndexedAt / colors / poll interval / fourth column out of scope. Poll Gherkin is Task #4. Coverage ~88% filter/toggle/POST.

Task #2 review 2026-08-30: APPROVED (Path: full). HTTP GET merge + POST flag object + publish_staged copy. graph-ui / adr_fill / MCP / sqlite_writer out of scope. Coverage ~87% merge/POST/copy.

Task #1 review 2026-08-30: APPROVED (Path: full). Store-only. HTTP/pipeline/graph-ui/sqlite_writer out of scope. Coverage ~88% set/load/copy.

Constitution (project docs/constitution.md I–IX; role-template I–X mapped below):
- I Source of truth: implements Task #1 DoD / US-005 persist shape only. No write to `.sdd-skill/` (I.2). No GET merge, POST, or UI.
- II Languages: C11 `cbm_store_spec_archive_*` + `CBM_SPEC_ARCHIVE_CAP`. Explicit NULL/empty `spec_id` → `CBM_STORE_ERR`. No graph-ui / new runtime. Store does not include `spec_board.h`.
- III Chrome/graph: no UI. N/A.
- IV Identity/APIs: no new HTTP/MCP. Project key is the `.db` filename (no project column). Table is not `store_meta` / `project_summaries`.
- V Testing: 9 C memory-store tests in `TEST_STORE_SRCS` + `suite_store_spec_archive`. Public set/load/copy covered. Gherkin Then: orphan row survives load; repeat archive idempotent. HTTP/UI Gherkin deferred to #2–#4.
- VI Security/HTTP: no secrets; parameterized UPSERT/SELECT; loopback/bind unchanged; no skill-file IO.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on `store.h`, `store.c` (new section), `test_store_spec_archive.c`.
- VIII Performance: one UPSERT; SELECT LIMIT ≤64. No poll. No `get_graph_schema`.
- IX Patterns: `init_schema` `CREATE TABLE IF NOT EXISTS` (same class as `project_summaries` / `index_coverage_meta`). `sqlite_master` probe matches `cbm_store_adr_get` (`store.c:8011-8028`); load maps missing-table to OK count 0 (list), not `NOT_FOUND` (single get). UPSERT + `iso_now` matches `cbm_store_adr_store`. Query-open (`cbm_store_open_path_query`) skips `init_schema` (callers of `init_schema`: write/memory opens only). Pattern Notes: ✓ (no deviation).
- X DRY (role template; no Section X in constitution.md): probe extracted as `spec_archive_table_probe`; no `src/shared/` duplicate; writer dump DDL untouched (copy hook is Task #2 / SDD-ADR-033).
- Naming (role template vs II.1): snake_case files + `cbm_` symbols match this C tree, not web kebab/camel. Consts UPPER_SNAKE. `spec_id[192]` / archived 0|1 match DoD.
- API (role template): no JSON surface this task.
- Monitoring: N/A (no new poll/metrics). Left unchecked, same as spec-001..005.

SOLID: set / load / copy / probe each do one thing; schema closed via CHECK + UPSERT (O); no fat interface; store handle is the existing abstraction (D).
OWASP: parameterized `?1/?2/?3`; no JWT/auth surface; no secrets/PII logs; no XSS (no HTML); access is local store API.
Type safety: C typed params/returns; no `any` / `!`.

Graph (mcp_idx=yes): `store.h` / `test_store_spec_archive.c` no_recorded_issue; `store.c` parse_partial at 5470/5508/5574/6228 (pre-existing, outside spec_archive 8164–8312). `init_schema` callers: `store_open_internal` → open/memory/path/path_existing — not query-open. set/load called from copy + suite tests; copy callers are tests only (pipeline is Task #2).

Task #2 constitution (I–IX + role I–X):
- I: Task #2 DoD / US-001 HTTP half + US-005 merge + US-006 errors. Zero `.sdd-skill/` writes (I.2). Leftover-true-on-todo is GET merge only (no hide).
- II: C11 `cbm_`; GET/POST split; no graph-ui / new runtime. `spec_board.c` fopen rb only; no `store.h`.
- III: no UI. N/A.
- IV.3: POST on `/api/spec-board` (SDD-ADR-030). No sibling path. No MCP archive tool. 200 flag object (SDD-ADR-031).
- V: C Gherkin in `test_httpd.c` + `test_spec_board.c` (GET merge/orphan/leftover; POST 200/409/404/400; idempotent; active.json bytes; 423; publish copy). UI Gherkin deferred to #3–#4.
- VI: loopback unchanged; `spec_id` looked up on board, not as a path; `cbm_json_escape` on 200; mutation lock like `handle_adr_save`.
- VII: breadcrumbs on `http_server.c` (apply/get/post), `spec_board.h`/`spec_board.c`, `pipeline.c` (`publish_staged`), both test files.
- VIII: one query-open + SELECT ≤64 on GET; one write-open + UPSERT on POST. No `get_graph_schema`.
- IX: merge after `cbm_spec_board_read` in HTTP (ADR-030); copy after ADR write in `publish_staged` (ADR-033); `adr_fill` untouched. Pattern Notes: ✓.
- X: apply/find helpers local to `http_server.c`; lock/423 string match `handle_adr_save`; no `src/shared/` duplicate.
- Monitoring: N/A. Left unchecked.

Graph Task #2 (mcp_idx=yes, index 15:23; files read as ground truth): `dispatch_request` → `handle_spec_board_get`/`handle_spec_board_post`. GET: `cbm_spec_board_read` → `spec_board_apply_archive_flags` (`open_path_query` + `spec_archive_load`, matching ids only) → `cbm_spec_board_to_json`. POST: `spec_archive_set` after done-check + lock; 200 is flag object not `to_json`. `cbm_pipeline_publish_staged` → `cbm_store_spec_archive_copy` after `adr_store`. parse_partial `http_server.c:1988` / `spec_board.c:243` / `pipeline.c:241-242` are pre-existing (outside Task #2 blocks).

Task #3 constitution (I–IX + role I–X):
- I: Task #3 DoD / US-001–004 UI half. No `.sdd-skill/` writes. Poll Gherkin left to Task #4 (not a gap this task).
- II: React 19; `archived?: boolean` + `=== true` (missing false); i18n en+zh Archive/Unarchive/Show archived; no new CSS.
- III: chrome `text-foreground/40`; `colorForLabel` / Graph untouched.
- IV.3: POST `/api/spec-board` then `await refresh()` (same `fetchBoard`). No MCP.
- V: 22 SpecBoardTab + i18n locks; mock hook + stub fetch; host selectProject/loading/not-sdd-skill hold. Poll rerender = Task #4.
- VI.3: N/A (not delete). Zero `window.confirm` / dialog / alertdialog.
- VII: accessible names + `aria-pressed`; breadcrumbs on SpecBoardTab / useSpecBoard / types / i18n.
- VIII: `POLL_MS` 4000 unchanged. No `get_graph_schema`.
- IX.2: spec-005 expand Set / blurb / Todo pending-only unchanged. Pattern Notes: ✓.
- X: `isArchived` helper; no `src/shared/` duplicate; no fourth Column.
- Monitoring: N/A. Left unchecked.

Graph Task #3 (mcp_idx=yes, index 15:40; files read as ground truth): four paths `no_recorded_issue`. `SpecBoardTab` → `useSpecBoard` → `fetch` GET; `persistArchive` → POST `/api/spec-board` then `refresh` (`refresh: fetchBoard`). `showArchived` reset with `expandedIds` on project change. Three Column children only.

Task #4 constitution (I–IX + role I–X):
- I: Task #4 DoD / US-001–004 + US-006 poll UI Thens only. No product change. No `.sdd-skill/` writes.
- II: Vitest + Testing Library; English assertions; no new CSS / i18n key / runtime.
- III.2: `colorForLabel("Function") === "#06b6d4"` locked in suite (`colors.test.ts`).
- IV.3: POST stays `/api/spec-board` (mocked). No MCP.
- V.1/V.2/V.4: every UI Gherkin Then has a Vitest owner; hook-mock + fetch stub; no live daemon.
- VI.3: N/A. Zero confirm/dialog/alertdialog asserted.
- VII: title button contains spec id; breadcrumbs on SpecBoardTab.test.tsx.
- VIII: poll interval unchanged (rerender simulates GET). No `get_graph_schema`.
- IX.2: spec-005 expand stays; document-wide Archive ban replaced (names only on expanded Done). IX.4: no Playwright. Pattern Notes: ✓.
- X: `expectNoArchiveOrUnarchive` scoped; no `src/shared/` duplicate.
- Monitoring: N/A. Left unchecked.

Graph Task #4 (mcp_idx=yes, index 15:57; files read as ground truth): `SpecBoardTab.test.tsx` + `SpecBoardTab.tsx` `no_recorded_issue`. `persistArchive` → `fetch` POST + `refresh`. `SpecBoardTab` → `useSpecBoard` / `isArchived` / `byColumn` / `Column`. No GraphTab/Three in the test file.

## Testing — @tester fills
Task #4 DEV 2026-08-30 (graph-ui Vitest; Playwright not run — LEVEL 1 Unit):
[x] happy paths (Task #4 UI) [x] edge cases (Task #4 Gherkin) [ ] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (count/leftover/empty-done/poll-resurrect)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| graph-ui Vitest | 156 passed / 23 files | PASS DEV | Task #4 owned: 24 SpecBoardTab + 3 colorForLabel (27/27). Gherkin: hide without confirm; Show archived + Unarchive; fresh visit hidden; Done count; leftover Todo; all-Done-archived empty + toggle; poll GET archived:true does not resurrect. Host selectProject/loading/not-sdd-skill. colorForLabel Function #06b6d4. Last task. HTTP C Gherkin = Tasks #1–#2 |

Task #3 DEV 2026-08-30 (graph-ui Vitest; Playwright not run — LEVEL 1 Unit):
[x] happy paths (Task #3 UI) [x] edge cases (Task #3 Gherkin) [ ] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (count/leftover/empty-done)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| graph-ui Vitest | 154 passed / 23 files | PASS DEV | SpecBoardTab 22/22. Poll-resurrect was Task #4. |

Task #2 DEV 2026-08-30 (C unit; Playwright not run — LEVEL 1 HTTP/spec_board):
[x] happy paths (Task #2 HTTP) [x] edge cases (Task #2 Gherkin) [x] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (orphan/leftover/idempotent/400/404/409)
Tools: [x] unit [C] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| spec_board + httpd (C) | 105 passed / 1 skip | PASS DEV | Gherkin: GET done+archived; orphan not invented; leftover todo JSON true column unchanged; repeat POST idempotent; POST 200 flag object; 409 spec not done + active.json unchanged; 404 spec/project; 400 missing project / invalid archived; to_json archived; publish_staged copy. UI hide/toggle = Tasks #3–#4 |

Task #1 DEV 2026-08-30 (C unit; Playwright not run — LEVEL 1 store-only):
[ ] happy paths (all US) [x] edge cases (Task #1 Gherkin) [x] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [C] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| store_spec_archive (C) | 9/9 | PASS DEV | Gherkin: orphan row survives load; repeat archive idempotent. DoD: schema, UPSERT, empty/NULL spec_id ERR, missing-table count 0, copy src→dst, no DELETE on 0, copy missing src no-op. HTTP invent-card / POST / UI = Tasks #2–#4 |

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
/sdd-skill hotfix spec-006-k3n-spec-archive "Bug: [d]" → creates spec-006-k3n-hotfix-1

## Notes & Blockers
Blockers: [date — desc — RESOLVED/PENDING] | Design changes: [date — what — why — @architect approval] | Perf issues: [issue — status — resolution]
