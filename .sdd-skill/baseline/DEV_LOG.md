# Dev Log
Updated: 2026-09-05T13:55:00Z
Active Spec: none / Last closed: spec-017-b4w-game-debt-chrome / Total Tasks Completed: 60 / Branch: local working tree

## [2026-09-05] — GitHub Actions fork trim
- Removed DCO + Pages (both red on main: no Signed-off-by; Pages not enabled)
- Dropped original-org release/community CI (npm/PyPI/VirusTotal, stale, CODEOWNERS @DeusData)
- Kept PR + CodeQL + dispatch smoke/repro; dropped ScanCode license-gate vs upstream
- Validation: remaining workflows under .github/workflows/; no dco.yml or pages.yml

## [2026-09-04] — README fork identity
- Repo: JMSolorzano-13/codebase-kanban-mcp; clone + `--with-ui` is the install path
- Upstream DeusData curl/npm/AUR labeled engine-only, not this kanban UI
- LICENSE: 2026 Juan M. Solórzano I. + 2025 DeusData portions
- Fork section + original URL at end of README
- Validation: README clone URL + Fork and original project

## [2026-09-04] — README scope + install
- README now leads with clone → `scripts/build.sh --with-ui` → `install` → `:9749`
- Scope: Dashboard, Graph, Specs (sdd|grill), Game XOR, ADR fill, read-only skills
- Usage: mixed Todo, archive, Game Inbox/registry/debt, Path 1:1
- Pre-built curl kept as release path (may lag this UI)
- Validation: README Contents + Install + Operator UI sections

## [2026-09-03] — spec-017 CLOSED
- Constitution IX.2 append confirmed (SDD-ADR-072..074)
- 3/3 PASS DEV; KPI met; debt=none
- Validation: game_board 74; httpd 131; graph-ui vitest 302

## [2026-09-03] — spec-017 CLOSEPREP READY
- 3/3 PASS DEV; same GET debt[]; Game strip after Blocked; zero skill writes
- KPI: open debt:* in chrome after BlockedStrip; never create backlog.md
- IX.2 constitution rec at close (SDD-ADR-072..074)
- Gate: read and understood — then close the spec
- Validation: game_board 74; httpd 131; graph-ui vitest 302

## [2026-09-03] — Task #3 GameBoardTab strip + leftover Vitest
- DebtStrip after BlockedStrip; reuse specBoard.openTechDebt; dead p; break-words
- parseGameBoard keeps debt (missing → []); Graph/ADR/header omit Game tags
- Show Dones does not hide strip; i18n / SpecBoardTab / colors.ts untouched
- Validation: graph-ui npx vitest run (25 files, 302 passed)

## [2026-09-03] — Task #2 HTTP GET additive + POST leftover locks
- Same GET read → card archive merge → to_json; HTTP does not fopen backlog.md
- GET 200 debt[]; no has_more; absent GET does not create; POST debt id 404
- Specs GET still TECH_DEBT.md only; skill trees byte-identical
- Validation: scripts/test.sh --suites httpd (131 passed / 1 skipped)

## [2026-09-03] — Task #1 game_board debt parse + JSON
- CBM_GAME_BOARD_MAX_DEBT 16; cbm_game_board_parse_backlog_debt; always-emit debt[{id,title}]
- game_debt_fill after overlay_blocked; fopen rb {root}/.gamedev/backlog.md; never create
- Closed iff body contains resolved-by; cap 16 open; no has_more; no spec_board helper
- Validation: make -f Makefile.cbm test-focused TEST_SUITES=game_board (74 passed)

## [2026-09-03] — spec-016 CLOSED
- Constitution IX.2 append confirmed (SDD-ADR-069..071)
- 3/3 PASS DEV; KPI met; debt=none
- Validation: game_board 56; httpd 122; graph-ui vitest 294

## [2026-09-02] — spec-016 CLOSEPREP READY
- 3/3 PASS DEV; registry sole hide when file present; Inbox id wrap
- KPI: present=regular file sole hide; absent keeps spec-011; wrap; zero skill writes
- IX.2 constitution rec at close (SDD-ADR-069..071)
- Gate: read and understood — then close the spec
- Validation: game_board 56; httpd 122; graph-ui vitest 294

## [2026-09-02] — Task #3 InboxCard wrap + leftover Vitest
- InboxCard id p: drop truncate; whitespace-normal break-all; full card.id
- TitleControl truncate + letter E unchanged; ArtifactCard id truncate locked
- Empty Inbox: header only; no "all tracked" / "no artifacts in this phase"
- Validation: graph-ui npx vitest run (25 files, 294 passed)

## [2026-09-02] — Task #2 HTTP GET leftover locks
- Same GET read → archive merge → to_json; HTTP does not fopen registry
- 200 omit hide-set inbox id; no has_more / registry key; 404 unchanged
- Absent GET does not create the file; spec-board still lists closed leftover
- Validation: scripts/test.sh --suites httpd (122 passed / 1 skipped)

## [2026-09-02] — Task #1 game_board registry parse + hide
- CBM_GAME_BOARD_MAX_REGISTRY 256; parse_epics_registry buffer-only
- Regular file present: skip load_conv; omit Plan+NNN hide-set only
- Absent: spec-011 Companion-to/roadmap unchanged; cap 64 no has_more
- Validation: make -f Makefile.cbm test-focused TEST_SUITES=game_board (56 passed)

## [2026-09-02] — spec-015 CLOSED
- Constitution IX.2 append confirmed (SDD-ADR-065..068)
- 4/4 PASS DEV; KPI met; debt=none
- Validation: spec_board 46; httpd 119; graph-ui vitest 290

## [2026-09-02] — spec-015 CLOSEPREP READY
- 4/4 PASS DEV; open TECH_DEBT.md in Specs chrome; Todo epic id wrap
- KPI: debt strip above 3-col; epic path wrap; zero skill writes; same GET
- IX.2 constitution rec at close (SDD-ADR-065..068)
- Gate: read and understood — then close the spec
- Validation: spec_board 46; httpd 119; graph-ui vitest 290

## [2026-09-02] — Task #4 PASS DEV
- spec-015 6/6 leftover UI Gherkin Thens; graph-ui vitest → 25 files, 290 passed
- Header omits TD-005; Companion-to Todo omits converted epic; grill-only Specs no strip/notSddSkill
- 16-row strip no Has more; Graph/ADR/Game omit Open tech debt; debt click no POST/clipboard/expand
- Stay-green: Function #06b6d4; grill-only Kanban; notSddSkill last-resort; 64-epic; spec-014 Specs lock; Task #3 strip/wrap
- Coverage reporter absent; Playwright not run; commit held
- Last task. Next: @human-trainer Trigger B closeprep (NOT @planner)
- Validation: graph-ui npx vitest run (290 passed)

## [2026-09-02] - leftover Vitest locks Task #4
- Header/Graph/ADR/Game omit Open tech debt; debt rows dead; grill-only Specs stays
- GameBoardTab.tsx / WorkspaceHeader.tsx unchanged; GraphTab mocked; strip SpecBoardTab-only
- Docs: task-4-leftover-vitest-locks.md + QUICK-DEBUG + ARCHITECTURE-VISUAL
- Last impl task; next review/tester then closeprep
- Validation: graph-ui vitest 290. No commit (held)

