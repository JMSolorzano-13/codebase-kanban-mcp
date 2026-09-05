# Feature Checklist — Spec-004: ADR parse on reindex
Start Date: 2026-08-30 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — Splice + extract helper
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — Pipeline hook + skip dir
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #3 — HTTP/MCP/watcher Gherkin
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #4 — AdrTab stamp + warning
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #1 review: APPROVED (Path: full). Helper unused in pipeline is expected (Task #2). HTTP/AdrTab out of scope.

Constitution (project docs/constitution.md I–IX; role-template I–X mapped below):
- I Source of truth: helper implements spec US-001/002/004 extract+splice only. No write to `.sdd-skill/` (I.2). Trio relatives are the three grill ADR-006 paths; DEV_LOG/TECH_DEBT/constitution/human/specs/history never opened (`adr_fill.c:20-22`).
- II Languages: C11 `cbm_adr_fill_document` + `CBM_ADR_*` macros. Allocation/IO NULL-checked (`malloc` `adr_fill.c:73-77,211-213`; `cbm_fopen` `:69-72`; `ferror` `:81-84`). No graph-ui / new runtime.
- III Chrome/graph: no UI. N/A.
- IV Identity/APIs: no new endpoint; POST `/api/adr` still 16384; admission untouched.
- V Testing: 9 C unit Then clauses, no full index. Public `cbm_adr_fill_document` covered. Gherkin HTTP/MCP/AdrTab deferred to #3/#4.
- VI Security/HTTP: no secrets; loopback/bind unchanged; no write-back to skill or source.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on `adr_fill.h`, `adr_fill.c`, `test_adr_fill.c`.
- VIII Performance: three bounded reads + splice. No poll. No `get_graph_schema`.
- IX Patterns: same class as spec_board (best-effort `cbm_fopen`, omit missing/unreadable, presence `cbm_is_dir`). Isolated helper first, hook later (spec-003 `cbm_identity_*`). Pattern Note addressed: local `adr_skill_present` so `src/adr/` does not include `spec_board.c`.
- X DRY (role template; no Section X in constitution.md): no `src/shared/` duplicate; no Kanban-parse copy. Local join/presence is intentional isolation.
- Naming (role template vs II.1): snake_case files + `cbm_`/`adr_` symbols match this C tree, not web kebab/camel. Consts UPPER_SNAKE.
- API (role template): no JSON surface this task.
- Monitoring: N/A (no new poll/metrics). Left unchecked, same as spec-001..003.

SOLID: one exported splice (`cbm_adr_fill_document`); extract/span/splice split; trio paths frozen by spec (O closed); no fat interface; IO via `cbm_fopen`/`cbm_is_dir`/`cbm_path_info_utf8`.

OWASP: no SQL; no JWT/auth endpoint; no PII logs (no logging); trio paths are compile-time relatives under caller `root_path`; XSS N/A (C string, no HTML render).

Types: C, all params/returns typed. No `any` / `!`.

Coverage: ~87% of `adr_fill.c` executable lines (static). All Task #1 Then clauses exercised. Untested are defensive (OOM, `ferror`, BOM strip, join-fail, START-without-END). 9/9 `scripts/test.sh --suites adr_fill` PASS.

