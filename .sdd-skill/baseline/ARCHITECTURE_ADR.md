# Architecture ADR
SDD ADRs live here. Grill product ADRs stay in `.grill/plans/executive-ui-ia/decisions/` until @architect promotes the ones this spec implements.

## Index
| ADR | Title | Status | Spec |
|---|---|---|---|
| SDD-ADR-001 | Adopt first spec is epic-001 not consolidate | Accepted | spec-001-w3q-executive-dashboard |
| SDD-ADR-002 | Stacked Dashboard: list then Control, one ScrollArea | Accepted | spec-001-w3q-executive-dashboard |
| SDD-ADR-003 | indexed_at shown as UTC locale via time[dateTime] | Superseded in part by SDD-ADR-034 | spec-001-w3q-executive-dashboard |
| SDD-ADR-004 | Delete TabBar.tsx stub | Accepted | spec-001-w3q-executive-dashboard |
| SDD-ADR-005 | Chrome grayscale; lock colorForLabel and EdgeLines hex | Accepted | spec-001-w3q-executive-dashboard |
| SDD-ADR-006 | useProjects is list_projects only | Accepted | spec-001-w3q-executive-dashboard |
| SDD-ADR-007 | Create-index POST root_path only | Accepted | spec-001-w3q-executive-dashboard |
| SDD-ADR-008 | TabId dashboard\|graph; stats/control/specs alias home | Superseded in part by SDD-ADR-009 | spec-001-w3q-executive-dashboard |
| SDD-ADR-009 | TabId dashboard\|graph\|specs\|adr; workspace needs project | Accepted | spec-002-p8w-project-workspace |
| SDD-ADR-010 | Specs omit-until-true via one-shot spec-board | Superseded in part by SDD-ADR-039 | spec-002-p8w-project-workspace |
| SDD-ADR-011 | AdrTab full pane; delete AdrButton; whole blob | Accepted | spec-002-p8w-project-workspace |
| SDD-ADR-012 | Dirty ADR uses window.confirm on leave and tab change | Accepted | spec-002-p8w-project-workspace |
| SDD-ADR-013 | Workspace last-indexed from useProjects only | Accepted | spec-002-p8w-project-workspace |
| SDD-ADR-014 | Path 1:1 via admission scan, no global UNIQUE | Accepted | spec-003-h7q-path-project-identity |
| SDD-ADR-015 | HTTP project field is reindex; bare POST is create | Accepted | spec-003-h7q-path-project-identity |
| SDD-ADR-016 | list_projects emits indexed_at and canonical_root | Accepted | spec-003-h7q-path-project-identity |
| SDD-ADR-017 | Index job guard keyed by canonical Path | Accepted | spec-003-h7q-path-project-identity |
| SDD-ADR-018 | MCP multi-owner without name is path_exists | Accepted | spec-003-h7q-path-project-identity |
| SDD-ADR-019 | ADR fill after capture; user-triggered only | Accepted | spec-004-j8k-adr-parse-on-reindex |
| SDD-ADR-020 | Bounded trio extract + in-document markers | Accepted | spec-004-j8k-adr-parse-on-reindex |
| SDD-ADR-021 | manage_adr stays whole-document | Accepted | spec-004-j8k-adr-parse-on-reindex |
| SDD-ADR-022 | Unreadable is open-fail; English role H1s | Accepted | spec-004-j8k-adr-parse-on-reindex |
| SDD-ADR-023 | .sdd-skill is ALWAYS_SKIP; fill is out-of-graph | Accepted | spec-004-j8k-adr-parse-on-reindex |
| SDD-ADR-024 | Spec-board GET stays; additive blurb; enrich every listed spec | Accepted | spec-005-v2m-spec-card-expand |
| SDD-ADR-025 | Blurb is 1-2 Executive Summary sentences, 512 B | Accepted | spec-005-v2m-spec-card-expand |
| SDD-ADR-026 | Dual done matcher: active bare Task #N; non-active needs spec id | Accepted | spec-005-v2m-spec-card-expand |
| SDD-ADR-027 | Expanded ids are a SpecBoardTab Set keyed by spec id | Accepted | spec-005-v2m-spec-card-expand |
| SDD-ADR-028 | Naive fopen per poll; one spec.md + one shared log; no cache | Accepted | spec-005-v2m-spec-card-expand |
| SDD-ADR-029 | spec_archive table in the project .db | Accepted | spec-006-k3n-spec-archive |
| SDD-ADR-030 | POST stays /api/spec-board; merge flags in HTTP after read | Accepted | spec-006-k3n-spec-archive |
| SDD-ADR-031 | POST 200 returns the flag object not the board | Accepted | spec-006-k3n-spec-archive |
| SDD-ADR-032 | Await GET refresh after POST; Show archived is session-only | Accepted | spec-006-k3n-spec-archive |
| SDD-ADR-033 | publish_staged copies spec_archive from the live db | Accepted | spec-006-k3n-spec-archive |
| SDD-ADR-034 | indexed_at visible text uses runtime TZ; dateTime/title stay ISO | Accepted | spec-007-n6p-last-indexed-local |
| SDD-ADR-035 | Same GET additive grill_skill_present + separate epics[]; kind only on epics | Accepted | spec-008-g8r-grill-epic-todo |
| SDD-ADR-036 | Grill walk in spec_board.c; HTTP stays archive merge | Accepted | spec-008-g8r-grill-epic-todo |
| SDD-ADR-037 | Epic JSON summary + plan_title; exact conversion; index.md then slug-asc | Accepted | spec-008-g8r-grill-epic-todo |
| SDD-ADR-038 | Epic mark E uses --color-epic-mark #7d8ec9 | Accepted | spec-008-g8r-grill-epic-todo |
| SDD-ADR-039 | Keep useSddSkillPresent; present is 200 and sdd OR grill | Accepted | spec-009-t4x-specs-tab-grill-presence |
| SDD-ADR-040 | Host Kanban on sdd OR grill; notSddSkill last-resort | Accepted | spec-009-t4x-specs-tab-grill-presence |
| SDD-ADR-041 | Specs grill presence is graph-ui only | Accepted | spec-009-t4x-specs-tab-grill-presence |
| SDD-ADR-042 | New GameBoardTab chrome only | Accepted | spec-010-c4h-game-tab-silent-win |
| SDD-ADR-043 | Sibling fallbacks plus small workspace router | Accepted | spec-010-c4h-game-tab-silent-win |
| SDD-ADR-044 | state.md compact keys plus legacy aliases | Accepted | spec-010-c4h-game-tab-silent-win |
| SDD-ADR-045 | Dedicated cbm_game_board_t and GET /api/game-board | Accepted | spec-010-c4h-game-tab-silent-win |
| SDD-ADR-046 | Game grill walk in game_board.c; no shared JSON | Accepted | spec-011-q5n-game-phase-board |
| SDD-ADR-047 | Roadmap convert is slug token plus table NNN | Accepted | spec-011-q5n-game-phase-board |
| SDD-ADR-048 | Cap 64 per column; widen card; grow to_json | Accepted | spec-011-q5n-game-phase-board |
| SDD-ADR-049 | Owner and track from compiled filesystem table | Accepted | spec-011-q5n-game-phase-board |
| SDD-ADR-050 | Keep useGameBoard one-shot | Accepted | spec-011-q5n-game-phase-board |
| SDD-ADR-051 | Clipboard writeText; select-text fallback; no toast | Accepted | spec-011-q5n-game-phase-board |
| SDD-ADR-052 | game_archive table; not spec_archive | Accepted | spec-012-m2k-game-expand-archive-deps |
| SDD-ADR-053 | POST /api/game-board; HTTP merge; publish copy | Accepted | spec-012-m2k-game-expand-archive-deps |
| SDD-ADR-054 | Keep one-shot; refresh after POST without unsetting settled | Accepted | spec-012-m2k-game-expand-archive-deps |
| SDD-ADR-055 | Blocked overlay in C; strip from state.md blocked lines only | Accepted | spec-012-m2k-game-expand-archive-deps |
| SDD-ADR-056 | game_board-local H2 extract; do not call spec_board extract_blurb | Accepted | spec-012-m2k-game-expand-archive-deps |
| SDD-ADR-057 | Reuse specBoard archive i18n; add inputs + blockedStrip | Accepted | spec-012-m2k-game-expand-archive-deps |
| SDD-ADR-058 | XOR trio: gamedev dir wins; no sdd merge or fallback | Accepted | spec-013-r9w-adr-fill-gamedev-trio |
| SDD-ADR-059 | Local cbm_is_dir in adr_fill; no spec_board import | Accepted | spec-013-r9w-adr-fill-gamedev-trio |
| SDD-ADR-060 | NULL means neither skill dir; empty gamedev dir still marks | Accepted | spec-013-r9w-adr-fill-gamedev-trio |
| SDD-ADR-061 | Do not ALWAYS_SKIP .gamedev this spec | Accepted | spec-013-r9w-adr-fill-gamedev-trio |
| SDD-ADR-062 | Game visibility filters are client React state only | Accepted | spec-014-x7m-filters |
| SDD-ADR-063 | Archived done requires Show Dones AND Show archived | Accepted | spec-014-x7m-filters |
| SDD-ADR-064 | Exclusive aria-pressed Track trio; keep specBoard.showArchived | Accepted | spec-014-x7m-filters |
| SDD-ADR-065 | Same GET always-emit debt[{id,title}]; cap 16 | Accepted | spec-015-s5k-specs-debt-and-path |
| SDD-ADR-066 | TECH_DEBT.md parse in spec_board.c; heading Status wins | Accepted | spec-015-s5k-specs-debt-and-path |
| SDD-ADR-067 | Specs-only debt strip; aria-label; dead text | Accepted | spec-015-s5k-specs-debt-and-path |
| SDD-ADR-068 | EpicCard id wraps; other card ids locked | Superseded in part by SDD-ADR-071 (Inbox only) | spec-015-s5k-specs-debt-and-path |
| SDD-ADR-069 | Registry regular file is sole Game Inbox hide | Accepted | spec-016-d9v-game-inbox-registry |
| SDD-ADR-070 | Registry parse last-wins NNN exact slug in game_board.c | Accepted | spec-016-d9v-game-inbox-registry |
| SDD-ADR-071 | InboxCard id wraps; Artifact id truncate stays | Accepted | spec-016-d9v-game-inbox-registry |
| SDD-ADR-072 | Same GET always-emit game debt[{id,title}]; cap 16 | Accepted | spec-017-b4w-game-debt-chrome |
| SDD-ADR-073 | Parse backlog.md in game_board.c; no spec_board helper | Accepted | spec-017-b4w-game-debt-chrome |
| SDD-ADR-074 | Game-only debt strip after BlockedStrip; reuse Specs i18n | Accepted | spec-017-b4w-game-debt-chrome |