## [2026-09-02] — Task #3 PASS DEV
- spec-015 6/6 UI Gherkin Thens; graph-ui vitest SpecBoardTab + i18n → 49 passed
- Strip region Open tech debt + TD-005 leftover cache; Todo still shown
- Empty/missing debt omits region; epic id wraps; spec id truncate; long title wraps
- i18n en "Open tech debt" supporting; Task #4 leftovers not FAIL
- Coverage reporter absent; Playwright not run; commit held
- Validation: npx vitest run src/components/SpecBoardTab.test.tsx src/lib/i18n.test.ts

## [2026-09-02] - SpecBoardTab strip + EpicCard wrap Task #3
- DebtStrip `role=region` aria-label Open tech debt above 3-col; omit when debt missing or []
- EpicCard id `whitespace-normal break-all`; title truncate stays; SpecCard id truncate locked
- Graph/ADR/Game/WorkspaceHeader do not host strip; same GET; missing debt = []
- Docs: task-3-specboardtab-strip-epiccard-wrap.md + QUICK-DEBUG + ARCHITECTURE-VISUAL
- Validation: graph-ui vitest SpecBoardTab + i18n (49 passing). No commit (held)

## [2026-09-02] — Task #2 PASS DEV
- spec-015 5/5 HTTP Gherkin Thens; scripts/test.sh --suites httpd → 119 passed / 1 skipped
- GET 200 debt TD-005 leftover cache; 17th omitted; no has_more
- POST epic id 404 spec not found; GET missing-proj 404; bytes identical
- game-board httpd stay green; UI strip/wrap = Task #3
- Coverage reporter absent; Playwright not run; commit held
- Validation: scripts/test.sh --suites httpd (119 passed)

## [2026-09-02] - HTTP GET additive + POST leftover locks Task #2
- Existing GET already emits Task #1 debt; this task locked HTTP Gherkin in test_httpd.c
- Same GET; no TECH_DEBT fopen in HTTP; archive merge specs-only; POST epic id 404 leftover
- Docs: task-2-http-get-additive-post-epic-404.md + QUICK-DEBUG + ARCHITECTURE-VISUAL comments
- http_server.c / spec_board.c not edited this task
- Validation: scripts/test.sh --suites httpd → 119 passed. No commit (held)

## [2026-09-02] — Task #1 PASS DEV
- spec-015 11/11 Gherkin Thens; scripts/test.sh --suites spec_board → 46 passed
- Open heading TD-005; all-resolved/missing/unreadable → debt []; heading vs table; unknown open; cap 16
- Companion-to omit leftover; GET-equivalent bytes identical
- HTTP/UI/Has more/grill-only Specs = later tasks (not FAIL)
- Coverage reporter absent; Playwright not run; commit held
- Validation: scripts/test.sh --suites spec_board (46 passed)

## [2026-09-02] - spec_board debt parse + additive JSON Task #1
- `CBM_SPEC_BOARD_MAX_DEBT` 16; parse in spec_board.c; heading Status wins; table fallback
- debt_fill after grill; fopen rb; missing/unreadable → debt []; conversion matcher untouched
- to_json always emits debt[{id,title}]; no has_more; HTTP does not fopen TECH_DEBT.md
- Docs: task-1-spec-board-debt-parse.md + QUICK-DEBUG + ARCHITECTURE-VISUAL
- Validation: tests/test_spec_board.c (46 passed). No commit (held)

## [2026-09-01] — spec-014 CLOSEPREP READY
- 2/2 PASS DEV; Show Dones + Track A/B/All; AND hide archived done
- KPI: first paint hides dones; Inbox unfiltered; Specs lock; client React state
- IX.2 constitution rec at close (SDD-ADR-062..064)
- Gate: read and understood — then close the spec
- Validation: graph-ui vitest 276

## [2026-09-01] - Game visibility filters Task #2
- Track A/B/All exclusive; All keeps A/B/H/other/null then dones/archived
- AND leftover invert: Show archived alone / remount / Unarchive / project change
- Inbox unfiltered; empty filtered phase = header only
- Specs lock: no Show Dones / Track A / All
- Filter click: no GET/POST/query/localStorage
- Validation: graph-ui vitest 276

## [2026-09-01] — Task #1 PASS DEV
- spec-014 5/5 Gherkin Thens; graph-ui npx vitest run → 25 files, 265 passed
- First paint hides dones + All; Show Dones toggle; leftover pending; remount; AND hide
- Track/AND leftover/Specs lock/no-persist = Task #2 (not FAIL)
- Coverage reporter absent; Playwright not run; commit held
- Validation: graph-ui npx vitest run (265 passed)

## [2026-09-01] - Game visibility filters Task #1
- `visiblePhaseCards(cards, showArchived, showDones, track)`; hide done unless Show Dones; archived done needs AND
- Chrome: Show archived `|` Show Dones `|` Track A/B/All; `aria-pressed`; first-paint dones off, All on
- `lastProjectRef` reset also clears `showDones` + `trackFilter`; GET refetch keeps
- i18n `gameBoard.showDones`/`trackA`/`trackB`/`trackAll` en+zh; keep `specBoard.showArchived`
- Invert first-paint unarchived-done (`SYS-003-done`, Archive hide)
- Validation: graph-ui vitest 265

## [2026-08-31] — spec-013 CLOSED
- 3/3 PASS DEV; KPI: gamedev trio on `.gamedev/` dir; leftover sdd omitted
- Constitution: IX.5 XOR + IX.2 append (SDD-ADR-058..061)
- Debt=none; ready `/sdd-skill feature new <name>`
- Validation: test_results.log Task #1–#3 PASS DEV

## [2026-08-31] — spec-013 CONSTITUTION GATE
- Human closeprep confirmed; KPI 3/3 PASS DEV
- Proposed MODIFIED IX.5 (XOR trio) + APPEND IX.2 (SDD-ADR-058..061)
- Spec not closed; wait constitution confirm
- Validation: test_results.log Task #1–#3 PASS DEV

## [2026-08-31] — spec-013 CLOSEPREP READY
- 3/3 PASS DEV; XOR fill + HTTP/MCP + AdrTab no cycle stamp
- KPI: gamedev trio on `.gamedev/` dir; leftover sdd omitted
- IX.5/IX.2 constitution rec at close
- Gate: read and understood — then close the spec

## [2026-08-31] — Task #3 PASS DEV
- Isolated AdrTab: formatIndexedAt + replace warning; no `gamedev-skill`
- AdrTab + colors + i18n Vitest → 18 passed
- Last impl task; closeprep next

## [2026-08-31] — Task #3 AdrTab generic chrome; no cycle stamp
- Isolated AdrTab: formatIndexedAt(indexed_at) + replaceWarning + one textarea
- document has no substring gamedev-skill (no cycle stamp)
- AdrTab.tsx / i18n.ts / App.tsx / colors.ts unchanged
- Validation: graph-ui `npx vitest run src/components/AdrTab.test.tsx src/lib/colors.test.ts` → 12 passed

## [2026-08-31] — Task #2 PASS DEV
- 12/12 job Gherkin (9 HTTP + 3 MCP)
- `scripts/test.sh --suites httpd,mcp` exit 0
- AdrTab chrome owned by Task #3
- Next: Task #3 AdrTab generic chrome; no cycle stamp

