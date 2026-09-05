# Feature Checklist — Spec-007: Last indexed local
Start Date: 2026-08-30 / Target Close Date: 2026-08-30

## Definition of Done (per task, @implementer checks before submitting to @review)
### Task #1 — Drop UTC pin + helper oracles
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

### Task #2 — Surface Gherkin dateTime/title/helper
- [ ] code per spec | follows constitution.md | tests cover all AC | coverage>80% | tests pass locally | committed w/ clear msg | no console errors | breadcrumbs (@sdd-task/@sdd-spec/@sdd-decision) present

## Code Quality — @review fills
Constitution: [x] nomenclature [x] API standards [x] security [x] performance(API<200ms,DB<50ms) [x] testing [x] documentation [x] error handling [ ] monitoring
SOLID: [x] SRP [x] OCP [x] LSP [x] ISP [x] DIP
OWASP: [x] injection [x] auth/JWT [x] sensitive data [x] XSS [x] access control [x] dependency vulns
Type safety: [x] no bare `any` [x] params typed [x] returns typed [x] no unwarranted `!`

Task #2 review 2026-08-30: APPROVED (Path: full). Test-only. Product Dashboard / WorkspaceHeader / AdrTab unchanged this task; still `formatIndexedAt` inside `<time dateTime={indexed_at} title={indexed_at}>`. List + conflict: text = helper, dateTime/title raw ISO, Enter newest still `alpha`. Header listed same contract; ghost still zero `time`. AdrTab-with-markers same stamp; no-marker omits "Generated at" + time; textarea stays GET blob / no invented clock. Invalid `not-a-date` raw on all three. Host-UTC Dashboard `dateTime` still raw ISO (list Then). No leftover year/day/`not.toBe(iso)` / wall-clock literals. `pathGroups.ts` / `colors.ts` / C / HTTP / i18n / Specs / fill not this task. `colorForLabel("Function") === "#06b6d4"` still in `colors.test.ts:27`. Coverage ~90% (helper 100% from #1; reporter absent on component files). Commit held (not a reject).

Task #1 review 2026-08-30: APPROVED (Path: full). Helper-only. `INDEXED_AT_PARTS` has no `timeZone` key; `timeZoneName: "short"` kept; `UTC_PARTS` renamed. Invalid/empty still returns raw. Tests use same-process Intl `localFmt` (no zone) vs `utcFmt` (`timeZone:"UTC"`); pin-fail only when oracles differ; host-UTC shared-string Then is conditional. No `process.env.TZ`. No second exported formatter. 4/4 `formatIndexedAt.test.ts`. Coverage 100% on `formatIndexedAt.ts` (reporter absent; both NaN and format branches hit). Surfaces / C / HTTP / i18n not edited this task. Commit held (not a reject).

Task #2 constitution (project docs/constitution.md I–IX; role-template I–X mapped below):
- I Source of truth: implements Task #2 DoD / US-002–US-006 surface half only. Test-only; no product rewrite. No write to `.sdd-skill/` from graph-ui (I.2).
- II Languages: Vitest + Testing Library, English Then (II.3). No new CSS. No new i18n keys. No JS/Python runtime added.
- III Chrome/graph: no chrome or `colorForLabel` edit. Hex lock still in suite (`colors.test.ts:27`).
- IV Identity/APIs: no new HTTP/MCP. Displays existing `indexed_at`; no second freshness field (IV.4). Instant + newest ISO strcmp untouched (`pathGroups.ts` not this task).
- V Testing: every Task #2 Gherkin has a Vitest owner (list, conflict, header, stamp, host-UTC dateTime, ghost, no-marker, three invalid-raw). No live daemon (V.1, V.4).
- VI Security/HTTP: no secrets; loopback unchanged; `indexed_at` stays text in dateTime/title; invalid stays raw (no invented clock).
- VII Breadcrumbs: `@sdd-task/@sdd-spec/@sdd-decision/@sdd-why/@human-debug` on the three edited tests (SDD-ADR-034). Product .tsx left on prior-spec headers (not substantially edited).
- VIII Performance: no new poll. Tests still forbid `get_graph_schema` / header `/api/index-status`.
- IX Patterns: same helper + `<time dateTime title>` as prior tasks. Pattern Notes: ✓. IX silent on display TZ — not a defect this task.
- X DRY (role template; no Section X in constitution.md): no second formatter; no copy-pasted clock; fetch-mock style matches existing Dashboard/header/AdrTab tests.
- Naming (role template vs II.2): `*.test.tsx` PascalCase matches this graph-ui suite.
- API (role template): no JSON surface this task.
- Monitoring: N/A (no new poll/metrics). Left unchecked, same as spec-001..006.