## SDD-ADR-001: Adopt first spec is epic-001 not consolidate
Status: Accepted
Date: 2026-08-29
Spec: spec-001-w3q-executive-dashboard
Problem: adopt.md default is spec-001-consolidate-existing. User ran init against a finished grill plan and ordered epic 001 first.
Decision: Skip stabilize-only spec. Baseline + constitution still written from the existing tree. First executable spec is the executive Dashboard.
Consequences: Existing engine/UI remain undocumented-as-US. Debt that is not in epic 001 stays in TECH_DEBT.md.
Alternatives Considered: consolidate then feature new (user rejected by starting at epic 001).
Related ADRs: grill ADR-002, ADR-003, ADR-010, ADR-011, ADR-012

## SDD-ADR-002: Stacked Dashboard: list then Control, one ScrollArea
Status: Accepted
Spec: spec-001-w3q-executive-dashboard | Date: 2026-08-29
Problem: Spec asked stack vs two-pane for list + Control on one screen. Control includes a 400px log viewer plus four gauges.
Decision: Vertical stack on a single Dashboard ScrollArea: Indexed Projects first, then full Control. ControlTab gains `embedded` to skip a nested ScrollArea.
Consequences: Empty list still shows Control below the CTA. Polls stay mounted with Dashboard. No compact Control teaser.
Alternatives: two-pane split (clips logs / dual scroll); Control as a leftover account tab (violates US-003).
Related ADRs: grill ADR-003

## SDD-ADR-003: indexed_at shown as UTC locale via time[dateTime]
Status: Superseded in part by SDD-ADR-034
Spec: spec-001-w3q-executive-dashboard | Date: 2026-08-29
Problem: Field exists on Project but is hidden. Locale vs raw ISO was open; tests must see a derived datetime.
Decision: `formatIndexedAt(iso, lang)` with Intl `en-US`/`zh-CN`, `timeZone: "UTC"`. Render `<time dateTime={indexed_at} title={indexed_at}>`. Invalid ISO falls back to raw string.
Consequences: Clock hour is UTC (CI-stable). No new backend freshness field.
Alternatives: browser-local TZ (flaky CI); raw ISO only (less readable); hover-only (violates grill ADR-010).
Related ADRs: grill ADR-010
Superseded in part: SDD-ADR-034 drops `timeZone: "UTC"` for visible text. Instant contract (`dateTime`, `title`, stored ISO, newest strcmp) stays.

## SDD-ADR-004: Delete TabBar.tsx stub
Status: Accepted
Spec: spec-001-w3q-executive-dashboard | Date: 2026-08-29
Problem: TD-003 — TabBar.tsx is `export {}`; tabs are inlined in App.tsx. Spec-001 removes the account tab strip.
Decision: Delete TabBar.tsx. Spec-002 workspace tabs are new code, not a revival of the stub.
Consequences: No dead export. Graph back control lives in the App header chip, not TabBar.
Alternatives: reuse stub as the real tab component (no account tabs to host); keep the stub "for compatibility" (debt remains).

## SDD-ADR-005: Chrome grayscale; lock colorForLabel and EdgeLines hex
Status: Accepted
Spec: spec-001-w3q-executive-dashboard | Date: 2026-08-29
Problem: TD-004 — global `--color-primary #1DA27E` / `--color-accent #1C8585` paint chrome teal. Graph uses the same hex in EdgeLines and categorical `colorForLabel`.
Decision: Rewrite chrome tokens to distinct grays. Swap chrome hardcoded `#0b1920`/`#0e2028` to `bg-card`/`bg-background`. Gauge healthy fill leaves `#1DA27E`. Do not edit `colors.ts` values. Lock `colorForLabel("Function") === "#06b6d4"` and EdgeLines CALLS/default hex in tests. Graph loader `#22d3ee` stays.
Consequences: FilterPanel/Sidebar `text-primary` becomes gray (chrome). 3D node/edge hues unchanged.
Alternatives: scope a second `--color-primary` under GraphTab (unnecessary — nodes do not read the CSS var); grayscale the galaxy (rejected, grill ADR-011).
Related ADRs: grill ADR-002, ADR-011

## SDD-ADR-006: useProjects is list_projects only
Status: Accepted
Spec: spec-001-w3q-executive-dashboard | Date: 2026-08-29
Problem: TD-001 — hook N+1 `get_graph_schema` after `list_projects`. Dashboard no longer shows nodes/edges.
Decision: `useProjects` returns `Project[]` from `list_projects` only. SpecBoard ProjectPicker uses name+path. Schema fetch is not this spec.
Consequences: Dashboard list paint = 0 schema RPC. SpecBoard if mounted later still works without schema.
Alternatives: `includeSchema` flag (easy to regress Dashboard); keep fetch-and-ignore (wastes RPC, fails Gherkin spy).

## SDD-ADR-007: Create-index POST root_path only
Status: Accepted
Spec: spec-001-w3q-executive-dashboard | Date: 2026-08-29
Problem: CreateIndexModal sends optional `project_name`, creating path aliases (grill ADR-012). C already derives name from path when the key is omitted.
Decision: Delete Project ID state/input. POST `{ "root_path" }` only. No C change.
Consequences: Existing custom-named DBs stay. MCP name override is not offered in this UI flow.
Alternatives: hide the field but keep state (risk of leftover send); add Path 1:1 reject in C (spec-003).
Related ADRs: grill ADR-012

## SDD-ADR-008: TabId dashboard|graph; stats/control/specs alias home
Status: Accepted
Spec: spec-001-w3q-executive-dashboard | Date: 2026-08-29
Problem: App default is Specs; four sibling tabs are the account IA. Spec requires Dashboard home and bookmark aliases without a workspace shell.
Decision: Internal tabs are `dashboard` and `graph` only. `readRoute` maps missing/unknown/`stats`/`control`/`specs` to Dashboard and clears project. Graph requires `tab=graph` plus a project name. Enter still `navigate("graph", name)`. Header has no Specs/Graph/Projects/Control tabs. SpecBoardTab stays in tree, unrouted.
Consequences: Old `?tab=stats` bookmarks work. `?tab=specs` does not open Specs in spec-001.
Alternatives: keep `specs` as a hidden deep link (leaks lab IA); build workspace tabs now (spec-002).
Related ADRs: grill ADR-003, ADR-008
Superseded in part: SDD-ADR-009 (`?tab=specs&project=` is a workspace tab; `?tab=specs` without project stays Dashboard).

## SDD-ADR-009: TabId dashboard|graph|specs|adr; workspace needs project
Status: Accepted
Spec: spec-002-p8w-project-workspace | Date: 2026-08-29
Problem: spec-001 TabId is dashboard|graph. Specs/ADR need first-class routes without bringing back account tabs.
Decision: One union `dashboard|graph|specs|adr`. `WORKSPACE_TABS = graph|specs|adr` is the documented extension point (hardcoded; no plugin). `readRoute`: workspace id + non-empty project → that tab; `stats`/`control`/unknown/missing/workspace-without-project → Dashboard. Tab strip is a `tablist` under the brand header, not header buttons named Graph/Specs.
Consequences: `?tab=specs&project=` is no longer a Dashboard alias. Spec-001 bookmarks without project are unchanged.
Alternatives: split AccountTab vs WorkspaceTab types (more files, same URL); revive deleted TabBar.tsx (wrong layer).
Related ADRs: SDD-ADR-008, grill ADR-003, ADR-004, ADR-008

## SDD-ADR-010: Specs omit-until-true via one-shot spec-board
Status: Superseded in part by SDD-ADR-039
Spec: spec-002-p8w-project-workspace | Date: 2026-08-29
Problem: Specs must not show a dead tab. `useSpecBoard` polls 4s. `GET /api/skill-presence` exists but is not the AC source.
Decision: `useSddSkillPresent` one-shot GET `/api/spec-board`. Show Specs only when `sdd_skill_present === true`. Loading/error/false → omit. `?tab=specs` while omitted → Graph immediately. Do not call `/api/skill-presence`. Do not poll for strip membership.
Consequences: A later true only adds the tab. Live JSON is `{sdd_skill_present,specs}` not Gherkin `columns`.
Alternatives: skill-presence (cheaper, rejected — spec frozen); wait for presence before URL fallback (violates omit-while-loading).
Related ADRs: grill ADR-004
Superseded in part: SDD-ADR-039 ORs `grill_skill_present === true`. One-shot GET `/api/spec-board`, omit-while-loading, no skill-presence, no 4s strip poll stay.

## SDD-ADR-011: AdrTab full pane; delete AdrButton; whole blob
Status: Accepted
Spec: spec-002-p8w-project-workspace | Date: 2026-08-29
Problem: ADR is a modal (`AdrButton`) that Dashboard must not mount. Phase 1 is the whole document.
Decision: New `AdrTab` pane. Same GET/POST `/api/adr`. Empty = empty textarea + existing placeholder; no POST until Save. Check `res.ok`; visible error/success. Delete = POST empty when `has_adr`. Delete `AdrButton`. No generated-region marker / `CBM-GENERATED`.
Consequences: spec-004 can add a fence later. Dashboard regression: 0 ADR controls.
Alternatives: wrap AdrButton and hide the trigger (modal leftover); split generated/manual now (spec-004).
Related ADRs: grill ADR-004, ADR-007