## [2026-08-31] — Task #2 HTTP + MCP + watcher Gherkin
- 12 job Gherkin: 9 HTTP + 3 MCP; spec-004 sdd fixtures stay green (`ui_adr_fill_tree` writes `.sdd-skill` only)
- Dual-tree XOR, create-index, switch/remove/both-gone, partial, empty dir, unreadable, POST replace
- MCP: `index_repository` same store blob; `manage_adr` manual survives; `adr_fill:false` no gamedev markers
- Unreadable HTTP: directory-at-path (chmod 0 fails semantic_manifest; `.gamedev` not ALWAYS_SKIP)
- Validation: `scripts/test.sh --suites httpd` → 117 passed, 1 skipped; `--suites mcp` → 201 passed, 2 skipped

## [2026-08-31] — Task #1 PASS DEV
- 8/8 unit Gherkin Thens (XOR / partial / empty dir / file-at-path / NULL / cap / unreadable / sdd)
- `scripts/test.sh --suites adr_fill` → 21 passed
- HTTP/MCP/AdrTab owned by Tasks #2/#3
- Next: Task #2 HTTP + MCP + watcher Gherkin

## [2026-08-31] — Task #1 XOR trio select + gamedev relatives
- `cbm_adr_fill_document`: `.gamedev/` dir → gamedev trio only; else sdd; neither dir → NULL
- Relatives: game_context.md + baseline/TECH_STACK.md + ARCHITECTURE_ADR.md; local `cbm_is_dir`
- Empty `.gamedev/` still marks; file-at-path `.gamedev` is not present; no sdd fallback
- Validation: `scripts/test.sh --suites adr_fill` → 21 passed
- No commit (held). Pipeline / ALWAYS_SKIP / constitution untouched

## [2026-08-31] — spec-013 PLAN APPROVED
- User approved plan; Task #1 XOR trio select + gamedev relatives
- Edit set: adr_fill.c/h + test_adr_fill.c only
- SDD-ADR-058..061; pipeline/ALWAYS_SKIP untouched
- Validation: C unit after Task #1 (no full index)

## [2026-08-31] — spec-013 PLAN READY
- Gherkin approved; XOR trio in `cbm_adr_fill_document` (gamedev dir wins)
- Pipeline hook / `adr_fill` flag / ALWAYS_SKIP unchanged; `.gamedev` not skipped
- 3 tasks: helper XOR → HTTP/MCP Gherkin; AdrTab chrome parallel
- SDD-ADR-058..061; no new HTTP/MCP
- Validation: plan gate — awaiting approved start development

## [2026-08-31] — spec-012 CLOSED
- Constitution: MODIFIED IX.2 (Game expand + archive + deps; game_archive; POST /api/game-board; SDD-ADR-052..057)
- 5/5 PASS DEV; KPI met; closeprep + constitution confirmed; debt=none
- In-place expand; done artifact archive hides on fresh visit; blocked strip; 0 skill writes
- Validation: graph-ui `npx vitest run` → 260 passed; C store_game_archive 10; game_board 38; httpd 108 passed, 1 skipped
- Next: `/sdd-skill feature new <name>`

## [2026-08-31] - Archive UI + refresh + remaining Vitest
- `useGameBoard.refresh` same GET; never sets settled/present false
- App passes project+refresh; Show archived in Game chrome (`aria-pressed`, session-only)
- Phase filter hides done+archived; Inbox unfiltered; leftover pending archived stays
- Archive/Unarchive on expanded done only; POST `/api/game-board` then await refresh; no confirm
- Validation: `cd graph-ui && npx vitest run` → 260 passed

## [2026-08-31] - GameBoardTab expand + blocked strip + i18n
- Title `<button aria-expanded>` Set by GET id; all collapsed; multi-open; project/remount clear
- Track A blurb+tasks/`noTasksYet`+Inputs; Track B last_decision+open; Inbox summary/plan_title only
- Blocked region from GET `blocked[]` above columns; overlay prefix on card; empty omits region
- parseGameBoard additive; missing archived→false blocked→[]; useGameBoard one-shot; no POST
- Validation: `cd graph-ui && npx vitest run` → 249 passed

## [2026-08-31] - HTTP POST + GET merge + publish copy
- GET merge game_archive after read; matching ids only; missing table/open → all false, 200
- POST /api/game-board flag object; 409 overlay/pending; Inbox epic 404; mutation lock 423
- dispatch GET vs POST like spec-board; publish_staged copies game_archive in same live-open
- spec-board POST Game card_id still 404 spec not found; zero skill writes
- Validation: scripts/test.sh --suites httpd → 108 passed, 1 skipped; game_board 38 passed

## [2026-08-31] - C expand parse + blocked overlay
- Card: blurb/inputs/open/last_decision/recent/blocked_by[512], tasks[48], archived=0 on read
- Track A ## What it does + checkbox tasks.md + ## Inputs*; B header only; H changelog/playtest
- state.md :blocked: strip cap 16; overlay owner match → work_state blocked; Inbox never
- to_json additive keys + blocked[]; empty blocked_by → null; fopen rb; no has_more
- Validation: scripts/test.sh --suites game_board → 38 passed; httpd 97 passed, 1 skipped

## [2026-08-31] - Store game_archive table
- init_schema: CREATE TABLE game_archive (card_id TEXT PK, archived 0/1 CHECK, updated_at)
- set UPSERT; empty/NULL/strlen>=256 → ERR; unarchive keeps row (no DELETE)
- load sqlite_master probe; missing table → count 0 OK; CBM_GAME_ARCHIVE_CAP 512
- copy src→dst via set; missing src table → OK no-op; not spec_archive
- Validation: scripts/test.sh --suites store_game_archive

## [2026-08-31] — spec-011 CLOSED
- Constitution: MODIFIED IX.2 (Game phase board; filled arrays + Inbox conversion; SDD-ADR-046..051)
- 5/5 PASS DEV; KPI met; closeprep + constitution confirmed; debt=none
- Four columns; exist-only artifacts; unconverted grill Inbox; same GET; 0 skill writes
- Validation: graph-ui `npx vitest run` → 238 passed; `scripts/test.sh --suites game_board,httpd` → 125 passed, 1 skipped
- Next: `/sdd-skill feature new <name>`

## [2026-08-31] — Task #5 Remaining Vitest Gherkin mapping
- Leftover UI: 01/03 aria-current, Done/Blocked, sit-beside, chrome continue stays p, no dim
- App fetch log never /api/skill-presence; GET game-board/spec-board stay GET-only (no POST)
- Inbox from game-board not spec-board epics; silent-win / Enter Graph / tab=specs→game stay
- colorForLabel("Function") === "#06b6d4" locked in-suite
- Validation: `cd graph-ui && npx vitest run` → 238 passed
- Next: @human-trainer Task #5 docs (Trigger A, NOT closeprep)

## [2026-08-31] — Task #4 Clipboard, no-drag, typed card parse
- parseGameBoard types GameBoardCard[]; skip objects without kind artifact|epic
- Artifact continue button writeText @role; Inbox copies /gamedev-skill continue
- Clipboard denied/missing: selectNodeContents; no toast; cards draggable=false
- Card activate no Archive/Unarchive/"No tasks planned yet"; no POST; chrome continue stays p
- Validation: `cd graph-ui && npx vitest run src/components/GameBoardTab.test.tsx src/hooks/useGameBoard.test.ts` → 26 passed
- Next: @human-trainer Task #4 docs