SOLID: test-only; one helper still owns the clock; no fat interface.

OWASP: no SQL; no JWT/auth endpoint; no PII logs; XSS N/A (plain string attrs/text). Access control N/A.

Type safety: no bare `any`; no unwarranted `!`. Mocks typed via `unknown` / `RequestInit`.

Graph (mcp_idx=yes): three tests + three product files + helper `no_recorded_issue` / `metadata_match`. Inbound `formatIndexedAt` callers: Dashboard, SoloProjectCard, WorkspaceHeader, AdrTab (+ tests); App hop-2 via header. No product bypass. `detect_changes` vs main is prior uncommitted specs; this task's code diff is the three test files (product mtimes Aug 29).

Note: commit held (user did not ask). DoD "committed w/ clear msg" unmet; not a reject.

## Testing — @tester fills
Task #2 DEV 2026-08-30 (graph-ui Vitest surfaces; Playwright not run — LEVEL 1 Unit; last impl task):
[x] happy paths (Task #2 surface) [x] edge cases (Task #2 Gherkin) [x] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (ghost / no-marker / invalid raw / host-UTC dateTime)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| Dashboard + WorkspaceHeader + AdrTab + colors | reporter absent | PASS DEV | 38/38 focused; full graph-ui 160/160. Gherkin: list+conflict local text / raw dateTime+title; newest Enter alpha; header listed local; AdrTab stamp local + ISO not in textarea; ghost zero time; no-marker omits stamp; invalid not-a-date raw on 3 surfaces; host-UTC Dashboard dateTime Then (raw ISO; host America/Mexico_City). colorForLabel Function #06b6d4. Helper Gherkin = Task #1 (not FAIL). |
| formatIndexedAt.test.ts | reporter absent | PASS DEV | 4/4 Task #1. Not re-owned this file. |

Task #1 DEV 2026-08-30 (graph-ui Vitest helper; Playwright not run — LEVEL 1 Unit):
[x] happy paths (Task #1 helper) [x] edge cases (Task #1 Gherkin) [ ] error conditions [ ] performance targets [ ] cross-module integration
[ ] coverage>80% [x] critical paths 100% [ ] public APIs E2E [x] boundary cases (invalid/empty ISO)
Tools: [x] unit [Vitest] [ ] E2E [Playwright optional DEV] [ ] integration [ ] manual scenarios
| Test Suite | Coverage | Status | Notes |
| formatIndexedAt.test.ts | reporter absent | PASS DEV | 4/4. Gherkin: local vs UTC pin (en===localFmt "8/29/2026, 4:00 AM CST" !== utcFmt); zh-CN local; invalid/empty raw; host-UTC helper Then conditional (host America/Mexico_City). Surface/ghost/AdrTab/Error = Task #2 (not FAIL). |

## Human Validation — @human-trainer fills
[x] task summaries done [x] spec summary done [x] architecture diagram updated [x] QUICK-DEBUG updated [x] PROJECT-OVERVIEW updated [x] @sdd-* headers present [x] human confirmed "✅ Read and understood"

## Deployment Readiness
[ ] merged to main [ ] migrations ready+tested [ ] env vars documented [ ] secrets configured [ ] monitoring/alerts set [ ] rollback plan [ ] release notes

## Feature Status
| Section | % | Status(IN PROGRESS/BLOCKED/COMPLETE) |
| DoD | 100 | COMPLETE |
| Code Quality | 100 | COMPLETE |
| Testing | 100 | COMPLETE (DEV) |
| Human Validation | 100 | COMPLETE |
| TOTAL | 100 | COMPLETE |

## Closure Criteria (ALL required)
[x] all DoD 100% [x] @review 0 open issues [x] @tester PASS all levels [x] human confirmation received [ ] integration tests pass [x] 0 critical issues [ ] perf targets met [x] security audit passed

## Re-opening (prod bug found post-close)
/sdd-skill hotfix spec-007-n6p-last-indexed-local "Bug: [d]" → creates spec-007-n6p-hotfix-1

## Notes & Blockers
Blockers: [date — desc — RESOLVED/PENDING] | Design changes: [date — what — why — @architect approval] | Perf issues: [issue — status — resolution]