## SDD-ADR-012: Dirty ADR uses window.confirm on leave and tab change
Status: Accepted
Spec: spec-002-p8w-project-workspace | Date: 2026-08-29
Problem: Leaving the ADR pane would drop an unsaved textarea.
Decision: Dirty = content ≠ last successful load/save. `requestNavigate` and leave call `window.confirm`. Dismiss keeps ADR + draft. No custom modal. popstate is not gated.
Consequences: Same confirm family as Dashboard delete. Tests stub `confirm`.
Alternatives: silent discard (fails Gherkin); in-app modal (extra chrome).

## SDD-ADR-013: Workspace last-indexed from useProjects only
Status: Accepted
Spec: spec-002-p8w-project-workspace | Date: 2026-08-29
Problem: Freshness must be visible in the workspace header without a second field.
Decision: Reuse `useProjects` + `formatIndexedAt` + `<time dateTime>`. Name not in list → show name, omit `time`. No `/api/index-status` or `get_graph_schema` for this.
Consequences: Ghost deep links still open Graph. Display TZ follows the shared helper (SDD-ADR-003 instant; SDD-ADR-034 local text).
Alternatives: index-status poll (spec forbids); hide name when unknown (violates ghost scenario).
Related ADRs: SDD-ADR-003, SDD-ADR-006, SDD-ADR-034, grill ADR-010

## SDD-ADR-014: Path 1:1 via admission scan, no global UNIQUE
Status: Accepted
Spec: spec-003-h7q-path-project-identity | Date: 2026-08-29
Problem: One Path can have two `.db` names. There is no central projects table.
Decision: Enforce 1:1 at create/reindex admission by scanning cache DBs and comparing `cbm_canonical_path`. Do not add SQLite UNIQUE `root_path`.
Consequences: Legacy clones remain until confirm-delete. Catalog cost = list_projects.
Alternatives: merge DBs (out of spec); UNIQUE in one file (does not span stores).
Related ADRs: grill ADR-001, ADR-009

## SDD-ADR-015: HTTP project field is reindex; bare POST is create
Status: Accepted
Spec: spec-003-h7q-path-project-identity | Date: 2026-08-29
Problem: Create modal and Reindex would share POST `/api/index`. Same `{root_path}` cannot be both 409 and 202.
Decision: Bare `{root_path}` = create (409 if Path owned). `{root_path, project}` = reindex only if that name owns the Path. HTTP `project_name` is an alias of `project`, never a new identity.
Consequences: CreateIndexModal stays path-only (SDD-ADR-007). Dashboard Reindex sends `project`.
Alternatives: new `/api/reindex` (extra endpoint); treat matching derived name as reindex (create modal would refresh instead of redirect).
Related ADRs: SDD-ADR-007, grill ADR-012

## SDD-ADR-016: list_projects emits indexed_at and canonical_root
Status: Accepted
Spec: spec-003-h7q-path-project-identity | Date: 2026-08-29
Problem: UI grouping cannot realpath. Newest needs `indexed_at`. Live list may omit it today.
Decision: Each list row includes `indexed_at` and `canonical_root` (realpath, or stored path if resolve fails). Display path stays `root_path`.
Consequences: Dashboard groups without a new endpoint. Tests can mock `canonical_root`.
Alternatives: client slash-fold only (misses symlinks); extra identity RPC (forbidden unless needed).
Related ADRs: SDD-ADR-003, grill ADR-010

## SDD-ADR-017: Index job guard keyed by canonical Path
Status: Accepted
Spec: spec-003-h7q-path-project-identity | Date: 2026-08-29
Problem: Mutation guard is per project name, so two derived names can index the same Path at once.
Decision: Find in-flight jobs by canonical Path. Second create → 409 `path_exists`. Reindex of the same running project may subscribe.
Consequences: Gherkin in-flight scenario is enforceable.
Alternatives: keep name-only guard (fails US-003).

## SDD-ADR-018: MCP multi-owner without name is path_exists
Status: Accepted
Spec: spec-003-h7q-path-project-identity | Date: 2026-08-29
Problem: Two clones of one Path + `index_repository` without `name` is ambiguous.
Decision: Zero owners → create (if name free). One owner → reindex. Two or more → isError `path_exists` + newest name. UI Reindex always sends `project`.
Consequences: Agents must name a clone to refresh a specific alias.
Alternatives: always reindex newest (silent pick); error `name_exists` (wrong code).

## SDD-ADR-019: ADR fill after capture; user-triggered only
Status: Accepted
Spec: spec-004-j8k-adr-parse-on-reindex | Date: 2026-08-30
Problem: Restore (`saved_adr`) and Phase 2 parse would fight if fill ran before capture or on every persist. HTTP, MCP, and watcher share one worker; incremental route is not “watcher”.
Decision: After ADR capture, if `adr_fill`, splice then publish that blob. `cbm_pipeline_new` defaults false. `index_repository` sets true unless args have `adr_fill: false`. Watcher jobs set false. Strip the key in job-args equality so subscribe still works.
Consequences: User Reindex that takes the incremental route still fills. Standalone watcher incremental does not. No new endpoint.
Alternatives: post-job HTTP-only hook (MCP/CLI drift); gate on incremental vs full (fails user Reindex).
Related ADRs: grill ADR-005

## SDD-ADR-020: Bounded trio extract + in-document markers
Status: Accepted
Spec: spec-004-j8k-adr-parse-on-reindex | Date: 2026-08-30
Problem: The real trio is ~22KiB. POST `/api/adr` is 16384. A byte-copy would 400. A second table is unnecessary if splice is in-document.
Decision: Four HTML comments, generated then manual. Per readable file: read ≤64KiB, take first 1536 bytes (cut to last newline if truncated). Raise POST body max to 32768. No generated-at line in the blob.
Consequences: Gherkin unique strings in the first 1536 still appear. Large Phase-1 manuals still save. DEV_LOG is never opened.
Alternatives: raise only the limit and copy whole files (unbounded); keep 16384 and 400 on real repos.
Related ADRs: grill ADR-006, ADR-007

## SDD-ADR-021: manage_adr stays whole-document
Status: Accepted
Spec: spec-004-j8k-adr-parse-on-reindex | Date: 2026-08-30
Problem: After markers exist, a write API could reject documents that drop them — or stay whole-doc and let parse migrate.
Decision: POST `/api/adr` and `manage_adr(mode=update)` remain whole-document. Unmarked saves migrate to manual on the next user-triggered parse. Generated hand-edits are replaced then.
Consequences: Phase-1 clients keep working. Parse, not the write API, protects manual.
Alternatives: reject dropped markers (breaks unmarked-save Gherkin and old clients).
Related ADRs: SDD-ADR-011, grill ADR-007

## SDD-ADR-022: Unreadable is open-fail; English role H1s
Status: Accepted
Spec: spec-004-j8k-adr-parse-on-reindex | Date: 2026-08-30
Problem: Tests need a deterministic “unreadable” that is not “missing”. Generated headings language was open.
Decision: Missing = no regular file. Unreadable = regular file exists and open/read fails (`chmod 0`, or a directory at that path if root ignores chmod). Generated H1s are `# Purpose`, `# Stack`, `# Decisions` — not basenames. Stamp/warning chrome is i18n.
Consequences: Job still succeeds; that extract is omitted. Blob headings stay English.
Alternatives: treat unreadable as job failure (violates planner default 12); basename H1s (ugly, not architecture language).

## SDD-ADR-023: .sdd-skill is ALWAYS_SKIP; fill is out-of-graph
Status: Accepted
Spec: spec-004-j8k-adr-parse-on-reindex | Date: 2026-08-30
Problem: `.md` is a language. `.sdd-skill` is not in `ALWAYS_SKIP_DIRS`. Fill must not add trio files as graph source. Constitution I.2 forbids writing cycle files from the indexer.
Decision: Add `.sdd-skill` to `ALWAYS_SKIP_DIRS`. Fill reads the three paths with fopen only. No write-back to skill or source.
Consequences: Specs/graph stay read-only adapters. Fill and discover cannot drift into indexing docs.
Alternatives: rely on .gitignore (not guaranteed); pass trio through extract and drop nodes later (still graph-on-docs).

## SDD-ADR-024: Spec-board GET stays; additive blurb; enrich every listed spec
Status: Accepted
Spec: spec-005-v2m-spec-card-expand | Date: 2026-08-30
Problem: Only `active_spec` gets title/tasks. Todo/Done cards cannot expand with real data. A second lazy endpoint would add a write-free fetch on every click.
Decision: Keep GET `/api/spec-board`. Add `blurb` on each JSON entry. Run title/blurb/tasks reads for every listed spec. `handle_spec_board` stays a thin wrapper. Caps 64/48 stay. Zero-write.
Consequences: Poll payload grows with N listed specs. UI can expand any card from the existing 4s poll. No overlay route.
Alternatives: lazy GET per spec id (rejected — planner default 1); overlay/modal page (grill ADR-002).
Related ADRs: grill ADR-002, SDD-ADR-010

## SDD-ADR-025: Blurb is 1-2 Executive Summary sentences, 512 B
Status: Accepted
Spec: spec-005-v2m-spec-card-expand | Date: 2026-08-30
Problem: Need a bounded C field and a rule for a third sentence vs byte overflow. KPI must never leak.
Decision: Source is `## Executive Summary` until the next H2 (`## `). First 1–2 sentences (`.?!` + isspace or EOS). Third sentence dropped. Then copy into `blurb[512]`; if longer, byte-truncate and walk back to the last space. Links → link text. Missing/empty → `""`. UI omits the region.
Consequences: Long first sentences lose a tail, never gain KPI or a third sentence. Extract is unit-testable via `cbm_spec_board_extract_blurb`.
Alternatives: KPI fallback (grill ADR-006 rejected); full section unbounded; 256 like title (tight for two sentences).
Related ADRs: grill ADR-003, ADR-006

