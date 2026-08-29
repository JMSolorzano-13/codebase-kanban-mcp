# Feature Checklist — Spec-003: Path–Project Identity
Start Date: 2026-08-29 / Target Close Date: TBD

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — Catalog + list fields
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg (held — commit only on request)

### Task #2 — Admit HTTP/MCP/jobs
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg (held — commit only on request)

### Task #3 — Conflict UI
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [ ] committed w/ clear msg (held — commit only on request)

### Task #4 — Create 409 redirect
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [x] committed w/ clear msg

### Task #5 — Reindex + compose
- [x] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present
- [x] committed w/ clear msg

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`
Task #1 review: APPROVED. Catalog uses store APIs (no raw SQL in prod). Foundation helpers have no store link. list_projects extra fields are additive JSON. Monitoring N/A (no new poll).
Task #2 review: APPROVED. Shared admit (no UNIQUE SQL). HTTP 409 JSON matches spec. MCP isError text has codes. Slot after admit. Test SQL is fixture-only. Monitoring N/A.

## Testing — @tester fills
[x] happy paths (all US) [x] edge cases (all EC-NNN) [x] error conditions [ ] performance targets [ ] cross-module integration
[x] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (empty/null/max)
Tools: [x] unit [Vitest + C] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| identity (C) | 18/18 Task #1 + #2 admit/MCP | PASS DEV | newest, admit, MCP reindex/clone/name_exists |
| httpd (C) | 69/69 + 1 skip | PASS DEV | 7 new: 409 path/name, trailing slash, tie, inflight, reindex + project_name |
| useProjects + i18n (Vitest) | 8/8 | PASS DEV | type still accepts Project; no schema RPC |
Task #2 Gherkin owned here: HTTP 409/202 + MCP Then clauses.

Task #3 DEV 2026-08-29 (Vitest; Playwright not run — LEVEL 1):
| Test Suite | Coverage | Status | Notes |
| Dashboard.test.tsx conflict | 4/4 Gherkin | PASS DEV | two-clone Enter newest; confirm delete older; three-clone a3 + per-older Delete; cancel keeps conflict |
| pathGroups.test.ts | 3/3 | PASS DEV | canonical_root group; root_path fallback; newest then greater name |
| i18n.test.ts | 6/6 | PASS DEV | conflict + deleteNamed en+zh |
| graph-ui Task #3 files | 25/25 | PASS DEV | includes spec-001 Dashboard list/modal regression |
Task #3 Gherkin owned here: US-004/005. Reindex Gherkin waits for #5.

Task #4 DEV 2026-08-29 (Vitest; Playwright not run — LEVEL 1):
| Test Suite | Coverage | Status | Notes |
| CreateIndexModal.test.tsx | 5/5 | PASS DEV | no Project ID; POST {root_path} only; path_exists → onPathExists no onCreated; name_exists stays; listed Path skips POST |
| App.test.tsx Task #4 | 4/4 Gherkin+notice | PASS DEV | 409 path_exists → Graph + role=status; name_exists stays no project=foo; skip POST still redirects; notice clears on leave |
| i18n.test.ts | 6/6 | PASS DEV | pathExistsNotice en+zh interpolates name |
| graph-ui Task #4 files | 31/31 | PASS DEV | includes spec-001/002 App routing regression |
Task #4 Gherkin owned here: US-001/006. Reindex Gherkin waits for #5.

## Human Validation — @human-trainer fills
[x] task summaries done [ ] spec summary done [x] architecture diagram updated [x] QUICK-DEBUG updated [ ] PROJECT-OVERVIEW updated [x] @sdd-* headers present [ ] human confirmed "✅ Read and understood"
Task #2 summary: `human/task-summaries/task-2-admit-create-reindex.md`

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status(IN PROGRESS/BLOCKED/COMPLETE) |
| DoD | 60 | IN PROGRESS |
| Code Quality | 40 | IN PROGRESS |
| Testing | 40 | IN PROGRESS |
| Human Validation | 10 | IN PROGRESS |
| TOTAL | 30 | IN PROGRESS |

## Closure Criteria (ALL required)
[ ] all DoD 100% [ ] @review 0 open issues [ ] @tester PASS all levels [ ] human confirmation received [ ] integration tests pass [ ] 0 critical issues [ ] perf targets met [ ] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-003-h7q-path-project-identity "Bug: [d]" → creates spec-003-h7q-hotfix-1

## Notes & Blockers
Blockers: none | Design changes: none | Perf issues: none