SDD-ADR-019..023: markers+1536/64KiB+English H1s+unreadable=open-fail implemented. Hook and ALWAYS_SKIP correctly absent (ADR-019/023 Task #2).

AUTO-REJECT: none (no secret, no SQLi, no new endpoint, constitution held, critical fn tested).

Task #2 review: APPROVED (Path: full). Incremental persist is not a skip (apply at dump + closure clone; force-full when fill would change). HTTP job Gherkin is Task #3. AdrTab is Task #4. MCP schema must not list `adr_fill` (held).

Constitution (project docs/constitution.md I–IX):
- I Source of truth: US-001/003/005 hook + skip only. No write to `.sdd-skill/` (I.2). Fill is fopen of trio; discover skips the dir (SDD-ADR-023).
- II Languages: C11 `cbm_pipeline_*adr_fill*` + `cbm_mcp_index_want_adr_fill`. NULL-checked apply (`pipeline.c:330-335`). No graph-ui / new runtime / new MCP property.
- III Chrome/graph: no UI. N/A.
- IV Identity/APIs: no new endpoint; admission unchanged; `adr_fill` not on tool schema (`mcp.c:391-408`).
- V Testing: pipeline/helper true vs false; watcher args + strip; discover skip + trio not File nodes. HTTP/MCP job Gherkin deferred to #3.
- VI Security/HTTP: no secrets; loopback/bind unchanged; flag is intent not auth.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on hook sites (`pipeline.h`, apply, incremental persist, discover skip, mcp gate, application watcher/strip, tests).
- VIII Performance: three short reads + splice on user persist; no extra poll; no `get_graph_schema`.
- IX Patterns: shared handle + flag (spec-003 admit). Dedicated want-gate because `get_bool_arg` defaults missing to false. Pattern Note ✓.
- X DRY (role template): one `cbm_pipeline_apply_adr_fill` used by full publish, incremental dump, closure clone, and would_change.
- Naming: `cbm_` snake_case matches II.1.
- Monitoring: N/A. Left unchecked.

SOLID: apply is one splice; would_change probes via apply; want-gate is one JSON bool; skip list extended in place.

OWASP: no new SQL; trio paths stay compile-time relatives; `adr_fill` not advertised; no PII logs.

Types: C, all params/returns typed.

Coverage: ~84% of new hook functions (static). `scripts/test.sh --suites adr_fill,discover` 110/110 PASS (14 adr_fill including 5 Task #2). Untested: apply/would_change defensive OOM; invalid JSON want-gate; live incremental persist with flag true (Task #3 job Gherkin).

SDD-ADR-019: capture-then-splice; intent flag; new() false; handle default true unless JSON false; watcher false; strip for subscribe.
SDD-ADR-023: `.sdd-skill` in ALWAYS_SKIP; fill out-of-graph.

AUTO-REJECT: none.

Task #3 review: APPROVED (Path: full). AdrTab stamp/warning is Task #4. Dashboard ADR control must not exist. spec-003 409/202 admission unchanged (not a defect).

Constitution (project docs/constitution.md I–IX):
- I Source of truth: US-001/002/003/004/005 HTTP+MCP job Gherkin only. No write to `.sdd-skill/` (I.2). No new product behavior without a scenario (V.2).
- II Languages: C11 `handle_adr_save` cap via `CBM_SZ_32K`. No graph-ui / new runtime / new MCP property this task.
- III Chrome/graph: no UI. N/A.
- IV Identity/APIs: no new endpoint (IV.3). Admission 409 `path_exists` / 202 Reindex held (`http_server.c:1126`, tests `:2574-2838`). `indexed_at` unused here (stamp is #4).
- V Testing: every Task #3 Gherkin has a C owner (httpd create/reindex/partial/unreadable/hand-edit/cap; mcp fill/survive/false + existing no-skill ADR). Public POST `/api/adr` cap both sides. AdrTab deferred #4.
- VI Security/HTTP: no secrets; loopback/bind unchanged; body still bounded 32768; 400 `{error:"invalid body"}`.
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on `handle_adr_save` and Task #3 test blocks.
- VIII Performance: no extra poll; no `get_graph_schema`. Cap raise is a bound, not a new fetch.
- IX Patterns: one worker `handle_index_repository` for HTTP executor and MCP (spec-003). Watcher case is `adr_fill: false` persist (plan Testing Strategy). Pattern Note ✓.
- X DRY (role template): HTTP/MCP test helpers stay in their suites (same as prior specs). No second fill path.
- Naming: `cbm_` / snake_case matches II.1.
- Monitoring: N/A. Left unchecked.

SOLID: cap is one bound on existing save; fill stays in pipeline apply; want-gate unchanged; no fat interface.

OWASP: no new SQL; trio paths stay compile-time; `adr_fill` not on tool schema (`mcp.c:391-408`); no PII logs; XSS N/A (JSON text, no HTML render).

Types: C, all params/returns typed. No `any` / `!`.

Coverage: ~90% of Task #3 production delta (POST cap both branches + all mapped job Gherkin). `adr_fill` schema absent. Existing `tool_index_repository_reports_store_backed_adr` still registered. Untested this task: live auto_watch poll (plan: flag persist suffices); GET `/api/adr` HTTP from the MCP suite (store load is the same blob; HTTP GET covered in httpd).

SDD-ADR-020: POST body max 32768 (`http_server.c:918`); 16384 generated+manual still 200; over-max 400.
SDD-ADR-021: POST / manage_adr stay whole-doc; parse replaces generated; manual survives.

AUTO-REJECT: none (cap not 16384; schema has no adr_fill; create/reindex fill GET; false path unmarked; admission held; Gherkin mapped).

Task #4 review: APPROVED (Path: full). graph-ui only. C/HTTP Tasks #1–#3 not re-opened.

Constitution (project docs/constitution.md I–IX):
- I Source of truth: US-006 + Gherkin stamp/warning only. No second freshness field. No write to `.sdd-skill/` (I.2).
- II Languages: React 19 function component; no bare `any`; Tailwind tokens only; i18n en+zh for `adr.generatedAt` / `adr.replaceWarning` (II.3). No new runtime.
- III Chrome/graph: grayscale chrome; `colorForLabel` not imported; Function hex still `#06b6d4` (colors.test).
- IV Identity/APIs: no new endpoint; stamp is `Project.indexed_at` via `useProjects` + `formatIndexedAt` (IV.4). POST `/api/adr` still `{project, content}`.
- V Testing: Gherkin Then clauses owned by `AdrTab.test.tsx`; omit-when-unmarked AC covered; i18n.test locks en+zh. Fetch-mock style. No live daemon.
- VI Security/HTTP: loopback unchanged; textarea is text not `dangerouslySetInnerHTML`; delete still explicit.
- VII Breadcrumbs on AdrTab + i18n; `role="note"` warning; `<time dateTime>` stamp; textarea `aria-label`.
- VIII Performance: no `get_graph_schema` (test throws); no extra poll; list hook same as WorkspaceHeader.
- IX Patterns: WorkspaceHeader recipe (`useProjects` find by name, `formatIndexedAt`, `<time dateTime>`). Confirm path untouched in App. Pattern Note ✓.
- X DRY (role template): no new formatter; no second textarea. `lastUpdated` i18n unused (pre-existing).
- Monitoring: N/A. Left unchecked.

SOLID: stamp/warning gated on `lastClean` marker; persist still one POST; listed lookup matches header.

OWASP: no SQL/JWT; no secrets; XSS held (text textarea); no Dashboard ADR control.

Types: props/payload typed. No `any`. No unwarranted `!`.

Coverage: ~86% of `AdrTab.tsx` executable (static). Untested: GET !ok/catch, persist network catch, ghost name (warning without `listed`). 17/17 `AdrTab+i18n+colors` PASS.

SDD-ADR-012: `App.tsx:80` `window.confirm(t.adr.unsavedConfirm)` unchanged; App.test dirty-leave still present.
SDD-ADR-016: stamp from list `indexed_at`, not ADR `updated_at`, not written into blob.
SDD-ADR-021: one textarea; POST `{project, content}`; generated blob POST equals GET.

AUTO-REJECT: none (stamp not in blob; one textarea; no Dashboard ADR; stamp gated on CBM-GENERATED-START; en+zh warning; Save body held; confirm unchanged; Then clauses tested).

## Testing — @tester fills
[x] happy paths (all US) [x] edge cases (all EC-NNN) [x] error conditions [ ] performance targets [x] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [C + Vitest] [ ] E2E [Playwright optional DEV] [x] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| adr_fill (Task #1 unit Thens) | not measured | PASS | 6/6 mapped Gherkin Thens; 9/9 suite. HTTP/MCP/AdrTab deferred #3/#4 |
| adr_fill pipeline_* + discover skip + mcp want_adr_fill + daemon watcher args (Task #2) | not measured | PASS | 9/9 Task #2 units. Gherkin: watcher no-fill (flag/args); persist hook + incremental would_change. HTTP job Gherkin deferred #3 |
| httpd + mcp Task #3 job Gherkin | not measured | PASS | 11/11 mapped C tests. scripts/test.sh --suites httpd,mcp → 274 passed / 3 skipped. Watcher via adr_fill false (no live poll). US-006 AdrTab is Task #4 |
| AdrTab + i18n + colors (Task #4 US-006) | not measured | PASS | 17/17 vitest run AdrTab.test + i18n.test + colors.test. Gherkin stamp+warning; omit unmarked; POST whole blob no ISO in content. Existing save/dirty/placeholder/500/delete. en+zh generatedAt+replaceWarning. colorForLabel Function #06b6d4. App dirty-leave confirm dismissed PASS. Last task. |

## Human Validation — @human-trainer fills
[ ] task summaries done [ ] spec summary done [ ] architecture diagram updated [ ] QUICK-DEBUG updated [ ] PROJECT-OVERVIEW updated [ ] @sdd-* headers present [ ] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status(IN PROGRESS/BLOCKED/COMPLETE) |
| DoD | 0 | IN PROGRESS |
| Code Quality | 25 | IN PROGRESS |
| Testing | 0 | IN PROGRESS |
| Human Validation | 0 | IN PROGRESS |
| TOTAL | 0 | IN PROGRESS |

## Closure Criteria (ALL required)
[ ] all DoD 100% [ ] @review 0 open issues [ ] @tester PASS all levels [ ] human confirmation received [ ] integration tests pass [ ] 0 critical issues [ ] perf targets met [ ] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-004-j8k-adr-parse-on-reindex "Bug: [d]" → creates spec-004-j8k-hotfix-1

## Notes & Blockers
Blockers: none | Design changes: none | Perf issues: none