## SDD-ADR-026: Dual done matcher: active bare Task #N; non-active needs spec id
Status: Accepted
Spec: spec-005-v2m-spec-card-expand | Date: 2026-08-30
Problem: One shared `test_results.log`. Today's active matcher is bare `Task #N` (cross-spec leak documented). Non-active Gherkin forbids a bare PASS from marking another spec's task.
Decision: Keep the active matcher unchanged. Non-active: line must contain that spec's id and `Task #N`; last such line wins; `done` true only if it contains `PASS`.
Consequences: Existing `spec_board_active_compact_state` stays valid. Non-active Todo filter in the UI is trustworthy. Two code paths in one apply-log helper.
Alternatives: one spec-id-qualified path for all (would change active AC / planner default 6); keep skipping non-active tasks (fails US-003/005).
Related ADRs: none

## SDD-ADR-027: Expanded ids are a SpecBoardTab Set keyed by spec id
Status: Accepted
Spec: spec-005-v2m-spec-card-expand | Date: 2026-08-30
Problem: SpecCard `useState(entry.active)` plus `useEffect` sync and `canExpand = active && task_count > 0` cannot meet multi-open, zero-task expand, or poll persistence.
Decision: `expandedIds: Set<string>` on `SpecBoardTab`. Seed active ids on first board for the project. Title button toggles membership. Opening one card does not collapse another. Poll must not reset the Set. Every listed card can expand.
Consequences: Active starts expanded on first paint. Operator-opened Todo/Done survive `setBoard`. Accordion is forbidden.
Alternatives: rely on React `key={id}` local state only (fragile if Column remounts); accordion (grill ADR-008 rejected).
Related ADRs: grill ADR-002, ADR-008

## SDD-ADR-028: Naive fopen per poll; one spec.md + one shared log; no cache
Status: Accepted
Spec: spec-005-v2m-spec-card-expand | Date: 2026-08-30
Problem: Enriching 64 specs every 4s could invite an mtime cache or a lazy endpoint.
Decision: Stay naive fopen. One `spec.md` per listed spec (title+blurb same buffer). One `tasks.md` per spec. One `test_results.log` applied to all. `state.md`/checklist remain active-only. No cache, no second GET.
Consequences: Worst case is 2N+shared small reads on loopback. Caps stay 64/48. `useSpecBoard` 4s stays.
Alternatives: mtime TTL cache (extra invalidation, not in spec); lazy expand GET (planner default 1).
Related ADRs: SDD-ADR-024

## SDD-ADR-029: spec_archive table in the project .db
Status: Accepted
Spec: spec-006-k3n-spec-archive | Date: 2026-08-30
Problem: Archive must persist across daemon restart, keyed by project name + spec folder id, without writing `.sdd-skill/` or localStorage. Existing project-db relations are graph tables, ADR (`project_summaries`), or `store_meta` (db_uid / mutation_gen).
Decision: New table `spec_archive (spec_id TEXT PRIMARY KEY, archived INTEGER NOT NULL CHECK (0,1), updated_at TEXT NOT NULL)` in that project's `.db`. Project identity is the filename (`db_path_for_project`). UPSERT; unarchive writes 0; do not DELETE. Orphan rows stay. Query-open GET probes `sqlite_master` and treats a missing table as zero flags (same posture as `cbm_store_adr_get`).
Consequences: Write-open `init_schema` creates the table. No skill sidecar. No daemon-global store. Writer dump DDL is unchanged.
Alternatives: reuse `store_meta` (wrong lifetime); reuse `project_summaries` (ADR blob); sidecar `{name}.spec_archive.db` (survives dump without copy, but spec asked for the project `.db`).
Related ADRs: grill ADR-001

## SDD-ADR-030: POST stays /api/spec-board; merge flags in HTTP after read
Status: Accepted
Spec: spec-006-k3n-spec-archive | Date: 2026-08-30
Problem: Today `dispatch_request` is GET-only for `/api/spec-board*` and `handle_spec_board` only calls `cbm_spec_board_read` + `to_json`. Mutate AC needs POST. `spec_board.c` must stay a skill-file reader.
Decision: Add POST on the same path. GET still reads skill files, then query-opens the store, `cbm_store_spec_archive_load`, applies matching ids onto `e->archived`, then `to_json`. Merge lives in HTTP, not in `cbm_spec_board_read`. No sibling `/api/spec-archive` (constitution IV.3).
Consequences: Additive `"archived"` on every GET entry. `spec_board.c` gains a bool field and JSON key only. Store does not include `spec_board.h`.
Alternatives: sibling path (unneeded); merge inside `spec_board.c` (would open SQLite in the reader); wrap/reparse `to_json` output (fragile).
Related ADRs: SDD-ADR-024, grill ADR-001

## SDD-ADR-031: POST 200 returns the flag object not the board
Status: Accepted
Spec: spec-006-k3n-spec-archive | Date: 2026-08-30
Problem: Planner default is `{project, spec_id, archived}`. Full board JSON would also satisfy "body includes spec_id and archived".
Decision: 200 body is the flag object only. Gherkin does not require other board fields. UI must refetch GET after 200 (SDD-ADR-032), so POST must not duplicate `cbm_spec_board_to_json`.
Consequences: Smaller mutate response. Clients cannot skip GET. Error bodies stay `{error:...}` with the frozen strings.
Alternatives: return full board (couples POST to GET shape; still need refresh for poll consistency).
Related ADRs: SDD-ADR-030, SDD-ADR-032

## SDD-ADR-032: Await GET refresh after POST; Show archived is session-only
Status: Accepted
Spec: spec-006-k3n-spec-archive | Date: 2026-08-30
Problem: `useSpecBoard` polls GET every 4s. A successful Archive that only hid locally would reappear on the next tick if state lagged the store. Show/hide must not survive remount (grill ADR-007).
Decision: After POST 200, `await refresh()` (`fetchBoard`). Filter is `!(done && archived && !showArchived)`. `showArchived` is `useState(false)`, reset on project change, never written to localStorage or CBM. Optional in-memory patch is not a substitute for refresh.
Consequences: Archive click waits one GET. Fresh Specs visit always starts hidden. Done count = visible cards only.
Alternatives: patch POST body only (poll can resurrect if GET was stale); persist toggle (grill ADR-007 rejected).
Related ADRs: grill ADR-004, ADR-007, SDD-ADR-027

## SDD-ADR-033: publish_staged copies spec_archive from the live db
Status: Accepted
Spec: spec-006-k3n-spec-archive | Date: 2026-08-30
Problem: Incremental persist clones the previous `.db` (`cbm_delta_stage_clone`) so extra tables survive. Full dump (`cbm_gbuf_dump_to_sqlite`) writes a new file without `spec_archive`. Reindex would drop flags and resurrect hidden Done cards — the same class of failure as poll-resurrect.
Decision: In `cbm_pipeline_publish_staged`, after ADR write, query-open `generation->final_db_path` and `cbm_store_spec_archive_copy` onto the stage store. Missing live file or missing table is OK. Copy failure fails publish (old db kept). Do not change `adr_fill` or `adr_content`.
Consequences: Watcher clone path stays a no-op upsert of the same rows. User Reindex keeps archive flags. Writer DDL unchanged.
Alternatives: accept wipe on full reindex (fails KPI after Reindex); add `spec_archive` to the hand-built writer (large, easy to drift); sidecar file (avoids copy, leaves the project `.db`).
Related ADRs: SDD-ADR-019, SDD-ADR-029

## SDD-ADR-034: indexed_at visible text uses runtime TZ; dateTime/title stay ISO
Status: Accepted
Spec: spec-007-n6p-last-indexed-local | Date: 2026-08-30
Problem: SDD-ADR-003 pinned `timeZone: "UTC"` so Dashboard, workspace header, and AdrTab stamps read as UTC, not the operator laptop clock. Grill ADR-005 reverses that display contract. Tests must not regress to hardcoded wall-clock strings.
Decision: One helper (`formatIndexedAt`). Drop the `timeZone` key; keep `timeZoneName: "short"` and `en-US`/`zh-CN`. Runtime default timezone. `<time dateTime>` and `title` stay the raw `indexed_at` ISO. Stored list ISO and conflict newest (ISO string compare) stay. Tests assert against same-process Intl oracles (`localFmt` vs `utcFmt`); do not require `TZ=`. Host UTC is allowed (`localFmt` may equal `utcFmt`).
Consequences: Visible hour follows the browser. Hover/title still ends in Z. CI on UTC hosts will not fail the pin-diff Then. No C/HTTP/MCP field. No i18n "UTC"/"local" suffix. QUICK-DEBUG UTC-pin note is obsolete after implementation.
Alternatives: `TZ=` in CI plus a hardcoded local string (extra, rejected as DoD); second per-surface formatter (clock drift); daemon display TZ (out of spec).
Related ADRs: SDD-ADR-003 (display TZ superseded), SDD-ADR-013, SDD-ADR-016, grill ADR-005 / ADR-010

## SDD-ADR-035: Same GET additive grill_skill_present + separate epics[]; kind only on epics
Status: Accepted
Spec: spec-008-g8r-grill-epic-todo | Date: 2026-08-30
Problem: Mixed Todo needs grill epics on the existing Kanban poll without stealing spec slots or wiring SpecCard expand/archive to epic paths. Grill ADR-008 wants the same GET; ADR-007 wants a separate epic cap; ADR-006 wants additive kind for a later skill.
Decision: Keep GET `/api/spec-board`. Add `grill_skill_present` and a sibling `epics[]` (cap 64). Epic objects emit `"kind":"epic"`. Spec objects do not gain `"kind":"spec"` this spec — array membership discriminates. POST and archive merge stay spec-only. Do not emit `has_more` or `gamedev_skill_present` on this GET.
Consequences: Old UIs ignore unknown keys. SpecCard/archive stay spec-only. A later gamedev plan can add a third array or additive kind without rewriting the family.
Alternatives: mix epics into `specs[]` (breaks archive find + shared 64 pool); `"kind":"spec"` on every spec (JSON churn, no Gherkin); sibling `/api/grill-board` (second poll, constitution IV.3).
Related ADRs: grill ADR-006, ADR-007, ADR-008; SDD-ADR-024, SDD-ADR-030

