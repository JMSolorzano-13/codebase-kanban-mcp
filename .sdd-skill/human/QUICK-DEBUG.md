# Quick Debugging Guide
Last updated: 2026-09-03 — spec-017-b4w-game-debt-chrome Task #3
Active: spec-017 Task #3 `task-3-gameboardtab-strip-leftover-vitest.md`. Prior this spec: `task-2-http-get-additive-post-leftover.md`, `task-1-game-board-debt-parse.md`. Prior closed: `spec-016-d9v-game-inbox-registry.md`. Prior spec-016 tasks: `task-1-game-board-registry-parse-hide.md`, `task-2-http-get-leftover-locks.md`, `task-3-inboxcard-wrap-leftover-vitest.md`. Prior closed: `spec-015-s5k-specs-debt-and-path.md`.

## Common Error Index (grouped by module)

### GameBoardTab strip + leftover Vitest (spec-017 Task #3)

GameBoardTab paints a local `DebtStrip` after `<BlockedStrip />` and before `grid-cols-4`. Accessible name reuses `t.specBoard.openTechDebt` (`en` === `"Open tech debt"`). Rows are dead `<p>` with `whitespace-normal break-words`. Omit when `(board.debt ?? []).length === 0`. `parseGameBoard` keeps `debt` (missing → `[]`) so live GET rows reach the tab. Show Dones / Track / Show archived do not filter the strip. Graph / ADR / WorkspaceHeader omit Game debt. i18n / SpecBoardTab / WorkspaceHeader.tsx / colors.ts not edited. Last impl task.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Live GET debt but no region | `graph-ui/src/hooks/useGameBoard.ts` | 121, 160 | Constructor dropped `debt` | `parseDebtArray`. Test `useGameBoard.test.ts:249` |
| Strip missing with mocked rows | `graph-ui/src/components/GameBoardTab.tsx` | 333, 504 | Length gate / `?? []` | Region after Blocked. Test `GameBoardTab.test.tsx:2177` |
| Title ellipsis / class `truncate` | `graph-ui/src/components/GameBoardTab.tsx` | 338 | Truncate leaked onto debt `<p>` | `break-words`. Test `:2210` |
| Graph/ADR show Open tech debt | `graph-ui/src/App.test.tsx` | leftover describe | GameBoardTab stayed mounted | Unmount on tab. Test App spec-017 |
| Header shows `debt:gate-preproduction` | `graph-ui/src/components/WorkspaceHeader.tsx` | — | Strip leaked into header | Header has no debt. Test `WorkspaceHeader.test.tsx:128` |
| Click POSTs / copies / expands | `graph-ui/src/components/GameBoardTab.tsx` | 337-340 | Row became a control | Dead `<p>`. Test `:2240` |
| Show Dones off hides debt | `graph-ui/src/components/GameBoardTab.tsx` | 504 | Strip entered phase filter | Debt is chrome. Test `:2258` |
| New i18n key `gameBoard.openTechDebt` | `graph-ui/src/lib/i18n.ts` | — | Duplicate label | Reuse `specBoard.openTechDebt` |
| Specs file edited | `graph-ui/src/components/SpecBoardTab.tsx` | — | Import instead of copy | Do not edit SpecBoardTab |

### HTTP GET additive + POST leftover locks (spec-017 Task #2)

`handle_game_board_get` is still read → archive merge on cards only → `to_json`. HTTP does not fopen `backlog.md`. GET 200 always has `"debt":[...]` (empty when omit). No `has_more`. Unknown project stays 404. GET/POST leave backlog / state / registry / grill index bytes. Absent GET does not create `backlog.md`. POST `card_id` `debt:gate-preproduction` is 404 `{"error":"card not found"}`. Specs GET still reads TECH_DEBT.md only. Inbox hide leftover stays. Strip landed in Task #3 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| GET 200 missing `debt` / has `has_more` | `src/ui/game_board.c` | to_json | Task #1 key dropped | Always emit. Test `test_httpd.c:5635` |
| GET created `backlog.md` | `src/ui/http_server.c` | no fopen | HTTP wrote | Do not fopen. Test `:5854` |
| GET changed backlog / state / registry / index | `src/ui/http_server.c` | 614 | Skill write | Test `:5778` |
| Specs debt has `debt:gate-*` | `src/ui/spec_board.c` | — | Opened backlog.md | TECH_DEBT.md only. Test `:5723` |
| POST debt id is 200 | `src/ui/http_server.c` | find | Debt taught as card | Card arrays only. Test `:5920` |
| Inbox hide leftover reappeared | `src/ui/game_board.c` | fill_inbox | Hide order changed | Test `:5695` |
| Unknown project not 404 | `src/ui/http_server.c` | 604-606 | Error string changed | `{"error":"project not found"}` |

### game_board debt parse + JSON (spec-017 Task #1)

`cbm_game_board_read` still walks state / artifacts / inbox / overlay. After overlay it fopens `{root}/.gamedev/backlog.md` `"rb"` only. Open `debt:<tag>` entries (no `resolved-by` in the entry body) fill `debt[]` cap 16. `to_json` always emits `"debt":[{id,title}]`. Missing / unreadable / directory → `[]`. Never create the file. Inbox hide and Specs TECH_DEBT.md parse stay. HTTP leftover and the Game strip are later tasks.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Open comment stays `debt: []` | `src/ui/game_board.c` | 2253, 2034 | Path not regular / tag charset | `debt:` + `[A-Za-z0-9][A-Za-z0-9_-]*`. Test `game_board_debt_open_comment` |
| Title keeps director / M1 | `src/ui/game_board.c` | title helper | Cut not applied | Cut at ` — ` or ` -- ` after strip `-->` |
| Following-line `resolved-by` still listed | `src/ui/game_board.c` | 2198 | Blank line ended body | Body continues to next start / `##` / EOF |
| `design` / `tech` rows appear | `src/ui/game_board.c` | 2043 | Prefix not required | Token must be `debt:` |
| 17th open listed / `has_more` | `src/ui/game_board.c` | 2238, 2542 | Cap or overflow key | Cap 16; never `has_more` |
| `backlog.md` is a card | `src/ui/game_board.c` | 1266 | Walk widened | Keep non-card |
| Registry leftover reappeared | `src/ui/game_board.c` | fill_inbox | Hide order changed | `debt_fill` after overlay only |
| Missing GET created `backlog.md` | `src/ui/game_board.c` | 2253 | Write leaked | fopen rb only |
| Called Specs TECH_DEBT helper | `src/ui/game_board.c` | — | Wrong parser | Do not call `cbm_spec_board_parse_tech_debt` |

### InboxCard wrap + leftover Vitest (spec-016 Task #3)

InboxCard id wraps the full `card.id` (`whitespace-normal break-all`, no `truncate`). TitleControl truncate + letter E stay. ArtifactCard id truncate stays. Empty Inbox = column header, 0 cards, no “all tracked” / “no artifacts in this phase”. Show Dones off still shows a leftover Inbox card. Hide is C-owned: UI mocks already-filtered `inbox`. `useGameBoard` / i18n / types / SpecBoardTab / WorkspaceHeader / colors.ts not edited. Last impl task.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Inbox id still ellipsis / class `truncate` | `graph-ui/src/components/GameBoardTab.tsx` | 301 | Id `<p>` still has `truncate` | `whitespace-normal break-all`. Test `GameBoardTab.test.tsx:2076` |
| Inbox title wraps | `graph-ui/src/components/GameBoardTab.tsx` | 192 | TitleControl lost `truncate` | Title keeps `truncate`. Test `:2104-2105` |
| Letter E missing / word Epic | `graph-ui/src/components/GameBoardTab.tsx` | 292 | Mark became a label | Literal `"E"`. Test `:2098` |
| Artifact id wraps | `graph-ui/src/components/GameBoardTab.tsx` | 259 | ArtifactCard id lost `truncate` | Keep `truncate`. Test `:2131` |
| Empty Inbox shows “all tracked” / “no artifacts in this phase” | `graph-ui/src/components/GameBoardTab.tsx` | Inbox col | Placeholder copy leaked | Header only. Test `:2109` |
| Empty Inbox still paints a hidden leftover id | `graph-ui/src/components/GameBoardTab.test.tsx` | 2117 | Mock put the id in `inbox[]` | Pass `inbox: []`. UI does not re-hide |
| Show Dones off hides a leftover Inbox card | `graph-ui/src/components/GameBoardTab.tsx` | visiblePhaseCards | Inbox entered the phase filter | Inbox never filtered. Test `:2155` |
| Wrap required a new i18n key | `graph-ui/src/lib/i18n.ts` | — | Empty-state string added | Do not edit i18n |
| Specs EpicCard wrap drifted | `graph-ui/src/components/SpecBoardTab.tsx` | EpicCard | Specs file edited this task | Do not edit SpecBoardTab |

### HTTP GET leftover locks (spec-016 Task #2)

`handle_game_board_get` is still read → archive merge → `to_json`. HTTP does not fopen `epics_registry.md`. Hide is Task #1 omit from `inbox[]`. Unknown project stays 404. GET does not create the registry. Specs GET still lists a leftover the Game registry marks `closed`. POST inbox epic stays 404. Inbox wrap landed in Task #3 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| GET 200 still lists a hide-set leftover | `src/ui/http_server.c` | 614-615 | fill_inbox not reached | Hide is Task #1. Test `test_httpd.c:5007` |
| `has_more` / `registry` / `epics_registry` in body | `src/ui/game_board.c` | to_json | New key leaked | Never emit. Test `:5055-5056` |
| Unknown project is not 404 | `src/ui/http_server.c` | 604-606 | Error string changed | `{"error":"project not found"}`. Test `:4837` |
| GET created `epics_registry.md` | `src/ui/http_server.c` | no fopen | Write leaked | Test `:5084` |
| GET changed registry / state / index / active.json | `src/ui/http_server.c` | 614 | Skill write | Test `:5007` |
| Specs Todo omitted a closed leftover | `src/ui/spec_board.c` | — | spec_board opened the registry | Do not edit spec_board. Test `:5150` |
| POST inbox epic is 200 | `src/ui/http_server.c` | find | Inbox persistable | Stay 404. Existing `:5070` |

### game_board registry parse + hide (spec-016 Task #1)

`game_grill_fill_inbox` still walks all `.grill/plans/*/epics/` files, cap 64, no `has_more`. Hide source is gated on `game_is_regular_file("{root}/.gamedev/epics_registry.md")`. Regular file present (including empty / header-only / unreadable / 0 rows): skip Companion-to/roadmap load; omit iff Plan exact slug AND Epic integer NNN AND Status in {in_progress, closed, parked}. Missing or a directory: existing spec-011 hide. Parse is buffer-only (`cbm_game_board_parse_epics_registry`); fopen `"rb"` stays in fill. HTTP leftover locks landed in Task #2. Inbox wrap landed in Task #3 (sections above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Leftover hidden though registry exists and has no matching row | `src/ui/game_board.c` | 1948-1961, 1888-1893 | `load_conv` still ran, or present used `is_dir` | Regular file only (`:685-687`). Skip `load_conv` when present. Test `test_game_board.c:1635` |
| Header-only / empty registry still hides via roadmap | `src/ui/game_board.c` | 1950-1955 | Empty file treated as absent | Present + 0 rows. Test `:1653` |
| `in_progress` card still in inbox | `src/ui/game_board.c` | 1465-1469, 1857-1863 | Status not cells[5] or n < 6 | GFM dummy is cells[0]. Test `:1547` |
| `not_started` omitted | `src/ui/game_board.c` | 1467-1469 | Hide-set too wide | Only `in_progress`/`closed`/`parked`. Test `:1590` |
| Later `not_started` still omitted after earlier `closed` | `src/ui/game_board.c` | 1470-1472 | First row kept | Last Plan+NNN overwrites hide. Tests `:1676` / `:1695` |
| `1` or `epic-001` does not hide NNN 1 | `src/ui/game_board.c` | 1391-1417 | Prefix / leftover suffix | Optional exact `epic-` then all digits. Tests `:1715` / `:1731` |
| Plan path leftover hid the card | `src/ui/game_board.c` | 1465-1466 | Basename match | Exact slug `strcmp`. Test `:1747` |
| Plan `native` hid a grill card | `src/ui/game_board.c` | 1466 | native stored as slug | Skip that row. Test `:1833` |
| Epic 0 / evergreen hid epic-001 | `src/ui/game_board.c` | 1467 | NNN 0 stored hide | `nnn != 0` required. Test `:1799` |
| Typo `closd` omitted the card | `src/ui/game_board.c` | 1467-1469 | Prefix / case-fold | Exact tokens. Test `:1816` |
| Oversize unreadable file hid via Companion-to | `src/ui/game_board.c` | 1950-1955, 100 | Treated as absent | Present + 0 rows. Test `:1850` |
| `n/a` Epic hid epic-001 | `src/ui/game_board.c` | 1465 | Unparseable row stored | Skip; keep parked. Test `:1890` |
| Registry / epic.md bytes changed | `src/ui/game_board.c` | fopen `"rb"` | Write mode leaked | Read-only. Test `:1547` |
| 65th leftover / `has_more` when file absent | `src/ui/game_board.c` | 1878-1879 | Cap or overflow field | Cap 64; no `has_more`. Test `:1765` |
| File absent no longer hides Companion-to | `src/ui/game_board.c` | 1892-1893 | Absent branch skipped | Keep `game_grill_epic_converted`. Test `:814` |
| File absent no longer hides roadmap slug+NNN | `src/ui/game_board.c` | 1849-1854 | Roadmap matcher rewritten | Unchanged. Test `:858` |
| HTTP fopen of epics_registry.md | `src/ui/http_server.c` | — | Skill IO leaked into GET | Fill stays in `game_grill_fill_inbox`. Task #2 |

### leftover Vitest locks (spec-015 Task #4)

Strip stays Specs-only (`SpecBoardTab.tsx:374`). Header / Graph / ADR / Game never host it — leftover locks live in tests; `GameBoardTab.tsx` / `WorkspaceHeader.tsx` unchanged. GraphTab stays mocked. Debt rows are dead `<p>`: click does not POST / copy / expand. Grill-only (sdd false, grill true, debt []) still paints Specs; region omitted; notSddSkill absent. Conversion leftover mocks already-filtered `epics`. Last impl task.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Header shows TD-005 or Open tech debt | `graph-ui/src/components/WorkspaceHeader.tsx` | no debt | Strip leaked into header | Do not edit WorkspaceHeader. Test `WorkspaceHeader.test.tsx:128` |
| Graph still shows Open tech debt | `graph-ui/src/App.tsx` | 171-176 | SpecBoardTab stayed mounted | Pane XOR unmounts Specs. GraphTab mock `App.test.tsx:16-20`. Test `:1232` |
| ADR still shows Open tech debt | `graph-ui/src/App.tsx` | 173-174 | Specs pane leaked under ADR | `paneTab === "adr"` → AdrTab. Test `:1256` |
| Game shows Open tech debt | `graph-ui/src/components/GameBoardTab.tsx` | no debt region | Strip copied into Game | Do not patch GameBoardTab to pass. Test `GameBoardTab.test.tsx:2076` |
| Debt click POSTs / copies / expands | `graph-ui/src/components/SpecBoardTab.tsx` | 220-223 | Row became a control | Dead `<p>`. Test `SpecBoardTab.test.tsx:1224` |
| "Has more" with 16 debt rows | `graph-ui/src/components/SpecBoardTab.tsx` | no such control | Overflow chrome leaked | Cap omit is C. Helper `:209`. Test `:1206` |
| Converted epic id in Todo | `graph-ui/src/components/SpecBoardTab.test.tsx` | 1190 | Mock put path in `epics[]` | UI does not re-match Companion-to. Pass `epics: []` |
| Grill-only hides Specs or shows notSddSkill | `graph-ui/src/components/SpecBoardTab.tsx` | 360 | Host OR dropped | sdd false + grill true still Kanban. Tests `:1166` / `App.test.tsx:1264` |
| Function hex drifted | `graph-ui/src/lib/colors.test.ts` | 27 | Chrome token leaked into Graph | `colorForLabel("Function") === "#06b6d4"`. Tests `:1220` / `:2094` |
| GraphTab boots Three | `graph-ui/src/App.test.tsx` | 16-20 | Mock removed | Keep `data-testid="graph-tab"` |

### SpecBoardTab strip + EpicCard wrap (spec-015 Task #3)

