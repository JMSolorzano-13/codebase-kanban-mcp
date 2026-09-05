# Spec-013-r9w: ADR fill from gamedev trio
Status: completed | Spec ID: spec-013-r9w-adr-fill-gamedev-trio
Ref Ticket: none | KPI: After a successful user-triggered index of a project whose root has `.gamedev/` as a directory, GET /api/adr generated region contains extracts from the gamedev trio and does not contain unique strings from a leftover sdd trio | Priority: P0
Created: 2026-08-31 | Tech Debt Ref: none

## Executive Summary
spec-004 fills the ADR generated region from the sdd-skill trio on user-triggered index. spec-010 silent-win already hides Specs on a `.gamedev/` path, but ADR still describes leftover `.sdd-skill/` (MVP1). Grill epic-004 / ADR-008: if `.gamedev/` is a directory, fill from the gamedev trio only. No merge. No sdd fallback.

Business impact: Reindex or `index_repository` on a Game cycle (e.g. bevy MVP2) makes ADR Purpose/Stack/Decisions match Game, not an archived sdd tree. Manual region, markers, watcher skip, and AdrTab chrome stay spec-004.

## User Stories

### US-001: User-triggered index fills generated from the gamedev trio
As an operator, I want Dashboard Reindex (and first create-index) of a `.gamedev/` repo to fill the ADR generated region from the gamedev trio, So that ADR matches the live Game cycle without an LLM.
Acceptance Criteria:
- [ ] Trigger is unchanged from spec-004: successful user-triggered index only — POST `/api/index` (bare create or `{root_path, project}` Reindex) and MCP/CLI `index_repository`
- [ ] Presence is `.gamedev/` as a directory at that project's `root_path` (same predicate as `cbm_spec_board_gamedev_skill_present`). File-at-path or missing is not gamedev
- [ ] Sources, relative to `root_path`, are exactly `.gamedev/game_context.md`, `.gamedev/baseline/TECH_STACK.md`, `.gamedev/baseline/ARCHITECTURE_ADR.md`
- [ ] After the job completes, GET `/api/adr?project=<name>` content includes `<!-- CBM-GENERATED-START -->` and `<!-- CBM-GENERATED-END -->`
- [ ] The generated region contains a unique substring from each gamedev trio file that exists and is readable
- [ ] English H1s inside generated stay `# Purpose`, `# Stack`, `# Decisions` (Purpose source may be `game_context.md`)
- [ ] DEV_LOG, constitution, GDD, playtest-log, TECH_DEBT, human/*, and non-trio files are not copied
- [ ] Trio reads are disk fopen. They are not added to the code graph as a new fill path
- [ ] Parse is deterministic and zero-token (no LLM)
- [ ] No write-back into `.gamedev/`, `.sdd-skill/`, or `.grill/`

### US-002: Dual-tree XOR — leftover sdd trio is not merged
As an operator on a path that has both `.gamedev/` and leftover `.sdd-skill/`, I want generated ADR to come only from the gamedev trio, So that MVP1 Purpose/Stack/Decisions do not sit next to MVP2.
Acceptance Criteria:
- [ ] When `.gamedev/` is a directory, fill opens the three gamedev relatives only. It does not open the sdd trio
- [ ] Unique strings that exist only in leftover `.sdd-skill/context_ai.md`, `.sdd-skill/baseline/TECH_STACK.md`, or `.sdd-skill/baseline/ARCHITECTURE_ADR.md` are absent from the generated region
- [ ] Missing or unreadable gamedev file does not fall back to the matching sdd file
- [ ] `.gamedev/` present as a directory with all three gamedev files missing: markers are written; generated has no trio extracts; leftover sdd strings stay absent
- [ ] Paths without `.gamedev/` as a directory still use the spec-004 sdd trio when `.sdd-skill/` is a directory

### US-003: Manual region, markers, and watcher skip stay spec-004
As an operator, I want hand-written ADR text to survive Reindex and background incremental to leave ADR alone, So that parse cannot delete notes and auto-watch cannot invent an ADR.
Acceptance Criteria:
- [ ] Parse never writes bytes between `<!-- CBM-MANUAL-START -->` and `<!-- CBM-MANUAL-END -->`
- [ ] Unmarked Phase-1 blob: first user-triggered index treats the entire existing content as manual, then prepends generated
- [ ] Empty ADR + gamedev trio present: generated is filled; manual exists and is empty or whitespace only
- [ ] POST `/api/adr` and `manage_adr(mode=update)` stay whole-document. Next user-triggered index refreshes generated and keeps the saved manual-region text
- [ ] Watcher / auto_watch / incremental does not run this fill even if the gamedev trio changes on disk
- [ ] Index job success does not depend on parse

### US-004: Partial trio and unreadable files omit that H1
As an operator, I want a missing, empty, or unreadable gamedev trio file to omit only that heading, So that Reindex still completes like spec-004.
Acceptance Criteria:
- [ ] Only a subset of the gamedev trio exists and is readable: generated includes extracts for those files; omitted files contribute no extract and no empty `# Purpose` / `# Stack` / `# Decisions`
- [ ] One trio file unreadable: job still succeeds; that file's H1 is omitted; readable siblings are still present
- [ ] Empty readable file (zero bytes after optional UTF-8 BOM): same omit-H1 as missing
- [ ] All three missing under an otherwise present `.gamedev/` directory: markers written; generated has no trio extracts; manual (if any) is preserved
- [ ] No ADR-tab error banner is required for a missing/unreadable file
- [ ] Extract stays the spec-004 bounded window (`CBM_ADR_EXTRACT_MAX` 1536, newline-bounded tail omit, 8K generated buffer). A unique suffix past that window is omitted. Cap is not raised this spec

### US-005: Cycle switch on add or remove of `.gamedev/`
As an operator, I want the next user-triggered index to follow whichever cycle directory is present now, So that ADR generated does not stay stuck on the old trio.
Acceptance Criteria:
- [ ] Former sdd project that gains a `.gamedev/` directory: one user-triggered index overwrites generated with gamedev extracts (or empty extracts if the gamedev trio is missing). Prior sdd unique strings leave generated. Manual is unchanged
- [ ] `.gamedev/` directory removed and `.sdd-skill/` is still a directory: next user-triggered index fills from the sdd trio again
- [ ] `.gamedev/` removed and `.sdd-skill/` is absent: fill does not run; existing ADR blob is unchanged (leftover gamedev generated may remain until a later skill dir appears)
- [ ] File-at-path `.gamedev` (not a directory) does not count as present; sdd fill applies if `.sdd-skill/` is a directory

### US-006: MCP, create-index, and AdrTab chrome stay one backend
As an operator and as an agent, I want every user-triggered index path to pick the same trio and the ADR tab to keep the generic replace warning, So that UI and MCP do not drift and chrome does not invent a second stamp.
Acceptance Criteria:
- [ ] Successful `index_repository` on a project that owns that Path fills the same store GET `/api/adr` reads
- [ ] `manage_adr(mode=get)` after that job matches GET `/api/adr` markers and extracts
- [ ] First create-index of a new Path that has `.gamedev/` fills immediately (same as spec-004 create)
- [ ] AdrTab stamp is still `Project.indexed_at` via `formatIndexedAt`. Warning copy stays the existing generic replace warning (en+zh). No "from gamedev-skill" / "from sdd-skill" chrome
- [ ] Editor remains one textarea. Save still POSTs `{project, content}`. Dirty-leave `window.confirm` is unchanged
- [ ] Generated headings in the blob stay English

## Acceptance Scenarios (Gherkin) — THE EXECUTABLE CONTRACT

```gherkin
Feature: ADR fill from gamedev trio

  # Happy Paths
  Scenario: Dashboard Reindex fills generated from the gamedev trio and omits leftover sdd
    Given project "bevy" owns root_path "/tmp/bevy" and is idle
    And "/tmp/bevy/.gamedev" exists as a directory
    And "/tmp/bevy/.gamedev/game_context.md" contains "PURPOSE-GAME-BEVY"
    And "/tmp/bevy/.gamedev/baseline/TECH_STACK.md" contains "STACK-GAME-BEVY"
    And "/tmp/bevy/.gamedev/baseline/ARCHITECTURE_ADR.md" contains "DECISION-GAME-BEVY"
    And "/tmp/bevy/.sdd-skill/context_ai.md" contains "PURPOSE-SDD-MVP1"
    And "/tmp/bevy/.sdd-skill/baseline/TECH_STACK.md" contains "STACK-SDD-MVP1"
    And "/tmp/bevy/.sdd-skill/baseline/ARCHITECTURE_ADR.md" contains "DECISION-SDD-MVP1"
    And "/tmp/bevy/.gamedev/phases/01-preproduction/gdd.md" contains "SECRET-GDD-BEVY"
    And "/tmp/bevy/.sdd-skill/baseline/DEV_LOG.md" contains "SECRET-DEVLOG-BEVY"
    And GET /api/adr?project=bevy returns {"has_adr":true,"content":"# Existing ADR\n"}
    When POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes successfully
    Then GET /api/adr?project=bevy content contains "<!-- CBM-GENERATED-START -->"
    And GET /api/adr?project=bevy content contains "<!-- CBM-GENERATED-END -->"
    And GET /api/adr?project=bevy content contains "<!-- CBM-MANUAL-START -->"
    And GET /api/adr?project=bevy content contains "PURPOSE-GAME-BEVY"
    And GET /api/adr?project=bevy content contains "STACK-GAME-BEVY"
    And GET /api/adr?project=bevy content contains "DECISION-GAME-BEVY"
    And GET /api/adr?project=bevy content contains "# Purpose"
    And GET /api/adr?project=bevy content contains "# Stack"
    And GET /api/adr?project=bevy content contains "# Decisions"
    And GET /api/adr?project=bevy content does not contain "PURPOSE-SDD-MVP1"
    And GET /api/adr?project=bevy content does not contain "STACK-SDD-MVP1"
    And GET /api/adr?project=bevy content does not contain "DECISION-SDD-MVP1"
    And GET /api/adr?project=bevy content does not contain "SECRET-GDD-BEVY"
    And GET /api/adr?project=bevy content does not contain "SECRET-DEVLOG-BEVY"
    And the text between CBM-MANUAL-START and CBM-MANUAL-END contains "# Existing ADR"
    And "/tmp/bevy/.gamedev/game_context.md" is byte-identical to its contents before the POST

  Scenario: MCP index_repository fills the same gamedev ADR blob
    Given project "bevy" owns root_path "/tmp/bevy" and is idle
    And "/tmp/bevy/.gamedev" exists as a directory
    And "/tmp/bevy/.gamedev/game_context.md" contains "PURPOSE-MCP-GAME"
    And GET /api/adr?project=bevy returns {"has_adr":false,"content":""}
    When index_repository completes successfully for repo_path "/tmp/bevy" and name "bevy"
    Then manage_adr mode get for project "bevy" content contains "<!-- CBM-GENERATED-START -->"
    And manage_adr mode get for project "bevy" content contains "PURPOSE-MCP-GAME"
    And GET /api/adr?project=bevy content contains "PURPOSE-MCP-GAME"

  Scenario: First create-index of a new Path with .gamedev/ writes generated and empty manual
    Given no project owns canonical Path "/tmp/gamma"
    And "/tmp/gamma/.gamedev" exists as a directory
    And "/tmp/gamma/.gamedev/game_context.md" contains "PURPOSE-GAMMA-NEW"
    And "/tmp/gamma/.gamedev/baseline/TECH_STACK.md" contains "STACK-GAMMA-NEW"
    And "/tmp/gamma/.gamedev/baseline/ARCHITECTURE_ADR.md" contains "DECISION-GAMMA-NEW"
    When POST /api/index is sent with JSON {"root_path":"/tmp/gamma"}
    And the index job for the derived name "gamma" completes successfully
    Then GET /api/adr?project=gamma content contains "<!-- CBM-GENERATED-START -->"
    And GET /api/adr?project=gamma content contains "PURPOSE-GAMMA-NEW"
    And GET /api/adr?project=gamma content contains "STACK-GAMMA-NEW"
    And GET /api/adr?project=gamma content contains "DECISION-GAMMA-NEW"
    And the text between CBM-MANUAL-START and CBM-MANUAL-END contains only whitespace or is empty

  Scenario: Adding .gamedev/ to a former sdd project overwrites generated in one job
    Given project "bevy" owns root_path "/tmp/bevy" and is idle
    And GET /api/adr?project=bevy content contains "PURPOSE-SDD-MVP1"
    And GET /api/adr?project=bevy content contains "<!-- CBM-GENERATED-START -->"
    And the text between CBM-MANUAL-START and CBM-MANUAL-END contains "# Keep notes"
    And "/tmp/bevy/.gamedev" exists as a directory
    And "/tmp/bevy/.gamedev/game_context.md" contains "PURPOSE-GAME-SWITCH"
    And "/tmp/bevy/.sdd-skill/context_ai.md" still contains "PURPOSE-SDD-MVP1"
    When POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes successfully
    Then GET /api/adr?project=bevy content contains "PURPOSE-GAME-SWITCH"
    And GET /api/adr?project=bevy content does not contain "PURPOSE-SDD-MVP1"
    And the text between CBM-MANUAL-START and CBM-MANUAL-END contains "# Keep notes"

  Scenario: ADR tab keeps the generic generated-at and replace warning
    Given list_projects returns one project named "bevy" with indexed_at "2026-08-31T12:00:00Z"
    And GET /api/adr?project=bevy returns content that includes "<!-- CBM-GENERATED-START -->"
    When the operator opens "?tab=adr&project=bevy"
    Then the document contains a visible datetime derived from "2026-08-31T12:00:00Z"
    And the document contains visible warning copy that generated-region edits are replaced on the next user-triggered index
    And the document does not contain the substring "gamedev-skill"
    And a textarea is present

  # Limit Cases
  Scenario: Limit Case — no .gamedev/ still fills from the sdd trio
    Given project "alpha" owns root_path "/tmp/alpha"
    And "/tmp/alpha" has no ".gamedev" directory
    And "/tmp/alpha/.sdd-skill/context_ai.md" contains "PURPOSE-ALPHA-GRAPH"
    And "/tmp/alpha/.sdd-skill/baseline/TECH_STACK.md" contains "STACK-ALPHA-C11"
    And "/tmp/alpha/.sdd-skill/baseline/ARCHITECTURE_ADR.md" contains "DECISION-ALPHA-PATH"
    When POST /api/index is sent with JSON {"root_path":"/tmp/alpha","project":"alpha"}
    And the index job for "alpha" completes successfully
    Then GET /api/adr?project=alpha content contains "PURPOSE-ALPHA-GRAPH"
    And GET /api/adr?project=alpha content contains "STACK-ALPHA-C11"
    And GET /api/adr?project=alpha content contains "DECISION-ALPHA-PATH"

  Scenario: Limit Case — only TECH_STACK.md exists; Purpose and Decisions headings omitted
    Given project "bevy" owns root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev" exists as a directory
    And "/tmp/bevy/.gamedev/baseline/TECH_STACK.md" contains "STACK-PARTIAL-ONLY"
    And "/tmp/bevy/.gamedev/game_context.md" does not exist
    And "/tmp/bevy/.gamedev/baseline/ARCHITECTURE_ADR.md" does not exist
    And "/tmp/bevy/.sdd-skill/context_ai.md" contains "PURPOSE-SDD-FALLBACK"
    When POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes successfully
    Then the index job result is success
    And GET /api/adr?project=bevy content contains "STACK-PARTIAL-ONLY"
    And GET /api/adr?project=bevy content contains "# Stack"
    And GET /api/adr?project=bevy content does not contain "# Purpose"
    And GET /api/adr?project=bevy content does not contain "# Decisions"
    And GET /api/adr?project=bevy content does not contain "PURPOSE-SDD-FALLBACK"

  Scenario: Limit Case — empty .gamedev/ directory does not fall back to leftover sdd
    Given project "bevy" owns root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev" exists as a directory
    And "/tmp/bevy/.gamedev/game_context.md" does not exist
    And "/tmp/bevy/.gamedev/baseline/TECH_STACK.md" does not exist
    And "/tmp/bevy/.gamedev/baseline/ARCHITECTURE_ADR.md" does not exist
    And "/tmp/bevy/.sdd-skill/context_ai.md" contains "PURPOSE-SDD-EMPTYDIR"
    When POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes successfully
    Then GET /api/adr?project=bevy content contains "<!-- CBM-GENERATED-START -->"
    And GET /api/adr?project=bevy content does not contain "PURPOSE-SDD-EMPTYDIR"

  Scenario: Limit Case — .gamedev/ removed and .sdd-skill/ remains restores sdd trio
    Given project "bevy" owns root_path "/tmp/bevy"
    And GET /api/adr?project=bevy content contains "PURPOSE-GAME-BEVY"
    And "/tmp/bevy" has no ".gamedev" directory
    And "/tmp/bevy/.sdd-skill/context_ai.md" contains "PURPOSE-SDD-RESTORED"
    When POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes successfully
    Then GET /api/adr?project=bevy content contains "PURPOSE-SDD-RESTORED"
    And GET /api/adr?project=bevy content does not contain "PURPOSE-GAME-BEVY"

  Scenario: Limit Case — both skill dirs gone leaves the last ADR blob
    Given project "bevy" owns root_path "/tmp/bevy"
    And GET /api/adr?project=bevy returns {"has_adr":true,"content":"# Last gamedev generated leftover\n"}
    And "/tmp/bevy" has no ".gamedev" directory
    And "/tmp/bevy" has no ".sdd-skill" directory
    When POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes successfully
    Then GET /api/adr?project=bevy content equals "# Last gamedev generated leftover\n"
    And GET /api/adr?project=bevy content does not contain "CBM-GENERATED"

  Scenario: Limit Case — watcher incremental does not fill ADR
    Given project "bevy" owns root_path "/tmp/bevy"
    And GET /api/adr?project=bevy returns {"has_adr":true,"content":"# Before watch\n"}
    And "/tmp/bevy/.gamedev/game_context.md" contains "PURPOSE-WATCHER-SHOULD-NOT-FILL"
    When an auto_watch incremental update completes for "/tmp/bevy/src/foo.c" without POST /api/index and without index_repository
    Then GET /api/adr?project=bevy content equals "# Before watch\n"
    And GET /api/adr?project=bevy content does not contain "PURPOSE-WATCHER-SHOULD-NOT-FILL"

  Scenario: Limit Case — extract omits the tail past the spec-004 window
    Given project "bevy" owns root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev" exists as a directory
    And "/tmp/bevy/.gamedev/baseline/ARCHITECTURE_ADR.md" starts with "DECISION-HEAD-BEVY" and after the first 2000 bytes contains "DECISION-TAIL-BEVY"
    When POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes successfully
    Then GET /api/adr?project=bevy content contains "DECISION-HEAD-BEVY"
    And GET /api/adr?project=bevy content does not contain "DECISION-TAIL-BEVY"

  # Error Scenarios
  Scenario: Error — unreadable game_context.md omits Purpose and does not fail the job
    Given project "bevy" owns root_path "/tmp/bevy"
    And "/tmp/bevy/.gamedev" exists as a directory
    And "/tmp/bevy/.gamedev/game_context.md" exists but is unreadable
    And "/tmp/bevy/.gamedev/baseline/TECH_STACK.md" contains "STACK-READABLE-GAME"
    And "/tmp/bevy/.gamedev/baseline/ARCHITECTURE_ADR.md" contains "DECISION-READABLE-GAME"
    And "/tmp/bevy/.sdd-skill/context_ai.md" contains "PURPOSE-SDD-UNREADABLE"
    When POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes
    Then the index job result is success
    And GET /api/adr?project=bevy content contains "STACK-READABLE-GAME"
    And GET /api/adr?project=bevy content contains "DECISION-READABLE-GAME"
    And GET /api/adr?project=bevy content does not contain "# Purpose"
    And GET /api/adr?project=bevy content does not contain "PURPOSE-SDD-UNREADABLE"

  Scenario: Error — manage_adr manual edit survives the next Reindex
    Given GET /api/adr?project=bevy already contains CBM-GENERATED and CBM-MANUAL markers
    And the manual region contains "# Old notes"
    When manage_adr mode update for project "bevy" writes a document whose manual region is "# New notes"
    And POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes successfully
    Then the text between CBM-MANUAL-START and CBM-MANUAL-END contains "# New notes"
    And the text between CBM-MANUAL-START and CBM-MANUAL-END does not contain "# Old notes"

  Scenario: Error — generated-region edits are replaced on the next Reindex
    Given "/tmp/bevy/.gamedev/game_context.md" contains "PURPOSE-CANONICAL-GAME"
    And GET /api/adr?project=bevy content contains "<!-- CBM-GENERATED-START -->"
    When POST /api/adr is sent with JSON whose generated region contains "PURPOSE-HAND-EDIT-GAME" and does not contain "PURPOSE-CANONICAL-GAME"
    And POST /api/index is sent with JSON {"root_path":"/tmp/bevy","project":"bevy"}
    And the index job for "bevy" completes successfully
    Then GET /api/adr?project=bevy content contains "PURPOSE-CANONICAL-GAME"
    And GET /api/adr?project=bevy content does not contain "PURPOSE-HAND-EDIT-GAME"
```

## Success Metrics
| Metric | Target | Current | Status |
| User-triggered index + `.gamedev/` dir → gamedev extract present | 100% | fill opens sdd trio only | not met |
| Leftover sdd unique strings in generated when `.gamedev/` present | 0 | sdd wins today | not met |
| Manual / Phase-1 text after parse | 100% preserved | spec-004 | met (must stay) |
| Watcher incremental fills | 0 | adr_fill false | met (must stay) |
| Non-trio files in generated | 0 | DEV_LOG / GDD never opened | met (must stay) |
| Skill-file writes from fill | 0 | 0 | met (must stay 0) |
Primary KPI: first two metrics plus manual preserved.

## Constraints & Assumptions
Technical:
- Same SQLite `project_summaries` blob and four HTML comment markers as spec-004. No second table
- Prefer existing POST `/api/index`, `index_repository`, GET/POST `/api/adr`, `manage_adr`. No new endpoints
- Bounded extract stays spec-004 (`CBM_ADR_EXTRACT_MAX` 1536, `CBM_ADR_GEN_BUF` 8K). Large bevy `ARCHITECTURE_ADR` loses the tail
- Pipeline capture/restore + `adr_fill` flag stay the user-triggered vs watcher gate
- graph-ui Vitest only if chrome copy is asserted (US-006). C tests for XOR, switch, partial, unreadable, sdd regression, MCP/HTTP
- Chrome grayscale. GraphTab `colorForLabel` hex unchanged
- Constitution I.2 / IX.5: do not write skill cycle files; do not treat fill reads as graph source
- spec-003 admission unchanged
- Presence predicate meaning matches Game tab (directory). Fill must not depend on GET `/api/game-board` or `/api/skill-presence`

Business:
- Grill add-gamedev-skill epic-004. ADR-008 XOR trio. ADR-001 silent win (ADR describes Game, not leftover sdd)
- No LLM. No write-back to `.gamedev/`
- MVP1 ADRs remain on disk / Graph only. They do not stay in ADR generated while `.gamedev/` is present

Planner defaults A (reject a Gherkin scenario to change these):
1. Ref ticket none. No TD.
2. Presence = `.gamedev/` is a directory (reuse the existing is-dir helper or an equivalent local check). Not a new predicate.
3. XOR: gamedev dir → gamedev trio only. No merge. No sdd fallback when a gamedev file is missing.
4. No gamedev dir + sdd dir → spec-004 sdd trio. Neither dir → no markers added; blob unchanged.
5. Add `.gamedev/` → one user-triggered index overwrites generated. Remove `.gamedev/` + sdd remains → restore sdd. Remove both → leave last blob.
6. H1 titles stay Purpose / Stack / Decisions. Empty or unreadable file omits that H1 (no empty heading).
7. Extract cap unchanged (1536 / 8K). Omit tail. Do not raise POST `/api/adr` 32768.
8. Create-index, Reindex, and `index_repository` all pick the same trio. Watcher never fills.
9. Same generic generated-at + replace warning. No "from gamedev-skill" stamp. Dirty confirm unchanged.
10. Zero skill writes. No new HTTP/MCP. AdrTab editor unchanged.

## Out of Scope
- LLM summarization or any token-consuming fill
- Watcher / auto_watch / incremental fill
- Merging sdd + gamedev extracts in one generated region
- Falling back to sdd when a gamedev trio file is missing
- Parsing GDD, playtest-log, DEV_LOG, TECH_DEBT, constitution, human/*, specs/*, history/*, or `.gamedev/agents.md` into ADR
- Writing back to `.gamedev/` or `.sdd-skill/`
- Changing Game tab, Specs tab, silent win, archive, or board GET/POST
- New Dashboard ADR control or workspace-header Reindex
- Changing spec-003 admission or Path 1:1
- Raising extract cap or POST `/api/adr` body limit
- Adding `.gamedev` to discover `ALWAYS_SKIP_DIRS` (architect may propose; not required to meet AC)
- Recolor 3D nodes/edges

## Acceptance Checklist
- [x] all US implemented [x] all AC met [x] ALL Gherkin scenarios pass [x] tests>80% on touched C parse [x] review approved [x] human docs confirmed [x] leftover sdd strings in gamedev generated = 0 [x] watcher fill = 0 [x] security passed [ ] manual test by owner done

## Architecture Considerations
- Extend `cbm_adr_fill_document`: if gamedev dir then three gamedev relatives; else existing sdd gate. Same splice/markers
- Do not hook fill to GET `/api/game-board`
- Layering: adr_fill should not import `spec_board.c`. Duplicate the 5-line is-dir check or share a tiny foundation helper
- C tests: dual-tree XOR, empty-dir no fallback, switch add/remove, sdd-only regression, partial, unreadable, tail omit, watcher skip, HTTP/MCP
- Vitest: existing AdrTab warning/stamp; assert chrome has no "gamedev-skill" substring
- Discover `ALWAYS_SKIP` stays `.sdd-skill` unless architect proves `.gamedev` trio `.md` must leave the graph this spec

## Questions for Architect (answered in plan.md)
- Local is-dir in adr_fill.c vs a shared foundation helper (must not link adr → spec_board)
- Whether `.gamedev` joins `ALWAYS_SKIP_DIRS` this spec or graph coverage of game files stays as today
- Whether `cbm_adr_fill_document` NULL meaning stays "no skill dir" (neither gamedev nor sdd)
- How unreadable is detected in tests (chmod vs missing) — match spec-004

## Questions for @implementer
- [ ] Map every Gherkin scenario to a test (C for parse/MCP/HTTP job; Vitest only for stamp/warning/no gamedev-skill chrome)
- [ ] Do not open sdd trio paths when `.gamedev/` is a directory
- [ ] Do not fall back to sdd on missing gamedev files
- [ ] Do not run fill from watcher/incremental
- [ ] Do not write back to `.gamedev/` or `.sdd-skill/`
- [ ] Do not copy GDD, DEV_LOG, or other non-trio files
- [ ] Do not change AdrTab warning copy or add a cycle stamp
- [ ] Do not change `colorForLabel` hex
- [ ] Keep spec-004 sdd-only fixtures green

## Related Specs
Depends on: spec-004-j8k-adr-parse-on-reindex (markers, splice, user-triggered gate, bounded extract)
Companion to: spec-010-c4h-game-tab-silent-win (ADR must describe the same cycle Game shows)
Companion to: .grill/plans/add-gamedev-skill/epics/epic-004-adr-fill-gamedev-trio.md (grill ADR-008, ADR-001)
Does not include: Game board, archive, expand, Specs kanban

## Revision History
| Date | Author | Change |
| 2026-08-31 | @planner | draft from grill epic-004 + ADR-008/001; defaults A; debt=no; ticket=none |
| 2026-08-31 | @planner | Gherkin approved (user: scenarios approved). Status: approved. Handoff @architect |
| 2026-08-31 | @planner | Constitution changes confirmed. Status completed. Spec closed. |

## Approval Sign-off
Spec Owner(@planner): closed / Gherkin scenarios(user): approved / Product Owner: constitution confirmed 2026-08-31 / Architecture(@architect): approved 2026-08-31