## SDD-ADR-036: Grill walk in spec_board.c; HTTP stays archive merge
Status: Accepted
Spec: spec-008-g8r-grill-epic-todo | Date: 2026-08-30
Problem: Architect question was spec_board.c vs a sibling reader called from HTTP. HTTP already owns CBM `spec_archive` merge after skill read (SDD-ADR-030). Grill is another skill tree (`fopen` rb), and conversion needs listed spec.md plus `active.json`.
Decision: Parse `.grill/` inside `cbm_spec_board_read` in `spec_board.c`. No `grill_board.c`. HTTP remains read → archive merge on `specs[]` → `to_json`. Do not opendir `.grill/` from `http_server.c`. Do not read `.gamedev/` from this GET.
Consequences: One heap board struct still serializes the poll. Skill-tree IO stays in the spec_board family. `/api/skill-presence` may still stat `.gamedev/` as today — out of this spec.
Alternatives: sibling called from HTTP (puts skill IO next to SQLite merge); new translation unit (Makefile churn for one GET).
Related ADRs: SDD-ADR-030; grill ADR-001, ADR-006

## SDD-ADR-037: Epic JSON summary + plan_title; exact conversion; index.md then slug-asc
Status: Accepted
Spec: spec-008-g8r-grill-epic-todo | Date: 2026-08-30
Problem: Field names, conversion match, and catalog order were open. Reusing spec `blurb` would run Executive Summary extract on epic.md. Fuzzy kebab match was rejected in grill ADR-002.
Decision: JSON `summary` = epic.md `name:`'s sibling `summary:` line (not `blurb`). `plan_title` = index.md table title, else plan.md frontmatter `title:`, else slug. Conversion = `strcmp` of epic `id` to the first `.grill/plans/`…`.md` token on a `Companion to:` line, or to `source.grill_epic`. index.md GFM data rows set plan-group order; unlisted plan directories append slug-ascending; within a plan, `epic-NNN` numeric. Unreadable epic skipped. Overflow past 64 omitted with no has-more.
Consequences: Implementer has a closed parser. Missing catalog still lists plan dirs. A similarly named spec without a link sits beside its epic.
Alternatives: reuse `blurb` / field `plan` (wrong contract); fuzzy name (false omit); CBM epic↔spec table (duplicates spec.md).
Related ADRs: grill ADR-002, ADR-003, ADR-005, ADR-007

## SDD-ADR-038: Epic mark E uses --color-epic-mark #7d8ec9
Status: Accepted
Spec: spec-008-g8r-grill-epic-todo | Date: 2026-08-30
Problem: Grill ADR-004 allows one discreet chromatic letter E. Constitution III.1 otherwise keeps chrome grayscale except health red/amber/green. SDD-ADR-005 locks `colorForLabel` and EdgeLines hex.
Decision: Add chrome token `--color-epic-mark: #7d8ec9` in `globals.css` `@theme inline`. EpicCard renders the literal character `E` with `text-[var(--color-epic-mark)]`. Same glyph in en and zh. Not a pill, not the word Epic. Do not edit `colors.ts`.
Consequences: Mixed Todo is scannable without recoloring the galaxy or health dots. Hex is dusty periwinkle, not Function `#06b6d4`, Class `#a855f7`, File `#3b82f6`, CALLS `#1DA27E`, TRPC `#a78bfa`, or destructive `#e05252`.
Alternatives: gray-only E (user overrode); health green (forbidden); reuse Function teal (graph lock).
Related ADRs: SDD-ADR-005; grill ADR-004; constitution III.1–III.2

## SDD-ADR-039: Keep useSddSkillPresent; present is 200 and sdd OR grill
Status: Accepted
Spec: spec-009-t4x-specs-tab-grill-presence | Date: 2026-08-30
Problem: Specs strip is sdd-only (`bodyHasSkill` = `sdd_skill_present === true`). spec-008 already emits `grill_skill_present` on the same GET. Planner asked keep vs rename the hook.
Decision: Keep export `useSddSkillPresent`, file, and `UseSddSkillPresentResult`. Change the predicate only: HTTP 200 AND (`sdd_skill_present === true` OR `grill_skill_present === true`). Missing `grill_skill_present` is false. One-shot GET `/api/spec-board` stays. `fallbackSpecsToGraph` stays a boolean kernel.
Consequences: Grill-only `?tab=specs` stays on Specs. Neither-skill / hang / non-200 still omit and rewrite to Graph. Comments must say OR. No file rename churn.
Alternatives: rename to `useSpecsTabPresent` (cheap, no Gherkin Then, breaks SDD-ADR-010 identity); wait for presence before URL fallback (violates omit-while-loading).
Related ADRs: SDD-ADR-010 (predicate superseded); SDD-ADR-035; grill ADR-001, ADR-008

## SDD-ADR-040: Host Kanban on sdd OR grill; notSddSkill last-resort
Status: Accepted
Spec: spec-009-t4x-specs-tab-grill-presence | Date: 2026-08-30
Problem: `SpecBoardTab` gates on `!board.sdd_skill_present` and shows `notSddSkill` even when grill is true. Grill-only must paint the Kanban. Planner asked keep vs drop that branch.
Decision: Paint the Kanban when `board` exists and (`sdd_skill_present === true` OR `grill_skill_present === true`). Keep `notSddSkill` as last-resort when `!board` after load or both flags false (one-shot strip vs 4s poll stale mount; direct host tests). Existing copy. No new i18n key. Loading (`loading && !board`) stays the loading string.
Consequences: Grill-only never shows "doesn't use sdd-skill". Neither-skill omits the tab so the operator pane is not reached. spec-008 host tests that assert notSddSkill when grill is true must invert.
Alternatives: drop the branch (stale both-false paints empty Kanban under a still-visible tab); new empty-state string (no Gherkin Then).
Related ADRs: SDD-ADR-039; SDD-ADR-035; grill ADR-001

## SDD-ADR-041: Specs grill presence is graph-ui only
Status: Accepted
Spec: spec-009-t4x-specs-tab-grill-presence | Date: 2026-08-30
Problem: Planner asked whether any C/HTTP work is required. spec-008 already walks `.grill/` in `cbm_spec_board_read` and emits `grill_skill_present` on GET 200 without sdd.
Decision: graph-ui only. Do not edit spec_board.c, http_server.c, MCP, or Makefile.cbm. Do not add a presence endpoint or MCP tool. Do not add a second poll (`useSpecBoard` 4000 ms stays pane refresh). Add C tests only if a live 200 is missing the flag — then stop and raise to @architect.
Consequences: Strip and host read flags already on the board GET. GET zero-write and no `gamedev_skill_present` stay spec-008 C owners.
Alternatives: C "presence" tweak (no missing key); `/api/skill-presence` for the strip (rejected — spec frozen, constitution IV.3).
Related ADRs: SDD-ADR-035, SDD-ADR-036, SDD-ADR-039; grill ADR-006, ADR-008; constitution IV.3

## SDD-ADR-042: New GameBoardTab chrome only
Status: Accepted
Spec: spec-010-c4h-game-tab-silent-win | Date: 2026-08-31
Problem: Game must show phase/focus/continue without painting Specs Kanban. Reusing SpecBoardTab as host would mount Todo/EpicCard/Archive on a silent-win path.
Decision: New `GameBoardTab` for chrome only. Do not reuse or subclass `SpecBoardTab`. May copy grayscale token/type patterns. SpecBoardTab and spec-board JSON stay unchanged.
Consequences: Two pane components. Silent win cannot leak Kanban. Epic 002 paints columns on GameBoardTab, not SpecBoardTab.
Alternatives: reuse SpecBoardTab host + hide columns (rejected — host gate and poll are spec-board); one shared "lifecycle pane" (no Gherkin, extra abstraction).
Related ADRs: grill ADR-001, ADR-009; SDD-ADR-010, SDD-ADR-040

## SDD-ADR-043: Sibling fallbacks plus small workspace router
Status: Accepted
Spec: spec-010-c4h-game-tab-silent-win | Date: 2026-08-31
Problem: `fallbackSpecsToGraph` rewrites leftover `?tab=specs` to Graph. On a gamedev path that must become `tab=game`. One opaque function would hide two different fallbacks.
Decision: Keep `fallbackSpecsToGraph(tab, present)` as a boolean kernel. Add `fallbackGameToGraph` with the same shape. Add `resolveWorkspaceTab` that applies specs-on-gamedev → game first, then the two kernels. App passes settled-aware `showSpecs` / `showGame`.
Consequences: Existing specs-without-skill tests stay. `?tab=specs` + gamedev present is game, not Graph. In-flight pane uses show* flags so Specs cannot flash.
Alternatives: one presence router with no sibling kernels (hides the two fallbacks); wait for both GETs before any URL rewrite (violates omit-while-loading for tab=game).
Related ADRs: SDD-ADR-010, SDD-ADR-039; grill ADR-001

## SDD-ADR-044: state.md compact keys plus legacy aliases
Status: Accepted
Spec: spec-010-c4h-game-tab-silent-win | Date: 2026-08-31
Problem: Current gamedev-skill and bevy-tetris use compact `phase=` / `focus=`. Pre-1.7.0 files may still use `active_phase` / `director_focus`. Skill update does not migrate state.md. Compact-only would leave chrome empty on leftover files.
Decision: Parse compact `phase=` / `focus=` first. Tolerate `active_phase` / `director_focus` as `=` or line-start `:`. Prefer compact when both exist. Emit JSON phase only for the three skill tokens. fopen rb; never write.
Consequences: bevy-tetris chrome fills. Legacy files fill without a CBM migration. Unknown tokens become null (missing chrome, not 500).
Alternatives: compact only (empty chrome on pre-1.7.0 leftovers); LLM-style "read anything" (not a closed C parser).
Related ADRs: grill ADR-009