## [2026-08-31] — Task #3 GameBoardTab four columns + cards + i18n
- Four headers always; empty = header only; aria-current on matching phase
- Artifact A/B/H + work-state; Inbox letter E; chrome continue stays text
- i18n Inbox + Pending/In progress/Done/Blocked en+zh; GameBoardCard types
- Validation: `cd graph-ui && npx vitest run src/components/GameBoardTab.test.tsx src/lib/i18n.test.ts` → 19 passed; full suite 221
- Next: @human-trainer Task #3 docs

## [2026-08-31] — Task #2 C Inbox walk + conversion + HTTP bytes PASS DEV
- Inbox walk game_grill_* ; convert Companion-to exact OR slug+table NNN
- Unconverted epic JSON; closed leftover stays; no .grill → inbox []; GET bytes identical
- kebab / plan-folder / table-NNN-without-slug do not convert
- Validation: `scripts/test.sh --suites game_board,httpd` → 125 passed / 1 skipped; game_board 28/28
- Next: @implementer Task #3 UI columns/cards

## [2026-08-31] — Task #1 C artifact walk + widen card JSON
- Widened `cbm_game_board_card_t`; `CBM_GAME_BOARD_LIST_MAX` 256; cap 64 after production sort
- Exist-only pre/prod/post walk; `status:` first token incl `ready`→`in_progress`; compiled owner/track
- `cbm_game_board_to_json` grow buffer; no `has_more`/`column`; inbox count 0; chrome unchanged
- Validation: `scripts/test.sh --suites game_board,httpd` → 18 game_board + 95 httpd, 1 skipped
- Next: @human-trainer Task #1 docs

## [2026-08-31] — spec-010 CLOSED
- Constitution: MODIFIED IX.2 (Game tab silent win; GET /api/game-board; SDD-ADR-042..045)
- 5/5 PASS DEV; KPI met; closeprep + constitution confirmed; debt=none
- `.gamedev/` shows Game and omits Specs; chrome phase/focus/continue; leftover `tab=specs` → game
- Validation: graph-ui `npx vitest run` → 211 passed; `scripts/test.sh --suites game_board,httpd` → 103 passed, 1 skipped
- Next: `/sdd-skill feature new <name>`

## [2026-08-31] — Task #4 App silent-win dual fetch
- useGameBoard one-shot GET /api/game-board; settled gates Game+Specs
- showGame/showSpecs; resolveWorkspaceTab; specs-on-gamedev → game
- mockAppFetch default game-board 200 present false; hang/500/true opt-in
- Validation: graph-ui `npx vitest run src/App.test.tsx src/hooks/useGameBoard.test.ts` → 48 passed
- Next: @human-trainer Task #4 docs

## [2026-08-31] — Task #3 GameBoardTab chrome only
- New GameBoardTab; props receive GameBoard JSON (no fetch); grayscale chrome
- phase 02-production → Production + focus + /gamedev-skill continue as text
- phase/focus null → state.md missing + continue; arrays not painted
- Validation: graph-ui `npx vitest run src/components/GameBoardTab.test.tsx` → 3 passed
- Next: @human-trainer Task #3 docs

## [2026-08-31] — Task #2 TabId, route kernels, strip, i18n
- WORKSPACE_TABS +game; readRoute/routeUrl accept tab=game; fallbackGameToGraph + resolveWorkspaceTab
- WorkspaceTabStrip showGame → Graph|Game|ADR; omit when false; no disabled placeholder
- i18n en+zh: tabs.game, state.md missing, Pre-production / Production / Post-production & Launch
- Validation: graph-ui `npx vitest run src/lib/route.test.ts src/components/WorkspaceTabStrip.test.tsx src/lib/i18n.test.ts` → 23 passed; App.test 30 stayed green
- Next: @human-trainer Task #2 docs

## [2026-08-31] — Task #1 tester PASS DEV
- 5/5 Task #1 C/HTTP Gherkin: empty arrays, spec-board no gamedev field, 404, 400, GET bytes identical
- Validation: scripts/test.sh --suites game_board,httpd → 103 passed, 1 skipped; game_board 9/9
- UI Gherkin not this task (not FAIL). Next: @implementer Task #2. NOT closeprep.

## [2026-08-31] — Task #1 C/HTTP GET /api/game-board
- New `game_board.c`/`game_board.h`: heap `cbm_game_board_t`; dir via `cbm_spec_board_gamedev_skill_present`; fopen rb `state.md`
- Parse compact `phase=`/`focus=`; aliases `active_phase`/`director_focus` (`=` or line-start `:`); JSON phase only 01/02/03 tokens
- GET `/api/game-board` only (400/404 same strings); arrays []; no POST; spec-board still has no gamedev field
- Validation: `make -f Makefile.cbm test-focused TEST_SUITES="game_board httpd"` → 103 passed, 1 skipped
- Next: @human-trainer Task #1 docs

## [2026-08-30] — spec-009 CLOSED
- Constitution: MODIFIED IX.2 (presence sdd OR grill; spec-008 omit-until-sdd superseded; SDD-ADR-039..041)
- 3/3 PASS DEV; KPI met; closeprep + constitution confirmed; debt=none
- graph-ui only; same GET; hook/host OR; restore inbound tab=specs
- Validation: hook 10 / SpecBoardTab 36 / App 30 Vitest; constitution gate confirmed
- Next: /sdd-skill feature new <name>