Specs paints a chrome strip above the three columns when `(board.debt ?? []).length > 0`. Region accessible name is "Open tech debt" (`role="region"` + `aria-label`; no required `<h2>`). Each row is a `<p>`: `id` then title, `whitespace-normal break-words`, no `truncate`, no onClick. Missing `debt` is `[]` → region omitted. EpicCard title keeps `truncate`; EpicCard id uses `whitespace-normal break-all`. SpecCard id still `truncate`. Graph / ADR / Game / WorkspaceHeader do not host the strip (header/Graph/ADR/Game/click-dead leftover is Task #4). Same GET; `useSpecBoard` untouched.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Strip missing though `debt` has TD-005 | `graph-ui/src/components/SpecBoardTab.tsx` | 217, 374 | `?? []` dropped or length gate inverted | `DebtStrip rows={board.debt ?? []}`. Test `SpecBoardTab.test.tsx:1052` |
| Strip inside a column or below Todo | `graph-ui/src/components/SpecBoardTab.tsx` | 373-375 | Region nested in `Column` | Parent `flex flex-col`; strip then `flex gap-6`. Test `:1068-1072` |
| Empty / missing `debt` still shows the region | `graph-ui/src/components/SpecBoardTab.tsx` | 217, 374 | Region mounted on `[]` or undefined | `rows.length === 0` → null. Tests `:1078` / `:1089` |
| Old mock without `debt` throws | `graph-ui/src/lib/types.ts` | 149 | `debt` required | `debt?` + `?? []` (`SpecBoardTab.tsx:374`). Test `:1089` |
| Epic id still ellipsis / class `truncate` | `graph-ui/src/components/SpecBoardTab.tsx` | 109 | Id line still `truncate` | `whitespace-normal break-all`. Test `:1100` |
| Epic title wraps (name no longer short) | `graph-ui/src/components/SpecBoardTab.tsx` | 105 | Title lost `truncate` | Title keeps `truncate`. Test `:1117` |
| Spec card id wraps | `graph-ui/src/components/SpecBoardTab.tsx` | 151 | SpecCard id lost `truncate` | Keep `truncate`. Test `:1122` |
| Long debt title ellipsis | `graph-ui/src/components/SpecBoardTab.tsx` | 221 | `truncate` / `break-all` on the row | `<p>` `break-words`. Test `:1133` |
| Region name is not "Open tech debt" | `graph-ui/src/lib/i18n.ts` | 117 | Copy drifted | `openTechDebt` === `"Open tech debt"`. Test `i18n.test.ts:124` |
| Row click POSTs / copies / expands | `graph-ui/src/components/SpecBoardTab.tsx` | 220-223 | Row became a control | Dead `<p>`. Test `:1065-1066`. Full click-dead = Task #4 |
| Strip on Graph / ADR / Game / header | `graph-ui/src/components/SpecBoardTab.tsx` | 374 only | Painted outside SpecBoardTab | Do not edit WorkspaceHeader / GameBoardTab. Task #4 |
| Grill-only shows notSddSkill or throws | `graph-ui/src/components/SpecBoardTab.tsx` | 360, 374 | Host OR dropped or missing `debt` required | Same sdd OR grill; `?? []`. Tests `:377` / `:395` |

### HTTP GET additive + POST leftover locks (spec-015 Task #2)

GET `/api/spec-board` is still read → archive merge on `specs[]` only → `to_json`. Task #1 already fills `debt` in `cbm_spec_board_read`. `http_server.c` has no `fopen` of TECH_DEBT.md. GET 200 always includes `debt[]` (empty when omit). Unknown project stays 404 `project not found`. POST with an epic path as `spec_id` is leftover 404 `spec not found` — `spec_board_find` never walks `epics[]`, no `spec_archive_set`. GET leaves `TECH_DEBT.md` / `active.json` / `.grill/index.md` byte-identical. Game GET is unchanged (no `debt` key required). Specs strip UI is the section above.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| GET 200 missing `debt` | `src/ui/http_server.c` | 508-511 | Wrapper skipped `to_json` | Same dispatch: read → merge → `to_json`. Parse is Task #1 (`spec_board.c:1699-1710`). Test `test_httpd.c:4541` |
| `has_more` / 17th open id on GET | `src/ui/spec_board.c` | 1699-1710, 1477 | Overflow field or shared cap | Never emit `has_more`. Cap 16 in read. Tests `test_httpd.c:4566` / `:4579` |
| GET unknown project is not 404 | `src/ui/http_server.c` | 497-499 | Different error string | `{"error":"project not found"}`. Test `:4447` |
| POST epic id is 200 / wrote a flag | `src/ui/http_server.c` | 815-826, 910-914 | `spec_board_find` walked `epics[]` or `set` before 404 | Loop `spec_count` only. `{"error":"spec not found"}`. Test `:4388` (`:4428-4429`) |
| Archive merge stamped `debt[]` | `src/ui/http_server.c` | 465-474 | Merge walked `debt` | Specs only. Test `:4568-4569` |
| GET/POST changed `TECH_DEBT.md` / `active.json` / `index.md` | `src/ui/http_server.c` | no TECH_DEBT `fopen` | HTTP opened skill trees | Debt IO stays in `spec_board.c` `"rb"`. Test `:4468` (`:4519`) |
| HTTP `fopen` of TECH_DEBT.md | `src/ui/http_server.c` | 488-519 | Sibling reader leaked into HTTP | No skill fopen here. SDD-ADR-066. Test `:4468` |
| Game GET required `debt` | `src/ui/http_server.c` | 2540-2542 | Cross-board leak | `/api/game-board` unchanged. Existing game tests stay green |

### spec_board debt parse + additive JSON (spec-015 Task #1)

`cbm_spec_board_read` still fills sdd specs and grill epics, then `debt_fill` after grill: join `{root}/.sdd-skill/baseline/TECH_DEBT.md`, `read_whole_file` (`"rb"`). JSON always emits `debt[]` (empty when missing, unreadable, or all resolved). Heading `Status:` (trimmed line-start or a `|` cell start) wins; else `## Debt Summary` GFM table row for that `TD-NNN`. Title is the heading-line remainder after `## TD-NNN:`. Open iff `strcmp` ≠ `"resolved"`. Cap 16 open in heading order; no `has_more`. Conversion matcher untouched. HTTP GET/POST Gherkin is Task #2 (section above). Strip UI is spec-015 Task #3 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Open heading present but `debt` is `[]` | `src/ui/spec_board.c` | 1493-1507, 1471-1476 | fopen failed or token is exact `resolved` | `debt_fill` then parse. `Status:` line-start or pipe cell (`:1113-1159`). Test `test_spec_board.c:1532` |
| Identified heading hidden by table `resolved` | `src/ui/spec_board.c` | 1162-1180, 1471 | Table used first | Heading token wins. Test `:1622` |
| No heading Status and table `in_progress` omitted | `src/ui/spec_board.c` | 1471-1475, 1353-1413 | `## Debt Summary` / `ID`+`Status` headers missed | Table is fallback only. Test `:1649` |
| Typo `resolvd` omitted | `src/ui/spec_board.c` | 1477 | Case-fold or prefix match | `strcmp` exact `"resolved"`. Test `:1677` |
| 17th open in JSON / `has_more` key | `src/ui/spec_board.c` | 1477, 1699-1710 | Shared cap or overflow field | Own cap 16; no `has_more`. Test `:1696` |
| Heading without `TD-NNN` became a row | `src/ui/spec_board.c` | 1183-1231 | Any `## ` accepted | `## ` + `TD-` + digits. Test `:1756` |
| Title from `Title:` field or table cell | `src/ui/spec_board.c` | 1217-1230 | Wrong title source | Remainder after `## TD-NNN:`. Test `:1532` |
| Missing / dir-at-path file 500 or leftover rows | `src/ui/spec_board.c` | 1502-1504, 46-48 | Unreadable treated as fatal | NULL → count 0. Tests `:1598` / `:1731` |
| `TECH_DEBT.md` / `active.json` / epic.md bytes changed | `src/ui/spec_board.c` | 36-37 | Write mode leaked | `cbm_fopen` `"rb"` only. Test `:1793` |
| Converted epic back in `epics[]` | `src/ui/spec_board.c` | 738-756, 322-344 | Matcher rewritten | Do not touch `grill_epic_converted`. Test `:1041` |
| `has_more` / `severity` / `category` / `status` on JSON | `src/ui/spec_board.c` | 1699-1708 | Extra keys in emit | `{id,title}` only. Test `:1557-1561` |
| HTTP opened TECH_DEBT.md | `src/ui/http_server.c` | — | Skill IO leaked into GET | Fill stays in `cbm_spec_board_read`. See spec-015 Task #2. Test `test_httpd.c:4468` |

### Game visibility filters — Track A/B/All, AND leftover, Specs lock, no-persist (spec-014 Task #2)

Same `visiblePhaseCards` as Task #1. Track A keeps `track==="A"` only; Track B keeps `"B"`; All keeps A, B, H, other, null — then dones/archived. Archived done paints iff Show Dones AND Show archived. Specs mount has no Show Dones / Track A / All. Filter click: no GET/POST, no `track` / `show_dones` / `show_archived` query, no localStorage key containing `showDones` or `gameTrack`. Inbox unfiltered. Empty filtered phase = header only.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Track A still shows B or H | `graph-ui/src/components/GameBoardTab.tsx` | 72, 466-467 | Filter not exclusive | `setTrackFilter("A")`. Drop `card.track !== "A"`. Test `GameBoardTab.test.tsx:1773` |
| Track B hides H; All does not restore it | `graph-ui/src/components/GameBoardTab.tsx` | 73, 474-486 | All treated as A/B only | `track === "all"` keeps H. Test `:1874` |
| Track A + Show Dones paints a Track B done | `graph-ui/src/components/GameBoardTab.tsx` | 72-75 | Track applied after keep-done | Track first. Test `:2019` |
| Show archived alone reveals an archived done | `graph-ui/src/components/GameBoardTab.tsx` | 74-76 | AND missed | Drop done+archived when `!showArchived`. Tests `:1992` / invert `:1339` |
| Both toggles on still hide the archived done | `graph-ui/src/components/GameBoardTab.tsx` | 74-78 | Extra drop | Keep when both pressed. Test `:1815` |
| After Unarchive the card vanishes with Show archived off | `graph-ui/src/components/GameBoardTab.test.tsx` | 1383 | Flag still true or Show Dones off | POST `archived: false`; stays while Show Dones on |
| Project change keeps Track / Show Dones | `graph-ui/src/components/GameBoardTab.tsx` | 398-403 | Reset skipped or localStorage | Reset three + expand. Tests `:1901` / `:1559` |
| GET refetch resets Track / Show Dones | `graph-ui/src/components/GameBoardTab.tsx` | 398-403 | New `board` treated as remount | Reset only on `project`. Test `:1901` |
| Specs paints Show Dones / Track A / All | `graph-ui/src/components/SpecBoardTab.tsx` | 245-256 | Game chrome copied | Show archived on Done only. Test `SpecBoardTab.test.tsx:1019` |
| Filter click GET/POST or `?track=` / `show_dones` | `graph-ui/src/components/GameBoardTab.test.tsx` | 127-145, 2045 | onClick fetched or wrote URL | Chrome `setState` only. No query params |
| localStorage has `showDones` or `gameTrack` | `graph-ui/src/components/GameBoardTab.test.tsx` | 148-156, 2045 | Pref written | React state only. SDD-ADR-062 |
| Inbox gone under Track / both toggles | `graph-ui/src/components/GameBoardTab.tsx` | 498-499 | Inbox ran through `visiblePhaseCards` | `board.inbox`. Test `:1843` |
| All-done phase lost header / "No specs yet" | `graph-ui/src/components/GameBoardTab.tsx` | 352-378 | Empty-state copy leaked | Header only; Show Dones stays chrome. Test `:1931` |

### Game visibility filters — Show Dones / first-paint hidden dones (spec-014 Task #1)

Same `GameBoardTab` pane chrome as Show archived. Cluster: Show archived, `|`, Show Dones, `|`, Track A, Track B, All. All five are `aria-pressed` buttons. First paint: Show Dones false, Show archived false, All true. `visiblePhaseCards` hides `work_state==="done"` unless Show Dones; archived done also needs Show archived (AND). Inbox uses `board.inbox` unfiltered. `lastProjectRef` reset clears Show Dones + Track + Show archived + expand. GET refetch does not reset. No `gameBoard.showArchived`. Track click / leftover AND / Specs lock / no-persist Gherkin landed in Task #2 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Done artifacts paint on first Game visit | `graph-ui/src/components/GameBoardTab.tsx` | 394, 65-79 | `showDones` defaulted on, or predicate still spec-012 | `useState(false)`. Drop done when `!showDones`. Test `GameBoardTab.test.tsx:1601` |
| SYS-003-done / Done label missing on first paint | `graph-ui/src/components/GameBoardTab.test.tsx` | 836, 861-863 | Test still expects unarchived done without the toggle | Click Show Dones first |
| Archive happy path finds no card | `graph-ui/src/components/GameBoardTab.test.tsx` | 1299, 1319 | Unarchived done is hidden | Press Show Dones, then expand + Archive |
| Archived done visible with Show Dones only | `graph-ui/src/components/GameBoardTab.tsx` | 74-76 | AND missed | Drop done+archived when `!showArchived`. Test `:1746` |
| All not pressed / two Track buttons pressed | `graph-ui/src/components/GameBoardTab.tsx` | 395, 464-487 | Radiogroup or toggle | Default `"all"`; exclusive onClick. Test `:1601` |
| Remount / project change keeps Show Dones or Track A | `graph-ui/src/components/GameBoardTab.tsx` | 398-403 | State leaked or localStorage | Reset with expand + Show archived. Tests `:1715` / `:1901` |
| GET refetch resets the three controls | `graph-ui/src/components/GameBoardTab.tsx` | 398-403 | New `board` treated as remount | Reset only on `project` change. Test `:1901` |
| Inbox disappeared under Show Dones / Track | `graph-ui/src/components/GameBoardTab.tsx` | 498-499 | Inbox ran through `visiblePhaseCards` | `board.inbox` unfiltered. Test `:1843` |
| Show Dones / Track in Inbox or a fifth column | `graph-ui/src/components/GameBoardTab.tsx` | 445-488 | Controls inside `PhaseColumn` | Chrome row under phase/focus/continue. Test `:1964` |
| Show Dones EN / Track labels drifted | `graph-ui/src/lib/i18n.ts` | 130-133, 252-255 | Copy edited or `gameBoard.showArchived` added | EN locked. Test `i18n.test.ts:111-122` |

### ADR tab no cycle stamp (spec-013 Task #3)

Isolated AdrTab. Stamp + replace warning stay spec-004. Document must not contain `gamedev-skill`. Test-only; `AdrTab.tsx` / `i18n.ts` not edited.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| `gamedev-skill` in AdrTab chrome | `graph-ui/src/components/AdrTab.test.tsx` | 206-207 | Cycle stamp added to AdrTab or i18n | Keep generic `adr.replaceWarning`. Do not mount GameBoardTab |
| Stamp/warning missing with markers | `graph-ui/src/components/AdrTab.tsx` | — | List cache miss or gate changed | `formatIndexedAt(indexed_at)`; existing test `:187` |

### ADR fill HTTP/MCP gamedev XOR (spec-013 Task #2)

Same persist path as spec-004. `.gamedev/` dir → gamedev trio in GET `/api/adr` after create/Reindex/`index_repository`. Watcher is `adr_fill: false`. Unreadable HTTP fixture is directory-at-path (chmod 0 fails the job because `.gamedev` is not ALWAYS_SKIP). spec-004 sdd HTTP trees must not grow a `.gamedev/` dir.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Dual-tree GET has leftover sdd strings | `tests/test_httpd.c` | 3262 | XOR lost or mixed fixture | `ui_adr_fill_gamedev_tree` `:3200` + leftover sdd `:3219` |
| Create unmarked with `.gamedev/` | `tests/test_httpd.c` | 3374 | Bare POST skipped fill | Same executor; empty/whitespace MANUAL |
| Add `.gamedev/` kept `PURPOSE-SDD-MVP1` | `tests/test_httpd.c` | 3433 | Generated not overwritten | One Reindex; `# Keep notes` stays |
| Empty `.gamedev/` filled leftover sdd | `tests/test_httpd.c` | 3532 | Empty dir treated as missing | Markers; no `PURPOSE-SDD-EMPTYDIR` |
| Remove `.gamedev/` still gamedev extract | `tests/test_httpd.c` | 3572 | Dir left or fill skipped | Next job `PURPOSE-SDD-RESTORED` |
| Both dirs gone invented markers | `tests/test_httpd.c` | 3617 | NULL path not taken | `# Last gamedev generated leftover` |
| Unreadable Purpose copied / job failed | `tests/test_httpd.c` | 3661 | chmod 0 hashed by discover | Directory-at-path `:3684-3689`; job success |
| `PURPOSE-HAND-EDIT-GAME` after Reindex | `tests/test_httpd.c` | 3716 | Fill skipped | GET has `PURPOSE-CANONICAL-GAME` |
| MCP get misses `PURPOSE-MCP-GAME` | `tests/test_mcp.c` | 6479 | Fill off or sdd trio | `index_repository` default fill; store = GET |
| `# New notes` gone after next index | `tests/test_mcp.c` | 6529 | MANUAL span dropped | Whole-doc update; splice keeps MANUAL |
| `adr_fill: false` wrote gamedev markers | `tests/test_mcp.c` | 6599 | Gate ignored bool false | Blob `# Before watch` |

### ADR fill XOR trio (spec-013 Task #1)

`cbm_adr_fill_document` picks one trio. `.gamedev/` as a directory → gamedev relatives only (never fopen leftover sdd trio paths). Else `.sdd-skill/` as a directory → existing sdd trio. Neither directory → NULL (blob unchanged). Empty `.gamedev/` still writes markers with no extracts. File-at-path `.gamedev` is not present. Extract window, H1s, splice, markers, and `adr_fill` flag stay spec-004. Pipeline / ALWAYS_SKIP / AdrTab not this task.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Leftover `PURPOSE-SDD-*` on a dual-tree | `src/adr/adr_fill.c` | 72-76, 170-194 | Select mixed relatives | `adr_select_trio` sets only `CBM_ADR_GAME_REL_*`. Test `adr_fill_gamedev_dual_tree_xor` |
| NULL on empty `.gamedev/` dir | `src/adr/adr_fill.c` | 72-76, 274-276 | Empty dir treated as missing | `cbm_is_dir` (`:57-64`); present always splices. Test `adr_fill_gamedev_empty_dir_no_sdd_fallback` |
| File named `.gamedev` skipped sdd | `src/adr/adr_fill.c` | 72, 78-82 | File-at-path counted present | `cbm_is_dir` only; then sdd. Test `adr_fill_file_at_path_gamedev_uses_sdd` |
| Neither dir but markers appeared | `src/adr/adr_fill.c` | 274-276 | Select returned true | NULL; no comments. Test `adr_fill_neither_skill_dir_null` |
| Missing gamedev file filled from sdd | `src/adr/adr_fill.c` | 170-194 | Fallback join on sdd paths | One trio; omit that H1 (`:156-157`). Test `adr_fill_gamedev_only_tech_stack` |
| Unreadable `game_context.md` still copied | `src/adr/adr_fill.c` | 97-102 | Directory-at-path opened as a file | Not regular / `fopen` fail → omit `# Purpose`. Test `adr_fill_gamedev_unreadable_game_context` |
| GDD / DEV_LOG in generated | `src/adr/adr_fill.c` | 24-26 | Extra relative opened | Only `game_context` + gamedev `TECH_STACK` + `ARCHITECTURE_ADR` |
| Text past 1536 still in generated | `src/adr/adr_fill.c` | 122-129 | Cap not applied | Same 1536 window. Test `adr_fill_gamedev_extract_cap_drops_tail` |
| Unmarked notes vanished | `src/adr/adr_fill.c` | 207-211 | Body already had both MANUAL markers | Keep MANUAL span only (`:197-223`) |
| NULL with a real skill dir and trio | `src/adr/adr_fill.c` | 242-244 | `malloc` in `adr_splice` failed | NULL is OOM or neither dir |

### Game archive UI + refresh (spec-012 Task #5)

Same `GameBoardTab` pane. Show archived lives in pane chrome (`aria-pressed` false on first paint / project change / remount; GET refetch does not reset; no localStorage). spec-014 Task #1 superseded first-paint done hide: unarchived dones are hidden unless Show Dones; archived done needs both toggles (section above). Leftover archived on pending stays; no Archive/Unarchive. Archive only expanded done && !archived; Unarchive only expanded done && archived. POST `/api/game-board` `{project, card_id, archived}` then `await refresh()`. `refresh` is the same GET and must not set `settled`/`present` false. No confirm. No fourth column. App passes `project`+`refresh`. Title expand + Blocked region stay Task #4 (section below).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Archived done visible on first paint | `graph-ui/src/components/GameBoardTab.tsx` | 60-62, 375, 441 | Filter missed `done`, or toggle defaulted on | `visiblePhaseCards`. `useState(false)`. Test `GameBoardTab.test.tsx:1272` |
| Show archived pressed after remount / project change | `graph-ui/src/components/GameBoardTab.tsx` | 378-382 | State leaked or localStorage | Reset with `expandedIds`. Tests `:1272` / `:1479` |
| GET refetch resets the toggle | `graph-ui/src/components/GameBoardTab.tsx` / `useGameBoard.ts` | 378-382 / 160-176 | New `board` treated as remount | Reset only on `project` change. Test `:1479` |
| Game tab vanishes after Archive | `graph-ui/src/hooks/useGameBoard.ts` | 160-176, 174 | `refresh` unset `settled`/`present` | Keep both true. Tests `useGameBoard.test.ts:270` / `:309` |
| Archive opened a confirm | `graph-ui/src/components/GameBoardTab.tsx` | 393-406, 188-212 | `window.confirm` or dialog | No confirm. Test `:1233` |
| Archive on Inbox / pending / blocked | `graph-ui/src/components/GameBoardTab.tsx` | 64-67, 341-348 | Eligibility not done-artifact | InboxCard has no persist. Tests `:1357` / `:1419` / `:1445` |
| After POST 200 the card comes back (hide on) | `graph-ui/src/components/GameBoardTab.tsx` | 401-402 | `refresh` not awaited | `if (!res.ok) return; await refresh()`. Test `:1233` |
| Unarchive while hide still hides | `graph-ui/src/components/GameBoardTab.tsx` | 60-62 | Stale `archived` true | Refresh sets false. Test `:1309` |
| Inbox archived epic disappeared | `graph-ui/src/components/GameBoardTab.tsx` | 441 | Inbox ran through phase filter | `board.inbox` unfiltered. Test `:1419` |
| All-archived column lost header / fifth column | `graph-ui/src/components/GameBoardTab.tsx` | 435-449, 423-432 | Extra column or toggle in Inbox | Four headers; chrome toggle. Test `:1385` |
| POST went to spec-board | `graph-ui/src/components/GameBoardTab.tsx` | 396-399 | Copied spec persist | `{project, card_id, archived}`. Test `:1264-1269` |
| Empty `project` on POST | `graph-ui/src/App.tsx` | 170 | Pane not given `selectedProject` | Pass project+refresh. Test `App.test.tsx:1127` |
| `/api/skill-presence` / Enter opens Game / leftover specs stays Specs | `graph-ui/src/App.tsx` / `App.test.tsx` | 36 / 1164 | Silent-win weakened | Stay GET game-board; Enter Graph; specs+gamedev → game. Tests `:1164` / `:1185` / `:1199` |
| Function hex drifted | `graph-ui/src/lib/colors.ts` | — | Do not edit | `colorForLabel("Function") === "#06b6d4"`. Test `GameBoardTab.test.tsx:1512` |

### GameBoardTab expand + blocked strip + i18n (spec-012 Task #4)

Same `GameBoardTab` pane. Title is a `<button aria-expanded>` (Set keyed by GET `id`; all collapsed; multi-open; project/remount clear). Track A: blurb + `#N name` or `specBoard.noTasksYet` + Inputs only when `inputs` non-empty. Track B/H: `last_decision` / `open` / `recent`. Inbox: summary/plan stay chrome; expand adds nothing; no Archive. Blocked region above columns when `blocked.length > 0`; overlay prefix `Blocked` + `blocked_by` collapsed and expanded. `useGameBoard` is still one-shot plus Task #5 `refresh()`. Archive POST / Show archived / App `project`+`refresh` landed in Task #5 (section above). spec-011 title-never-expands is inverted; body-click still does not expand.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Title click does not expand (title still `<p>`) | `graph-ui/src/components/GameBoardTab.tsx` | 148-167, 184, 213 | TitleControl dropped; title is a plain `<p>` | Wrap title in `<button aria-expanded>`. Test `GameBoardTab.test.tsx:822` |
| Strip missing though GET has blocked rows | `graph-ui/src/hooks/useGameBoard.ts` / `GameBoardTab.tsx` | 73-81, 137 / 236, 338 | `blocked` not parsed, or region mounted only with a heading | Missing/non-array → `[]` omits region; non-empty `role=region` name Blocked. Tests `useGameBoard.test.ts:191`, `GameBoardTab.test.tsx:869` / `:1047` |
| Inputs heading on Track B | `graph-ui/src/components/GameBoardTab.tsx` | 107-131 vs 133-145 | Track A expand body used for B | Inputs only when `track === "A"`. Test `:836-865` |
| Body click expands / POSTs | `graph-ui/src/components/GameBoardTab.tsx` | 183, 158-161 | Toggle on `<article>` | Only title button toggles. Test `:608-638` |
| Continue toggles expand | `graph-ui/src/components/GameBoardTab.tsx` | 80-82 | `stopPropagation` dropped | Nested continue button. Test `:1002-1044` |
| Overlay text only when expanded | `graph-ui/src/components/GameBoardTab.tsx` | 90-103, 191 | Prefix inside expand body | `BlockedByLine` always. Test `:914-917` |
| Empty `blocked` still shows Blocked | `graph-ui/src/components/GameBoardTab.tsx` | 236 | Region mounted on `[]` | `rows.length === 0` → null. Test `:1047-1066` |
| Missing `archived` paints true | `graph-ui/src/hooks/useGameBoard.ts` | 105, 137 | Default not `=== true` | Missing → false; missing `blocked` → `[]`. Test `useGameBoard.test.ts:184-188` |

### HTTP POST + GET merge + publish copy (spec-012 Task #3)

GET `/api/game-board` merges `game_archive` onto matching ids after `cbm_game_board_read`. POST is the same path: flag object 200; 409 `card not done` (pending or overlay blocked) writes nothing; Inbox epic 404 `card not found`; mutation lock 423 like ADR. `publish_staged` copies flags live→stage after the ADR write. UI hide/show landed in Task #5 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| GET archived always false | `src/ui/http_server.c` | 549-583, 615 | Merge skipped | Missing table → all false 200. Test `test_httpd.c:4366` |
| Orphan row invented a card | `src/ui/http_server.c` | 531-546 | Append on unmatched id | Matching ids only. Test `:4366` |
| Inbox POST 200 | `src/ui/http_server.c` | 669-671 | kind epic accepted | Find returns NULL. Test `:4338` |
| 409 still persisted | `src/ui/http_server.c` | 763-767 | set before done check | 409 before lock. Tests `:4392` / `:4440` |
| `archived:"yes"` not 400 | `src/ui/http_server.c` | 721-724 | Non-bool accepted | `invalid archived`. Test `:4520` |
| POST spec-board archived a Game id | `src/ui/http_server.c` | spec_board_find | Wrong table | 404 spec not found. Test `:4572` |
| Flags vanish after Reindex dump | `src/pipeline/pipeline.c` | 1802-1811 | Copy skipped | Same live-open as spec_archive. Test `:4651` |
| POST wrote skill trees | `src/ui/http_server.c` | 756 | fopen write | SQLite only. Test `:4392` |

### C expand parse + blocked overlay (spec-012 Task #2)

Same GET `/api/game-board`. Additive card keys + top-level `blocked[]`. Overlay runs in C after artifact fill. `archived` stays false until Task #3. fopen `"rb"` only. UI expand is Task #4. POST is Task #3.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Blurb is header `open` though What it does exists | `src/ui/game_board.c` | 419-421, 967 | H2 miss / fallback first | Non-empty `## What it does` wins. Test `test_game_board.c:1190` |
| Track A tasks include Subagent/Path/`<!--` | `src/ui/game_board.c` | 898-924 | Skip list dropped | Checkbox `- [ ]`/`[x]` only. Test `:1112` |
| 49th task or `has_more` | `src/ui/game_board.c` | 924, 2016 | Cap 48 not applied | Omit overflow. Test `:1282` |
| playtest `recent` is an earlier Round | `src/ui/game_board.c` | 423 | First `## Round ` kept | Last block only. Test `:1209` |
| level `recent` includes `<!--` | `src/ui/game_board.c` | 485 | Comment lines kept | Last 8 non-empty non-comment. Test `:1243` |
| `needs_review` is a strip row | `src/ui/game_board.c` | 580, 663-677 | Any status parsed | `:blocked:` only. Test `:1324` |
| empty blocked omitted from JSON | `src/ui/game_board.c` | 2079 | Key skipped | Always `"blocked":[]`. Test `:1347` |
| overlay misses SYS / Inbox overlaid | `src/ui/game_board.c` | 1014-1040 | Owner without `@`; inbox walked | `@`+slug; phase arrays only. Test `:1367` |
| GET wrote spec.md / tasks.md / state.md | `src/ui/game_board.c` | 1895 | Write mode | `"rb"` only. Test `:1408` |

### Store game_archive set/load/copy (spec-012 Task #1)

Flags live in the project `.db` table `game_archive`, not in `.gamedev/` and not in `spec_archive`. Write-open creates the table. Query-open skips `init_schema` — load must probe `sqlite_master` and treat a missing table as zero rows. Unarchive UPSERTs `archived=0`; it does not DELETE. HTTP merge / POST / publish copy is Task #3. UI expand is Task #4; archive UI landed in Task #5 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Legacy `.db` load 500 | `src/store/store.c` | 8331-8349, 8408-8414 | SELECT without probe | `SQLITE_DONE` → OK, count 0. Test `test_store_game_archive.c:177` |
| Empty/NULL card_id wrote a row | `src/store/store.c` | 8361-8364 | Guard skipped | Both → `CBM_STORE_ERR`. Test `:82` |
| 256-byte card_id stored | `src/store/store.c` | 8365-8368 | Length check skipped | `strlen >= 256` → ERR. Test `:93` |
| Unarchive deleted the row | `src/store/store.c` | 8369, 8372-8375 | DELETE on 0 | UPSERT 0. Test `:115` |
| Repeat archive = two rows | `src/store/store.c` | 8372-8375 | No `ON CONFLICT` | PK UPSERT. Test `:136` |
| Orphan id dropped on load | `src/store/store.c` | 8419-8448 | Store filtered by board | No board in store. Test `:157` |
| Copy failed, src table missing | `src/store/store.c` | 8461-8464 | Missing treated as ERR | load OK count 0 → no-op. Test `:219` |
| Table missing after write-open | `src/store/store.c` | 334-343 | DDL omitted | `CREATE TABLE IF NOT EXISTS game_archive` in `init_schema` |
| Flags in `spec_archive` / ADR / `store_meta` | `src/store/store.h` | 806-815 | Wrong table reused | New APIs only. Writer DDL unchanged |

### Remaining Vitest Gherkin (spec-011 Task #5)

Test-only leftover Thens. Pane: 01/03 `aria-current`, Done/Blocked, sit-beside Inbox+SYS, chrome continue stays `<p>`, no dim. App: filled GET paints cards; Inbox from game-board not spec-board `epics`; afterEach + dedicated + filled fetch log never `/api/skill-presence`; game-board/spec-board stay GET. Silent-win / Enter Graph / leftover `tab=specs`→game stay on a filled board. spec-009 grill-only / neither / spec-board 500 stay green on default game-board 200 false. Function `#06b6d4` still locked.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| 01/03 phase missing `aria-current` | `graph-ui/src/components/GameBoardTab.tsx` | 34-39, 188 | Map only handled `02-production` | `01` → Pre-production; `03` → Post-production & Launch. Inbox never. Test `GameBoardTab.test.tsx:642-657` |
| Done / Blocked not painted | `graph-ui/src/components/GameBoardTab.tsx` / `i18n.ts` | 47-48 | `workStateLabel` dropped those tokens | `workStateDone` / `workStateBlocked`. Tests `:659-685`, `i18n.test.ts:101-102` |
| Non-current columns dimmed or hidden | `graph-ui/src/components/GameBoardTab.tsx` | 138, 181-192 | `opacity` / `hidden` on `!current` | Header stays visible. Test `:687-706` |
| Sit-beside empty (Inbox or SYS missing) | `graph-ui/src/components/GameBoardTab.tsx` | 187 | inbox + production arrays unread | Both paint. Test `:708-738` |
| Chrome continue became a button | `graph-ui/src/components/GameBoardTab.tsx` | 177-179 | Pane `<p>` replaced | Chrome stays text. Card button is Task #4. Test `:741-765` |
| `/api/skill-presence` in App fetch | `graph-ui/src/App.tsx` / `useGameBoard.ts` / `App.test.tsx` | fetch / 104 / 315 | App or hook called the unused GET | Presence is GET `/api/game-board` only. afterEach `:327` / `:740` / `:829` / `:1082`. Dedicated `:1046-1078`. Filled `:1145-1164` |
| Game Inbox shows a spec-board epic | `graph-ui/src/App.test.tsx` / `App.tsx` | 1115 / showGame | Inbox read `epics[]` | Game pane uses game-board `inbox` only. Test `:1115-1143` |
| POST game-board or spec-board from Game | `graph-ui/src/App.test.tsx` | 138-149 | Mutate on activate | Boards stay GET. Tests `:1112`, `:1142`, `:1162` |
| Enter opens Game on a filled board | `graph-ui/src/App.tsx` / `App.test.tsx` | navigate graph / 1166 | Default tab changed | Enter stays Graph. Test `:1166-1178` |
| `tab=specs` + filled gamedev stays Specs | `graph-ui/src/lib/route.ts` / `App.test.tsx` | resolveWorkspaceTab / 1180 | Silent-win skipped | Leftover specs + gamedev → `tab=game`. Test `:1180-1196` |
| spec-009 grill-only hides Specs or shows Game | `graph-ui/src/App.test.tsx` / `App.tsx` | 207, 748 | Default mock present true | Keep `gameBoard ?? "false"`. Tests `:748` / `:792` / `:546` |
| Function hex drifted | `graph-ui/src/lib/colors.test.ts` | 27 | `LABEL_COLORS` or CSS vars leaked | `colorForLabel("Function") === "#06b6d4"` |

### Clipboard, no-drag, typed card parse (spec-011 Task #4)

Same `GameBoardTab` pane. Chrome continue stays a `<p>` (spec-010). Card continue is a button: artifact copies `/gamedev-skill continue @role`; Inbox copies `/gamedev-skill continue`. Denied or missing clipboard → `selectNodeContents` on that button. No toast either path. Title expand is spec-012 Task #4; **body** click (not title) still does not expand / Archive / POST. `draggable={false}`; drop does not move. `useGameBoard` stays one-shot. `parseGameBoard` skips objects without `kind` `artifact`|`epic`.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Artifact continue does not copy `@role` | `graph-ui/src/components/GameBoardTab.tsx` | 73-86, 57-63 | Chrome `<p>` used as the control, or `writeText` skipped | Card button name is the continue string. Test `GameBoardTab.test.tsx:459-485` |
| Inbox copies `@role` | `graph-ui/src/components/GameBoardTab.tsx` | 119 | Artifact continue painted on epic | Inbox string is `/gamedev-skill continue`. Test `:491-518` |
| Copy fail / denied shows a toast | `graph-ui/src/components/GameBoardTab.tsx` | 64-70 | Success/fail painted `role="status"` or "Copied" | Catch → `selectNodeContents` only. Test `:521-549` |
| Missing clipboard API does nothing | `graph-ui/src/components/GameBoardTab.tsx` | 59-61, 64-70 | Missing `writeText` not thrown into fallback | Treat missing as fail. Test `:551-580` |
| Chrome continue became a button | `graph-ui/src/components/GameBoardTab.tsx` | 177-179 | Pane `<p>` replaced | Chrome stays text. Empty-board tests `:153`, `:183` |
| Card **body** click shows Archive / Unarchive / "No tasks planned yet" | `graph-ui/src/components/GameBoardTab.tsx` | 183, 608 | Toggle on article, or Archive leaked | Body click stays collapsed. Title expand is spec-012 Task #4. Test `:608-638` |
| Drop moves a card to another column | `graph-ui/src/components/GameBoardTab.tsx` | 92, 107 | Local reorder or `draggable` true | `draggable={false}`; no drop handler. Test `:612-640` |
| Junk objects paint (kind `spec` / missing kind) | `graph-ui/src/hooks/useGameBoard.ts` | 36, 50-57 | Cast-through parse | Skip unless `artifact`\|`epic`. Tests `useGameBoard.test.ts:132-182`, `:184-205` |
| 4s game-board poll / `/api/skill-presence` | `graph-ui/src/hooks/useGameBoard.ts` | 89-130 | `setInterval` or leftover path | One-shot. Tests `:116-130`, `:203-204` |

### GameBoardTab four columns + cards + i18n (spec-011 Task #3)

Same `GameBoardTab` pane (not SpecBoardTab). spec-010 chrome stays (phase label, focus, continue as text). Four headers always: Inbox | Pre-production | Production | Post-production & Launch. Empty = header only; no placeholder; not dimmed. `aria-current="true"` on the matching phase column only; Inbox never; phase null → none. Artifact: A/B/H + work-state EN; no E. Inbox: letter E `--color-epic-mark`; no track; no Pending. Clipboard / drag / kind-filter: Task #4 section above.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Columns missing / only chrome | `graph-ui/src/components/GameBoardTab.tsx` | 181-192, 18, 27-32 | Grid dropped or `max-w-2xl` hid columns | Always map `COLUMN_KEYS`. Tests `GameBoardTab.test.tsx:213-237`, `App.test.tsx:1033-1036` |
| Production missing `aria-current` | `graph-ui/src/components/GameBoardTab.tsx` | 34-39, 138, 188 | Inbox leaked into the map, or token not matched | Inbox always false. `02-production` → Production. Test `:232-235` |
| Inbox has `aria-current` / phase null still highlights | `graph-ui/src/components/GameBoardTab.tsx` | 34-35 | Null/Inbox gate dropped | Inbox never; `phase === null` → none. Tests `:233`, `:374-393` |
| Inbox E missing / gray / word Epic | `graph-ui/src/components/GameBoardTab.tsx` / `globals.css` | 109 | EpicCard imported or raw English | Literal `"E"` + `text-[var(--color-epic-mark)]`. Test `:346` |
| Work-state not EN (Pending / In progress / Done / Blocked) | `graph-ui/src/components/GameBoardTab.tsx` / `i18n.ts` | 41-50 / 124-127 | Raw JSON token or key drifted | `workStateLabel` → `t.gameBoard.workState*`. Tests `:263`, `:322`, `i18n.test.ts:99-102` |
| Empty column placeholder / dimmed / hidden | `graph-ui/src/components/GameBoardTab.tsx` | 142-154, 181-192 | Placeholder copy or `opacity`/`hidden` | Header + 0 articles. No "no artifacts in this phase". Test `:356-372` |

### C Inbox walk + conversion + HTTP bytes (spec-011 Task #2)

Same GET `/api/game-board`. present true + `.grill/` dir fills `inbox` with unconverted epics (index.md then slug-asc; epic-NNN). Omit Companion-to exact path or slug token + roadmap table NNN. No kebab. No `active.json`. Plan-folder cite without a cell does not convert. Cap 64 independent of phase arrays. fopen `"rb"` only; GET leaves skill trees byte-identical. UI Inbox paint is spec-011 Task #3 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Inbox empty with leftover grill | `src/ui/game_board.c` | 1178-1180, 1132-1133 | `.grill` missing, or conversion matched | Dir required. Omit only exact Companion-to or slug+NNN. Tests `test_game_board.c:882` / `:714` |
| Companion-to exact still in Inbox | `src/ui/game_board.c` | 966-999, 1098-1101 | Token parse skipped | First `.grill/plans/`…`.md`; notes after `.md` ignored; never fopen the token. Tests `:757` / `test_httpd.c:4070` |
| slug+NNN still in Inbox | `src/ui/game_board.c` | 902-919, 922-963, 1103-1108 | Token bounds or header row as cell | `[A-Za-z0-9-]`; GFM data cells `001` / `epic-001`. Test `:801` |
| kebab SYS name hid the epic | `src/ui/game_board.c` | 1090-1108 | Fuzzy name match | No kebab. Test `:956` |
| plan-folder cite without cell hid Inbox | `src/ui/game_board.c` | 1103-1108 | Slug-only treated as convert | Slug token and table NNN. Test `:931` |
| GET wrote `.grill/` / `.gamedev/` or created `.sdd-skill/` | `src/ui/game_board.c` | 90 | Write mode or mkdir | `"rb"` only. Tests `test_game_board.c:1008` / `test_httpd.c:4108` |

### C artifact walk + widen card JSON (spec-011 Task #1)

Same GET `/api/game-board`. Chrome / 400 / 404 / spec-board gamedev-free unchanged. present true + existing files fill `preproduction` / `production` / `postproduction`. Inbox fill is Task #2 (section above). Owner/track from compiled table, not `agents.md`. `ready` → `in_progress`. Cap 64 after production sort. No `has_more` / `column`. fopen `"rb"` only. UI columns are spec-011 Task #3 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| gdd owner follows `agents.md` | `src/ui/game_board.c` | 50-63, 254-264 | Project agents.md parsed | Compiled table. Test `test_game_board.c:365-378` |
| missing narrative-bible still a card | `src/ui/game_board.c` | 410-412 | Placeholder emit | Exist-only. Test `:398` |
| SYS loose.md is a card / no-spec.md omitted | `src/ui/game_board.c` | 499-505, 361-367 | File as dir, or header required | SYS dirs only; missing spec.md → pending. Test `:421` |
| `status: ready` stays pending | `src/ui/game_board.c` | 304-307 | Token map missed `ready` | `ready` → `in_progress`. Test `:495` |
| 65th SYS kept or `has_more` | `src/ui/game_board.c` | 391-396, 594-612 | Cap before sort | Scratch 256, sort, copy 64. Test `:525` |
| present false still walks phases | `src/ui/game_board.c` | 638-640 | Walk before return | Helper false → return. Tests `:632` / `test_httpd.c:3936` |
| leftover grill missing from Inbox | see Task #2 section | — | Inbox fill is Task #2 | `game_grill_fill_inbox` after artifacts |
| 400 / 404 string drifted | `src/ui/http_server.c` | 533-536, 540-542 | Handler edited | Same spec-board strings. Tests `test_httpd.c:4003` / `:3982` |
| spec-board has gamedev field | `src/ui/spec_board.c` | 1215 | Extra `to_json` key | Helper `:1096` unused by read/to_json. Test `:3975` |
| GET wrote skill trees | `src/ui/game_board.c` | 89 | Write mode or mkdir | `"rb"` only. Tests `:658` / `test_httpd.c:4024` |

### Remaining Vitest Gherkin (spec-010 Task #5)

Test-only leftover Thens (empty-dir + fetch log). Empty-dir tab+chrome in App: `gameBoard: "true"` is `emptyGameBoard(true)` (present, phase/focus null). afterEach + dedicated fetch log never `/api/skill-presence` — owners now also cover spec-011 Task #5 filled board (section above). Game pane paints four headers (spec-011 Task #3); launcher / gate-review still forbidden. spec-009 grill-only / neither / spec-board 500 stay green on default game-board 200 false. Function `#06b6d4` still locked.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Empty-dir omits Game tab | `graph-ui/src/App.test.tsx` / `App.tsx` | 1016 / showGame | Mock present false, or `showGame` ignored present+null chrome | `gameBoard: "true"` → `emptyGameBoard(true)` (`:51-61`). Test `:1016-1044` |
| Empty-dir tab shown but chrome is Production | `graph-ui/src/App.test.tsx` | 51-61, 1030 | Fixture filled `phase`, or used production board | `emptyGameBoard(true)` keeps phase/focus null. Assert `:1030-1036` |
| `/api/skill-presence` in App fetch | `graph-ui/src/App.tsx` / `useGameBoard.ts` / `App.test.tsx` | fetch / 104 / 315 | App or hook called the unused GET | Presence is GET `/api/game-board` only. afterEach `:327` / `:740` / `:829` / `:1082`. Dedicated `:1046-1078`. Filled leftover: spec-011 Task #5 section |
| Launcher / gate-review in Game pane | `graph-ui/src/components/GameBoardTab.tsx` / `GameBoardTab.test.tsx` | 177-179 / 134 | Chrome continue is a `<button>`, or Specs copy leaked | Chrome continue is `<p>`. Card button is spec-011 Task #4. Tests `:134`, `:159`. Chrome+cards lock `:741-765` |
| spec-009 grill-only hides Specs or shows Game | `graph-ui/src/App.test.tsx` / `App.tsx` | 207, 748 | Default mock present true | Keep `gameBoard ?? "false"`. Tests `:748` / `:792` / `:546` |
| Function hex drifted | `graph-ui/src/lib/colors.test.ts` | 27 | `LABEL_COLORS` or CSS vars leaked | `colorForLabel("Function") === "#06b6d4"` |

### App silent-win, dual fetch, deep-links (spec-010 Task #4)

Dual one-shot: `useGameBoard` GET `/api/game-board` + unchanged `useSddSkillPresent` GET spec-board. `showGame = settled && present`. `showSpecs = settled && !showGame && specsPresent`. In-flight omits both. game-board 500 omits Game; Specs stays spec-009. `resolveWorkspaceTab` for URL + pane. Inbound `tab=game` restore. `tab=specs` + gamedev → game. Enter stays Graph. spec-009 strip rows below now depend on `game.settled`.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| In-flight shows Game or Specs | `graph-ui/src/App.tsx` / `useGameBoard.ts` | 35-36 / 63-65 | `showSpecs` used raw specs `present` before settle | Both flags need `gameSettled`. Hang never settles. Test `App.test.tsx:876-891` |
| game-board 500 hides Specs | `graph-ui/src/hooks/useGameBoard.ts` / `App.tsx` | 71-75 / 36 | Non-200 left `settled` false | 4xx/5xx/throw → settled true, present false. Test `:894-902` |
| `tab=specs` + gamedev stays Specs or goes Graph | `graph-ui/src/lib/route.ts` / `App.tsx` | 59 / 94-99 | Silent-win skipped or specs restore won | `tab==="specs" && gamePresent` → `"game"`. Test `:786-800` |
| Inbound `tab=game` stays Graph after present | `graph-ui/src/App.tsx` | 38, 86-92 | `pendingGameDeepLink` not captured | Restore when `showGame`. Test `:855-873` |
| Enter opens Game | `graph-ui/src/App.tsx` | 179 | Default tab changed | `navigate("graph", p)`. Test `:833-842` |
| Grill-only / sdd-only hides Specs or shows Game | `graph-ui/src/App.tsx` / `App.test.tsx` | 36 / 118 | Default mock present true | `gameBoard ?? "false"`. Tests `:803-818` / `:845-852` |
| Specs hook fetches game-board | `graph-ui/src/hooks/useSddSkillPresent.ts` | 34 | Hook taught a second URL | Still GET spec-board only. AND NOT is App |
| `/api/skill-presence` or 4s game poll | `graph-ui/src/hooks/useGameBoard.ts` | 54-95 | Interval or leftover path | One fetch. Tests `useGameBoard.test.ts:116-129` |

### GameBoardTab chrome (spec-010 Task #3)

Dedicated pane (not SpecBoardTab). Props = GET JSON; no second board fetch. i18n phase map. Continue is selectable text. Missing-state when phase and focus are both null. Four columns + cards are spec-011 Task #3 (section above). App mounts this pane when `paneTab === "game" && showGame` (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Production missing | `graph-ui/src/components/GameBoardTab.tsx` / `i18n.ts` | 19-24, 143-145 / 121 | Token not mapped or EN key drifted | `02-production` → `t.gameBoard.phaseProduction`. Test `GameBoardTab.test.tsx:116` |
| Chrome row paints all three phase labels | `graph-ui/src/components/GameBoardTab.tsx` | 19-24, 141-145 | Chrome rendered all EN labels | Chrome uses only `mapped`. Column headers always show all four (Task #3). Tests `:116`, `:177` |
| Launcher button | `graph-ui/src/components/GameBoardTab.tsx` | 177-179 | Chrome continue wrapped in `<button>` | `<p className="... select-text">`. Empty-board tests `:153` / `:183`. Card button is spec-011 Task #4 |
| Second fetch | `graph-ui/src/components/GameBoardTab.tsx` | 131 | Pane GETs `/api/game-board` | Props only. Locale fetch is `/api/ui-config` |
| SpecBoardTab reused | `graph-ui/src/components/GameBoardTab.tsx` | 1-9 | Kanban host / EpicCard imported | Dedicated file. `SpecBoardTab.tsx` still spec-009. Tests `:122-125` |
| Missing-state copy wrong | `graph-ui/src/components/GameBoardTab.tsx` / `i18n.ts` | 135, 141-142 / 119 | Both-null gate dropped or raw English | `t.gameBoard.stateMdMissing`. Test `:142` |
| Columns / cards missing | see spec-011 Task #3 section | — | spec-010 no-columns lock inverted | Four headers + cards. Tests `:183-201` / `:156-181` |

### TabId, route kernels, strip, i18n (spec-010 Task #2)

Closed set is `WORKSPACE_TABS` `["graph","specs","adr","game"]`. `fallbackSpecsToGraph(tab, present)` signature stays spec-009. Sibling `fallbackGameToGraph` + `resolveWorkspaceTab` (specs + gamedev → game, then the two kernels). Same `WorkspaceTabStrip`, additive `showGame` → Graph then Game then ADR; Game wins if both flags true. App now passes settled-aware `showGame` / `showSpecs` (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| `tab=game` rejected (Dashboard / unknown) | `graph-ui/src/lib/types.ts` / `route.ts` | 93 / 25-26 | `WORKSPACE_TABS` missing `game` | Include `"game"`. `isWorkspaceTab` + project. Tests `route.test.ts:24-26` / `:38-39` / `:72` |
| `fallbackSpecsToGraph` signature changed | `graph-ui/src/lib/route.ts` | 40-43 | Extra args or folded into one router | Keep `(tab, present): TabId`. Test `:75-81` |
| `resolveWorkspaceTab` sends specs+gamedev to graph | `graph-ui/src/lib/route.ts` | 54-61 | Silent-win step skipped or kernels first | `tab === "specs" && gamePresent` → `"game"` then Game then Specs kernels. Test `:93-96` |
| Strip order wrong (const order / Specs beside Game) | `graph-ui/src/components/WorkspaceTabStrip.tsx` | 37-41 | Used `WORKSPACE_TABS` order | `showGame` → `["graph","game","adr"]`. Test `:71-86` |
| Both `showGame`+`showSpecs` paints both | `graph-ui/src/components/WorkspaceTabStrip.tsx` | 37-41 | Game-wins branch dropped | `showGame` first. Test `:117-130`. App must not pass both (Task #4) |
| Missing i18n `tabs.game` / phase labels | `graph-ui/src/lib/i18n.ts` | 18, 118-123, 130, 229-234 | Keys omitted or EN drifted | en "Game"; "state.md missing"; "Pre-production" / "Production" / "Post-production & Launch". Test `i18n.test.ts:87-96` |
| Disabled Game placeholder | `graph-ui/src/components/WorkspaceTabStrip.tsx` | 37-41, 52-66 | Hidden tab left `disabled` | Omit when `showGame` false. Tests `:89-91` / `:94-114` |

### GET /api/game-board (spec-010 Task #1)

GET `/api/game-board?project=` is a sibling of spec-board: same 400/404 strings, heap calloc, no archive merge, no POST, no MCP tool. Presence is `cbm_spec_board_gamedev_skill_present` (`.gamedev/` is a directory). `state.md` is fopen `"rb"` only. JSON `phase` is a skill token or null — never English labels. spec-011 Task #1 fills phase arrays when files exist; empty `.gamedev/` and present false still emit `[]`. `inbox` stays `[]` until spec-011 Task #2. spec-board still has no `gamedev_skill_present`.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| 400 missing project | `src/ui/http_server.c` | 533-536 | Query missing/empty, or string drifted | `{"error":"missing project parameter"}` — same as spec-board `:492`. Test `test_httpd.c:3978` |
| 404 unknown | `src/ui/http_server.c` | 540-542 | `resolve_project_root_path` miss, or string drifted | `{"error":"project not found"}` — same as spec-board `:498`. Test `:3957` |
| present false empty continue | `src/ui/game_board.c` | 189-191 | Return after present skipped, or continue always set | Helper false → return after `memset`. JSON `"continue":""` `:236-238`. Test `test_game_board.c:76` |
| unknown phase token → null | `src/ui/game_board.c` | 172-174, 220-223 | Token not `01-preproduction` / `02-production` / `03-postproduction` | `phase_token_ok` (`:25-28`) clears phase → JSON `null`. Focus kept. Test `:228` |
| compact vs `active_phase` / `director_focus` prefer compact | `src/ui/game_board.c` | 154-166 | Alias ran first, or compact ignored | Compact `phase=` / `focus=` win when both exist. Aliases `=` or line-start `:`. Tests `:170` / `:190` / `:208` |
| spec-board still no gamedev field | `src/ui/spec_board.c` | 1215 | Extra key in `to_json` | Do not emit `gamedev_skill_present`. Helper `:1096` unused by read/to_json. Test `test_httpd.c:3936` |
| GET 200 must not write skill trees | `src/ui/game_board.c` | 32, 195-201 | Write mode or mkdir leaked | `cbm_fopen` `"rb"` only. Handler `:550-558` does not write. Tests `test_game_board.c:270` / `test_httpd.c:3999` |
| empty arrays when no artifacts / present false | `src/ui/game_board.c` | 638-640, 743-804 | Expected empty only if dir empty or present false | Filled cards are spec-011 Task #1 (section above). Empty-dir HTTP `test_httpd.c:3910` |

### App strip, deep-link, Enter, omit (spec-009 Task #3)

These strip rows now depend on `game.settled` (spec-010 Task #4 section above). `showSpecs` is `gameSettled && !showGame && specsPresent` (`App.tsx:36`), not raw hook `present`. In-flight game-board omits Specs even when spec-board is already 200. After game-board settles !present / 500, grill-only and sdd-only still follow spec-009.

`useSddSkillPresent` is still sdd OR grill (Task #1). Omit-until-true still rewrites `tab=specs` → Graph while Specs is omitted. After `showSpecs` is true, inbound `tab=specs` is restored (`pendingSpecsDeepLink`) unless gamedev won (then `tab=game`). Enter stays Graph. Host Kanban gate is spec-009 Task #2. Game strip order is spec-010 Task #2.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Grill-only omits Specs | `graph-ui/src/hooks/useSddSkillPresent.ts` / `App.tsx` | 17 / 36 | Hook sdd-only, or `showSpecs` waited on game hang | `{ sdd: false, grill: true }` + game default false (`:118`). Test `App.test.tsx:651` |
| `?tab=specs` becomes `tab=graph` on grill-only 200 | `graph-ui/src/App.tsx` | 39, 101-107 | Restore skipped or gamedev stole it | Capture inbound specs `:39`; restore when `showSpecs`. Test `:672` |
| neither-skill keeps `tab=specs` | `graph-ui/src/App.tsx` / `route.ts` | 108-110 / 40-43 | Fallback not applied | `"false"` = both false + game default false. Tests `:341` / `:695` |
| Enter opens Specs | `graph-ui/src/App.tsx` | 179 | Default tab changed | `navigate("graph", p)`. Test `:718` |
| Tab order / name drifted | `graph-ui/src/components/WorkspaceTabStrip.tsx` | 37-41 | Strip product edited | Graph then Specs then ADR when `showSpecs`. Test `:651-669` |
| gamedev-only shows Specs | `graph-ui/src/App.test.tsx` / `App.tsx` | 705-716 / 36 | Extra flag, or game-board still in flight | Both spec flags false; game default false. Present-true silent win is Task #4 |
| spec-board GET 500 shows Specs | `graph-ui/src/App.tsx` | 36 | `specsPresent` true on error | Keep omit. Test `:450`. game-board 500 does not hide Specs (Task #4) |

### SpecBoardTab host Kanban on grill-only (spec-009 Task #2)

Host paints the Kanban when `board` exists and (`sdd_skill_present === true` OR `grill_skill_present === true`). Grill-only must not show `notSddSkill`. Last-resort: `!board` after load or both flags false → existing `notSddSkill` copy (no new i18n key). Loading (`loading && !board`) stays `t.common.loading`. Strip hook is the same OR (Task #1). App strip / deep-link / Enter is spec-009 Task #3 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Grill-only shows notSddSkill | `graph-ui/src/components/SpecBoardTab.tsx` | 345 | Host still sdd-only | OR `grill_skill_present === true`. Test `SpecBoardTab.test.tsx:372` |
| Grill-only empty epics/specs shows notSddSkill | `graph-ui/src/components/SpecBoardTab.tsx` | 345 | Empty lists treated as absent | Kanban + three `noSpecs`. Test `:404` |
| Both flags false paints Todo / epic id | `graph-ui/src/components/SpecBoardTab.tsx` | 345 | Last-resort dropped | `!(sdd \|\| grill)` → `notSddSkill`. Test `:443` |
| `board` null after load paints columns | `graph-ui/src/components/SpecBoardTab.tsx` | 345 | `!board` skipped | Last-resort. Test `:427` |
| First load shows notSddSkill | `graph-ui/src/components/SpecBoardTab.tsx` | 340-342 | Loading branch lost | `loading && !board` → `t.common.loading`. Test `:338` |
| sdd-true missing grill hides the pane | `graph-ui/src/components/SpecBoardTab.tsx` | 345 | Grill required | sdd OR; missing grill is false. Test `:355` |

### Presence predicate sdd OR grill (spec-009 Task #1)

`useSddSkillPresent` still one-shots GET `/api/spec-board?project=`. `bodyHasSkill` is `sdd_skill_present === true || grill_skill_present === true`. Missing grill key and non-boolean truthy stay false. Export name and `present` boolean unchanged. SpecBoardTab host gate is the same OR (spec-009 Task #2).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Specs missing on grill-only (GET 200 + grill true) | `graph-ui/src/hooks/useSddSkillPresent.ts` | 17 | `bodyHasSkill` still sdd-only | OR `grill_skill_present === true`. Test `useSddSkillPresent.test.ts:41` |
| Specs shows when neither skill | `graph-ui/src/hooks/useSddSkillPresent.ts` | 17 | Missing key or `"true"` / `1` treated truthy | Strict `=== true`. Missing grill + sdd false → false. Tests `:86` / `:101` / `:114` |
| Interval or `/api/skill-presence` | `graph-ui/src/hooks/useSddSkillPresent.ts` | 23-50 | Hook reused `useSpecBoard` or a second URL | One fetch; no `setInterval(4000)`. Test `:167` |
| 404 / 500 / hang / throw shows Specs | `graph-ui/src/hooks/useSddSkillPresent.ts` | 30, 36-38, 43 | `present` defaulted true | `setPresent(false)` until 200 + OR. Tests `:129` / `:140` / `:152` |

### Vitest Gherkin remaining UI grill-epic (spec-008 Task #4)

UI Gherkin lives in `SpecBoardTab.test.tsx`. Conversion is C-owned: mocks pass already-filtered `epics` (omit = `epics: []`). Two-plan document order is mock array order then product concat. 64-epic mock must not show a "Has more" control. Done / In progress never receive `epics`. Host Kanban on grill-only is spec-009 Task #2 (section above). Function hex locked in the same file.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Two-plan Todo ids not index then NNN then spec | `graph-ui/src/components/SpecBoardTab.tsx` | 267-278 | Mock shuffled, or specs mapped first | Mock `[planAFirst, planASecond, planBOther]` then planned. `todoEpics.map` then `entries.map`. Test `SpecBoardTab.test.tsx:911` (`:920-925`) |
| "Has more" appears with 64 epics | `graph-ui/src/components/SpecBoardTab.tsx` | — | Overflow chrome leaked | Cap omit is C. UI paints the 64 mock. No button/link/text "Has more". Test `:948` (`:960-962`) |
| Omitted epic id still in Todo | `graph-ui/src/components/SpecBoardTab.test.tsx` | 878 | Mock put the path in `epics[]` | UI does not re-match Companion-to. Pass `epics: []`. Assert `:887-889` |
| Done / In progress shows an epic id | `graph-ui/src/components/SpecBoardTab.tsx` | 238, 371-393 | `epics` passed off todo, or mock listed it | Only Todo gets `board.epics ?? []`. Done-claim mock is `epics: []`. Test `:893` (`:902-906`) |
| Host picker leaked when `project` is set | `graph-ui/src/components/SpecBoardTab.test.tsx` | 322 | Workspace host broke | No `selectProject`. Tests `:322` / `:338` / `:355` |
| grill-only shows notSddSkill | `graph-ui/src/components/SpecBoardTab.tsx` | 345 | Host still sdd-only | sdd OR grill. See spec-009 Task #2. Test `:372` |
| Function hex drifted | `graph-ui/src/lib/colors.test.ts` | 27 | `LABEL_COLORS` or CSS vars leaked | `colorForLabel("Function") === "#06b6d4"`. Also `SpecBoardTab.test.tsx:963` |

### SpecBoardTab EpicCard + Todo epics-then-specs (spec-008 Task #3)

Todo paints `EpicCard` then spec todos. Letter E uses `--color-epic-mark #7d8ec9` (not i18n, not a pill, not health/graph hex). EpicCard is display-only: no expand, no Archive/Unarchive, no POST. In progress / Done stay spec-only. Missing `epics` is `[]`. Host paints Kanban on sdd OR grill (spec-009 Task #2). Strip hook is the same OR (spec-009 Task #1). Live :9749 may be a pre-spec-008 binary (GET has no `epics`) — Vitest is the proof until `scripts/build.sh --with-ui`. Remaining UI Gherkin (two-plan order, 64-cap no Has more, Companion-to omit mock, Done/In progress no epic id) landed in Task #4 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Letter E is gray / missing | `graph-ui/src/styles/globals.css` | 28 | Token missing from `@theme` | `--color-epic-mark: #7d8ec9`. Class `text-[var(--color-epic-mark)]` (`SpecBoardTab.tsx:104`). Test `:819-821` |
| E is a pill / the word Epic | `graph-ui/src/components/SpecBoardTab.tsx` | 104 | Badge or i18n leaked | Literal `"E"`; no `rounded-full`. Test `:822-823` |
| Function nodes went periwinkle | `graph-ui/src/lib/colors.ts` | 19-20 | Graph map imported the chrome var | `colorForLabel("Function") === "#06b6d4"`. Do not edit `colors.ts`. Test `:963` |
| Epic follows the spec in Todo | `graph-ui/src/components/SpecBoardTab.tsx` | 267-278 | Specs mapped first | `todoEpics.map` then `entries.map`. Test `:828` |
| Epic id in In Progress or Done | `graph-ui/src/components/SpecBoardTab.tsx` | 238, 371-393 | `epics` passed off todo | Only Todo gets `board.epics ?? []`. Tests `:831-832` / Task #4 `:893` |
| Click epic expands / Archive / POST | `graph-ui/src/components/SpecBoardTab.tsx` | 100-111 | EpicCard reused SpecCard | Display-only `<div>`; `persistArchive` SpecCard only (`:321-334`). Test `:837` |
| Missing `epics` throws | `graph-ui/src/lib/types.ts` | 141-143 | Keys required | Optional; `board.epics ?? []` (`SpecBoardTab.tsx:362`). Test `:373` |
| grill-only shows notSddSkill | `graph-ui/src/components/SpecBoardTab.tsx` | 345 | Host still sdd-only | sdd OR grill. See spec-009 Task #2. Test `:372`. Strip hook is the same OR (`useSddSkillPresent.ts:17`) |
| Live :9749 has no epic cards | daemon binary | — | Pre-spec-008 embed | Rebuild `--with-ui`. GET must include `epics` |

### HTTP GET additive + POST epic-id 404 (spec-008 Task #2)

GET `/api/spec-board` is still read → archive merge on `specs[]` only → `to_json`. `http_server.c` has no `fopen` / `opendir` of `.grill/` (grill parse is `cbm_spec_board_read`). GET 200 includes `grill_skill_present` + `epics`. Unknown project stays 404 `project not found`. POST with an epic path as `spec_id` is 404 `spec not found` — `spec_board_find` never walks `epics[]`, no `spec_archive_set`. GET and that POST leave `index.md` / epic.md / `active.json` byte-identical. EpicCard UI is the Task #3 section above.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| GET 200 missing `grill_skill_present` / `epics` | `src/ui/http_server.c` | 507-510 | Wrapper skipped `to_json` | Same dispatch: read → merge → `to_json`. Test `test_httpd.c:3675` |
| GET unknown project is not 404 | `src/ui/http_server.c` | 496-498 | Different error string | `{"error":"project not found"}`. Test `:3804` |
| POST epic id is 200 / wrote a flag | `src/ui/http_server.c` | 531-543, 626-630 | `spec_board_find` walked `epics[]` or `set` before 404 | Loop `spec_count` only. `{"error":"spec not found"}`. Test `:3745` (`:3785-3786`) |
| GET/POST changed `index.md` / epic.md / `active.json` | `src/ui/http_server.c` | no `fopen` | HTTP opened skill trees | Grill IO stays in `spec_board.c` `"rb"`. Tests `:3825` / `:3675` / `:3745` |
| Grill `fopen` in `http_server.c` | `src/ui/http_server.c` | 487-518 | Sibling reader leaked into HTTP | No `fopen` in this file. SDD-ADR-036. Test `:3825` |
| Epic JSON has `"archived"` | `src/ui/http_server.c` | 464-473 | Merge walked `epics[]` | Specs only. Test `:3717` |

### spec_board grill read + additive JSON (spec-008 Task #1)

`cbm_spec_board_read` still fills sdd specs, then walks `root/.grill/` when that path is a directory. JSON always emits `grill_skill_present` and `epics[]` (empty when missing). Conversion is `strcmp` of the epic relative id to listed spec.md `Companion to:` (first `.grill/plans/`…`.md`, trailing notes ignored) or `active.json` `source.grill_epic`. fopen `"rb"` only. Cap 64 after omit; no `has_more`. HTTP GET/POST Gherkin is Task #2 (section above). EpicCard UI is the Task #3 section above.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| `.grill/` on disk but flag false | `src/ui/spec_board.c` | 1087-1093, 1170-1171 | Not a directory (file-at-path or missing) | `cbm_is_dir(root/.grill)`. Test `test_spec_board.c:1328` |
| Flag true, `epics` [] with epic.md present | `src/ui/spec_board.c` | 1172-1173, 1012-1073 | Walk skipped, converted, or name not `epic-NNN-*.md` | `grill_fill_epics` after present. Parse `:712-735`. Test `:946` |
| Converted epic still listed | `src/ui/spec_board.c` | 738-756, 322-344 | Companion-to token missed | First `.grill/plans/`…`.md` on `Companion to:` (`:334-344`). Tests `:1016` / `:1092` |
| `source.grill_epic` omit missed | `src/ui/spec_board.c` | 275-283, 744-745 | `source` object not read | yyjson string exact `strcmp`. Test `:1062` |
| Done spec still leaves epic in JSON | `src/ui/spec_board.c` | 750-754 | Match skipped non-todo | Any listed spec’s `companion_grill`. Test `:1123` |
| 65th epic in JSON / spec cap shrunk | `src/ui/spec_board.c` | 967-968, 1026-1028, 1067-1068 | Shared pool or `has_more` key | Own cap 64; specs stay 64. Test `:1277` |
| Unreadable epic aborts the board | `src/ui/spec_board.c` | 978-984 | Dir-at-path treated as fatal | Skip; still fills others. Test `:1360` |
| `index.md` / epic.md / `active.json` bytes changed | `src/ui/spec_board.c` | 36-37 | Write mode leaked | `cbm_fopen` `"rb"` only. Test `:1386` |
| No `.sdd-skill/` → no epics | `src/ui/spec_board.c` | 1170-1173 | Grill gated on sdd | Fill independent of sdd. Test `:1438` |
| `gamedev_skill_present` / `has_more` on JSON | `src/ui/spec_board.c` | 1215, 1247-1262 | Extra keys in `to_json` | Additive `grill_skill_present` + `epics` only. Tests `:196-197` |

### Last indexed surfaces — local text, raw ISO dateTime (spec-007 Task #2)

Visible text on Dashboard (list + conflict), WorkspaceHeader, and AdrTab generated stamp equals `formatIndexedAt` (runtime-local Intl, SDD-ADR-034). `<time dateTime>` and `title` stay the raw `indexed_at` ISO. Invalid `not-a-date` stays raw on all three (AdrTab only when markers are present). Ghost header omits every `time`. AdrTab without `CBM-GENERATED-START` omits the stamp. Conflict Enter newest is still `alpha` (ISO compare). Product `.tsx` already called the helper — this task is test-only.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| List/conflict text !== `formatIndexedAt` | `graph-ui/src/components/Dashboard.tsx` | 154-155, 248-249 | Surface bypassed helper or hardcoded UTC | Children must be `formatIndexedAt(indexed_at, lang)`. Tests `Dashboard.test.tsx:113` / `:450` |
| Header text !== `formatIndexedAt` | `graph-ui/src/components/WorkspaceHeader.tsx` | 31-32 | Header bypassed helper | Same helper. Test `WorkspaceHeader.test.tsx:76-78` |
| Stamp text !== `formatIndexedAt` | `graph-ui/src/components/AdrTab.tsx` | 123-124 | Stamp bypassed helper | Same helper. Test `AdrTab.test.tsx:199-201` |
| `dateTime` / `title` show formatted local text | Dashboard / header / AdrTab `<time>` | 154 / 31 / 123 | Helper output written into attributes | Instant stays raw `indexed_at`. Tests `:111-112` / `:76-77` / `:199-200` |
| `not-a-date` invented a clock | same surfaces + `formatIndexedAt.ts` | 27-28 | Formatter ran on Invalid Date | Text and `dateTime` must be `not-a-date`. Tests `:147-149` / `:91-93` / `:217-218` |
| Ghost header shows a `time` | `graph-ui/src/components/WorkspaceHeader.tsx` | 21, 28 | Wrong-name match or index-status | `projects.find` by exact `name`. Test `:97-105` |
| No-marker AdrTab shows stamp / "Generated at" | `graph-ui/src/components/AdrTab.tsx` | 113, 120 | Gate used live `content` | `hasGenerated && listed`. Test `:223-235` |
| ISO / formatted clock in ADR textarea | `graph-ui/src/components/AdrTab.tsx` | 77-81, 144 | Chrome copied into `content` | POST equals GET blob. Tests `:203` / `:219-220` |
| Conflict Enter opens `alpha-old` | `graph-ui/src/lib/pathGroups.ts` | 17-26 | Display TZ leaked into newest | ISO string then name. Dashboard Enter `:141`. Test `:455-457` |
| Host-UTC Dashboard `dateTime` off the ISO | `graph-ui/src/components/Dashboard.tsx` | 248 | Attribute used helper output | `dateTime` stays raw even when `localFmt === utcFmt`. Test `:111` |
| Function hex drifted | `graph-ui/src/lib/colors.test.ts` | 27 | `LABEL_COLORS` or CSS vars leaked | `colorForLabel("Function") === "#06b6d4"` |

### Last indexed helper — runtime local Intl (spec-007 Task #1)

`formatIndexedAt` no longer pins `timeZone: "UTC"`. Visible text is `Intl.DateTimeFormat` with year/month/day/hour/minute + `timeZoneName: "short"` and no `timeZone` key (host default). Locale is still `en-US` / `zh-CN`. Invalid or empty input still returns the raw string. Surfaces lock `dateTime` / `title` as raw ISO (Task #2 section above). Host UTC: `localFmt === utcFmt` is OK.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Visible text equals raw ISO on a valid instant | `graph-ui/src/lib/formatIndexedAt.ts` | 26-29 | `Date` parse NaN, or parts dropped | Invalid/empty is supposed to return raw. Valid ISO must hit `:30`. Test `formatIndexedAt.test.ts:44-47` |
| Visible text still UTC while host is not | `graph-ui/src/lib/formatIndexedAt.ts` | 16-23, 30 | `timeZone` leaked into `INDEXED_AT_PARTS` | Do not pass `timeZone`. Compare to `localFmt` (no zone key). Test `:23-35` |
| en !== `localFmt` or zh !== zh-CN local | `graph-ui/src/lib/formatIndexedAt.ts` | 11-14, 30 | Locale or parts drifted | `en`→`en-US`, `zh`→`zh-CN`. Same parts, no pin. Tests `:29-31` / `:37-42` |
| Host-UTC Then fails | `graph-ui/src/lib/formatIndexedAt.test.ts` | 49-60 | Required pin-diff on a UTC host | `localFmt === utcFmt` → assert the shared string. No `TZ=` |
| Test hardcodes a wall-clock / `"… UTC"` | `graph-ui/src/lib/formatIndexedAt.test.ts` | 13-20 | Oracle replaced by a literal | Rebuild `localFmt` / `utcFmt` in-process |
| `dateTime` / `title` show formatted local text | Dashboard / header / AdrTab `<time>` | 154 / 31 / 123 | Surface wrote helper output into attributes | Instant stays raw `indexed_at`. See Task #2 section |

### Vitest Gherkin + poll after archive (spec-006 Task #4)

UI Gherkin lives in `SpecBoardTab.test.tsx`. Click the title button whose name contains the spec id. Poll-after-archive proof is a rerender with a new GET board (`archived: true`) while Show archived stays off — the hidden card must not come back. spec-005 "no Archive on the document" is replaced: those names exist only on an expanded Done card.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Poll resurrects a hidden archived card | `graph-ui/src/components/SpecBoardTab.tsx` | 323-328, 267 | Rerender used `archived: false`, or `showArchived` leaked true, or filter skipped `=== true` | GET payload must stay `archived: true`; toggle off. Filter is `done && isArchived && !showArchived`. Test `SpecBoardTab.test.tsx:612` (`:632-639`) |
| Show archived pressed after a poll rerender | `graph-ui/src/components/SpecBoardTab.tsx` | 267-278 | New `board` object treated as a project change | Only `project` change resets the toggle. Test `:632` |
| Archive / Unarchive on Todo / In Progress | `graph-ui/src/components/SpecBoardTab.tsx` | 115-116 | Column gate dropped | Names only on expanded Done. Test `:643`. Leftover Todo `:519`. In Progress `:314` |
| Archive opened confirm / dialog | `graph-ui/src/components/SpecBoardTab.tsx` | 296-308 | `window.confirm` or a modal leaked | No confirm. Test `:542` / `:554-556` |
| Host picker leaked when `project` is set | `graph-ui/src/components/SpecBoardTab.test.tsx` | 234 | Workspace host broke | No `selectProject`. Tests `:234` / `:250` / `:265` |
| Function hex drifted | `graph-ui/src/lib/colors.test.ts` | 27 | `LABEL_COLORS` or CSS vars leaked | `colorForLabel("Function") === "#06b6d4"` |

### SpecBoardTab archive filter + session toggle (spec-006 Task #3)

Done hides `archived && !showArchived`. Show archived is session-only (`useState(false)`, reset with `expandedIds` on project change — no localStorage). Archive / Unarchive only on an expanded Done card. POST `/api/spec-board` then `await refresh()` (same GET as the 4s poll). No confirm. spec-005 expand Set / blurb / Todo pending-only stay. Poll-after-archive Gherkin landed in Task #4 (section above).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Archived Done visible on first paint | `graph-ui/src/components/SpecBoardTab.tsx` | 323-328, 267 | Filter missed `archived === true` or toggle defaulted on | `isArchived` is `=== true`. `useState(false)`. Test `SpecBoardTab.test.tsx:453` |
| Toggle still pressed after remount / `?project=` change | `graph-ui/src/components/SpecBoardTab.tsx` | 273-278 | State leaked or localStorage added | Reset with `expandedIds` (`:277-278`). Test `:466` / `:482` |
| Archive opens confirm / dialog | `graph-ui/src/components/SpecBoardTab.tsx` | 174-176, 296-308 | `window.confirm` or a modal leaked | No confirm. Test `:542` / `:554-556` |
| Archive on a Todo expand | `graph-ui/src/components/SpecBoardTab.tsx` | 115-116 | Column gate dropped | Only expanded Done. Test `:519` |
| After 200 the card is still visible (hide on) | `graph-ui/src/components/SpecBoardTab.tsx` | 304-305 | `refresh` not awaited or mock did not set `archived` | `await refresh()` after `res.ok`. Test `:542` / `:557` |
| Poll resurrects a hidden archived card | `graph-ui/src/components/SpecBoardTab.tsx` | 323-328 | Filter missed GET `archived: true` | See Task #4 section. Test `:612` |
| Unarchive while hide still hides the card | `graph-ui/src/components/SpecBoardTab.tsx` | 326 | Filter used a stale `archived` true | After refresh the entry is false. Test `:572` / `:606-608` |
| Missing `archived` hides a Done card | `graph-ui/src/lib/types.ts` | 120 | Truthy-missing / default true | Optional; only `=== true`. Test `:529` |
| Fourth column / "Archived" title | `graph-ui/src/components/SpecBoardTab.tsx` | 333-368 | Extra `Column` | Three children only |
| Show archived on Todo / In Progress header | `graph-ui/src/components/SpecBoardTab.tsx` | 225-236 | `id === "done"` gate dropped | Done header only. Test `:462-463` |
| Empty archived Done lost the toggle | `graph-ui/src/components/SpecBoardTab.tsx` | 225-236, 244 | Toggle inside the empty branch | Header keeps the control. Copy is `noSpecs`. Test `:508` |
| Accessible name drifted | `graph-ui/src/lib/i18n.ts` | 113-115, 217-219 | Keys renamed | en Archive / Unarchive / Show archived. Lock `i18n.test.ts:81-86` |
| Archive button went teal | `graph-ui/src/components/SpecBoardTab.tsx` | 171-174, 230-232 | New CSS or `text-primary` on chrome | `text-foreground/40`. No new CSS file |
| Expand Set reset on poll / Todo shows done `#1` | `graph-ui/src/components/SpecBoardTab.tsx` | 273-285, 67 | Expand Set or TaskList filter rewritten | spec-005 Set + pending-only unchanged. Tests `:287` / `:388` |

### HTTP spec-board GET merge + POST archive (spec-006 Task #2)

GET `/api/spec-board` still reads skill files, then HTTP merges `spec_archive` onto matching ids only. Missing table or query-open fail → all `archived` false, still 200. POST is the same path: flag object 200; 409 `spec not done` writes nothing; mutation lock 423 like ADR save. Epic path as `spec_id` is 404 `spec not found` (spec-008 Task #2). `publish_staged` copies flags live→stage after the ADR write. UI hide/show landed in Task #3. Poll-after-archive Vitest landed in Task #4.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| GET `archived` always false after set | `src/ui/http_server.c` | 454-460, 498-499 | Query-open/load failed (degrades) | Apply after `cbm_spec_board_read`. Test `test_httpd.c:3310` |
| GET invents an orphan card | `src/ui/http_server.c` | 462-469 | Store rows appended to `specs` | Match `e->id` only. Test `:3330` |
| Leftover Todo lost `archived` true | `src/ui/http_server.c` | 462-469 | Merge skipped non-done | Id-only apply. Test `:3331-3332` |
| POST 200 is the full board | `src/ui/http_server.c` | 642-643 | Returned `to_json` | Flag object. Test `:3366` |
| 409 still persisted | `src/ui/http_server.c` | 605-609 | `set` before done check | 409 before lock/`set`. Test `:3402` |
| `archived:"yes"` not 400 | `src/ui/http_server.c` | 563-566 | Non-bool accepted | `invalid archived`. Test `:3495` |
| 423 still wrote | `src/ui/http_server.c` | 612-616 | `set` before `mutation_begin` | Same lock as ADR `:1119`. Test `:3545` |
| Flags vanish after Reindex dump | `src/pipeline/pipeline.c` | 1799-1808 | Copy skipped or fail ignored | After ADR write; copy fail fails publish. Test `test_httpd.c:3575` |
| `active.json` changed on POST | `tests/test_httpd.c` | 3352-3371, 3415-3433 | Skill write leaked | Snapshot around 200 and 409 |
| Read invents `archived` true | `src/ui/spec_board.c` | header / `:818` test | SQLite in the reader | `to_json` emits; read leaves 0. Test `test_spec_board.c:818` |

### Store spec_archive set/load/copy (spec-006 Task #1)

Flags live in the project `.db` table `spec_archive`, not in `.sdd-skill/`. Write-open creates the table. Query-open skips `init_schema` — load must probe `sqlite_master` and treat a missing table as zero rows. Unarchive UPSERTs `archived=0`; it does not DELETE. HTTP merge / POST landed in Task #2. UI is Task #3–#4.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Legacy `.db` load/GET 500 | `src/store/store.c` | 8171-8189, 8248-8251 | SELECT without probe | `SQLITE_DONE` → OK, count 0. Test `test_store_spec_archive.c:148` |
| Empty/NULL spec_id wrote a row | `src/store/store.c` | 8201-8204 | Guard skipped | Both → `CBM_STORE_ERR`. Test `:81` |
| Unarchive deleted the row | `src/store/store.c` | 8209, 8212-8215 | DELETE on 0 | UPSERT 0. Test `:92` |
| Repeat archive = two rows | `src/store/store.c` | 8212-8215 | No `ON CONFLICT` | PK UPSERT. Test `:111` |
| Orphan id dropped on load | `src/store/store.c` | 8259-8288 | Store filtered by board | No board in store. Test `:130` |
| Copy failed, src table missing | `src/store/store.c` | 8301-8304 | Missing treated as ERR | load OK count 0 → no-op. Test `:187` |
| Table missing after write-open | `src/store/store.c` | 325-333 | DDL omitted | `CREATE TABLE IF NOT EXISTS spec_archive` in `init_schema` |
| Flags in ADR / `store_meta` | `src/store/store.h` | 790-797 | Wrong table reused | New APIs only. Writer DDL unchanged |

### Vitest Gherkin + poll persist + no Archive (spec-005 Task #4)

UI Gherkin lives in `SpecBoardTab.test.tsx`. Click the title button whose name contains the spec id. Poll proof is a rerender with a new `board` object (same ids). Multi-expand is two blurbs at once. spec-005 "no Archive on the document" is superseded: Archive/Unarchive are on expanded Done only (spec-006). Poll-after-archive (GET `archived: true` does not resurrect) is spec-006 Task #4.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Archive on Todo / In Progress | `graph-ui/src/components/SpecBoardTab.tsx` | 115-116 | Column gate dropped | Done expand only. Helper `SpecBoardTab.test.tsx:201-206`; leftover Todo `:497` |
| Poll collapse | `graph-ui/src/components/SpecBoardTab.tsx` | 220-231 | Seed/reset ran on new `board` identity | `seededRef` stays true. Only `project` change clears. Test `:304` |
| Multi-expand lost | `graph-ui/src/components/SpecBoardTab.tsx` | 233-239 | Toggle replaced the Set | `next.add(id)` keeps other ids. Test `:249` both blurbs `:260-261` |
| In Progress missing 1/2 tasks | `graph-ui/src/components/SpecBoardTab.tsx` | 136-139 | Chrome inside expand body or i18n drifted | First paint `:218` — blurb, both tasks, implementer, `tasksDone(1, 2)` `:230-231` |
| Todo shows done `#1` | `graph-ui/src/components/SpecBoardTab.tsx` | 63 | Filter skipped | Pending-only. Test `:205` — `#2` present, `#1` absent `:214-215` |
| Function hex drifted | `graph-ui/src/lib/colors.test.ts` | 26 | `LABEL_COLORS` or CSS vars leaked | `colorForLabel("Function") === "#06b6d4"` |

### SpecCard expand Set + TaskList filter (spec-005 Task #3)

`expandedIds` lives on `SpecBoardTab` (spec id). First board for a project seeds active ids. Later `board` objects must not clear the Set. Title `<button>` is the only expand control. Todo TaskList is `done === false`. Empty blurb omits `[data-region='blurb']`. Zero tasks still expand with `noTasksYet`. Archive/Unarchive on expanded Done is spec-006 Task #3 (section above) — not a leak.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Expand collapses on poll | `graph-ui/src/components/SpecBoardTab.tsx` | 273-285 | Seed/reset ran on board identity | `seededRef` stays true. Only `project` change clears (`:273-278`). Test `SpecBoardTab.test.tsx:370` |
| Todo shows done tasks | `graph-ui/src/components/SpecBoardTab.tsx` | 67 | Filter skipped or used all tasks | `column === "todo" ? tasks.filter((task) => !task.done) : tasks`. Test `:269` |
| Empty blurb still shown | `graph-ui/src/components/SpecBoardTab.tsx` | 113, 164-168 | Region painted when `blurb` is `""` | `entry.blurb ?? ""` then `blurb !== ""` before `data-region="blurb"`. Test `:332` |
| Zero tasks no expand | `graph-ui/src/components/SpecBoardTab.tsx` | 112, 124-129 | `canExpand` still `active && task_count > 0` | `canExpand = true`. Test `:358` |
| Archive on Todo / In Progress | `graph-ui/src/components/SpecBoardTab.tsx` | 115-116 | Column gate dropped | Done expand only. Test `:497` |
| In Progress starts collapsed | `graph-ui/src/components/SpecBoardTab.tsx` | 24-30, 282-285 | First-board seed missed `entry.active` | `seedActiveIds`. Test `:282` |
| Chrome gone when collapsed | `graph-ui/src/components/SpecBoardTab.tsx` | 138-160 | Agent / N/M / checklist inside `{expanded &&}` | Chrome stays outside TaskList. Test `:412` |
| Missing JSON blurb paints a region | `graph-ui/src/lib/types.ts` | 118 | `blurb` required but mock omitted; coalesce skipped | `entry.blurb ?? ""` (`SpecBoardTab.tsx:113`). Test `:345` |
| Second card closes the first | `graph-ui/src/components/SpecBoardTab.tsx` | 287-294 | Set replaced instead of add | Toggle adds/removes one id. Test `:315` |

### Spec-board enrich all + dual done (spec-005 Task #2)

`cbm_spec_board_read` fills every listed spec from one `spec.md` (title + blurb) and one `tasks.md`. `history/test_results.log` is opened once and applied to all. Active done = bare `Task #N` last-line-wins. Non-active done needs spec id + `Task #N` + PASS. Zero writes.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Listed spec title/blurb still `""` with a real spec.md | `src/ui/spec_board.c` | 289-313 | Path wrong or `read_whole_file` NULL | `read_spec_md` opens `spec_dir/spec.md` once (`:291`); extract on same buffer (`:311`) |
| KPI in live GET blurb | `src/ui/spec_board.c` | 311, 469-472 | Extract skipped or H2 stop missed | Immediate H2 / no ES → `""`. Test `test_spec_board.c:469` |
| Non-active done from bare `Task #N` | `src/ui/spec_board.c` | 556-568 | Active matcher used for all | `qualified = e->active \|\| strstr(line, e->id)` (`:558`). Test `:510` |
| Active compact PASS/FAIL broken | `src/ui/spec_board.c` | 558, 564 | Active required spec id | Bare `Task #N`; last line wins. Fixture `:186` |
| Missing tasks.md still has tasks | `src/ui/spec_board.c` | 496-502 | Headings invented | NULL file → return; `task_count` 0. Test `:604` |
| Unreadable spec.md still has blurb | `src/ui/spec_board.c` | 37-39, 293-296 | Dir treated as text | `cbm_fopen` rb fails. `sb_spec_md_as_dir` (`test_spec_board.c:66-71`) |
| Ghost spec 500 / present false | `src/ui/spec_board.c` | 640-645, 672-674 | Missing file aborted the board | Present stays true. Ghost `""`. Test `:559` |
| Todo card has agent / checklist | `src/ui/spec_board.c` | 676-697 | Active-only block ran for all | `if (!e->active) continue`. Test `:551-552` |
| Log fopen per spec | `src/ui/spec_board.c` | 664-666, 674 | Open inside the loop | One `log_buf`; apply per entry |
| Idle no-file ids gained tasks | `tests/test_spec_board.c` | 167-173 | Enrich invented data | No files → `task_count==0`, blurb `""` |

### Spec-board blurb extract (spec-005 Task #1)

Public helper parses `## Executive Summary` into `entry.blurb` (512 B). `to_json` always emits escaped `"blurb"`. Live read now calls extract from `read_spec_md` (Task #2) — GET can have a real blurb when `spec.md` exists.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| KPI / `KPI-MUST-NOT-BLURB` in blurb | `src/ui/spec_board.c` | 322-336, 472 | Heading too loose or body not cut at next H2 | Exact `## Executive Summary` + space/EOL (`:331-334`); stop at `\n## ` (`:472`) |
| Empty ES still has text | `src/ui/spec_board.c` | 469-470, 486-488 | Next `## ` on the following line, or whitespace-only body | Immediate H2 or trim-empty → `""` |
| H1 / `### Executive Summary` used | `src/ui/spec_board.c` | 331-334 | `#` or `###` accepted | Must be H2 + isspace or EOL |
| URL or third sentence in blurb | `src/ui/spec_board.c` | 344-378, 383-397 | Cut before link strip, or third kept | `strip_md_links` then `cut_after_two_sentences` (`:482-483`) |
| Third sentence after 512 cap | `src/ui/spec_board.c` | 434-448 | Truncate used leftover third-sentence bytes | Drop third first (`:483`); then `copy_blurb_truncated` |
| Mid-word cut at 512 | `src/ui/spec_board.c` | 442-446 | Walkback skipped | Last space in the window becomes NUL |
| NULL / empty md leaves garbage | `src/ui/spec_board.c` | 450-457 | `out` not cleared | `out[0] = 0` before parse |
| JSON missing `"blurb"` | `src/ui/spec_board.c` | 752-756 | Format string dropped the key | Every spec emits `"blurb"`; empty extract is `""` |
| Quotes unescaped in JSON | `src/ui/spec_board.c` | 746 | Raw blurb in `APP` | `esc_blurb[1024]` + `cbm_json_escape` |
| Listed spec has `""` despite ES text | `src/ui/spec_board.c` | 289-313 | Task #2 open failed | `read_spec_md` must call extract (`:311`). Missing/unreadable → both `""` |
| Alloc fail leaves stale output | `src/ui/spec_board.c` | 478-480 | `malloc` for stripped body failed | Return with `out` already `""` |

### AdrTab stamp + warning (spec-004 Task #4)

Stamp and replace warning appear only when last GET/save includes `CBM-GENERATED-START`. Visible datetime is list `indexed_at` via `formatIndexedAt` (runtime-local Intl, SDD-ADR-034). `<time dateTime>` / `title` stay raw ISO (Task #2 landed). Invalid `not-a-date` stays raw. No-marker omits stamp. One textarea. Dirty leave is still App `window.confirm`.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Stamp missing with `CBM-GENERATED-START` in GET | `graph-ui/src/components/AdrTab.tsx` | 113, 120 | `lastClean` empty, or name not in `useProjects` | `hasGenerated && listed`; mock `list_projects` with that `name` + ISO |
| Stamp text !== `formatIndexedAt` | `graph-ui/src/components/AdrTab.tsx` | 123-124 | Helper skipped or UTC string hardcoded | Valid ISO is local Intl. Test `AdrTab.test.tsx:199-201`. See Last indexed surfaces |
| Visible stamp is raw ISO | `graph-ui/src/components/AdrTab.tsx` | 124 | Invalid `indexed_at` (expected) or helper skipped | Invalid stays raw (`:217-218`). Valid ISO is local Intl, not UTC pin |
| `dateTime` / `title` not the raw ISO | `graph-ui/src/components/AdrTab.tsx` | 123 | Attributes used helper output | Instant stays raw. Test `:199-200` |
| Stamp on unmarked `# Existing ADR` | `graph-ui/src/components/AdrTab.tsx` | 113 | Gate used live `content` | `lastClean.includes("CBM-GENERATED-START")`. Test `:223-235` |
| Warning missing with markers | `graph-ui/src/components/AdrTab.tsx` | 129-132 | `role="note"` removed or i18n drifted | `t.adr.replaceWarning`; lock `i18n.ts:81-82` / zh `:183` |
| Warning/stamp label wrong language | `graph-ui/src/lib/i18n.ts` | 80-82, 182-183 | Keys drifted | en `Generated at` / zh `生成于`; warning strings locked in `i18n.test.ts:63-68` |
| ISO written into textarea / POST | `graph-ui/src/components/AdrTab.tsx` | 77-81 | Chrome copied into `content` | POST `{project, content}` equals GET blob; test `:203` / `:219-220` |
| Two textareas | `graph-ui/src/components/AdrTab.tsx` | 144 | Split generated/manual editors | One `textbox`; Save unchanged |
| Dirty leave not confirmed | `graph-ui/src/App.tsx` | 80 | `onDirtyChange` unwired or confirm deleted | `window.confirm(t.adr.unsavedConfirm)`; AdrTab dirty `:69-71` |

### HTTP / MCP / watcher Gherkin (spec-004 Task #3)

User-triggered create/Reindex/`index_repository` fill the same `project_summaries` blob GET `/api/adr` and `manage_adr` get read. POST `/api/adr` body max is 32768. Watcher-style `adr_fill: false` does not insert markers.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| GET after Reindex has no `CBM-GENERATED` (skill present) | `src/mcp/mcp.c` | 8189 | Pipeline left at `new()` default false | HTTP executor omits `adr_fill` (`test_httpd.c:2330`); want defaults true |
| Job stays `indexing` | `tests/test_httpd.c` | 2308-2338 | `index_repository` not `"status":"indexed"` | Read `exec.last_resp`; `http_wait_index_done` (`:2459`) |
| Create `{root_path}` unmarked with trio | `tests/test_httpd.c` | 2906, 2936 | Bare POST skipped the real executor | Same fill as Reindex; derived name from path |
| `manage_adr` get misses `PURPOSE-MCP-FILL` | `tests/test_mcp.c` | 6287 | Fill off or trio missing | `index_repository` without `adr_fill: false`; store `:6322` |
| Watcher / `adr_fill: false` writes markers | `tests/test_mcp.c` | 6409 | Gate ignored JSON bool false | `mcp.c:1561-1563`; watcher args `application.c:3399` |
| `# New notes` gone after next index | `src/mcp/mcp.c` | 11029-11031 | MANUAL span dropped before parse | Whole-doc update; splice keeps MANUAL (`adr_fill.c:189-191`) |
| `# Old notes` still after survive | `tests/test_mcp.c` | 6337 | Update did not write `# New notes` | Expect `updated` then `mcp_between` (`:6393`) |
| `PURPOSE-HAND-EDIT` survives Reindex | `tests/test_httpd.c` | 3091 | Fill skipped or trio lacks `PURPOSE-CANONICAL` | POST `/api/adr` then Reindex; GET `:3130` |
| 16KiB Save is 400 `invalid body` | `src/ui/http_server.c` | 918 | Cap still 16384 or stale binary | `body_len > CBM_SZ_32K` (`constants.h:40`); test `:3140` |
| Body >32768 is not 400 | `src/ui/http_server.c` | 918 | Cap raised too far or skipped | 400 `invalid body`; transport max is 1MiB |
| `SECRET-DEVLOG-ALPHA` in GET | `src/adr/adr_fill.c` | 20-22 | Extra relative opened | HTTP fixture writes DEV_LOG only to prove omit (`test_httpd.c:2859`) |
| Unreadable ARCHITECTURE still copied | `tests/test_httpd.c` | 3048 | chmod 0 still readable | Directory-at-path fallback (`:2361`) |
| No-skill tree gained markers | `tests/test_httpd.c` | 2965 | Fill ran without `.sdd-skill/` | Content stays `# Hand only\n` |
| No-skill MCP ADR test now fails | `tests/test_mcp.c` | 6130 | Markers invented on a tree without skill | `tool_index_repository_reports_store_backed_adr` must stay green |

### Pipeline hook + skip (spec-004 Task #2)

Fill runs after ADR capture when `adr_fill` is true. Watcher jobs encode false. `.sdd-skill` is ALWAYS_SKIP. Live HTTP/MCP Gherkin is the Task #3 section above.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Watcher incremental writes `CBM-GENERATED` | `src/daemon/application.c` | 3399 | Watcher args omitted `adr_fill: false` | `application_build_index_args(..., require_live_watch)`; seam `:3580` |
| Watcher still fills with false in JSON | `src/mcp/mcp.c` | 1561-1563 | Gate missed the bool | Only JSON bool false turns want off; hook `:8189` |
| User Reindex / `index_repository` leaves unmarked | `src/mcp/mcp.c` | 8189 | Pipeline left at `new()` default false | `cbm_pipeline_set_adr_fill` from `cbm_mcp_index_want_adr_fill` |
| Full persist unmarked with skill + flag true | `src/pipeline/pipeline.c` | 1997 | Apply not called after capture | Splice before publish; NULL fill keeps prior (`:334-335`) |
| Incremental persist unmarked with flag true | `src/pipeline/pipeline_incremental.c` | 2898 | Apply skipped on dump | Same apply after capture; staging clone `:2278-2290` |
| Unchanged-tree Reindex skips fill | `src/pipeline/pipeline_incremental.c` | 2448, 2523 | Trio never dirties the manifest | `adr_fill_would_change` must force full |
| Fill OOM / no-skill fails the job | `src/pipeline/pipeline.c` | 330-335 | NULL treated as fatal | Leave `*saved_adr`; job rc stays success |
| `cbm_pipeline_new` starts with fill on | `src/pipeline/pipeline.c` | 277 | Default flipped | Must stay false |
| Watcher cannot subscribe to a user job | `src/daemon/application.c` | 1583 | `adr_fill` kept in args-equal | Strip the key in `normalize_defaults` |
| Trio files appear as File nodes | `src/discover/discover.c` | 57, 351 | Dir not in ALWAYS_SKIP | `cbm_should_skip_dir(".sdd-skill")` |
| `adr_fill` listed on the MCP tool | `src/mcp/mcp.c` | 391-408 | Flag leaked into schema | Watcher-only; not a tool property |

### ADR fill helper (spec-004 Task #1; lines updated spec-013)

Unit-level (`cbm_adr_fill_document`). Pipeline still calls it after capture when `adr_fill` is true. Trio select is now XOR — see spec-013 Task #1 section above. NULL means neither skill directory (not “no sdd only”).

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| NULL return but `.sdd-skill/` exists and `.gamedev/` is not a dir | `src/adr/adr_fill.c` | 78-82, 274-276 | `adr_dir_present` false (empty `root_path` or not a dir) | `cbm_is_dir` on `root/.sdd-skill` (`:57-64`); `adr_join` rejects empty root (`:47-55`) |
| Unmarked `# Existing ADR` missing after fill | `src/adr/adr_fill.c` | 207-211 | Body already had both MANUAL markers | Only the MANUAL span is kept (`:197-223`); incomplete pair treats the whole body as manual (`:216-219`) |
| `# Purpose` / `# Stack` / `# Decisions` omitted | `src/adr/adr_fill.c` | 97-102 | Not a regular file, `fopen`/`fread` fail, or empty | `adr_read_extract` NULL → `adr_append_section` skips that H1 (`:156-157`) |
| `SECRET-DEVLOG` in generated | `src/adr/adr_fill.c` | 21-26 | Extra relative opened | Selected trio only; never DEV_LOG / GDD |
| Markers on a tree with no skill dir | `src/adr/adr_fill.c` | 274-276 | Select skipped | Must return NULL; do not write comments |
| Generated block after manual | `src/adr/adr_fill.c` | 247-263 | Splice order flipped | GENERATED start/end first, then MANUAL start/end |
| Skill dir, trio missing, no markers | `src/adr/adr_fill.c` | 274-279 | Returned NULL because extracts were empty | Skill present always splices; generated inner may be whitespace |
| Text past 1536 still in generated | `src/adr/adr_fill.c` | 122-129 | Cap not applied | First 1536 after 64KiB read; walk back to last newline |
| Unreadable ARCHITECTURE_ADR still copied | `src/adr/adr_fill.c` | 97-102 | Directory-at-path opened as a file | Not regular or `fopen` fail → omit `# Decisions` |
| NULL with skill dir + readable trio | `src/adr/adr_fill.c` | 242-244 | `malloc` failed in `adr_splice` | NULL is OOM; keep prior `existing` |
| Unreadable fixture still readable as root | `tests/test_adr_fill.c` | 107-120 | `chmod 0` ignored | Helper replaces the file with a directory at that path |

### Dashboard Reindex (spec-003 Task #5)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Reindex opens Graph | `graph-ui/src/components/Dashboard.tsx` | 47-73 | `onSelectProject` on the Reindex button | `reindexProject` only; Enter is line 256 (solo) / 141 (conflict) |
| POST has `project_name` or omits `project` | `graph-ui/src/components/Dashboard.tsx` | 53 | Wrong JSON key | `{ root_path: p.root_path, project: p.name }` |
| 202 does not show IndexProgress | `graph-ui/src/components/Dashboard.tsx` | 55-57, 81-83 | `setIndexing` skipped | On 202 set `indexing` true then `refresh` |
| 202 navigates to Graph | `graph-ui/src/App.tsx` | — | Reindex called `onPathExists` | Stay Dashboard; `onPathExists` is create 409 only |
| 500 drops the row | `graph-ui/src/components/Dashboard.tsx` | 60-69 | `refresh()` on error | Set `reindexError` only; do not refresh |
| 500 has no visible text | `graph-ui/src/components/Dashboard.tsx` | 76, 104-107 | Error not in `listError` | `error \|\| reindexError`; `role="alert"` |
| Conflict member has no Reindex | `graph-ui/src/components/Dashboard.tsx` | 160-167 | Only solo card got the control | Each member `aria-label={t.projects.reindex}` |
| `getByRole` cannot find Reindex | `graph-ui/src/lib/i18n.ts` | 51 | Copy drifted | en `"Reindex"`; zh line 150 `"重新索引"` |
| Reindex on workspace header | `graph-ui/src/components/WorkspaceHeader.tsx` | — | Control added to chrome | Header is leave + name + time only |
| Create modal has Reindex or sends `project` | `graph-ui/src/components/CreateIndexModal.tsx` | 124 | Reindex leaked into create | `{ root_path: path }` only; no Reindex button |

### Create 409 redirect (spec-003 Task #4)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| 409 `path_exists` starts IndexProgress | `graph-ui/src/components/CreateIndexModal.tsx` | 127-133 | 409 treated as 202 | `onPathExists` + `onClose`; never `onCreated` |
| 409 `path_exists` stays on Dashboard | `graph-ui/src/App.tsx` | 142 | `onPathExists` not wired | Pass `openExistingProject`; Dashboard 206-209 must forward the name |
| Graph opens but no status notice | `graph-ui/src/App.tsx` | 55-62 | Route used `navigate` | `openExistingProject` sets `pathNotice` then `setRoute`; `navigate` clears it (line 44) |
| Notice still visible after Back | `graph-ui/src/App.tsx` | 44 | Leave did not clear | `navigate` and popstate (line 36) set `pathNotice` null |
| 409 `name_exists` opens Graph | `graph-ui/src/components/CreateIndexModal.tsx` | 127 | Code treated as `path_exists` | Only `path_exists` + `existing_project`; else throw `data.error` (line 132) |
| Create body has `project` / `project_name` | `graph-ui/src/components/CreateIndexModal.tsx` | 124 | Reindex key leaked | `{ root_path: path }` only |
| Listed Path still POSTs `/api/index` | `graph-ui/src/components/CreateIndexModal.tsx` | 112-116 | Skip missed or list not passed | `findNewestForPath`; Dashboard must pass `existingProjects={projects}` (line 210) |
| Skip opens the older clone | `graph-ui/src/lib/pathGroups.ts` | 47-55 | First row used | `pickNewest` — `indexed_at` then greater `name` |
| Status text lacks the project name | `graph-ui/src/lib/i18n.ts` | 72 | Copy not interpolated | `Already indexed as ${name}` / zh line 168 |
| Notice has no `role="status"` | `graph-ui/src/App.tsx` | 123 | Wrong or missing role | Must be `role="status"` |

### Dashboard conflict groups (spec-003 Task #3)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Two same-folder names stay as solo cards | `graph-ui/src/lib/pathGroups.ts` | 36-37 | Grouped on stored `root_path` while slash/symlink spellings differ | Use `canonical_root` when the list sent it |
| Enter opens the older clone | `graph-ui/src/lib/pathGroups.ts` | 17-33 | Newest pick drifted from C | `indexed_at` string; tie → greater `name`; Dashboard Enter uses `group.newest.name` (line 141) |
| Path repeats next to every clone | `graph-ui/src/components/Dashboard.tsx` | 136-137 | Member rows still print `root_path` | Path once on the group header |
| One control deletes two names | `graph-ui/src/components/Dashboard.tsx` | 168-175 | Bulk-delete button added | Per-name `deleteProject(p.name)` only |
| Delete cancel still DELETEs | `graph-ui/src/components/Dashboard.tsx` | 38-40 | `confirm` skipped | Same helper as solo rows; return before `fetch` |
| Confirm-yes hits the wrong URL | `graph-ui/src/components/Dashboard.tsx` | 40 | Query key changed | `DELETE /api/project?name=<that name>` |
| Conflict region has no name | `graph-ui/src/lib/i18n.ts` | 50 | `projects.conflict` missing | en "Path conflict"; `aria-label={t.projects.conflict}` |
| Cannot find Delete a1 | `graph-ui/src/lib/i18n.ts` | 48 | Solo `deleteTitle` reused | Conflict uses `deleteNamed(name)` |

### Admit create vs reindex (spec-003 Task #2)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Bare POST `/api/index` returns 202 on an owned Path | `src/ui/http_server.c` | 1232 | Admit skipped or cache not the test/prod dir | Set `CBM_CACHE_DIR`; 409 must fire before a job slot |
| 409 JSON lacks `code` / `existing_project` | `src/ui/http_server.c` | 1108 | Generic error reply | `path_exists` or `name_exists` + newest name |
| Trailing slash indexes as a new project | `src/store/identity_catalog.c` | 254 | Path not canonicalized | Same `canonical_root` as the owner |
| `project_name` created a new `.db` | `src/ui/http_server.c` | 1192 | Treated as create | Alias is reindex key only; unknown name → `name_exists` |
| MCP no-name cloned the Path | `src/mcp/mcp.c` | 7906 | Worker args omitted bind `name` | Admit REINDEX copies owner into args |
| MCP `isError` check always true | `tests/test_identity.c` | `id_mcp_is_error` | Key exists on success | Require `"isError":true` |
| Second create 202 while first job runs | `src/ui/http_server.c` | 1226 | In-flight snapshot empty | Collect status==1 jobs; CREATE + same Path → 409 |
| Two daemon jobs on one folder, two names | `src/daemon/application.c` | 1616 | Subscribe by name only | PATH_CONFLICT when Path matches and `project_key` differs |

### Path identity catalog (spec-003 Task #1)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Two same-folder rows have different `canonical_root` | `src/foundation/identity.c` | 20 | Folder missing; realpath failed | Fallback is stored `root_path`; make the dir exist or wait for Task #3 to group on the field as sent |
| Newest pick is `alpha` when times match `beta` | `src/foundation/identity.c` | 36 | Tie uses greater name | `strcmp(name)`; `beta` wins over `alpha` |
| Catalog skips a `.db` | `src/store/identity_catalog.c` | 73 | `_` prefix, ghost file, or no primary project row | `::missed` shadows do not count as the sole name |
| `list_projects` lacks `indexed_at` / `canonical_root` | `src/mcp/mcp.c` | 2545 | Old binary | Rebuild daemon; both fields are always emitted |
| UI `root_path` was rewritten to realpath | `src/mcp/mcp.c` | 2541 | Display path mutated | Keep stored `root_path`; only `canonical_root` resolves |

### AdrTab (spec-002 Task #4)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Save 500 looks successful | `graph-ui/src/components/AdrTab.tsx` | persist `res.ok` | Status updated before HTTP check | On `!res.ok` set `role=alert`, do not touch `lastClean` |
| Empty open POSTs placeholder | `graph-ui/src/components/AdrTab.tsx` | textarea | Placeholder copied into `value` | `has_adr:false` → value `""`; placeholder is attribute |
| Delete does not POST `""` | `graph-ui/src/components/AdrTab.tsx` | Delete button | Still using AdrButton modal | `persist("")`; do not assert GET `has_adr:false` |
| Dirty leave not confirmed | `graph-ui/src/App.tsx` | Task #5 | `onDirtyChange` not wired | `window.confirm(t.adr.unsavedConfirm)` |

### Workspace header (spec-002 Task #2 / spec-007 Task #2)

Listed clock is `formatIndexedAt` (local Intl). `dateTime` / `title` stay raw ISO. Ghost still omits every `time`. Invalid `not-a-date` stays raw. No `/api/index-status`.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Ghost shows a `time` | `graph-ui/src/components/WorkspaceHeader.tsx` | 21, 28 | Name matched a list row by accident, or index-status called | `projects.find` by exact `name`; omit `time` when miss. Test `WorkspaceHeader.test.tsx:97-105` |
| Listed text !== `formatIndexedAt` | `graph-ui/src/components/WorkspaceHeader.tsx` | 31-32 | Header bypassed helper or hardcoded UTC | Same helper as Dashboard. Test `:76-78` |
| `dateTime` / `title` not the raw ISO | `graph-ui/src/components/WorkspaceHeader.tsx` | 31 | Attributes used helper output | Instant stays raw. Test `:76-77` |
| Listed `not-a-date` invented a clock | `graph-ui/src/components/WorkspaceHeader.tsx` | 31-32 | Formatter skipped NaN fallback | Text and `dateTime` must be `not-a-date`. Test `:83-95` |
| Listed name has no clock | `graph-ui/src/hooks/useProjects.ts` | list fetch | List still loading or `indexed_at` missing | Wait for `list_projects`; same `<time dateTime>` as Dashboard |
| Leave has no accessible name | `graph-ui/src/lib/i18n.ts` | 32 | `backToDashboard` missing | `aria-label={t.graph.backToDashboard}` |

### Route kernel (spec-002 Task #1)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| `?tab=specs&project=` reads as Dashboard | `graph-ui/src/lib/route.ts` | 23-24 | `WORKSPACE_TABS` missing specs or App inlined old parse | `isWorkspaceTab` + truthy project; App imports `readRoute` |
| `?tab=adr` without project stays adr | `graph-ui/src/lib/route.ts` | 23-26 | Empty-project guard dropped | Return Dashboard + `project: null` |
| `?tab=stats` keeps `project=` | `graph-ui/src/lib/route.ts` | 26 | Alias returned the query project | Dashboard always clears project |
| Duplicate parse in App | `graph-ui/src/App.tsx` | 11 | Local `readRoute` brought back | Import from `./lib/route` only |

### App routing / header IA

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Wrong default tab (Specs / tab strip) | `graph-ui/src/App.tsx` | 40, 72-77 | `SpecBoardTab` / `TabBar` remounted | Only `graph` + project paints GraphTab this task; else Dashboard. Header is brand only — no Specs/Graph/Projects/Control buttons |
| `?tab=stats` bookmark not Dashboard | `graph-ui/src/lib/route.ts` | 23-26 | Alias missing; `replaceState` skipped | `stats`/`control`/unknown/workspace-without-project → `{ tab: "dashboard", project: null }` |
| Graph without project still opens Graph | `graph-ui/src/lib/route.ts` | 23-24 | Guard dropped (`&& project`) | Missing/empty `project` is Dashboard; `showGraph` needs both tab and name |
| Back leaves `project=` on home | `graph-ui/src/lib/route.ts` | 29-33 | Back passed a name, or `routeUrl` always sets `project` | `navigate("dashboard", null)`; set `project` only when truthy |
| Back has no accessible name | `graph-ui/src/lib/i18n.ts` | 32, 121 | `backToDashboard` missing | en "Back to Dashboard"; zh "返回仪表盘"; `aria-label={t.graph.backToDashboard}` |
| Specs picker appears on load | `graph-ui/src/App.tsx` | 72-77 | `SpecBoardTab` routed again | File may stay on disk; App must mount Dashboard or GraphTab only until Task #5 |

### Dashboard (list + Control)

List and conflict member clocks are `formatIndexedAt` (local Intl). `dateTime` / `title` stay raw ISO. Invalid `not-a-date` stays raw. Enter newest is still ISO-compared (`alpha`). No UTC pin on visible text.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Row text !== `formatIndexedAt` | `graph-ui/src/components/Dashboard.tsx` | 248-249 | Surface bypassed helper or hardcoded UTC | Children must be `formatIndexedAt`. Test `Dashboard.test.tsx:113` |
| Conflict member text !== helper | `graph-ui/src/components/Dashboard.tsx` | 154-155 | Member row used a different clock | Same helper. Test `:448-453` |
| `dateTime` / `title` not the raw ISO | `graph-ui/src/components/Dashboard.tsx` | 154, 248 | Attributes used helper output | Instant stays raw. Tests `:111-112` / `:448-452` |
| `not-a-date` invented a clock | `graph-ui/src/components/Dashboard.tsx` | 248-249 | Formatter skipped NaN fallback | Text and `dateTime` must be `not-a-date`. Test `:135-150` |
| Enter opens the older clone | `graph-ui/src/lib/pathGroups.ts` | 17-26 | Display TZ leaked into newest | ISO string then name. Enter `:141`. Test `:455-457` |
| List error banner, Control missing | `graph-ui/src/components/Dashboard.tsx` | 104-107, 198-200 | Early return after `error` skipped Control | Error is an inline `role="alert"`; `ControlTab embedded` stays after the list |
| Empty CTA, no Control | `graph-ui/src/components/Dashboard.tsx` | 110-119, 198-200 | Empty state replaced the page | CTA only; Control is always below the top border |
| Graph opens after index 202 | `graph-ui/src/components/Dashboard.tsx` | 205 | `onCreated` called `onSelectProject` | Only `setIndexing(true)` + `refresh()`; Graph open is `onPathExists` only (409 / skip) |
| Index POST body has `project_name` | `graph-ui/src/components/CreateIndexModal.tsx` | 124 | Leftover Project ID state | `JSON.stringify({ root_path: path })` only |
| Index POST fail closes modal | `graph-ui/src/components/CreateIndexModal.tsx` | 132 | Success path ran on 400 / `name_exists` | Throw `data.error`; modal stays; show body text |
| Delete cancel still DELETEs | `graph-ui/src/components/Dashboard.tsx` | 38-40 | `confirm` not checked | `if (!confirm(...)) return` before `fetch` |
| Control missing on Dashboard | `graph-ui/src/components/Dashboard.tsx` | 198-200 | `embedded` mount removed | `<ControlTab embedded />` after the list, inside the same ScrollArea |
| Control clipped / polls stop | `graph-ui/src/components/ControlTab.tsx` | 220-222 | Nested page ScrollArea or `embedded` false | Embedded returns `{body}` only; polls stay 3s / 2s |
| IndexProgress never shows | `graph-ui/src/components/IndexProgress.tsx` | 28-34 | `indexing` false, or `[]` treated as done | Empty status list keeps polling; do not `onDone` on `[]` |
| ADR button on a row | `graph-ui/src/components/Dashboard.tsx` | 15-18 | `AdrButton` imported | Dashboard imports HealthDot + IndexProgress only; Reindex is a row button, not ADR |

### Project list (useProjects)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| List stays empty after a live daemon | `graph-ui/src/hooks/useProjects.ts` | 28-31 | RPC/HTTP fail or `result.projects` missing | Check `POST /rpc` `list_projects`; missing array becomes `[]` |
| Non-empty error, projects `[]` | `graph-ui/src/hooks/useProjects.ts` | 30-31 | `callTool` threw | Read `error` string; HTTP non-OK or JSON-RPC `error` |
| Spy / Network shows `get_graph_schema` on list paint | `graph-ui/src/hooks/useProjects.ts` | 28 | Hook or consumer still names that tool | Must be `list_projects` only (SDD-ADR-006) |
| Refresh does nothing | `graph-ui/src/hooks/useProjects.ts` | 41 | Caller did not invoke `refresh` | `refresh` is `fetchProjects` |

### Specs picker (SpecBoardTab)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Picker empty with projects in cache | `graph-ui/src/components/SpecBoardTab.tsx` | 24-32 | Hook loading/error, not schema | Inspect `useProjects` — picker is name+path only |
| Click does not select | `graph-ui/src/components/SpecBoardTab.tsx` | 38 | `onSelectProject` not wired | Parent must pass `(project: string) => void` |

### Chrome tokens (globals.css / shell)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Wrong chrome color (header/buttons teal) | `graph-ui/src/styles/globals.css` | 18-30 | `--color-primary` / `--color-accent` / `--color-ring` back to `#1DA27E` / `#1C8585` | Restore gray tokens; `chrome-tokens.test.ts` forbids those hex in `@theme` |
| Header / modal still teal-black | `graph-ui/src/App.tsx` | 80 | leftover `bg-[#0b1920]` / `bg-[#0e2028]` | Use `bg-card` (also GraphTab:402, NodeDetailPanel:122, DisplaySettingsMenu:108) |
| Surfaces look the same (no card vs page) | `graph-ui/src/styles/globals.css` | 12-28 | background / card / hover / border not distinct | Four different hex; test asserts set size 4 |

### Graph palette (must stay colorful)

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Graph nodes went gray | `graph-ui/src/lib/colors.ts` | 19-20 | `colorForLabel` imported CSS vars or Function hex "aligned" to primary | Function must be `#06b6d4`; do not pass `--color-primary` into the map |
| CALLS edges went gray | `graph-ui/src/components/EdgeLines.tsx` | 35, 61-64 | graph map edited to match chrome | `GRAPH_EDGE_PALETTE.CALLS` = `#1DA27E`; default = `#1C8585` |
| Loader constellation went gray | `graph-ui/src/styles/globals.css` | 52-60 | `#22d3ee` replaced by a chrome token | Keep hardcoded loader cyan |

### Control Gauge / HealthDot

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Gauge lost red / amber | `graph-ui/src/components/ControlTab.tsx` | 14-17 | `gaugeFillColor` always returns gray | `>80` → `#e05252`; `>50` → `#eab308`; else `#a3a3a3` |
| Healthy gauge still teal | `graph-ui/src/components/ControlTab.tsx` | 17 | fill still `#1DA27E` | Healthy is `#a3a3a3` (chrome); do not reuse CALLS teal |
| HealthDot went gray or teal | `graph-ui/src/components/HealthDot.tsx` | 48-51 | semantic hex replaced by `text-primary` | Keep `#34d399` / `#fbbf24` / `#f87171` / `#555` |

### Last indexed (`formatIndexedAt`)

Visible text is runtime-local Intl (SDD-ADR-034). Do not pin `timeZone: "UTC"`. `dateTime` / `title` stay raw ISO (Task #2 landed — Dashboard / header / AdrTab). Invalid still returns raw. Host-UTC: `localFmt === utcFmt` is OK. Newer rows: spec-007 Task #2 + Task #1 sections above.

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Visible text equals raw ISO | `graph-ui/src/lib/formatIndexedAt.ts` | 26-29 | `Date` parse failed (`Invalid Date`) | Invalid/empty is supposed to return the raw string; check `indexed_at` from `list_projects` |
| Visible text still UTC while host is not | `graph-ui/src/lib/formatIndexedAt.ts` | 16-23, 30 | `timeZone` leaked back into `INDEXED_AT_PARTS` | Options must omit `timeZone`. Runtime default. Test `formatIndexedAt.test.ts:23-35` |
| Host UTC looks "still pinned" | `graph-ui/src/lib/formatIndexedAt.test.ts` | 49-60 | `localFmt === utcFmt` treated as a fail | Allowed Limit Case. `dateTime` / `title` stay raw ISO (Dashboard.test.tsx:111) |
| Time stays English on zh UI | `graph-ui/src/lib/formatIndexedAt.ts` | 11-14, 30 | `lang` not passed into the helper | Callers pass `lang`; locale is `en-US` / `zh-CN`, not a TZ pick |

### Tests

| Error | File | Line | Common Cause | Quick Fix |
|---|---|---|---|---|
| Default URL not `?tab=dashboard` | `graph-ui/src/App.test.tsx` | 102-113 | `replaceState` missing or still writes specs | After render, search is `?tab=dashboard` and `project` is absent |
| Back URL still has `project=alpha` | `graph-ui/src/App.test.tsx` | 130-135 | `navigate("dashboard", name)` | After back, search is `?tab=dashboard` and must not contain `project=alpha` |
| GraphTab boots Three | `graph-ui/src/App.test.tsx` | 15-19 | `vi.mock("./components/GraphTab")` missing | Mock returns `data-testid="graph-tab"` |
| `get_graph_schema` length ≠ 0 | `graph-ui/src/hooks/useProjects.test.ts` | 83, 90, 111 | Unexpected RPC tool name | Grep `get_graph_schema` in graph-ui |
| `indexed_at` undefined | `graph-ui/src/hooks/useProjects.test.ts` | 79-80 | Mock payload omitted field | List payload must include ISO `indexed_at` |
| Function ≠ `#06b6d4` | `graph-ui/src/lib/colors.test.ts` | 26-27 | `LABEL_COLORS` drifted | Restore hex; never import CSS vars into `colors.ts` |
| `@theme` still has teal primary | `graph-ui/src/lib/chrome-tokens.test.ts` | 36-42 | `globals.css` rewrite missed | Primary/accent/ring must not be `#1DA27E` / `#1C8585` |
| CALLS drifted | `graph-ui/src/lib/chrome-tokens.test.ts` | 77-79 | EdgeLines map edited | Export-only lock; do not change values |
| 202 calls `onSelectProject` | `graph-ui/src/components/Dashboard.test.tsx` | 211-233 | Create wired navigate | `onCreated` must not call `onSelectProject` |
| Polls = 1 after 3.5s | `graph-ui/src/components/Dashboard.test.tsx` | 183-200 | Fake timers not advanced | `advanceTimersByTimeAsync(3500)`; expect processes/logs ≥ 2 |

## Debugging by Module

### If Game archive hide, Show archived, or refresh unmounts Game (`GameBoardTab.tsx` / `useGameBoard.ts`)

1. Filter: `visiblePhaseCards` (`GameBoardTab.tsx:60-62`) hides `work_state==="done" && archived && !showArchived`. Inbox uses `board.inbox` unfiltered (`:441`). Leftover pending archived stays; no Archive/Unarchive (`:64-67`). Tests `:1272` / `:1357` / `:1419`.
2. Show archived is pane chrome (`:423-432`), not Inbox header, not a fourth column. `aria-pressed` false on mount. Reset with `expandedIds` on `project` change (`:378-382`). GET refetch same project keeps the toggle (`:1479`). No localStorage.
3. Archive/Unarchive only on expanded done artifacts (`:64-67`, `:247-253`). POST `/api/game-board` `{project, card_id, archived}` then `await refresh()` (`:393-406`). No `window.confirm`. Tests `:1233` / `:1309`.
4. `useGameBoard.refresh` (`:160-176`) is the same GET. Must not set `settled`/`present` false. 500 keeps the pane. Tests `useGameBoard.test.ts:270` / `:309`.
5. App passes `project` + `refresh` (`App.tsx:170`). Empty project skips persist (`GameBoardTab.tsx:394`).
6. All-archived phase column = header only; Show archived stays in chrome. Test `:1385`.
7. Silent-win leftovers stay: no `/api/skill-presence`; Enter Graph; leftover `tab=specs` + gamedev → `tab=game`. Function `#06b6d4`. Tests `App.test.tsx:1164` / `:1185` / `:1199`; `GameBoardTab.test.tsx:1512`.
8. Reference: `human/task-summaries/task-5-archive-ui-refresh-remaining-vitest.md`

### If tab=game, route kernels, or showGame strip fails (`route.ts` / `WorkspaceTabStrip.tsx`)

1. Closed set: `WORKSPACE_TABS` at `types.ts:93` is `["graph","specs","adr","game"]`. `readRoute` (`route.ts:25-26`) keeps `tab=game` only with a non-empty project. Test `route.test.ts:24-26` / `:38-39`. `routeUrl("game", name)` `:31-36` / test `:72`.
2. `fallbackSpecsToGraph(tab, present)` signature must stay (`route.ts:40-43`). `game` + !present stays `game` (that kernel does not own Game). Test `:75-81`.
3. `fallbackGameToGraph` (`:47-50`): `tab === "game" && !present` → `"graph"`. Test `:84-90`.
4. `resolveWorkspaceTab` (`:54-61`): specs + `gamePresent` → `"game"` first, then Game kernel, then Specs kernel. Test `:93-101`. App passes settled-aware `showSpecs` / `showGame` (`App.tsx:108`, `:125-128`).
5. Strip: `showGame` → `["graph","game","adr"]` (`WorkspaceTabStrip.tsx:37-41`). Accessible name `t.tabs.game` (`:24`, i18n `:18`). Both flags → Game wins (`:117-130`). Omit when false — no disabled placeholder (`:94-114`).
6. i18n: `tabs.game` / `gameBoard.stateMdMissing` / three EN phase labels (`i18n.ts:18`, `:118-122`). Inbox + work-state keys are spec-011 Task #3 (`:123-127`). Lock `i18n.test.ts:87-106`.
7. App silent-win: `showGame` / `showSpecs` (`App.tsx:35-36`). Dual fetch + restore is the Task #4 Common Error Index.
8. Reference: `human/task-summaries/task-2-tabid-route-kernels-strip.md`

### If Game clipboard, toast, drag, expand, or parse skip fails (`GameBoardTab.tsx` / `useGameBoard.ts`)

1. Card continue is a `<button type="button">` named with the card `continue` string (`GameBoardTab.tsx:73-86`). Artifact copies `/gamedev-skill continue @role`. Inbox copies `/gamedev-skill continue`. Tests `:459-485` / `:491-518`.
2. `copyContinueText` (`:57-71`): `writeText` first. Missing API or throw → `selectNodeContents` on that button. No toast / Copied / `role="status"` either path. Tests `:521-580`.
3. Chrome continue stays `<p className="... select-text">` (`:177-179`). Empty-board tests still see zero buttons (`:153`, `:183`).
4. Card activate: no Archive / Unarchive / "No tasks planned yet"; no POST (`:88-122`). Test `:583-610`.
5. `draggable={false}` on both card articles (`:92`, `:107`). Drop must not move. Test `:612-640`.
6. `parseGameBoard` skips objects whose `kind` is not `artifact`|`epic` (`useGameBoard.ts:36`, `:50-57`). One-shot stays (`:89-130`). Tests `useGameBoard.test.ts:132-205`.
7. Reference: `human/task-summaries/task-4-clipboard-no-drag-typed-card-parse.md`

### If Game columns, cards, aria-current, or Inbox E fail (`GameBoardTab.tsx`)

1. Four headers always, document order Inbox then Pre-production then Production then Post-production & Launch (`GameBoardTab.tsx:18`, `:27-32`, `:181-192`). Empty column = header + 0 `article`s; no "no artifacts in this phase"; no `opacity`/`hidden`. Test `GameBoardTab.test.tsx:213-237` / `:356-372`.
2. `aria-current="true"` only on the phase column matching GET `phase` (`:34-39`, `:138`). Inbox never. `phase` null → no column current. Tests `:232-235` / `:374-393`.
3. Artifact card: id, title, track A/B/H, work-state EN, owner, continue control. No letter E (`:88-103`). Inbox card: literal `E` via `text-[var(--color-epic-mark)]`, title, summary, plan_title, id, continue without `@role`. No track. No Pending (`:105-122`). Tests `:240-267` / `:326-354`.
4. Work-state EN from i18n (`:41-50`; `i18n.ts:124-127`). `in_progress` → "In progress" (includes C `ready` map). Tests `:322-323` / `:419-441` / `i18n.test.ts:99-102`.
5. Chrome continue stays `<p className="... select-text">` (`:177-179`). Card continue is the Task #4 button (section above). No `SpecBoardTab` / `EpicCard` import.
6. Reference: `human/task-summaries/task-3-gameboardtab-columns-cards-i18n.md`

### If GET /api/game-board 400/404/phase/bytes fails (`game_board.c` / `http_server.c`)

1. 400 / 404 strings must match spec-board: missing query `http_server.c:533-536` (`test_httpd.c:4003`); unknown name `:540-542` (`:3982`).
2. Present: `cbm_spec_board_gamedev_skill_present` (`spec_board.c:1096-1102`) from `game_board.c:638`. File-at-path or missing → false, `continue` `""`, no phase walk (`:638-640`). Empty dir is still true (`test_game_board.c:107`).
3. Parse: compact `phase=` / `focus=` first (`:211-223`). Aliases `active_phase` / `director_focus` as `=` or line-start `:`. Compact wins. Unknown token → phase null (`:229-231`).
4. JSON: grow `cbm_game_board_to_json` (`:743`). Chrome keys unchanged. Phase arrays fill when artifacts exist; `inbox` fills when `.grill/` has unconverted epics (Task #2). No English labels. No `has_more` / `column`. spec-board `to_json` (`spec_board.c:1215`) must not grow `gamedev_skill_present` (`test_httpd.c:3975`). UI paint is spec-011 Task #3.
5. Zero-write: fopen `"rb"` (`game_board.c:89`). Snapshot `state.md`; `.sdd-skill/` and `.grill/` must not be created (`test_httpd.c:4024`).
6. Dispatch is GET only (`http_server.c:2295-2298`). No POST. No MCP tool.
7. Artifact walk / owner / cap: spec-011 Task #1 Common Error Index above. Reference: `human/task-summaries/task-1-c-artifact-walk-widen-card-json.md`

### If Specs strip, deep-link, Enter, or omit fails (`App.tsx`)

These steps now depend on `game.settled` (spec-010 Task #4 Common Error Index). After game-board settles !present, grill-only behavior below still holds.

1. Strip membership: `showSpecs = gameSettled && !showGame && specsPresent` (`App.tsx:36`). `specsPresent` is Task #1 OR (`useSddSkillPresent.ts:17`). Grill-only 200 (game default false) must show tab `"Specs"`, order Graph then Specs then ADR. Test `App.test.tsx:651`.
2. Omit-until-true: `resolveWorkspaceTab` → `fallbackSpecsToGraph` (`route.ts:40-43`, `:54-61`) still rewrites `tab=specs` while Specs is omitted — including spec-board hang (`App.test.tsx:331`) and GET 500 (`:450`). In-flight game-board omits Specs too (`:876-891`).
3. Deep-link restore: inbound `tab=specs` is captured (`App.tsx:39`). When `showSpecs` becomes true, `replaceRoute("specs", …)` (`:101-107`). If `showGame`, same pending goes to `tab=game` (`:94-99`). Grill-only must keep `tab=specs` and SpecBoardTab. Test `:672`.
4. neither-skill: both spec flags false (`mockAppFetch` default / `"false"`) and game default false (`:118`). Strip is Graph then ADR. `?tab=specs` stays rewritten to Graph. Tests `:341` / `:695`.
5. Enter: `onSelectProject` is `navigate("graph", p)` (`App.tsx:179`). Grill-only Enter is still GraphTab + Specs in the strip. Test `:719-727`.
6. gamedev present: Game shown, Specs omitted (Task #4). Without gamedev, mock `gameBoard: "false"` (`:118`). Do not read `.gamedev/` in graph-ui.
7. GraphTab mock must stay (`App.test.tsx:16-20`). `useSddSkillPresent` still does not call `/api/skill-presence`.
8. Reference: `human/task-summaries/task-3-app-strip-deep-link-enter.md` / `task-4-app-silent-win-dual-fetch.md`

### If grill-only Kanban, notSddSkill last-resort, or loading copy fails (`SpecBoardTab.tsx`)

1. Gate: `if (!board || !(board.sdd_skill_present === true || board.grill_skill_present === true))` (`SpecBoardTab.tsx:345`). Grill-only must paint. Test `:372` (epic in Todo, noSpecs in spec columns, no notSddSkill).
2. Empty grill-only: `epics: []` + `specs: []` still Kanban — three `noSpecs`, not notSddSkill. Test `:404`.
3. Last-resort: `board` null after load (`:427`) or both flags false (`:443`) → `t.specBoard.notSddSkill`. No Todo heading, no epic id.
4. Loading: `loading && !board` (`:340-342`) → `t.common.loading`. Must not be notSddSkill. Test `:338`.
5. sdd-true missing `grill_skill_present` still paints (`:355`). Strict `=== true`.
6. Strip hook is the same OR (`useSddSkillPresent.ts:17`). App strip / `?tab=specs` / Enter is spec-009 Task #3 (section above).
7. Reference: `human/task-summaries/task-2-specboardtab-host-kanban-grill-only.md`

### If remaining UI Gherkin (two-plan order, Has more, omit mock, column isolation) fails (`SpecBoardTab.test.tsx`)

1. Two-plan order: mock `epics: [planAFirst, planASecond, planBOther]` then planned (`:911-916`). Product concat is `todoEpics.map` then `entries.map` (`SpecBoardTab.tsx:267-278`). Assert id texts `:920-925`. C already sorted index.md then NNN — UI must not reshuffle.
2. 65th / Has more: mock exactly 64 epics (`sixtyFourEpics` `:193-203`). No button/link/text named "Has more" (`:960-962`). Cap omit is C; UI must not invent overflow chrome.
3. Companion-to omit: pass `epics: []` (`:878-883`). UI does not fopen spec.md or re-match. Todo still shows the planned spec (`:887-889`).
4. Done / In progress isolation: Done-claim mock is `epics: []` (`:893-898`). Done shows `closed.id`; neither Done nor In Progress contains the epic path (`:902-906`). Product: only Todo gets `board.epics ?? []` (`SpecBoardTab.tsx:238`, `:362`, `:371-393`).
5. Host gate (spec-009 Task #2): grill-only paints Kanban (`:372`). both-false / `!board` → notSddSkill (`:443` / `:427`). No `selectProject` when project is set (`:322` / `:338`). Loading is not notSddSkill (`:338`).
6. Function hex: `colorForLabel("Function") === "#06b6d4"` (`:963` and `colors.test.ts:27`).
7. Reference: `human/task-summaries/task-4-vitest-gherkin-grill-epic.md`

### If EpicCard paint, Todo order, or grill-without-sdd fails (`SpecBoardTab.tsx`)

1. E token: `--color-epic-mark: #7d8ec9` at `globals.css:28`. Letter class `text-[var(--color-epic-mark)]` (`SpecBoardTab.tsx:104`). Literal `"E"` — not i18n, not "Epic", not a pill. Hex is not destructive `#e05252`, not Function `#06b6d4`, not CALLS `#1DA27E`. Test `SpecBoardTab.test.tsx:819-823`.
2. Todo order: Column `id === "todo"` uses `epics` (`:238`); map EpicCard then SpecCard (`:267-278`). `pendingCount` is both (`:239`, `:258`). In progress / Done omit `epics` (`:371-393`). Tests `:828` / `:831-832`. Two-plan Gherkin is Task #4 (`:911`).
3. No expand / archive / POST: EpicCard is a `<div>` (`:100-111`). Click must not add a title button, `noTasksYet`, or Archive. `persistArchive` stays SpecCard (`:321-334`). Test `:837`. Spec expand + Done Archive still work with epics present (`:860`).
4. Missing `epics`: `SpecBoard.epics?` (`types.ts:143`); host `board.epics ?? []` (`:362`). Old `{ sdd_skill_present, specs }` must not throw. Test `:373`.
5. Host gate (spec-009 Task #2): `if (!board || !(sdd === true || grill === true))` (`SpecBoardTab.tsx:345`). Grill-only must paint. Last-resort both-false / `!board`. Loading (`:340-342`) stays `t.common.loading`. Strip hook `useSddSkillPresent.ts:17` is the same OR. See host section above.
6. Live :9749 with no epic cards: the running binary may predate spec-008 (GET body has no `epics`). Rebuild `scripts/build.sh --with-ui`. Vitest mocks the board.
7. Remaining UI Gherkin: Task #4 section above. Reference: `human/task-summaries/task-3-epiccard-todo-order.md` / `task-4-vitest-gherkin-grill-epic.md`

### If archive hide, Show archived, or Archive/Unarchive fails (`SpecBoardTab.tsx`)

1. Session toggle: `showArchived` at `SpecBoardTab.tsx:267`. Default false. Project change resets it with `expandedIds` (`:273-278`). Remount starts unpressed. Do not write localStorage. Tests `:434` / `:447` / `:463`.
2. Done filter: `byColumn` (`:323-328`) omits `done && isArchived && !showArchived`. Count is the filtered length (`:237-238`). Leftover Todo `archived` true still paints (`:497`). Missing field is false (`:20-22`, test `:507`).
3. Archive chrome: `showArchive` / `showUnarchive` (`:115-116`) only when expanded Done. POST `{project, spec_id, archived}` then `await refresh()` (`:296-305`). No confirm. Tests `:519` / `:544`.
4. Refresh: `useSpecBoard` `refresh` is `fetchBoard` (`useSpecBoard.ts:15`, `:59`). `!res.ok` leaves GET as truth (`SpecBoardTab.tsx:304`). Poll-after-archive: later GET with `archived: true` must stay hidden while toggle off (`SpecBoardTab.test.tsx:612`).
5. Empty Done: all archived + hide on → `t.specBoard.noSpecs` + count 0; toggle stays in the Done header (`:225-236`, `:244`). Test `:487`.
6. Expand Set / blurb / Todo pending-only: unchanged spec-005 (next section).
7. Live UI was not clicked — Vitest only. Reference: `human/task-summaries/task-3-specboard-archive-filter.md` / `task-4-vitest-gherkin-archive.md`

### If spec-card expand, poll persist, Todo filter, or Archive leaks (`SpecBoardTab.tsx`)

1. Set lifetime: `expandedIds` at `SpecBoardTab.tsx:266`. Project change clears + unseeds (`:273-278`). First board seeds active ids (`:282-285`, `seedActiveIds` `:24-30`). Poll must not reset — `seededRef` stays true. Test `:370`.
2. Toggle: title `<button>` only (`:124-136`). `canExpand = true` (`:112`) including `task_count === 0`. `onToggle` adds/removes one id (`:287-294`). Opening one card does not remove another. Multi-expand `:315`.
3. Blurb: `entry.blurb ?? ""` (`:113`). Region only when non-empty (`:164-168`, `data-region="blurb"`). Empty ES / missing field → no region. Tests `:332` / `:345`.
4. TaskList: Todo pending-only (`:67`). In Progress / Done pass every task. Zero visible → `t.specBoard.noTasksYet` (`:69`, `i18n.ts:111`). Tests `:269` / `:282` / `:358`.
5. Chrome: agent / N/M / checklist / blocked stay outside `{expanded &&}` (`:138-160`). Collapsed active still shows implementer + `1/2 tasks` (`:412`).
6. Archive on Done expand is spec-006 Task #3 (section above). Archive on Todo / In Progress is still a leak (`:115-116`, test `:497`).
7. Multi-expand: toggle add/remove (`:287-294`). Both blurbs `:315`. Poll persist `:370`.
8. Live UI was not clicked — Vitest only. Reference: `human/task-summaries/task-3-speccard-expand-set-filter.md` / `task-3-specboard-archive-filter.md`

### If GET additive JSON, POST epic-id 404, or skill-tree bytes change (`http_server.c`)

1. Dispatch: `handle_spec_board_get` (`http_server.c:487`) is read (`:507`) → specs-only merge (`:508`) → `to_json` (`:510`). There is no `fopen` / `opendir` of `.grill/` in this file. Grill parse is `cbm_spec_board_read`. Test `test_httpd.c:3675`.
2. Unknown project: `resolve_project_root_path` miss → 404 `{"error":"project not found"}` (`:496-498`). Same string as POST unknown project. Test `:3804`.
3. POST epic id: `spec_board_find` loops `spec_count` only (`:531-543`). Epic path → NULL → 404 `{"error":"spec not found"}` (`:626-630`) before `cbm_store_spec_archive_set` (`:656`). Do not invent `"epic not found"`. Test `:3745`.
4. Zero-write: GET 200 snapshots `index.md` / epic.md / `active.json` (`:3825`, also mixed GET `:3675`). POST 404 snapshots epic.md / `active.json` and `spec_archive` has no epic id (`:3786-3793`).
5. Merge must not stamp `archived` on epics (`:464-473`). Test `:3717`.
6. Reference: `human/task-summaries/task-2-http-get-additive-post-epic-404.md`

### If grill epics, conversion omit, cap, or missing .grill fails (`spec_board.c`)

1. Presence: `cbm_spec_board_grill_skill_present` at `spec_board.c:1087-1093` (`cbm_is_dir(root/.grill)`). File-at-path or missing → false. Read always sets the flag then walks (`:1170-1173`). Missing dir: flag false, `epics: []`. Test `test_spec_board.c:1328`.
2. Conversion: `grill_epic_converted` (`:738-756`). `source.grill_epic` from `read_active_json` (`:275-283`). Companion-to token in `read_spec_md` (`:322-344`) — first `.grill/plans/`…`.md`, never fopen’d. Done/in_progress claim still omits. Tests `:1016` / `:1062` / `:1123`.
3. Order: `grill_collect_plans` (`:873-955`) — index.md GFM data rows, then unlisted slug-asc. Within a plan, `epic-NNN-*.md` numeric (`:712-735`, `:1063-1064`). Tests `:1156` / `:1459`.
4. Cap / skip: stop appending at 64 (`:967-968`, `:1026-1028`). Dir-at-path or `read_whole_file` NULL skips that epic (`:978-984`). No `has_more`. Tests `:1277` / `:1360`.
5. Zero-write: `read_whole_file` fopen `"rb"` (`:36-37`). Grill fill without sdd (`:1438`). Byte snapshot `:1386`.
6. JSON: `to_json` (`:1201`) emits `grill_skill_present` (`:1215`) and `epics` with `kind`/`summary`/`plan_title` (`:1247-1260`). Specs have no `kind`. `gamedev_skill_present` is not this GET (`:1096-1102` unused here).
7. HTTP: GET is read → specs-only merge → `to_json` (`http_server.c:487-510`); no `fopen` of `.grill/` in HTTP. POST epic id 404 `spec not found` (`:626-630`) before `set` (`:656`). Tests `test_httpd.c:3675` / `:3745` / `:3804` / `:3825`. EpicCard UI is the section above. Reference: `human/task-summaries/task-1-spec-board-grill-read.md` / `task-2-http-get-additive-post-epic-404.md` / `task-3-epiccard-todo-order.md`

### If spec-board enrich, dual done, or per-entry degrade fails (`spec_board.c`)

1. List then enrich: `cbm_spec_board_read` at `spec_board.c:1105`. After `read_active_json` + active append, one log (`:1133-1134`) then every entry `read_spec_md` + `read_tasks_md` + `apply_test_results_log` (`:1136-1142`). Grill fill runs after that loop (`:1170-1173`).
2. One spec.md: `read_spec_md` (`:298-320`). Title from first-line H1. Blurb via extract on the same buffer (`:320`). Missing/unreadable → both `""` (`:302-305`). Ghost test `:559`. Unreadable (dir-at-path) `test_spec_board.c:729`.
3. Dual matcher: `apply_test_results_log` (`:541`). `Task #` then `qualified = e->active || strstr(line, e->id)` (`:556-558`). Last matching line wins; `done = PASS` (`:564`). Non-active bare PASS must not mark (`:510`). Active compact stays (`:186`).
4. Missing tasks.md: `read_tasks_md` (`:496-502`) returns; `task_count` 0. Test `:604`. Idle no-file ids still zeros (`:167-173`).
5. Active-only chrome: state/checklist/current after `if (!e->active) continue` (`:1144-1166`). Todo must not get agent (`:551-552`).
6. HTTP: `handle_spec_board_get` (`http_server.c:487`) is read + specs-only archive merge + `to_json`. 200 even when one spec degrades. Grill walk is inside `cbm_spec_board_read`, not in HTTP (SDD-ADR-036).
7. Reference: `human/task-summaries/task-2-enrich-all-dual-done-matcher.md` / `task-2-http-get-additive-post-epic-404.md`

### If spec-board blurb extract or JSON `"blurb"` fails (`spec_board.c`)

1. Empty / missing heading: `cbm_spec_board_extract_blurb` at `spec_board.c:450`. NULL, empty, or no `## Executive Summary` line → `out[0] = 0` (`:454-457`). H1 / `###` do not count (`:331-334`). Tests: `test_spec_board.c:349` (KPI), `:372` (empty ES), `:399` (NULL), extract not-H1/H3 after that.
2. Body bounds: after the heading newline (`:463-467`). Immediate next `## ` is empty (`:469-470`). Else until `\n## ` (`:472`) so `## KPI` is never copied.
3. Shape: links first (`:482`), then two sentences (`:483`), then newlines → space (`:484`), then trim (`:485`). Cap/walkback is `copy_blurb_truncated` (`:434-448`).
4. JSON: `cbm_spec_board_to_json` at `:1201`. Escape `e->blurb` into `esc_blurb[1024]` (`:1222`). Format always includes `"blurb"` (`:1228-1234`). Additive grill keys are after specs (`:1215`, `:1247`).
5. Live board: `read_spec_md` (`:298`) calls extract (`:320`). Empty blurb now means missing ES, missing/unreadable file, or whitespace-only body — not "extract skipped".
6. Reference: `human/task-summaries/task-1-blurb-extract-helper.md`

### If ADR tab stamp, warning, or dirty confirm fails (`AdrTab.tsx`)

1. Stamp gate: `lastClean.includes("CBM-GENERATED-START")` (`AdrTab.tsx:113`). Unmarked GET must omit `<time>` and `role="note"` (`AdrTab.test.tsx:223-235`).
2. Stamp paint: `hasGenerated && listed` (`:120-127`). List miss (ghost name) omits `<time>` the same way WorkspaceHeader does. Visible text is `formatIndexedAt(listed.indexed_at, lang)` (runtime local) — not GET `updated_at`, not a line in the blob. `dateTime` / `title` stay raw ISO (`:123`, test `:199-200`). Invalid `not-a-date` stays raw (`:208-221`).
3. Warning: `role="note"` + `t.adr.replaceWarning` (`:129-132`). Copy lock: `i18n.ts:81-82` (en) / `:183` (zh).
4. Editor: one textarea (`:144`). Save POST `{project, content}` (`:77-81`). ISO must not appear in `content` (`AdrTab.test.tsx:220`).
5. Dirty leave: unchanged `App.tsx:80` `window.confirm(t.adr.unsavedConfirm)`. AdrTab only reports dirty (`:69-71`).
6. Reference: `human/task-summaries/task-4-adrtab-generated-at-warning.md` / `task-2-surface-gherkin-local-indexed.md`

### If HTTP/MCP fill, watcher skip, or Save 32768 fails (`http_server.c` / `test_httpd.c` / `test_mcp.c`)

1. Fill vs watcher: user jobs omit `adr_fill` or set true (`mcp.c:8189`). JSON bool false skips (`mcp.c:1561-1563`). Watcher args encode false (`application.c:3399`). Persist proof: `tool_index_repository_adr_fill_false_leaves_unmarked` (`test_mcp.c:6409`) — blob stays `# Before watch`.
2. HTTP create/Reindex: executor calls `index_repository` (`test_httpd.c:2330-2332`). After `"status":"done"`, GET `/api/adr` is `handle_adr_get` (`http_server.c:852`). Reindex migrate: `:2841`. Create empty manual: `:2906`.
3. MCP same blob: `tool_index_repository_fills_adr_same_as_store_get` (`test_mcp.c:6287`) — `manage_adr` get and `cbm_store_adr_get` both contain `PURPOSE-MCP-FILL`.
4. manage_adr survive: update is whole-doc (`mcp.c:11029-11031`, `semantics: whole_document_replaced`). Next user index keeps `# New notes` in MANUAL (`test_mcp.c:6337`). Generated hand-edit dies: POST then Reindex (`test_httpd.c:3091`) — `PURPOSE-CANONICAL` wins.
5. Body cap: `handle_adr_save` rejects `body_len == 0` or `> CBM_SZ_32K` with 400 `invalid body` (`http_server.c:918`). 16384 generated+manual still 200 (`test_httpd.c:3140`). Transport max is 1MiB — do not confuse with this route cap. manage_adr has no HTTP cap.
6. Limits: no `.sdd-skill` → `# Hand only\n` unmarked (`test_httpd.c:2965`). Partial trio still 202 (`:3009`). Unreadable ARCHITECTURE omits extract (`:3048`).
7. Reference: `human/task-summaries/task-3-http-mcp-watcher-gherkin.md`

### If ADR fill runs on watcher or skips user persist (`pipeline.c` / `mcp.c`)

1. `cbm_pipeline_new` leaves `adr_fill` false (`pipeline.c:277`). Direct `cbm_pipeline_run` tests must stay inert.
2. User path: `handle_index_repository` sets the flag from args (`mcp.c:8189`). Missing key → true (`:1553-1554`).
3. Watcher path: args include `"adr_fill":false` (`application.c:3399`). `cbm_mcp_index_want_adr_fill` then returns false (`mcp.c:1561-1563`).
4. Apply after capture: full `pipeline.c:1997`; incremental dump `pipeline_incremental.c:2898`. NULL / no skill keeps prior (`pipeline.c:334-335`).
5. Unchanged tree: trio is ALWAYS_SKIP, so force full when `adr_fill_would_change` (`pipeline_incremental.c:2448` / `:2523`).
6. Subscribe: strip `adr_fill` in args-equal (`application.c:1583`) or a watcher poll cannot join a user job.
7. Trio as File nodes: `.sdd-skill` must be in ALWAYS_SKIP (`discover.c:57`, skip at `:351`).
8. Reference: `human/task-summaries/task-2-pipeline-hook-skip.md`

### If ADR fill helper returns the wrong blob (`adr_fill.c`)

1. Neither `.gamedev/` nor `.sdd-skill/` is a directory → NULL at `adr_fill.c:274-276`. Existing text must stay unmarked. Empty `.gamedev/` dir is present and must still mark.
2. `.gamedev/` dir wins (`:72-76`) — gamedev relatives only (`:24-26`). Else sdd (`:78-82`, `:21-23`). File-at-path `.gamedev` is not present.
3. Skill present → always a marked document (`:277-279`), even if all three selected-trio files are missing (generated inner is whitespace). Leftover sdd strings must stay absent.
4. Trio paths are the selected constants. A DEV_LOG / GDD string in the result means a fourth path was opened.
5. Unreadable vs missing: missing/not-regular is `:97-98`. Open/read fail is `:100-102` / `:112-114`. Tests use chmod 0, then a directory at that path (`test_adr_fill.c:107-120`).
6. Manual survival: unmarked body is `:207-211`. Marked refill keeps only the MANUAL span (`:221-223`). Generated hand-edits are dropped on the next call.
7. Reference: `human/task-summaries/task-1-xor-trio-select-gamedev-relatives.md` (spec-004 helper: `task-1-splice-extract-helper.md`)

### If `?tab=adr` without project is not Dashboard (`route.ts`)

1. `readRoute` at `route.ts:23-26` — workspace tab needs a truthy `project`; else Dashboard + `project: null`.
2. App must not keep a local copy of this function (`App.tsx:11`).
3. Assert in `route.test.ts` "maps a workspace tab without project to Dashboard".
4. Reference: `human/task-summaries/task-1-tabid-readroute.md`

### If the wrong default tab opens (`App.tsx`)

1. Paint: GraphTab only when `tab=graph` and `project` is non-empty (`App.tsx:40`, `72-77`).
2. Main must mount `Dashboard` or `GraphTab` until Task #5, never `SpecBoardTab` / `TabBar`.
3. Header must have no buttons/tabs named Specs, Graph, Projects, or Control.
4. Reference: `human/task-summaries/task-5-app-routing-tabbar.md`

### If a stats bookmark does not alias Dashboard (`route.ts`)

1. `?tab=stats` (also `control` / unknown / missing / workspace without project) hits Dashboard at `route.ts:26`.
2. First-load `replaceState` at `App.tsx:21-24` writes the canonical query from `readRoute`.
3. Assert in `App.test.tsx` "aliases ?tab=stats to Dashboard".
4. Reference: `human/task-summaries/task-1-tabid-readroute.md`

### If Graph opens without a project (`route.ts`)

1. `params.get("project")` empty/null fails `isWorkspaceTab(rawTab) && project` (`route.ts:23-24`).
2. `showGraph` at `App.tsx:40` is `activeTab === "graph" && Boolean(selectedProject)`.
3. `?tab=graph` alone must canonicalize to `?tab=dashboard` (no `project` key).
4. Reference: `human/task-summaries/task-1-tabid-readroute.md`

### If back leaves `project=` on Dashboard (`route.ts`)

1. Chip button at `App.tsx:63` must call `navigate("dashboard", null)`.
2. `routeUrl` at `route.ts:29-33` sets `project` only when the argument is truthy.
3. After back, search is `?tab=dashboard` and must not contain `project=alpha`.
4. Reference: `human/task-summaries/task-5-app-routing-tabbar.md`

### If the list RPC fails (`Dashboard.tsx`)

1. Destructive `role="alert"` at `Dashboard.tsx:104-107` must have non-empty text.
2. Control heading `Control Panel` must still be in the document (`Dashboard.tsx:198-200`).
3. Hook catch is `useProjects.ts:30-31` — HTTP non-OK or JSON-RPC `error`.
4. Reference: `human/task-summaries/task-4-dashboard-page.md`

### If create 409 does not redirect (`CreateIndexModal.tsx` / `App.tsx`)

1. 409 `path_exists` at `CreateIndexModal.tsx:127-130` must call `onPathExists(existing_project)` and `onClose` — never `onCreated` (line 133).
2. App `onPathExists` is `openExistingProject` (`App.tsx:55-62`, wired at 142). URL becomes `?tab=graph&project=<name>`. Notice is `role="status"` at 121-126.
3. Do not route this through `navigate` — that clears `pathNotice` (`App.tsx:44`).
4. 409 `name_exists` falls through to throw `data.error` (line 132). Modal stays; no Graph; no status notice.
5. Listed Path skip: `findNewestForPath` at 112-116; Dashboard passes `existingProjects={projects}` (210). Trailing slash: `pathGroups.ts:41-43`.
6. Reference: `human/task-summaries/task-4-create-409-redirect.md`

### If index POST fails (`CreateIndexModal.tsx`)

1. Network: `POST /api/index` body is `{ "root_path": "<path>" }` only (`CreateIndexModal.tsx:124`).
2. HTTP 400 / 409 `name_exists` `{ "error":"..." }` → modal stays; show `data.error` (`CreateIndexModal.tsx:132`).
3. Live C may say `"directory not found"`; Gherkin mocks `"not a directory"`. Do not change C.
4. Reference: `human/task-summaries/task-4-dashboard-page.md`

### If same-Path clones do not group (`pathGroups.ts` / `Dashboard.tsx`)

1. `groupKey` at `pathGroups.ts:36-37` is `canonical_root || root_path` — not a JS realpath.
2. Conflict is `members.length >= 2` (`Dashboard.tsx:90`). Solo cards are unchanged.
3. Enter on the region is `onSelectProject(group.newest.name)` (`Dashboard.tsx:141`). Newest is `pathGroups.ts:17-33` (same strcmp as C).
4. Path once: header `group.newest.root_path` (`Dashboard.tsx:136-137`). Do not print path on each member.
5. Delete is still confirm + `DELETE /api/project?name=` (`Dashboard.tsx:38-40`). One name per click. Reindex is a separate control on each member (160-167).
6. Reference: `human/task-summaries/task-3-dashboard-conflict-groups.md`

### If delete cancel still removes the row (`Dashboard.tsx`)

1. `deleteProject` at `Dashboard.tsx:38-39` must return when `confirm` is false.
2. No `DELETE /api/project?name=...` until confirm is true (line 40). Conflict members use the same helper.
3. Stub `window.confirm` in tests; cancel leaves the name in the list.
4. Reference: `human/task-summaries/task-3-dashboard-conflict-groups.md`

### If Reindex fails or navigates (`Dashboard.tsx`)

1. Body at `Dashboard.tsx:53` is `{ root_path, project }` — never `project_name`.
2. HTTP 202 at 55-57 sets `indexing` and `refresh`. `IndexProgress` mounts at 81-83. Do not call `onSelectProject`.
3. HTTP 500 at 60-69 sets `reindexError` (i18n fallback `reindexError` at `i18n.ts:52`). `listError` at 76 / 104-107 is `role="alert"`. Do not `refresh` on error.
4. Conflict members: Reindex at 160-167. Solo: 261-268. Header and create modal must not have this control.
5. App compose: `App.test.tsx:465-543` — URL must not contain `tab=graph`.
6. Reference: `human/task-summaries/task-5-dashboard-reindex.md`

### If 202 navigates to Graph (`Dashboard.tsx` / `CreateIndexModal.tsx`)

1. `onCreated` at `Dashboard.tsx:205` only sets `indexing` and calls `refresh`.
2. `onSelectProject` is Enter only (`Dashboard.tsx:141` / `256`). Graph on create is `onPathExists` (409 / listed skip), not 202.
3. Task #5 Reindex stays on Dashboard with IndexProgress (`Dashboard.tsx:47-73`). Create 202 must not call `onSelectProject`.
4. Reference: `human/task-summaries/task-5-dashboard-reindex.md` / `task-4-create-409-redirect.md`

### If Control is missing (`Dashboard.tsx` / `ControlTab.tsx`)

1. `ControlTab embedded` must render after the list (`Dashboard.tsx:198-200`).
2. Empty and error paths must not unmount it.
3. `embedded` at `ControlTab.tsx:220-222` skips the outer ScrollArea; polls stay 3000 / 2000.
4. There is no standalone Control tab. Control lives on Dashboard only.
5. Reference: `human/task-summaries/task-4-dashboard-page.md`

### If something fails in the project list (`graph-ui/src/hooks/useProjects.ts`)

1. Confirm the C daemon is serving UI HTTP (default port 9749).
2. In Network: one `POST /rpc` whose `params.name` is `list_projects`. Zero `get_graph_schema`.
3. Check `useProjects.ts:28-31` — catch sets `error`; `result.projects ?? []` hides a missing array as empty, not as error.
4. Reference: `human/task-summaries/task-1-useProjects-list-only.md`

### If something fails in Specs picker (`graph-ui/src/components/SpecBoardTab.tsx`)

1. SpecBoardTab is unrouted (spec-002). If it appears on load, App remounted it — `App.tsx:96-101`.
2. Empty copy is i18n `noIndexedProjects` — not a schema miss.
3. Reference: `human/task-summaries/task-1-useProjects-list-only.md`

### If chrome is the wrong color (`graph-ui/src/styles/globals.css`)

1. Inspect `--color-primary` at `globals.css:18` — must not be `#1DA27E`.
2. Search `bg-[#0b1920]` / `bg-[#0e2028]` under `graph-ui/src`. Panels use `bg-card`.
3. FilterPanel / Sidebar `text-primary` looking gray is expected (chrome).
4. Reference: `human/task-summaries/task-2-chrome-tokens-palette-lock.md`

### If graph nodes went gray (`graph-ui/src/lib/colors.ts`)

1. `colorForLabel("Function")` must be `#06b6d4` — not a CSS variable.
2. Edge teal lives only in `EdgeLines.tsx` (`GRAPH_EDGE_PALETTE`). Do not gray it to match chrome.
3. Loader cyan `#22d3ee` in `globals.css` is constellation, not chrome.
4. Reference: `human/task-summaries/task-2-chrome-tokens-palette-lock.md`

### If the gauge lost red / amber (`graph-ui/src/components/ControlTab.tsx`)

1. `gaugeFillColor` at lines 14-17: `>80` red, `>50` amber, else gray.
2. Healthy gray is intentional. Red/amber are semantic, not chrome accents.
3. Reference: `human/task-summaries/task-2-chrome-tokens-palette-lock.md`

## Useful Debugging Commands

```bash
# spec-015 Task #2 HTTP GET additive + POST leftover locks
scripts/test.sh --suites httpd

# spec-015 Task #1 spec_board debt parse + additive JSON
scripts/test.sh --suites spec_board

# spec-012 Task #5 Archive UI + refresh + remaining Vitest
cd graph-ui && npx vitest run src/components/GameBoardTab.test.tsx src/hooks/useGameBoard.test.ts src/App.test.tsx

# spec-012 Task #4 GameBoardTab expand + blocked strip + i18n
cd graph-ui && npx vitest run src/components/GameBoardTab.test.tsx src/hooks/useGameBoard.test.ts src/lib/i18n.test.ts

# spec-011 Task #4 Clipboard, no-drag, typed card parse
cd graph-ui && npx vitest run src/components/GameBoardTab.test.tsx src/hooks/useGameBoard.test.ts

# spec-011 Task #3 GameBoardTab four columns + cards + i18n
cd graph-ui && npx vitest run src/components/GameBoardTab.test.tsx src/lib/i18n.test.ts

# spec-011 Task #2 C Inbox walk + conversion + HTTP bytes
scripts/test.sh --suites game_board,httpd

# spec-011 Task #1 C artifact walk + widen card JSON
scripts/test.sh --suites game_board,httpd

# spec-010 Task #4 App silent-win / dual fetch / deep-links
cd graph-ui && npx vitest run src/App.test.tsx src/hooks/useGameBoard.test.ts

# spec-010 Task #3 GameBoardTab chrome only
cd graph-ui && npx vitest run src/components/GameBoardTab.test.tsx

# spec-010 Task #2 TabId / route kernels / strip / i18n
cd graph-ui && npx vitest run src/lib/route.test.ts src/components/WorkspaceTabStrip.test.tsx src/lib/i18n.test.ts

# spec-010 Task #1 C/HTTP GET /api/game-board
make -f Makefile.cbm test-focused TEST_SUITES="game_board httpd"

# spec-009 Task #3 App strip / deep-link / Enter / omit
cd graph-ui && npx vitest run src/App.test.tsx

# spec-009 Task #2 SpecBoardTab host Kanban on grill-only
cd graph-ui && npx vitest run src/components/SpecBoardTab.test.tsx

# spec-009 Task #1 Presence predicate sdd OR grill
cd graph-ui && npx vitest run src/hooks/useSddSkillPresent.test.ts

# spec-008 Task #4 Vitest Gherkin remaining UI grill-epic
cd graph-ui && npx vitest run src/components/SpecBoardTab.test.tsx src/lib/colors.test.ts

# spec-008 Task #3 SpecBoardTab EpicCard + Todo epics-then-specs
cd graph-ui && npx vitest run src/components/SpecBoardTab.test.tsx

# spec-008 Task #2 HTTP GET additive + POST epic-id 404
scripts/test.sh --suites spec_board,httpd

# spec-008 Task #1 spec_board grill read + additive JSON
scripts/test.sh --suites spec_board

# spec-007 Task #2 Surface Gherkin local text + raw ISO dateTime
cd graph-ui && npx vitest run src/components/Dashboard.test.tsx src/components/WorkspaceHeader.test.tsx src/components/AdrTab.test.tsx src/lib/colors.test.ts

# spec-007 Task #1 Drop UTC pin + helper oracles
cd graph-ui && npx vitest run src/lib/formatIndexedAt.test.ts

# spec-006 Task #4 Vitest Gherkin + poll after archive
cd graph-ui && npx vitest run src/components/SpecBoardTab.test.tsx src/lib/colors.test.ts

# spec-006 Task #3 SpecBoardTab filter + Archive/Unarchive
cd graph-ui && npx vitest run src/components/SpecBoardTab.test.tsx src/lib/i18n.test.ts

# spec-005 Task #4 Vitest Gherkin + poll persist + no Archive (superseded: Archive is spec-006)
cd graph-ui && npx vitest run src/components/SpecBoardTab.test.tsx src/lib/colors.test.ts

# spec-005 Task #3 SpecCard expand Set + blurb + TaskList filter
cd graph-ui && npx vitest run

# spec-005 Task #2 enrich all + dual done matcher
scripts/test.sh --suites spec_board

# spec-005 Task #1 blurb extract helper + JSON (no HTTP)
scripts/test.sh --suites spec_board

# spec-004 Task #4 AdrTab stamp + warning
cd graph-ui && npx vitest run src/components/AdrTab.test.tsx src/lib/i18n.test.ts src/lib/colors.test.ts

# spec-004 Task #3 HTTP + MCP + watcher Gherkin
scripts/test.sh --suites httpd,mcp

# spec-004 Task #2 pipeline hook + skip
scripts/test.sh --suites adr_fill,discover,mcp,daemon_application,incremental

# spec-004 Task #1 splice + extract helper (not a full index)
scripts/test.sh --suites adr_fill

# spec-003 Task #5 Dashboard Reindex
cd graph-ui && npx vitest run src/components/Dashboard.test.tsx src/App.test.tsx src/lib/i18n.test.ts src/components/CreateIndexModal.test.tsx src/components/WorkspaceHeader.test.tsx

# spec-003 Task #4 create 409 redirect
cd graph-ui && npx vitest run src/components/CreateIndexModal.test.tsx src/App.test.tsx src/lib/pathGroups.test.ts src/lib/i18n.test.ts

# spec-003 Task #3 conflict groups
cd graph-ui && npx vitest run src/lib/pathGroups.test.ts src/components/Dashboard.test.tsx src/lib/i18n.test.ts

# spec-002 Task #1 route kernel
cd graph-ui && npx vitest run src/lib/route.test.ts src/App.test.tsx

# spec-002 Task #5 workspace App
cd graph-ui && npx vitest run src/App.test.tsx

# spec-001 Task #5 routing + header IA
cd graph-ui && npx vitest run src/App.test.tsx src/lib/i18n.test.ts

# Task #4 Dashboard Gherkin
cd graph-ui && npx vitest run src/components/Dashboard.test.tsx src/components/IndexProgress.test.tsx src/components/AdrTab.test.tsx

# UI unit tests (Task #1 hook)
cd graph-ui && npx vitest run src/hooks/useProjects.test.ts

# Task #2 chrome + palette lock
cd graph-ui && npx vitest run src/lib/colors.test.ts src/lib/chrome-tokens.test.ts

# spec-001 Task #3 freshness helper (display TZ superseded by spec-007 Task #1)
cd graph-ui && npx vitest run src/lib/formatIndexedAt.test.ts

# Full graph-ui suite
cd graph-ui && npm test

# Who still names the forbidden tool
rg get_graph_schema graph-ui/src

# Teal leaked back into chrome tokens (should only hit EdgeLines + tests)
rg -n '#1DA27E|#1C8585|#0b1920|#0e2028' graph-ui/src

# StatsTab must be gone
rg -n StatsTab graph-ui/src

# Daemon + embedded UI (default :9749)
scripts/build.sh --with-ui
```

## Quick References

`human/PROJECT-OVERVIEW.md` | `human/task-summaries/` | `human/ARCHITECTURE-VISUAL.mmd` | `baseline/ARCHITECTURE_ADR.md`