## SDD-ADR-045: Dedicated cbm_game_board_t and GET /api/game-board
Status: Accepted
Spec: spec-010-c4h-game-tab-silent-win | Date: 2026-08-31
Problem: spec-board must stay gamedev-free. Epic 002 needs the same GET and heap shape for four columns. A thin JSON builder would be ripped later. Constitution IV.3 allows a new endpoint when the current contract cannot meet the spec.
Decision: New GET `/api/game-board`. New `game_board.c` / `.h` with heap `cbm_game_board_t` including empty `inbox` / `preproduction` / `production` / `postproduction` slots (counts 0 this spec). Reuse `cbm_spec_board_gamedev_skill_present` for the dir check. No POST. No MCP tool. Do not fopen `.gamedev/` from `cbm_spec_board_read` / `to_json`.
Consequences: Makefile.cbm lists the new .c. Epic 002 fills arrays on the same struct/path. graph-ui presence is this GET, not `/api/skill-presence`.
Alternatives: additive `gamedev_skill_present` on spec-board (rejected — frozen lock); thin sprintf builder (rip later); skill-presence for the strip (rejected — spec frozen).
Related ADRs: SDD-ADR-035, SDD-ADR-036, SDD-ADR-041; grill ADR-001; constitution IV.3

## SDD-ADR-046: Game grill walk in game_board.c; no shared JSON
Status: Accepted
Spec: spec-011-q5n-game-phase-board | Date: 2026-08-31
Problem: Game Inbox needs the same catalog order as spec-008 (index.md then epic-NNN) but a different conversion rule (Game artifact Companion-to OR roadmap slug+NNN, not active.json / spec.md). spec_board `grill_*` helpers are static and bound to `cbm_spec_board_t`. Sharing `to_json` would couple two GET families.
Decision: Duplicate the catalog algorithm as `game_grill_*` statics in `game_board.c`. Copy the Companion-to token rule. Do not export spec_board helpers. Do not add `grill_catalog.c`. Do not share serializers. Do not read `active.json` for Game Inbox.
Consequences: Two copies of index.md/epic parse. Conversion cannot leak spec-008 rules onto Game. spec_board.c stays untouched.
Alternatives: extract shared catalog .c (Makefile + spec-008 refactor, no Gherkin); call `grill_epic_converted` (would omit via active.json).
Related ADRs: SDD-ADR-036, SDD-ADR-037, SDD-ADR-045; grill ADR-006

## SDD-ADR-047: Roadmap convert is slug token plus table NNN
Status: Accepted
Spec: spec-011-q5n-game-phase-board | Date: 2026-08-31
Problem: If a roadmap heading and a table disagree, a heading parser would invent NNN. Grill ADR-006 and US-004 require both a plan slug token and a table cell.
Decision: No heading/NNN parser. Slug = contiguous token (non `[A-Za-z0-9-]` bounds) in `roadmap.md` and/or `game_context.md`. NNN = GFM table data-row cell in `roadmap.md` only, exactly `001` or `epic-001`. Both required. Plan-folder cite without a cell does not convert. Table cell without a slug token does not convert.
Consequences: Closed C parser. Heading prose cannot empty Inbox. game_context.md can cite the slug but cannot supply the cell.
Alternatives: heading regex (ambiguous); plan-folder converts all (rejected — leftover epics hidden).
Related ADRs: grill ADR-006; SDD-ADR-037

## SDD-ADR-048: Cap 64 per column; widen card; grow to_json
Status: Accepted
Spec: spec-011-q5n-game-phase-board | Date: 2026-08-31
Problem: `cbm_game_board_card_t` is id+title and `to_json` hardcodes `[]` in 8192 B. bevy-sized production can list more than 64 SYS/art dirs. A second heap or `has_more` is out of spec.
Decision: Keep `CBM_GAME_BOARD_MAX_CARDS` 64 per array. Widen the card on the existing calloc'd board. List production into a 256-slot scratch, sort, then copy ≤64. Replace snprintf `[]` with a grow buffer (local, not spec_board's). No `has_more`.
Consequences: 65th omitted after sort. Real boards no longer 500 on overflow of 8192. One board heap.
Alternatives: per-card malloc (fragmentation); poll-sized cache (not in spec); raise cap (frozen 64).
Related ADRs: SDD-ADR-045; SDD-ADR-035

## SDD-ADR-049: Owner and track from compiled filesystem table
Status: Accepted
Spec: spec-011-q5n-game-phase-board | Date: 2026-08-31
Problem: US-002 gives a default table when `.gamedev/docs/agents.md` is absent. Skill agents.md is prose/cluster tables, not a closed Path→owner schema. No Gherkin Then for override.
Decision: Compile the spec/filesystem.md owner+track map in `game_board.c`. Do not fopen project or skill `docs/agents.md` this spec. Cards are never unlabeled.
Consequences: Override is deferred. A later spec can add a parse once a table contract exists.
Alternatives: parse Owns cells (guessy); unlabeled when agents.md missing (fails Gherkin).
Related ADRs: grill ADR-005

## SDD-ADR-050: Keep useGameBoard one-shot
Status: Accepted
Spec: spec-011-q5n-game-phase-board | Date: 2026-08-31
Problem: Filling the board is a larger walk than spec-010 chrome. Spec US-005 allows keeping the one-shot or adding an interval. A 4s poll could re-settle presence and redo fopen/opendir on loopback.
Decision: `useGameBoard` stays one-shot (`useEffect` on project, no `setInterval`). Same GET. No second URL. Operator remounts or changes project to refresh. `useSpecBoard` 4000 ms stays Specs-only.
Consequences: Game map is not live. Strip membership does not flicker from a poll.
Alternatives: 4s pane poll (heavier, no Gherkin); poll only after settled without flipping present (extra state).
Related ADRs: SDD-ADR-043, SDD-ADR-045

## SDD-ADR-051: Clipboard writeText; select-text fallback; no toast
Status: Accepted
Spec: spec-011-q5n-game-phase-board | Date: 2026-08-31
Problem: Gherkin requires clipboard text on continue-control activate and forbids a toast. Clipboard may be denied. spec-010 chrome continue is selectable text, not a launcher.
Decision: Card continue is a button. Try `navigator.clipboard.writeText`. On failure, select the button text. No toast / Copied / `role="status"` either path. Chrome continue stays a `select-text` `<p>`.
Consequences: Tests stub `writeText`. Denied path remains user-copyable. No process spawn.
Alternatives: toast on success (rejected); clipboard-only with silent fail (fails denied operators).
Related ADRs: SDD-ADR-042; grill ADR-009

## SDD-ADR-052: game_archive table; not spec_archive
Status: Accepted
Spec: spec-012-m2k-game-expand-archive-deps | Date: 2026-08-31
Problem: Done Game artifacts must hide across visits without writing `.gamedev/`. `spec_archive.spec_id` is a 192-byte spec folder id. Mixing Game path ids into that table would collide keys and let spec-board POST mutate Game flags.
Decision: New `game_archive (card_id TEXT PK, archived 0|1, updated_at)` in the project `.db`. C `card_id[256]`. CAP 512. UPSERT; unarchive writes 0 (no DELETE). Query-open probes `sqlite_master`. Do not add the table to sqlite_writer dump DDL.
Consequences: Two archive tables. Pipeline must copy both on dump replace.
Alternatives: namespace into spec_archive (rejected); localStorage (fails fresh visit); skill sidecar (constitution I.2).
Related ADRs: SDD-ADR-029; grill ADR-004

## SDD-ADR-053: POST /api/game-board; HTTP merge; publish copy
Status: Accepted
Spec: spec-012-m2k-game-expand-archive-deps | Date: 2026-08-31
Problem: GET-only cannot persist archive. A sibling `/api/game-archive` would violate constitution IV.3. Full dump replace drops extra tables (same as spec_archive).
Decision: POST on `/api/game-board` `{project, card_id, archived}`. 200 is the flag object, not the board. Merge flags in HTTP after `cbm_game_board_read` (matching ids only). `publish_staged` copies `game_archive` live→stage in the same query-open block as `spec_archive`. Mutation lock like spec-board POST.
Consequences: spec-011 "no POST" is superseded. spec-board POST stays specs-only.
Alternatives: sibling path (IV.3); merge inside game_board.c (would open SQLite in the skill reader).
Related ADRs: SDD-ADR-030, SDD-ADR-031, SDD-ADR-033, SDD-ADR-045

## SDD-ADR-054: Keep one-shot; refresh after POST without unsetting settled
Status: Accepted
Spec: spec-012-m2k-game-expand-archive-deps | Date: 2026-08-31
Problem: After archive the UI must refetch so hide survives. A 4s poll would redo the expand walk and could flip presence. spec-010 in-flight omits Game when `settled` is false.
Decision: `useGameBoard` stays one-shot (SDD-ADR-050). Add `refresh()` that re-GETs and replaces `board` without setting `settled`/`present` to false. Await it after POST 200. No `setInterval`.
Consequences: Game map is still not live except after Archive/Unarchive. Show archived is session state and must not reset on refresh.
Alternatives: 4s poll (rejected); local hiddenIds without refresh (poll/resurrect N/A on one-shot, but next GET would revive).
Related ADRs: SDD-ADR-050, SDD-ADR-032, SDD-ADR-043

## SDD-ADR-055: Blocked overlay in C; strip from state.md blocked lines only
Status: Accepted
Spec: spec-012-m2k-game-expand-archive-deps | Date: 2026-08-31
Problem: Gherkin requires GET `work_state` `"blocked"` on that owner's artifact cards and POST 409 when overlaying a done header. UI-only overlay would let POST persist. Roadmap graph is out of scope.
Decision: Parse `slug:blocked:"task":"blocked-by"` from `.gamedev/state.md` (cap 16, document order). Overlay in C before JSON: artifact `owner` match → `work_state` `"blocked"` + `blocked_by` text. Inbox never. Do not parse `roadmap.md` or agents.md Needs.
Consequences: POST eligibility uses JSON after overlay. `needs_review` is not a strip row.
Alternatives: UI-only overlay (fails 409 Then); roadmap graph (grill ADR-010 rejected).
Related ADRs: grill ADR-010

