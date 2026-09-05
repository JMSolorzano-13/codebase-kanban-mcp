# Spec-004-j8k: ADR parse on reindex
Status: closed | Spec ID: spec-004-j8k-adr-parse-on-reindex
Ref Ticket: none | KPI: After a successful user-triggered index of a project that has the Phase 2 trio, GET /api/adr contains a generated region derived from those three files and any prior manual text is still present | Priority: P0
Created: 2026-08-30 | Tech Debt Ref: none

## Executive Summary
Phase 1 (spec-002) made ADR a workspace tab over one markdown blob. That blob is still empty or hand-written. Grill epic-004 is Phase 2: on a user-triggered index only, parse three sdd-skill files into a generated region. No LLM. The manual region is never rewritten by parse. Watcher and incremental must not fill.

Business impact: an sdd-skill repo's ADR tab matches PURPOSE / STACK / decisions after Reindex or `index_repository`, the same family as Specs parse and node extraction, without deleting human notes.

## User Stories

### US-001: User-triggered index fills the generated region from the trio
As an operator, I want Dashboard Reindex (and first create-index) of an sdd-skill repo to fill the ADR generated region from the Phase 2 trio, So that architecture notes appear without an LLM.
Acceptance Criteria:
- [ ] Trigger is a successful user-triggered index only: POST `/api/index` (bare create or `{root_path, project}` Reindex) and MCP/CLI `index_repository`. Same fill rules for first create and later reindex
- [ ] Sources, relative to that project's `root_path`, are exactly `.sdd-skill/context_ai.md`, `.sdd-skill/baseline/TECH_STACK.md`, `.sdd-skill/baseline/ARCHITECTURE_ADR.md` (grill ADR-006)
- [ ] After the job completes, GET `/api/adr?project=<name>` content includes `<!-- CBM-GENERATED-START -->` and `<!-- CBM-GENERATED-END -->`
- [ ] The generated region contains a unique substring from each trio file that exists and is readable
- [ ] DEV_LOG.md, TECH_DEBT.md, constitution.md, human/*, specs/*, history/* are not copied into the generated region
- [ ] The three files are read from disk. They are not added to the code graph as indexed source
- [ ] Parse is deterministic and zero-token (no LLM, no network completion)

### US-002: Manual region survives parse; unmarked Phase-1 blobs migrate to manual
As an operator, I want hand-written ADR text to survive Reindex, So that parse cannot delete notes.
Acceptance Criteria:
- [ ] Parse never writes bytes between `<!-- CBM-MANUAL-START -->` and `<!-- CBM-MANUAL-END -->`
- [ ] A Phase-1 blob with no markers: first user-triggered index treats the entire existing content as the manual region, then prepends a generated region. Existing text is not treated as generated
- [ ] Empty ADR + trio present: generated is filled; manual region exists and is empty (or whitespace only)
- [ ] POST `/api/adr` and `manage_adr(mode=update)` still send a whole document. After save, the next user-triggered index refreshes generated from the trio and keeps the manual-region text from that saved document
- [ ] If a saved document has no markers, the next parse migrates the whole saved body to manual (same as Phase-1)

### US-003: No skill and watcher incremental do not fill
As an operator, I want projects without sdd-skill and background incremental to leave ADR alone, So that auto-watch cannot overwrite or invent an ADR.
Acceptance Criteria:
- [ ] Project with no `.sdd-skill/` directory: user-triggered index does not insert `CBM-GENERATED` markers; prior ADR content is unchanged
- [ ] Watcher / auto_watch / incremental / supervised-worker incremental does not run this fill even if the trio changes on disk
- [ ] Index job success does not depend on parse (parse failure is not a job failure)

### US-004: Partial trio and unreadable files are best-effort
As an operator, I want a missing or unreadable trio file to omit only that extract, So that Reindex still completes like Specs parse.
Acceptance Criteria:
- [ ] `.sdd-skill/` present but only a subset of the trio exists: generated includes extracts for files that exist; omitted files contribute no extract
- [ ] One trio file unreadable: job still succeeds; that file's extract is omitted; readable siblings are still present
- [ ] All three missing under an otherwise present `.sdd-skill/`: markers are written; generated region has no trio extracts; manual (if any) is preserved
- [ ] No error banner on the ADR tab is required for a missing/unreadable file (silent best-effort, same class as Specs)

### US-005: MCP and CLI use the same fill gate as HTTP
As an agent, I want `index_repository` to fill the same store the ADR tab reads, So that UI and MCP stay one backend.
Acceptance Criteria:
- [ ] Successful `index_repository` on a project that owns that Path fills the generated region (spec-003 admission unchanged)
- [ ] `manage_adr(mode=get)` after that job returns the same markers and extracts as GET `/api/adr`
- [ ] `manage_adr(mode=update)` of the manual region survives the next user-triggered index; generated is refreshed from the trio

### US-006: ADR tab shows generated-at and a replace warning
As an operator, I want the ADR tab to show when generated content was last filled and that edits inside the generated region die on the next Reindex, So that I do not treat generated text as durable notes.
Acceptance Criteria:
- [ ] When GET content includes `CBM-GENERATED-START`, the ADR tab shows a visible datetime derived from `Project.indexed_at` via existing `formatIndexedAt` (list cache). No second freshness field
- [ ] When markers are absent, that stamp is omitted
- [ ] Visible warning (i18n en+zh) that text inside the generated region is replaced on the next user-triggered index
- [ ] Generated markdown headings inside the blob are English (skill files are English). Stamp and warning chrome are i18n
- [ ] Editor remains one textarea of the whole blob. Save still POSTs `{project, content}`. Dashboard still has no ADR control
- [ ] Dirty-leave `window.confirm` from spec-002 is unchanged

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: ADR parse on reindex

  # Happy Paths
  Scenario: Dashboard Reindex fills generated from the trio and migrates unmarked text to manual
    Given project "alpha" owns root_path "/tmp/alpha" and is idle
    And "/tmp/alpha/.sdd-skill/context_ai.md" contains "PURPOSE-ALPHA-GRAPH"
    And "/tmp/alpha/.sdd-skill/baseline/TECH_STACK.md" contains "STACK-ALPHA-C11"
    And "/tmp/alpha/.sdd-skill/baseline/ARCHITECTURE_ADR.md" contains "DECISION-ALPHA-PATH"
    And "/tmp/alpha/.sdd-skill/baseline/DEV_LOG.md" contains "SECRET-DEVLOG-ALPHA"
    And GET /api/adr?project=alpha returns {"has_adr":true,"content":"# Existing ADR\n"}
    When POST /api/index is sent with JSON {"root_path":"/tmp/alpha","project":"alpha"}
    And the index job for "alpha" completes successfully
    Then GET /api/adr?project=alpha content contains "<!-- CBM-GENERATED-START -->"
    And GET /api/adr?project=alpha content contains "<!-- CBM-GENERATED-END -->"
    And GET /api/adr?project=alpha content contains "<!-- CBM-MANUAL-START -->"
    And GET /api/adr?project=alpha content contains "PURPOSE-ALPHA-GRAPH"
    And GET /api/adr?project=alpha content contains "STACK-ALPHA-C11"
    And GET /api/adr?project=alpha content contains "DECISION-ALPHA-PATH"
    And GET /api/adr?project=alpha content does not contain "SECRET-DEVLOG-ALPHA"
    And the text between CBM-MANUAL-START and CBM-MANUAL-END contains "# Existing ADR"

  Scenario: MCP index_repository fills the same ADR blob
    Given project "alpha" owns root_path "/tmp/alpha" and is idle
    And "/tmp/alpha/.sdd-skill/context_ai.md" contains "PURPOSE-MCP-FILL"
    And GET /api/adr?project=alpha returns {"has_adr":false,"content":""}
    When index_repository completes successfully for repo_path "/tmp/alpha" and name "alpha"
    Then manage_adr mode get for project "alpha" content contains "<!-- CBM-GENERATED-START -->"
    And manage_adr mode get for project "alpha" content contains "PURPOSE-MCP-FILL"
    And GET /api/adr?project=alpha content contains "PURPOSE-MCP-FILL"

  Scenario: First create-index of a new Path with the trio writes generated and empty manual
    Given no project owns canonical Path "/tmp/beta"
    And "/tmp/beta/.sdd-skill/context_ai.md" contains "PURPOSE-BETA-NEW"
    And "/tmp/beta/.sdd-skill/baseline/TECH_STACK.md" contains "STACK-BETA-NEW"
    And "/tmp/beta/.sdd-skill/baseline/ARCHITECTURE_ADR.md" contains "DECISION-BETA-NEW"
    When POST /api/index is sent with JSON {"root_path":"/tmp/beta"}
    And the index job for the derived name "beta" completes successfully
    Then GET /api/adr?project=beta content contains "<!-- CBM-GENERATED-START -->"
    And GET /api/adr?project=beta content contains "PURPOSE-BETA-NEW"
    And GET /api/adr?project=beta content contains "STACK-BETA-NEW"
    And GET /api/adr?project=beta content contains "DECISION-BETA-NEW"
    And the text between CBM-MANUAL-START and CBM-MANUAL-END contains only whitespace or is empty

  Scenario: ADR tab shows generated-at and replace warning
    Given list_projects returns one project named "alpha" with indexed_at "2026-08-30T12:00:00Z"
    And GET /api/adr?project=alpha returns content that includes "<!-- CBM-GENERATED-START -->"
    When the operator opens "?tab=adr&project=alpha"
    Then the document contains a visible datetime derived from "2026-08-30T12:00:00Z"
    And the document contains visible warning copy that generated-region edits are replaced on the next user-triggered index
    And a textarea is present

  # Limit Cases
  Scenario: Limit Case — no .sdd-skill leaves ADR unmarked
    Given project "gamma" owns root_path "/tmp/gamma"
    And "/tmp/gamma" has no ".sdd-skill" directory
    And GET /api/adr?project=gamma returns {"has_adr":true,"content":"# Hand only\n"}
    When POST /api/index is sent with JSON {"root_path":"/tmp/gamma","project":"gamma"}
    And the index job for "gamma" completes successfully
    Then GET /api/adr?project=gamma content equals "# Hand only\n"
    And GET /api/adr?project=gamma content does not contain "CBM-GENERATED"

  Scenario: Limit Case — only context_ai.md exists; job still succeeds
    Given project "alpha" owns root_path "/tmp/alpha"
    And "/tmp/alpha/.sdd-skill/context_ai.md" contains "PURPOSE-PARTIAL-ONLY"
    And "/tmp/alpha/.sdd-skill/baseline/TECH_STACK.md" does not exist
    And "/tmp/alpha/.sdd-skill/baseline/ARCHITECTURE_ADR.md" does not exist
    When POST /api/index is sent with JSON {"root_path":"/tmp/alpha","project":"alpha"}
    And the index job for "alpha" completes successfully
    Then GET /api/adr?project=alpha content contains "PURPOSE-PARTIAL-ONLY"
    And GET /api/adr?project=alpha content contains "<!-- CBM-GENERATED-START -->"
    And the index job result is success (HTTP 202 accepted and completed, not a failed job)

  Scenario: Limit Case — watcher incremental does not fill ADR
    Given project "alpha" owns root_path "/tmp/alpha"
    And GET /api/adr?project=alpha returns {"has_adr":true,"content":"# Before watch\n"}
    And "/tmp/alpha/.sdd-skill/context_ai.md" contains "PURPOSE-WATCHER-SHOULD-NOT-FILL"
    When an auto_watch incremental update completes for "/tmp/alpha/src/foo.c" without POST /api/index and without index_repository
    Then GET /api/adr?project=alpha content equals "# Before watch\n"
    And GET /api/adr?project=alpha content does not contain "PURPOSE-WATCHER-SHOULD-NOT-FILL"

  # Error Scenarios
  Scenario: Error — unreadable ARCHITECTURE_ADR.md omits that extract and does not fail the job
    Given project "alpha" owns root_path "/tmp/alpha"
    And "/tmp/alpha/.sdd-skill/context_ai.md" contains "PURPOSE-READABLE"
    And "/tmp/alpha/.sdd-skill/baseline/TECH_STACK.md" contains "STACK-READABLE"
    And "/tmp/alpha/.sdd-skill/baseline/ARCHITECTURE_ADR.md" exists but is unreadable
    When POST /api/index is sent with JSON {"root_path":"/tmp/alpha","project":"alpha"}
    And the index job for "alpha" completes
    Then the index job result is success
    And GET /api/adr?project=alpha content contains "PURPOSE-READABLE"
    And GET /api/adr?project=alpha content contains "STACK-READABLE"
    And GET /api/adr?project=alpha content does not contain a full copy of ARCHITECTURE_ADR.md

  Scenario: Error — manage_adr manual edit survives the next Reindex
    Given GET /api/adr?project=alpha already contains CBM-GENERATED and CBM-MANUAL markers
    And the manual region contains "# Old notes"
    When manage_adr mode update for project "alpha" writes a document whose manual region is "# New notes"
    And POST /api/index is sent with JSON {"root_path":"/tmp/alpha","project":"alpha"}
    And the index job for "alpha" completes successfully
    Then the text between CBM-MANUAL-START and CBM-MANUAL-END contains "# New notes"
    And the text between CBM-MANUAL-START and CBM-MANUAL-END does not contain "# Old notes"

  Scenario: Error — generated-region edits are replaced on the next Reindex
    Given "/tmp/alpha/.sdd-skill/context_ai.md" contains "PURPOSE-CANONICAL"
    And GET /api/adr?project=alpha content contains "<!-- CBM-GENERATED-START -->"
    When POST /api/adr is sent with JSON whose generated region contains "PURPOSE-HAND-EDIT" and does not contain "PURPOSE-CANONICAL"
    And POST /api/index is sent with JSON {"root_path":"/tmp/alpha","project":"alpha"}
    And the index job for "alpha" completes successfully
    Then GET /api/adr?project=alpha content contains "PURPOSE-CANONICAL"
    And GET /api/adr?project=alpha content does not contain "PURPOSE-HAND-EDIT"
```

## Success Metrics
| Metric | Target | Current | Status |
| User-triggered index + trio → generated extract present | 100% | GET /api/adr has generated extract after create/Reindex/`index_repository` | met |
| Manual / Phase-1 text after parse | 100% preserved | unmarked migrates to CBM-MANUAL; manage_adr survive | met |
| Watcher incremental fills | 0 | adr_fill false / watcher args leave blob unmarked | met |
| Non-trio skill files in generated | 0 | DEV_LOG and siblings never opened | met |
Primary KPI: first metric plus manual preserved.

## Constraints & Assumptions
Technical:
- One SQLite `project_summaries` blob. Markers live in that markdown. No second table required unless architect proves splice cannot be done in-document
- Prefer existing POST `/api/index`, `index_repository`, GET/POST `/api/adr`, `manage_adr`. New endpoints only if architect proves fill cannot hook the current job
- POST `/api/adr` body limit is 16384 today. Generated content is a bounded extract, not a byte-copy of the trio (this repo's trio is already ~22KiB). Architect sets the extract algorithm and may raise the HTTP limit; Gherkin uses small fixture strings
- Pipeline already captures/restores ADR across full reindex (`saved_adr`). Parse must run after that restore so it splices, not fights, preservation
- graph-ui Vitest for US-006. C tests for fill, migrate, skip-no-skill, skip-watcher, partial, unreadable, MCP parity
- Chrome stays spec-001 grayscale. GraphTab `colorForLabel` hex unchanged
- Constitution I.2 / IX: do not write agent cycle files from the indexer; do not index `.sdd-skill/` as code graph
- spec-003 admission unchanged (409 `path_exists`, Reindex `{root_path, project}`)

Business:
- Grill epic 004 only. No LLM fill. No write-back to `.sdd-skill/` or source
- Specs tab and graph nodes stay read-only adapters

Planner defaults (reject a Gherkin scenario to change these):
1. Markers are the four HTML comments `CBM-GENERATED-START/END` and `CBM-MANUAL-START/END`. Order: generated block, then manual block
2. Unmarked Phase-1 (or unmarked save) → entire body becomes manual; generated is prepended
3. First create-index and later Reindex both fill. Watcher/incremental never fill
4. Dashboard Reindex and MCP/CLI `index_repository` both fill
5. Missing/unreadable trio file = omit that extract; job still succeeds; no ADR-tab error required
6. `.sdd-skill/` present but trio all missing → markers written, generated empty of extracts
7. No `.sdd-skill/` → no markers added; content unchanged
8. Whole-doc update (HTTP and manage_adr) remains allowed. Parse, not the write API, is what protects manual. Generated hand-edits die on next user-triggered index
9. Generated-at is `indexed_at` in the ADR tab chrome, not a line written into the blob
10. Warning chrome is required; generated headings in the blob are English
11. Extract is bounded; DEV_LOG and other non-trio files are never copied
12. Parse does not fail the index job

## Out of Scope
- LLM summarization or any token-consuming fill
- Watcher / auto_watch / incremental fill
- Parsing DEV_LOG, TECH_DEBT, constitution, human/*, specs/*, history/* into ADR
- Writing back to `.sdd-skill/` or source
- Recolor 3D nodes/edges or redesign GraphTab / Specs Kanban
- New Dashboard ADR control
- Workspace-header Reindex control
- Changing spec-003 admission, conflict UI, or Path 1:1
- UNIQUE `root_path` SQL
- Raising HTTP bind/auth beyond loopback

## Acceptance Checklist
- [x] all US implemented [x] all AC met [x] ALL Gherkin scenarios pass [x] tests>80% on touched C parse + graph-ui ADR chrome [x] review approved [x] human docs confirmed [x] watcher fill = 0 [x] security passed [x] manual test by owner done (closeprep confirmed)

## Architecture Considerations
- C parse pass after ADR restore on user-triggered index only; file IO from `root_path`, out of graph
- Shared helper so HTTP create, HTTP Reindex, and MCP `index_repository` cannot drift
- Marker splice function used by parse (and tests)
- Bounded extract so POST `/api/adr` 16KiB does not 400 after a real trio
- i18n en+zh for generated-at + replace warning
- Constitution may gain a Phase 2 ADR-region rule at close (architect proposes ADR)

## Questions for Architect (answered in plan.md)
- Exact hook: pipeline post-persist vs job-complete callback; how user-triggered is distinguished from watcher incremental
- Extract algorithm and byte cap; whether to raise POST `/api/adr` 16384
- Whether `manage_adr(mode=update)` should reject documents that drop markers, or stay whole-doc (planner default: stay whole-doc)
- How unreadable is detected in tests (chmod vs missing)
- Whether generated extract uses file basenames as English H1s

## Questions for @implementer
- [ ] Map every Gherkin scenario to a test (C for parse/MCP/HTTP job; Vitest for stamp/warning)
- [ ] Do not index trio `.md` files into the code graph
- [ ] Do not run fill from watcher/incremental
- [ ] Do not write back to `.sdd-skill/`
- [ ] Do not copy DEV_LOG or other non-trio files
- [ ] Do not change `colorForLabel` hex
- [ ] Do not add a Dashboard ADR control

## Related Specs
Depends on: spec-002-p8w-project-workspace (AdrTab + whole-doc blob), spec-003-h7q-path-project-identity (one store per Path; Reindex `{root_path, project}`)
Blocks: none
Supersedes: spec-002 US-004 "no generated-region marker" for projects that have been user-indexed with sdd-skill after this spec
Companion to: .grill/plans/executive-ui-ia/epics/epic-004-adr-parse-on-reindex.md (grill ADR-005, ADR-006, ADR-007)

## Revision History
| Date | Author | Change |
| 2026-08-30 | @planner | draft from grill epic-004 + ADR-005/006/007; debt=no |
| 2026-08-30 | @planner | User: "scenarios approved". Status=approved. Handoff @architect. |
| 2026-08-30 | @architect | plan.md + tasks.md + checklist.md written. Architecture sign-off not approved. |
| 2026-08-30 | @architect | User: "approved - start development". Architecture approved. Handoff @implementer Task #1. |
| 2026-08-30 | @planner | Closed. KPI met. Constitution IX.2 MODIFIED + IX.5 ADDED confirmed. |

## Approval Sign-off
Spec Owner(@planner): approved 2026-08-30 / Gherkin scenarios(user): approved 2026-08-30 / Product Owner: approved via Gherkin / Architecture(@architect): approved 2026-08-30