## [2026-08-30] — spec-009 closeprep (human-trainer Trigger B)
- Summary: `human/spec-summaries/spec-009-t4x-specs-tab-grill-presence.md`
- PROJECT-OVERVIEW: Specs present sdd OR grill; Enter still Graph; not closed
- ARCHITECTURE-VISUAL: verified (strip OR + restore + host Kanban already from #1–#3)
- Patterns: ✓ same GET / same hook name / same host OR / no C (SDD-ADR-039..041)
- CONSTITUTION RECOMMENDATION: MODIFIED IX.2 append; spec-008 omit-until-sdd superseded. Gate waiting. NOT @planner close

## [2026-08-30] — Task #3 human docs (App strip / deep-link / Enter / omit)
- Summary: `human/task-summaries/task-3-app-strip-deep-link-enter.md` (Path=full)
- QUICK-DEBUG: grill-only strip; restore inbound tab=specs; Enter stays Graph; neither / gamedev omit
- ARCHITECTURE-VISUAL: omit-until-true + pendingSpecsDeepLink restore; Enter → Graph
- Patterns: ✓ same present / same fallback signature / no C (SDD-ADR-041)
- Validation: docs only. Trigger A (NOT closeprep). Handoff @review

## [2026-08-30] — Task #3 App strip, deep-link, Enter, omit Gherkin
- mockAppFetch: independent sdd_skill_present / grill_skill_present + optional epics; default neither omits Specs
- Grill-only strip/deep-link/Enter; sdd+grill show; neither + gamedev-only omit; GET 500 kept
- App restores inbound tab=specs after present (omit-until-true); fallbackSpecsToGraph signature unchanged
- Validation: `cd graph-ui && npx vitest run src/App.test.tsx` (30 passed); full graph-ui 182 passed
- No commit (held). Path=full. SDD-ADR-041. Last impl task.

## [2026-08-30] — Task #2 human docs (SpecBoardTab host Kanban grill-only)
- Summary: `human/task-summaries/task-2-specboardtab-host-kanban-grill-only.md` (Path=full)
- QUICK-DEBUG: host gate sdd OR grill; notSddSkill last-resort; stale spec-008 host rows retargeted
- ARCHITECTURE-VISUAL: HostGate sdd OR grill; last-resort both-false / !board
- Patterns: ✓ same OR as hook; last-resort kept; no second poll
- Validation: docs only. Trigger A (NOT closeprep). Handoff @review

## [2026-08-30] — Task #1 human docs (presence predicate sdd OR grill)
- Summary: `human/task-summaries/task-1-presence-predicate-sdd-or-grill.md` (Path=compact)
- QUICK-DEBUG: hook is sdd OR grill; stale "sdd-only strip" rows retargeted; host still sdd-only
- ARCHITECTURE-VISUAL: Presence node sdd OR grill (host comment until Task #2)
- Patterns: ✓ same GET / same hook name / same present boolean; OR is intended
- Validation: docs only. Trigger A (NOT closeprep). Handoff @review

## [2026-08-30] — Task #1 Presence predicate sdd OR grill
- bodyHasSkill: sdd_skill_present === true || grill_skill_present === true; missing grill / non-boolean = false
- useSddSkillPresent export + UseSddSkillPresentResult unchanged; one-shot GET /api/spec-board
- Old Then "present true only when sdd === true" replaced; grill-only / sdd-only / both / neither / missing grill
- Validation: `cd graph-ui && npx vitest run src/hooks/useSddSkillPresent.test.ts` (10 passed)
- No commit (held). Path=compact. SDD-ADR-039

## [2026-08-30] — spec-008 closeprep (human-trainer Trigger B)
- Summary: `human/spec-summaries/spec-008-g8r-grill-epic-todo.md`
- PROJECT-OVERVIEW: Mixed Todo epics + letter E; tab still sdd-only; not closed
- ARCHITECTURE-VISUAL: verified (EpicCard + grill walk already from #1–#3)
- Patterns: ✓ GET family; grill in spec_board.c; kind on epics only; E chrome token; zero skill writes
- CONSTITUTION RECOMMENDATION: MODIFIED IX.2 append. Gate waiting. NOT @planner close

## [2026-08-30] — Task #4 human docs (Vitest Gherkin remaining UI)
- Summary: `human/task-summaries/task-4-vitest-gherkin-grill-epic.md`
- QUICK-DEBUG: two-plan order; 64-epic no Has more; Companion-to omit mock; Done/In progress no epic id
- ARCHITECTURE-VISUAL: unchanged (test-only)
- Patterns: ✓ already-filtered epics; host not-sdd-skill; colorForLabel #06b6d4
- Validation: docs only. Last impl task — Trigger A (NOT closeprep). Handoff @review

## [2026-08-30] — Task #4 Vitest Gherkin mapping remaining UI scenarios
- SpecBoardTab.test.tsx maps remaining UI Gherkin Thens; conversion stays C-owned (mock already-filtered epics)
- Host: no selectProject when project set; loading / not-sdd-skill hold with grill_skill_present true
- Two-plan document order; Done/In progress no epic id; 64-epic mock no Has more; Function hex #06b6d4
- Epic activate: no Archive/Unarchive, no "No tasks planned yet", POST not called
- Validation: `cd graph-ui && npx vitest run` (170 passed, 34 SpecBoardTab). No commit (held). Last impl task. Trigger A (NOT closeprep)

## [2026-08-30] — Task #3 human docs (EpicCard + Todo epics-then-specs)
- Summary: `human/task-summaries/task-3-epiccard-todo-order.md`
- QUICK-DEBUG: E token #7d8ec9; Todo order; no expand/archive/POST on epic; missing epics []; grill+sdd false hides Specs; live :9749 may be pre-spec-008
- ARCHITECTURE-VISUAL: EpicCard + Todo epics-then-specs
- Patterns: ✓ III E hue; IX.2 expand+archive on SpecCard; omit-until sdd
- Validation: docs only. Trigger A (NOT closeprep). Handoff @review

## [2026-08-30] — Task #3 SpecBoardTab EpicCard + Todo epics-then-specs
- SpecBoardEpic (kind/id/title/summary/plan_title/column todo); SpecBoard.grill_skill_present? + epics?
- `--color-epic-mark: #7d8ec9`; EpicCard literal E via `text-[var(--color-epic-mark)]` (not i18n, not Epic, not a pill)
- Todo document order: epics then spec todos; pendingCount includes both; In progress/Done spec-only
- EpicCard display-only (no expand/archive/POST); spec-005 expand and spec-006 Archive stay on SpecCard
- Host: missing epics → []; sdd false + grill true → not-sdd-skill copy
- Validation: `cd graph-ui && npx vitest run` (165 passed, 29 SpecBoardTab). Live :9749 is pre-spec-008 binary (GET has no epics). No commit (held)

## [2026-08-30] — Task #2 human docs (HTTP GET additive + POST epic 404)
- Summary: `human/task-summaries/task-2-http-get-additive-post-epic-404.md`
- QUICK-DEBUG: GET missing-proj 404; POST epic 404 spec not found; skill-tree bytes identical; no grill IO in HTTP
- ARCHITECTURE-VISUAL: GET additive epics; POST find specs-only → 404
- Patterns: ✓ read → specs-only merge → to_json; POST find specs only
- Validation: docs only. Trigger A (NOT closeprep). Handoff @review

## [2026-08-30] — Task #2 HTTP GET additive + POST epic-id 404
- GET still read → specs-only archive merge → to_json; no grill fopen in http_server.c
- GET 200 includes grill_skill_present + epics; unknown project 404 project not found
- POST epic id 404 spec not found; no store write; epic.md and active.json identical
- GET 200 leaves index.md, epic.md, active.json byte-identical
- Validation: `scripts/test.sh --suites spec_board,httpd` (124 passed, 1 skipped). No commit (held)

## [2026-08-30] — Task #1 human docs (grill read)
- Summary: `human/task-summaries/task-1-spec-board-grill-read.md`
- QUICK-DEBUG: grill present / conversion / cap 64 / missing .grill / unreadable skip
- ARCHITECTURE-VISUAL: `.grill/` walk on same GET; epics skip archive merge
- Patterns: ✓ fopen rb / additive JSON / own cap / zero skill writes
- Validation: docs only. Trigger A (NOT closeprep). Handoff @review

## [2026-08-30] — Task #1 spec_board grill read + additive JSON
- `CBM_SPEC_BOARD_MAX_EPICS` 64; `grill_skill_present` + `epics[]` (kind/id/title/summary/plan_title/column todo)
- Conversion: strcmp Companion-to first `.grill/plans/`…`.md` or `source.grill_epic`; trailing notes ignored
- Order: index.md rows then unlisted slug-asc; within plan epic-NNN; cap 64 after omit; no has_more
- fopen rb only; grill fill without sdd; dir-at-path skipped; specs have no kind
- Validation: `scripts/test.sh --suites spec_board` (35 passed). No commit (held)

## [2026-08-30] — spec-007 Trigger B closeprep
- Summary: `human/spec-summaries/spec-007-n6p-last-indexed-local.md`
- PROJECT-OVERVIEW: last-indexed visible text is browser local TZ; dateTime/title stay ISO
- ARCHITECTURE-VISUAL: comment only (same helper, no timeZone pin)
- CONSTITUTION RECOMMENDATION: MODIFIED IX.2 append (SDD-ADR-034)
- Patterns: ✓ one helper / local Intl / raw ISO attrs / newest ISO / zero C-HTTP
- Validation: docs only. Gate waiting "✅ Read and understood — you can close the spec". NOT @planner close

## [2026-08-30] — Task #2 Surface Gherkin local text + raw ISO dateTime
- Surfaces already call helper; test-only: Dashboard/header/AdrTab assert text===formatIndexedAt + dateTime/title raw ISO
- Invalid `not-a-date` stays visible raw on all three; AdrTab textarea does not invent a clock
- Ghost header still omits `time`; AdrTab without CBM-GENERATED-START omits stamp; Enter newest still `alpha`
- Validation: `cd graph-ui && npx vitest run src/components/Dashboard.test.tsx src/components/WorkspaceHeader.test.tsx src/components/AdrTab.test.tsx src/lib/colors.test.ts` (38 passed). No commit (held)

## [2026-08-30] — Task #1 Drop UTC pin in formatIndexedAt
- Dropped `timeZone:"UTC"`; `INDEXED_AT_PARTS` keeps year/month/day/hour/minute + `timeZoneName:"short"`
- Invalid/empty input still returns the raw string; no second formatter; no `process.env.TZ`
- Tests: Intl `localFmt` (no timeZone) vs `utcFmt` (`timeZone:"UTC"`); no hardcoded wall-clock
- Host-UTC Limit: when `localFmt===utcFmt`, return equals that shared string
- Validation: `cd graph-ui && npx vitest run src/lib/formatIndexedAt.test.ts` (4 passed). No commit (held)

## [2026-08-30] — spec-006 Trigger B closeprep
- Summary: `human/spec-summaries/spec-006-k3n-spec-archive.md`
- PROJECT-OVERVIEW + ARCHITECTURE-VISUAL: CBM archive flag; GET merge; POST flag object; session Show archived; await refresh
- CONSTITUTION RECOMMENDATION: MODIFIED IX.2 (replace "Archive is not part of spec-005")
- Patterns: ✓ store / HTTP merge / POST / session toggle / await refresh / zero skill writes
- Validation: docs only. Gate waiting "✅ Read and understood — close the spec". NOT @planner close

## [2026-08-30] — Task #4 Vitest Gherkin mapping remaining UI scenarios
- SpecBoardTab.test.tsx maps all UI Gherkin Then (hide/no-confirm, show+unarchive, fresh hide, count, leftover Todo, empty Done+toggle)
- Poll: after Archive POST, rerender GET `archived: true`; card stays hidden while Show archived off
- spec-005 "no Archive/Unarchive" replaced: those names only on expanded Done
- colorForLabel("Function") === "#06b6d4"; host selectProject/loading/not-sdd-skill hold
- Validation: `cd graph-ui && npx vitest run` (156 passed, 24 SpecBoardTab). No commit (held)

## [2026-08-30] — Task #3 SpecBoardTab filter + session toggle + Archive/Unarchive
- SpecBoardEntry.archived; missing treated as false. i18n archive/unarchive/showArchived en+zh
- showArchived useState(false); reset with expandedIds on project change; no localStorage
- Done header "Show archived" aria-pressed; filter `archived && !showArchived`; count = shown
- Archive/Unarchive POST `/api/spec-board` then await refresh(); no confirm; three columns
- Validation: `cd graph-ui && npx vitest run` (154 passed, 22 SpecBoardTab). No commit (held)

## [2026-08-30] — Task #2 HTTP POST + GET merge + publish copy
- GET `/api/spec-board` merges `spec_archive` after `cbm_spec_board_read`; missing table/open → all false
- POST same path: flag object 200; 409 `spec not done` no write; 404 spec/project; 400 invalid archived
- Mutation lock 423 like ADR save; `publish_staged` copies spec_archive live→stage after ADR
- Validation: `scripts/test.sh --suites spec_board,httpd` (105 passed, 1 skipped). No commit (held)

## [2026-08-30] — Task #1 store spec_archive set/load/copy
- `init_schema` CREATE spec_archive (spec_id PK, archived 0/1 CHECK, updated_at iso_now)
- set UPSERT; empty/NULL spec_id ERR; unarchive writes 0 (no DELETE)
- load: sqlite_master probe, missing table → count 0 OK; cap 64
- copy: load src then set each dst row; missing src table OK no-op
- Validation: `scripts/test.sh --suites store_spec_archive` (9 passed). No commit (held)

## [2026-08-30] — spec-005 CLOSED
- KPI met. 4/4 tasks PASS DEV. Closeprep + constitution confirmed
- Constitution: 1 MODIFIED (IX.2) — Specs-tab expand / enrich-all / dual matcher / Todo pending-only
- Archive not in this spec (grill epic-002)
- Validation: test_results.log Tasks #1–#4 PASS DEV. Next: `/sdd-skill feature new <name>`

## [2026-08-30] — spec-005 Trigger B closeprep
- Summary: `human/spec-summaries/spec-005-v2m-spec-card-expand.md`
- PROJECT-OVERVIEW: in-place Specs expand; GET blurb; Todo pending-only; no Archive
- CONSTITUTION RECOMMENDATION: MODIFIED IX.2 (card expand / enrich-all / dual matcher / zero Archive)
- Blocked on human "✅ Read and understood — close the spec" — role stays @human-trainer

## [2026-08-30] — Task #4 Vitest Gherkin + poll persist + no Archive
- SpecBoardTab.test maps all UI Gherkin Thens; host selectProject/loading/not-sdd-skill held
- Multi-expand two blurbs; poll rerender keeps opened Todo blurb
- Done expand: document has no Archive/Unarchive control
- In Progress first paint: expanded, blurb, both tasks, implementer, tasksDone 1/2
- Validation: `cd graph-ui && npx vitest run src/components/SpecBoardTab.test.tsx src/lib/colors.test.ts` (16 passed). No commit (held)

## [2026-08-30] — Task #3 SpecCard expand Set + blurb + TaskList
- expandedIds Set on SpecBoardTab; project change clears+reseeds; poll keeps Set
- Title button only expand; canExpand every card incl. task_count 0
- Blurb text region if non-empty; Todo pending-only; IP/Done full list + chrome
- Validation: `cd graph-ui && npx vitest run` (144 passed). No commit (held)

## [2026-08-30] — Task #2 enrich all + dual done matcher
- Every listed spec: one spec.md (title+blurb) + one tasks.md; log once
- Active: bare Task #N last-line-wins; non-active needs spec id + Task #N
- Missing/unreadable spec.md or tasks.md degrades that entry only; zero writes
- Validation: `scripts/test.sh --suites spec_board` (18 passed). No commit (held)

## [2026-08-30] — Task #1 blurb extract helper
- `CBM_SPEC_BOARD_BLURB_MAX` 512; public `cbm_spec_board_extract_blurb`
- `to_json` emits escaped `"blurb"` on every spec; unread extract is `""`
- ES 1–2 sentences; KPI/H1/H3 never copied; links → text; 512 walkback
- Validation: `scripts/test.sh --suites spec_board` (12 passed). No commit (held)

## [2026-08-30] — spec-005 PLAN READY
- GET /api/spec-board additive `blurb`; enrich title/tasks for every listed spec
- Dual done matcher; SpecBoardTab expanded Set; no Archive; no TZ change
- SDD-ADR-024..028; 4 tasks; stack unchanged
- Validation: architect artifacts only (no implementer run)

## [2026-08-30] — graph-ui tsc -b for --with-ui
- `reindexError` assign typed as `string` (literal union blocked server error text)
- `tsc -b` excludes `*.test.ts(x)` so chrome-tokens node: imports stay Vitest-only
- Unused `init` dropped in Dashboard list test
- Validation: `cd graph-ui && npm run build`

## [2026-08-30] — spec-004 closed
- User-triggered index fills ADR generated from trio; manual preserved; watcher fill = 0
- Constitution: IX.2 MODIFIED, IX.5 ADDED (fence, adr_fill, ALWAYS_SKIP)
- Validation: Task #1–#4 PASS DEV (6+9+11+18 mapped). No commit (held)

## [2026-08-30] — Task #4 AdrTab generated-at + replace warning
- Stamp from `Project.indexed_at` via `formatIndexedAt` + `useProjects` (`<time dateTime>`) when GET has `CBM-GENERATED-START`
- Markers absent → stamp + warning omitted; chrome only, ISO not written into the blob
- Warning i18n en+zh: generated-region edits replaced on next user-triggered index
- One textarea; Save still POSTs `{project, content}`; dirty-leave confirm unchanged
- Validation: `npx vitest run src/components/AdrTab.test.tsx src/lib/i18n.test.ts src/lib/colors.test.ts` (17 passed). No commit (held)

## [2026-08-30] — Task #3 HTTP + MCP + watcher Gherkin
- POST `/api/adr` body max 32768; 16384 generated+manual still 200; above max → 400
- POST `/api/index` create and `{root_path, project}` Reindex fill GET `/api/adr`
- `index_repository` fills; `manage_adr` get matches the same blob
- `adr_fill: false` / no `.sdd-skill` leave unmarked; partial + unreadable omit extract
- Validation: `scripts/test.sh --suites httpd,mcp` (274 passed). No commit (held)

## [2026-08-30] — Task #2 pipeline hook + `.sdd-skill` skip
- `adr_fill` default false; `index_repository` true unless `adr_fill: false`
- Watcher args encode false; normalize strips key so subscribe still works
- Splice after capture on full + incremental persist + closure
- Exact no-op forces full only when fill would change the stored ADR
- `.sdd-skill` in ALWAYS_SKIP_DIRS; trio files are not File nodes
- Validation: `scripts/test.sh --suites adr_fill,discover,mcp,daemon_application,incremental`. No commit (held)

## [2026-08-30] — Task #1 splice + extract helper
- `cbm_adr_fill_document`: trio extract + four HTML comment markers; no store/HTTP
- 1536 B/file after 64KiB read; unmarked body → manual; no `.sdd-skill/` → NULL
- Unreadable = regular file open/read fail (`chmod 0` or directory-at-path)
- Validation: `scripts/test.sh --suites adr_fill` (9 passed). No commit (held)

## [2026-08-30] — spec-004 PLAN READY
- Capture-then-splice before publish; watcher `adr_fill: false`; not incremental-vs-full
- Extract 1536 B/file; POST /api/adr max 32768; manage_adr stays whole-doc
- SDD-ADR-019..023; 4 tasks; constitution not rewritten
- Validation: plan.md + tasks.md + checklist.md in specs/spec-004-j8k-adr-parse-on-reindex/

## [2026-08-30] — spec-003 CLOSED
- KPI met: 409+Graph notice; conflict newest+confirm-delete; Reindex same row
- Constitution: IV.1–IV.4 + IX.2 confirmed (SDD-ADR-014..018)
- TD-002 resolved; active.json idle; next spec-004
- Validation: 5/5 Task DEV PASS in test_results.log

## [2026-08-29] — spec-003 Trigger B closeprep
- Summary: `human/spec-summaries/spec-003-h7q-path-project-identity.md`
- PROJECT-OVERVIEW: Path 1:1, conflict UI, create 409 redirect, Dashboard Reindex
- CONSTITUTION RECOMMENDATION: rewrite IV.1 (admission 1:1; store still name-keyed)
- Blocked on human "✅ Read and understood — close the spec" — role stays @human-trainer

## [2026-08-29] — Task #5 tester DEV PASS
- Vitest: Dashboard+App+i18n+colors+WorkspaceHeader 57/57
- Gherkin: Reindex 202 stay Dashboard; custom name kept; 500 stay+alert
- colorForLabel("Function") still #06b6d4
- Next: @human-trainer Trigger B closeprep (not @planner)

## [2026-08-29] — Task #5 Dashboard Reindex (human-trainer Trigger A)
- Summary: `human/task-summaries/task-5-dashboard-reindex.md`
- QUICK-DEBUG + ARCHITECTURE-VISUAL: Reindex POST `{root_path, project}`; 202 IndexProgress stay; 500 alert stay
- No WorkspaceHeader / CreateIndexModal Reindex; i18n en+zh locked
- Patterns: ✓ — Next: @review Task #5

## [2026-08-29] — Task #5 Dashboard Reindex + i18n
- Per-row Reindex POSTs `{root_path, project}` (never `project_name`); 202 → IndexProgress, stay Dashboard
- HTTP 500 → role=alert + i18n fallback; row remains; no Graph navigation
- i18n en+zh: Reindex, reindexError, nameExists; conflict + pathExistsNotice unchanged
- No Reindex on WorkspaceHeader or CreateIndexModal
- Validation: `npx vitest run` → 132/132; `colorForLabel("Function")` still `#06b6d4`
- Next: @human-trainer docs for Task #5

## [2026-08-29] — Task #4 Create 409 redirect (human-trainer Trigger A)
- Summary: `human/task-summaries/task-4-create-409-redirect.md`
- QUICK-DEBUG + ARCHITECTURE-VISUAL: path_exists → Graph + role=status; name_exists stays
- Listed `canonical_root` skips POST; create body still `{root_path}` only
- Patterns: ✓ — Next: @review Task #4

## [2026-08-29] — Task #4 Create modal path_exists redirect + notice
- POST still `{root_path}` only; 409 `path_exists` → `onPathExists` (no `onCreated` / IndexProgress)
- App: `?tab=graph&project=<existing>` + `role=status` notice; clears on next navigate
- 409 `name_exists` stays in modal with body.error; listed `canonical_root` skips POST
- Validation: `npx vitest run src/components/CreateIndexModal.test.tsx src/App.test.tsx` → 25/25; full graph-ui 123/123
- Next: @human-trainer docs for Task #4

## [2026-08-29] — Task #3 Dashboard conflict groups (human-trainer Trigger A)
- Group by `canonical_root` (`root_path` fallback); newest = `indexed_at` then greater `name` (same as C)
- Conflict region: all names + times + Path once; Enter newest; per-name confirm `DELETE /api/project?name=`
- i18n en+zh: `projects.conflict`, `deleteNamed`; no Reindex (Task #5)
- Validation: `pathGroups.test.ts` + Dashboard conflict cases + i18n locks
- Next: @review Task #3

## [2026-08-29] — Task #2 Admit create vs reindex (HTTP + MCP + jobs)
- `cbm_identity_admit`: CREATE / REINDEX / MCP; 409 `path_exists`|`name_exists` before HTTP slot
- HTTP `project`/`project_name` is reindex key only; MCP empty name + 1 owner reindexes that name
- Daemon same Path + different `project_key` → PATH_CONFLICT (`path_exists`)
- Validation: `scripts/test.sh --suites identity` → 18/18; `--suites httpd` → 69/69 (+1 skip)
- Next: Task #3 Dashboard conflict groups (parallel after #1)

## [2026-08-29] — Task #1 Identity catalog + list_projects fields
- `cbm_identity_*`: canonical_root, newest/tie, cache `.db` catalog (no UNIQUE SQL)
- `list_projects` always emits `indexed_at` + `canonical_root`; display `root_path` unchanged
- `Project.canonical_root?: string` in graph-ui
- Validation: `scripts/test.sh --suites identity` → 7/7; `npx vitest run src/hooks/useProjects.test.ts src/lib/i18n.test.ts` → 8/8
- Next: Task #2 admit 409 (depends on #1)

## [2026-08-29] — Close spec-002-p8w-project-workspace
- Workspace: Graph default, Specs omit-until-true, ADR tab, last-indexed in header
- AdrButton deleted; no C change; KPI met; constitution unchanged (still draft)
- Validation: test_results.log Tasks #1–#5 PASS DEV (106 Vitest)
- Next: /sdd-skill feature new <name> (planned: spec-003 path identity)

## [2026-08-29] — Task #5 App compose + Gherkin
- Workspace: header + tablist under header + Graph / Specs? / AdrTab
- fallbackSpecsToGraph replaceState; dirty leave confirm; AdrButton deleted
- Validation: `cd graph-ui && npx vitest run` → 21 files, 106 tests
- Next: @review / @tester Task #5 then closeprep

## [2026-08-29] — Task #3 Specs presence + tab strip
- useSddSkillPresent: one-shot GET /api/spec-board; present only on 200 && sdd_skill_present===true
- WorkspaceTabStrip: tablist Graph | Specs? | ADR; fallbackSpecsToGraph helper (App #5 wires replaceState)
- SpecBoardTab: non-null project never shows picker; mock body {sdd_skill_present,specs}
- Validation: `cd graph-ui && npx vitest run` → 22 files, 98 tests
- Next: Task #5 mounts strip + helper; do not poll strip with useSpecBoard

## [2026-08-29] — Task #4 AdrTab pane
- AdrTab GET/POST /api/adr; res.ok; alert on 500; status on save; onDirtyChange
- Empty textarea + AdrButton placeholder; Delete posts ""; no CBM-GENERATED
- AdrButton still on disk; App not mounted
- Validation: AdrTab.test.tsx (5 cases). App compose is Task #5
- Next: @review/@tester Task #4; then Task #5 when #3 also reviewed

## [2026-08-29] — Task #2 Workspace header
- WorkspaceHeader: route name + optional time[dateTime] from useProjects + formatIndexedAt
- Ghost name omits time; leave calls onLeave (confirm is Task #5)
- Not mounted in App yet
- Validation: `cd graph-ui && npx vitest run src/components/WorkspaceHeader.test.tsx` → 3 passed
- Next: @human-trainer after #3/#4 land; App compose is Task #5

## [2026-08-29] — Task #1 TabId + readRoute + routeUrl
- TabId dashboard|graph|specs|adr; WORKSPACE_TABS is the extension point
- readRoute/routeUrl in lib/route.ts; workspace+project keeps tab; else Dashboard project=null
- Specs-without-skill fallback and workspace chrome not in this task
- Validation: `cd graph-ui && npx vitest run` → 17 files, 76 tests
- Next: @human-trainer docs for Task #1

## [2026-08-29] — Close spec-001-w3q-executive-dashboard
- Dashboard is account home; list_projects only; grayscale chrome; TabBar deleted
- KPI: default URL = Dashboard + Control, last-indexed visible, 0 Nodes/Edges
- TD-001 / TD-003 / TD-004 resolved; constitution unchanged (still draft)
- Validation: test_results.log Tasks #1–#5 PASS DEV (70 Vitest)
- Next: /sdd-skill feature new <name> (planned: spec-002 workspace)

## [2026-08-29] — Task #5 App routing + TabBar delete
- TabId dashboard|graph; readRoute aliases stats/control/specs/missing/unknown; graph needs project
- Header: brand only on Dashboard; Graph chip + backToDashboard; no Specs/Graph/Projects/Control tabs
- TabBar.tsx deleted; SpecBoardTab unrouted; replaceState ?tab=dashboard without project
- Validation: `cd graph-ui && npx vitest run` → 16 files, 70 tests pass
- Next: @human-trainer docs for Task #5 (last spec-001 task)

## [2026-08-29] — Task #4 Dashboard page
- Dashboard: list + Control on one ScrollArea max-w-4xl; rows name/path/time/HealthDot/Enter/Delete
- CreateIndexModal POST {root_path} only; no Project ID; 202 stays + IndexProgress
- ControlTab embedded polls 3s/2s; StatsTab deleted; AdrButton not imported
- Validation: `cd graph-ui && npx vitest run` → 15 files, 63 tests pass
- Next: @human-trainer docs for Task #4; #5 routing pending

## [2026-08-29] — Task #3 formatIndexedAt
- formatIndexedAt(iso, en|zh): Intl en-US/zh-CN, UTC, year/month/day/hour/minute, timeZoneName short
- Invalid ISO returns the raw string; no JSX, Dashboard wiring is Task #4
- Validation: `cd graph-ui && npx vitest run src/lib/formatIndexedAt.test.ts` → 3 passed
- Next: @human-trainer docs for Task #3; #4 can start (#1 #2 #3 done)

## [2026-08-29] — Task #2 grayscale chrome + palette lock
- globals.css primary/accent/ring gray; --color-hover; surfaces distinct
- Gauge healthy #a3a3a3; >80 red >50 amber; GRAPH_EDGE_PALETTE export
- Chrome panels bg-card; colors.ts + graph-loader #22d3ee unchanged
- Validation: `cd graph-ui && npx vitest run` → 12 files, 54 tests pass
- Next: @human-trainer docs for Task #2; #3 still pending

## [2026-08-29] — Task #1 useProjects list-only (TD-001)
- useProjects: list_projects only; return { projects: Project[], loading, error, refresh }
- Dropped ProjectInfo + per-project get_graph_schema; no schema: null placeholder
- SpecBoardTab ProjectPicker maps Project name + path; StatsTab p.project → p.name
- Validation: `cd graph-ui && npx vitest run` → 10 files, 40 tests pass
- Next: @human-trainer docs for Task #1; #2/#3 still pending

## [2026-08-29] — Architect plan spec-001
- plan.md: stack Dashboard+Control, UTC `<time>`, delete TabBar, no C change
- tasks.md: 5 tasks (#1 useProjects, #2 chrome lock, #3 formatIndexedAt, #4 Dashboard, #5 routing)
- SDD-ADR-002..008 in ARCHITECTURE_ADR.md
- Validation: `cd graph-ui && npm test` after implement (not run this step)
- Gate: user must approve before @implementer

## Recently Completed
Adopt init. Architect plan written; waiting approval.

## Known Issues
- UI home is StatsTab: aggregate nodes/edges + per-card schema chips + ADR modal. Planned removal on Dashboard (this spec).
- `useProjects` is list_projects only (Task #1 done). Dashboard home still StatsTab until #4/#5.
- Project identity is name, not `root_path` (PENDING — grill epic 003).
- Optional UI `project_name` creates aliases (PENDING — remove in this spec's create flow).
- Global tabs default to Specs (PENDING — Dashboard becomes default).

## Performance Metrics
| Metric | Target | Current | Status |
| Dashboard list | no per-project schema RPC | 0 schema calls in useProjects | OK (Task #1) |
| UI port | 9749 | 9749 | OK |

## Dependencies Updated
none this init

## Next Steps
1. @human-trainer docs for Task #2 → @review
2. Task #3 formatIndexedAt (parallel-ready)
3. Task #4 Dashboard after #1 #2 #3