## SDD-ADR-056: game_board-local H2 extract; do not call spec_board extract_blurb
Status: Accepted
Spec: spec-012-m2k-game-expand-archive-deps | Date: 2026-08-31
Problem: Track A blurb is `## What it does` plus header fallback; Inputs is `## Inputs*`. `cbm_spec_board_extract_blurb` is locked to `## Executive Summary`. Sharing JSON/extract with spec_board would touch a closed spec-005 surface.
Decision: Copy sentence-end, markdown-link strip, and 512 truncate-walkback into `game_board.c` as `game_extract_*`. Export What-it-does (and Round/changelog helpers if useful) for C tests. Do not include spec_board extract. Do not add a shared `.c`.
Consequences: Two similar extract helpers. Heading contracts stay independent.
Alternatives: parameterize spec_board extract (scope creep); dump full spec.md (forbidden).
Related ADRs: SDD-ADR-025, SDD-ADR-046

## SDD-ADR-057: Reuse specBoard archive i18n; add inputs + blockedStrip
Status: Accepted
Spec: spec-012-m2k-game-expand-archive-deps | Date: 2026-08-31
Problem: Gherkin English Archive / Unarchive / Show archived / No tasks planned yet already exist on specBoard. Inputs and the Blocked region name do not.
Decision: GameBoardTab uses `specBoard.archive` / `unarchive` / `showArchived` / `noTasksYet`. Add `gameBoard.inputs` and `gameBoard.blockedStrip` (en+zh). Tests may assert English.
Consequences: Game chrome reuses Specs archive wording. zh Archive strings stay one source.
Alternatives: duplicate gameBoard.archive copies (drift); new wording (fails Gherkin English).
Related ADRs: SDD-ADR-032

## SDD-ADR-058: XOR trio: gamedev dir wins; no sdd merge or fallback
Status: Accepted
Spec: spec-013-r9w-adr-fill-gamedev-trio | Date: 2026-08-31
Problem: spec-004 always opens the sdd trio when `.sdd-skill/` is a directory. A Game path with leftover `.sdd-skill/` then shows MVP1 Purpose/Stack/Decisions. Grill ADR-008 forbids merge and sdd fallback.
Decision: Change trio selection inside `cbm_adr_fill_document` only. If `.gamedev/` is a directory, fopen the three gamedev relatives and no sdd path. Else keep spec-004 sdd. Pipeline `adr_fill` flag, markers, extract cap, and HTTP/MCP stay.
Consequences: One user-triggered index after adding `.gamedev/` overwrites generated. Missing gamedev files omit that H1; leftover sdd is never opened.
Alternatives: merge both trios (rejected); sdd fallback on missing gamedev file (rejected); hook fill to GET `/api/game-board` (rejected — presence is is-dir).
Related ADRs: SDD-ADR-019, SDD-ADR-020; grill ADR-008, ADR-001

## SDD-ADR-059: Local cbm_is_dir in adr_fill; no spec_board import
Status: Accepted
Spec: spec-013-r9w-adr-fill-gamedev-trio | Date: 2026-08-31
Problem: Game presence already lives as `cbm_spec_board_gamedev_skill_present`. Importing `spec_board.h` from `src/adr/` would link fill to the UI board reader.
Decision: Duplicate the join + `cbm_is_dir(root/.gamedev)` next to the existing sdd `adr_skill_present` check. Meaning matches Game (directory only). No new foundation helper this spec.
Consequences: Two five-line is-dir checks. Predicate drift is a review item, not a shared .c.
Alternatives: call spec_board (layering); new `foundation/skill_present.c` (Yagni this spec).
Related ADRs: SDD-ADR-045

## SDD-ADR-060: NULL means neither skill dir; empty gamedev dir still marks
Status: Accepted
Spec: spec-013-r9w-adr-fill-gamedev-trio | Date: 2026-08-31
Problem: spec-004 NULL means "no `.sdd-skill/` directory". After XOR, "no skill dir" must include gamedev. An empty `.gamedev/` is still present (Game tab shows). Returning NULL there would leave leftover sdd generated in place.
Decision: NULL only when neither `.gamedev/` nor `.sdd-skill/` is a directory. Empty gamedev dir → splice markers with no extracts. File-at-path `.gamedev` is not a directory → sdd if that dir exists. Both dirs gone → NULL; existing blob unchanged.
Consequences: Header comment updates. Watcher still never calls fill (`adr_fill` false).
Alternatives: NULL on empty gamedev dir (fails no-fallback Gherkin); invent markers when neither dir (fails leave-last-blob).
Related ADRs: SDD-ADR-019, SDD-ADR-058

## SDD-ADR-061: Do not ALWAYS_SKIP .gamedev this spec
Status: Accepted
Spec: spec-013-r9w-adr-fill-gamedev-trio | Date: 2026-08-31
Problem: `.sdd-skill` is ALWAYS_SKIP so trio `.md` are not graph File nodes (SDD-ADR-023). Adding `.gamedev` would drop GDD/state.md from already-indexed Game trees. Fill does not need that: it fopens fixed relatives.
Decision: Leave `ALWAYS_SKIP_DIRS` unchanged. Fill stays out-of-graph IO. Graph coverage of `.gamedev/` files stays as today.
Consequences: Game cycle `.md` may still appear as File nodes. Not an AC. Revisit later if noise hurts.
Alternatives: add `.gamedev` now (coverage change, not required); skip only the three trio files (discover is dirname-based).
Related ADRs: SDD-ADR-023

## SDD-ADR-062: Game visibility filters are client React state only
Status: Accepted
Spec: spec-014-x7m-filters | Date: 2026-09-01
Problem: Operators need to hide dones and isolate Track A/B. A GET query or C filter would add HTTP surface. localStorage would survive remount and fail the reset Then.
Decision: Filter in `GameBoardTab` over the existing GET `/api/game-board` arrays. `showDones` default false, `trackFilter` default `"all"`. No new path, query param, POST field, CBM key, or localStorage. Inbox not filtered.
Consequences: Remount / `?project=` reset. GET refetch keeps state. C/HTTP stay spec-012.
Alternatives: GET `?track=` (forbidden); persist prefs (fails remount Gherkin).
Related ADRs: SDD-ADR-050, SDD-ADR-054

## SDD-ADR-063: Archived done requires Show Dones AND Show archived
Status: Accepted
Spec: spec-014-x7m-filters | Date: 2026-09-01
Problem: spec-012 reveals an archived done card when Show archived is pressed. Unarchived dones always paint. Human asked to hide dones by default and to AND the two toggles for archived dones.
Decision: Hide `work_state==="done"` unless Show Dones is pressed. If also `archived`, require Show archived too. Leftover archived pending still shows with Show Dones off.
Consequences: spec-012 tests that click Show archived alone and expect the card must invert. Archive happy path must press Show Dones first to see the unarchived done card.
Alternatives: Show archived alone (rejected); Show Dones replaces Show archived (would hide leftover pending rule).
Related ADRs: SDD-ADR-054, SDD-ADR-057

## SDD-ADR-064: Exclusive aria-pressed Track trio; keep specBoard.showArchived
Status: Accepted
Spec: spec-014-x7m-filters | Date: 2026-09-01
Problem: Gherkin asserts `aria-pressed` on "Track A", "Track B", "All". A radiogroup would use `aria-checked`. Show archived English already lives on specBoard.
Decision: Five chrome buttons, all `aria-pressed`. Track exclusive in the click handler. Two `aria-hidden` `|` spans. Reuse `specBoard.showArchived`. Add `gameBoard.showDones` / `trackA` / `trackB` / `trackAll` (en+zh).
Consequences: All includes H and any other/null track. Specs tab does not copy the strings into controls.
Alternatives: radiogroup (fails Gherkin); duplicate showArchived under gameBoard (drift vs SDD-ADR-057).
Related ADRs: SDD-ADR-057

## SDD-ADR-065: Same GET always-emit debt[{id,title}]; cap 16
Status: Accepted
Spec: spec-015-s5k-specs-debt-and-path | Date: 2026-09-02
Problem: Specs needs open TECH_DEBT.md rows on the existing Kanban poll. A second GET or omitted `debt` key when empty would fork the board contract and make Gherkin `debt is []` ambiguous.
Decision: Keep GET `/api/spec-board`. Always emit `"debt":[{id,title}]` (empty array when the file is missing, unreadable, or every item is resolved). Cap `CBM_SPEC_BOARD_MAX_DEBT` 16, independent of specs 64 / epics 64. Overflow omit. Never emit `has_more`. Items have only `id` and `title`. POST and archive merge stay spec-only.
Consequences: Old UIs ignore the key. New UI treats missing as `[]` but C always writes the key. Filling debt does not steal spec or epic slots.
Alternatives: omit key when empty (planner rejected); sibling `/api/tech-debt` (constitution IV.3); mix into `specs[]` (would collide with archive + kind E).
Related ADRs: grill ADR-007, ADR-001; SDD-ADR-035, SDD-ADR-024

## SDD-ADR-066: TECH_DEBT.md parse in spec_board.c; heading Status wins
Status: Accepted
Spec: spec-015-s5k-specs-debt-and-path | Date: 2026-09-02
Problem: Heading block and Debt Summary table can disagree. A mid-sentence `Status:` in Description would false-close. Title could be taken from a `Title:` field or the table.
Decision: Parse inside `cbm_spec_board_read` via fopen `"rb"` of `{root}/.sdd-skill/baseline/TECH_DEBT.md`. Export `cbm_spec_board_parse_tech_debt` for buffer tests. Heading `Status:` wins: first match that is trimmed line-start or a `|` cell start, token until whitespace/`|`/EOL, `strcmp` to exact `resolved`. Else use `## Debt Summary` GFM table row for that TD-NNN. Title is the heading-line remainder after `## TD-NNN:`. Skip blocks without `TD-`+digits. Unknown/typo = open. Missing/unreadable → count 0. HTTP stays archive merge only. Do not read backlog.md or epics_registry.md. Do not change conversion.
Consequences: This-repo all-resolved file yields `debt: []`. Stale table cannot hide an open heading. Skill IO stays in the spec_board family.
Alternatives: table-only (fails heading-wins Gherkin); any-substring `Status:` (false close); parse in HTTP (splits skill IO).
Related ADRs: grill ADR-002, ADR-005; SDD-ADR-036

## SDD-ADR-067: Specs-only debt strip; aria-label; dead text
Status: Accepted
Spec: spec-015-s5k-specs-debt-and-path | Date: 2026-09-02
Problem: WorkspaceHeader would leak TD rows onto Graph/ADR. A visible heading is not required by Gherkin. Clickable rows would invite POST/clipboard/expand.
Decision: Paint only in SpecBoardTab, above the three-column row, when `debt.length > 0`. `role="region"` + `aria-label` from `specBoard.openTechDebt` (`en` = `"Open tech debt"`). No required visible heading. Rows are `<p>` text: `id` then title, `whitespace-normal break-words`, no truncate. No onClick, clipboard, or expand. Omit the region when `debt` is missing or `[]`. Do not edit WorkspaceHeader or GameBoardTab.
Consequences: Graph/ADR/Game mounts have no region. Tests use `getByRole("region", { name: "Open tech debt" })`. Chrome stays grayscale.
Alternatives: visible `<h2>` (not required); WorkspaceHeader (grill ADR-008 rejected); buttons/copy (dead-text lock).
Related ADRs: grill ADR-008, ADR-001; SDD-ADR-005; SDD-ADR-055 (a11y pattern only)

## SDD-ADR-068: EpicCard id wraps; other card ids locked
Status: Superseded in part by SDD-ADR-071 (InboxCard id only)
Spec: spec-015-s5k-specs-debt-and-path | Date: 2026-09-02
Problem: EpicCard id uses CSS `truncate`, so two epics in one plan look alike. `break-words` can still overflow a long kebab segment in a 1/3 column. Wrapping Spec/Inbox/Artifact ids is out of this spec.
Decision: EpicCard title stays `truncate`. EpicCard id drops `truncate` and uses `whitespace-normal break-all`. Same `epic.id` field; no second identifier. SpecCard, Artifact, and Inbox id lines keep `truncate`. Conversion keys unchanged.
Consequences: Full `.grill/plans/<slug>/epics/epic-NNN-<name>.md` is readable. Game Inbox wrap stays grill epic-002 until spec-016 / SDD-ADR-071.
Alternatives: `break-words` on the path (overflow risk); wrap every card id (out of spec); tooltip-only (grill ADR-006 rejected).
Related ADRs: grill ADR-006; SDD-ADR-038; SDD-ADR-071

## SDD-ADR-069: Registry regular file is sole Game Inbox hide
Status: Accepted
Spec: spec-016-d9v-game-inbox-registry | Date: 2026-09-02
Problem: spec-011 hides Inbox via Companion-to exact or roadmap slug+NNN. Operators with gamedev Epic Tracking still see leftovers `@director` already marked in_progress/closed/parked. Mixing both predicates when the file exists would hide unlisted leftovers.
Decision: Present = `game_is_regular_file("{root}/.gamedev/epics_registry.md")`. When present (including empty/unreadable/0 rows), hide is solely Plan=slug AND Epic=NNN with Status in {in_progress, closed, parked}. Skip `game_grill_load_conv` in that branch. Directory or missing path keeps spec-011 hide. CBM never creates the file. Hide = omit from `inbox[]` on the existing GET. No new JSON key.
Consequences: Inbox means “faltan” against the skill file when it exists; older games unchanged. Unreadable regular file shows leftovers (Companion-to must not hide).
Alternatives: fopen-success as present (would treat unreadable as absent and re-apply prior hide); keep both predicates (grill ADR-004 rejected).
Related ADRs: grill ADR-003, ADR-004; SDD-ADR-046, SDD-ADR-047

## SDD-ADR-070: Registry parse last-wins NNN exact slug in game_board.c
Status: Accepted
Spec: spec-016-d9v-game-inbox-registry | Date: 2026-09-02
Problem: Skill table cells vary (`001`/`1`/`epic-001`; path leftovers in Plan). HTTP must not grow a registry reader. Fuzzy match would hide the wrong leftover.
Decision: Parse in `game_board.c` (fopen `"rb"`). Reuse `game_grill_split_gfm_row` (Epic=cells[1], Plan=cells[2], Status=cells[5]; n<6 skip). Duplicate Plan+NNN: last data row wins. Epic cell: trim, optional `epic-`, then all-digits integer. Plan: exact slug; `native` skipped. Origin unused. Epic 0 / evergreen never hide. Cap 256 unique Plan+NNN. spec_board does not read this file.
Consequences: Unit tests can call `cbm_game_board_parse_epics_registry` without fopen. Path leftover in Plan keeps the card.
Alternatives: basename/path match (spec rejected); first-row-wins (planner default last-wins); parse in HTTP (splits skill IO).
Related ADRs: grill ADR-003; SDD-ADR-036, SDD-ADR-066

## SDD-ADR-071: InboxCard id wraps; Artifact id truncate stays
Status: Accepted
Spec: spec-016-d9v-game-inbox-registry | Date: 2026-09-02
Problem: InboxCard id uses CSS `truncate`, so two grill leftovers look alike. SDD-ADR-068 locked Inbox truncate for spec-015.
Decision: InboxCard title stays TitleControl (may truncate). InboxCard id drops `truncate` and uses `whitespace-normal break-all`. Same `card.id` field. ArtifactCard, SpecCard, and WorkspaceHeader path stay truncated. No new i18n empty-state string.
Consequences: Full Inbox path is readable. SDD-ADR-068 Inbox lock is lifted; EpicCard wrap from spec-015 stays.
Alternatives: `break-words` on the path (overflow risk); wrap artifact ids (out of spec).
Related ADRs: grill ADR-006; SDD-ADR-068

## SDD-ADR-072: Same GET always-emit game debt[{id,title}]; cap 16
Status: Accepted
Spec: spec-017-b4w-game-debt-chrome | Date: 2026-09-03
Problem: Game needs open backlog.md `debt:*` rows on the existing one-shot board GET. A second GET or omitted `debt` key when empty would fork the board contract and make Gherkin `debt is []` ambiguous. Mixing debt into Inbox would collide with grill cards and registry hide.
Decision: Keep GET `/api/game-board`. Always emit `"debt":[{id,title}]` (empty array when the file is missing, unreadable, a directory, or every item is closed). Cap `CBM_GAME_BOARD_MAX_DEBT` 16, independent of cards 64 / blocked 16 / spec-board debt 16. Overflow omit. Never emit `has_more`. Items have only `id` and `title`. POST and archive merge stay card-only. Debt ids are not archive targets.
Consequences: Old UIs ignore the key. New UI treats missing as `[]` but C always writes the key. Filling debt does not steal card or blocked slots. Matches spec-015 / SDD-ADR-065 on the Game GET family.
Alternatives: omit key when empty (planner rejected); sibling `/api/game-debt` (constitution IV.3); mix into `inbox[]` (would collide with kind E + registry hide).
Related ADRs: grill ADR-001, ADR-007; SDD-ADR-065, SDD-ADR-045

## SDD-ADR-073: Parse backlog.md in game_board.c; no spec_board helper
Status: Accepted
Spec: spec-017-b4w-game-debt-chrome | Date: 2026-09-03
Problem: gamedev debt is `debt:<tag>` lines in `.gamedev/backlog.md` (comment / list / heading), closed by `resolved-by` in the entry body. `cbm_spec_board_parse_tech_debt` is locked to TECH_DEBT.md heading Status. Sharing it would teach the Specs helper a second file format. HTTP must not grow a backlog reader.
Decision: Parse inside `cbm_game_board_read` via fopen `"rb"` of `{root}/.gamedev/backlog.md`. Export `cbm_game_board_parse_backlog_debt` for buffer tests. Tag = `[A-Za-z0-9][A-Za-z0-9_-]*` immediately after `debt:` (no `.` in tag). `id` is the full token. Entry body runs until the next start, the next ATX `##`, or EOF; blank lines do not end it. Closed iff the body contains `resolved-by`. Title is the start-line remainder after the tag, cut at ` — ` or ` -- `. Duplicate id: first start wins. Missing / unreadable / directory-at-path → count 0. Do not call `cbm_spec_board_parse_tech_debt`. Do not fopen backlog.md from `http_server.c` or `spec_board.c`. Never create the file.
Consequences: Two debt parsers, two file formats. Specs GET stays TECH_DEBT.md-only. Inbox hide is untouched.
Alternatives: call spec_board parse (wrong format, locked helper); parse in HTTP (splits skill IO); allow `.` in tags (planner rejected).
Related ADRs: grill ADR-002; SDD-ADR-066, SDD-ADR-056, SDD-ADR-070

## SDD-ADR-074: Game-only debt strip after BlockedStrip; reuse Specs i18n
Status: Accepted
Spec: spec-017-b4w-game-debt-chrome | Date: 2026-09-03
Problem: WorkspaceHeader would leak Game debt onto Graph/ADR. A visible heading is not required. A new i18n key with the same English can drift. Clickable rows would invite POST/clipboard/expand. Filter-row placement would imply Show Dones / Track apply to debt.
Decision: Paint only in GameBoardTab, after `<BlockedStrip />` and before the four columns, when `debt.length > 0`. `role="region"` + `aria-label` from existing `specBoard.openTechDebt` (`en` = `"Open tech debt"`). No new `gameBoard.openTechDebt`. No required visible heading. Rows are `<p>` text: `id` then title, `whitespace-normal break-words` (same as Specs DebtStrip). No onClick, clipboard, or expand. Omit the region when `debt` is missing or `[]`. Do not edit WorkspaceHeader or SpecBoardTab. Show Dones / Track / Show archived do not filter the strip.
Consequences: Graph/ADR/Specs mounts have no Game debt rows. Specs may still show its own TECH_DEBT.md strip. Tests use `getByRole("region", { name: "Open tech debt" })`. Chrome stays grayscale.
Alternatives: visible `<h2>` (not required); new gameBoard key (drift); WorkspaceHeader / filter row (grill ADR-009 rejected); buttons/copy (dead-text lock); `break-all` (would shatter prose; Specs already uses `break-words`).
Related ADRs: grill ADR-008, ADR-009; SDD-ADR-067, SDD-ADR-057, SDD-ADR-005
