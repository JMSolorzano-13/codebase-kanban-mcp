# Work Breakdown — Spec-011: Game phase board
Total Tasks: 5 / Estimated Total Effort: 16h

## Task Dependency Graph
```
#1 C artifact walk + widen card/to_json ──► #2 C Inbox + conversion + HTTP Gherkin
                                           └──► #3 GameBoardTab columns + cards + i18n
                                                └──► #4 Clipboard + no-drag + typed parse
                                                     └──► #5 remaining Vitest Gherkin
```
#2 and #3 may start after #1 JSON field names exist (types from plan). Critical path: #1 (5h) → #2 (3.5h) → #3 (3.5h) → #4 (2h) → #5 (2h) = 16h.

## Tasks

### Task #1 — C artifact walk + widen card JSON
Definition of Done:
- [x] `cbm_game_board_card_t` widened: `kind`, `id`, `title`, `track`, `work_state`, `owner`, `continue_cmd`, `summary`, `plan_title` (sizes per plan.md)
- [x] `CBM_GAME_BOARD_MAX_CARDS` stays 64; `CBM_GAME_BOARD_LIST_MAX` 256 scratch for production listing
- [x] `cbm_game_board_read` still uses `cbm_spec_board_gamedev_skill_present`; present false leaves all counts 0 (no phases walk)
- [x] Pre-prod / production / post-prod fill exist-only; orders and non-card exclusions per plan.md
- [x] `status:` first-token map including `ready` → `in_progress`; missing header file → pending
- [x] Owner/track from compiled table only; do not fopen `docs/agents.md`
- [x] `cbm_game_board_to_json` grow buffer; emits card objects; artifacts have non-null track/work_state; `summary`/`plan_title` `""`; no `has_more`; no `column`
- [x] Chrome keys (`phase`, `focus`, `continue`) unchanged from spec-010
- [x] GET `/api/spec-board` still has no `gamedev_skill_present`
- [x] 400/404 strings unchanged; no POST; no MCP; do not edit `spec_board.c` / `mcp.c` / `Makefile.cbm`
- [x] `tests/test_game_board.c` + keep httpd 400/404/present-false-empty/spec-board-no-gamedev
- [x] tests pass locally; breadcrumbs
User Stories Addressed: US-002, US-005 (artifact half)
Gherkin covered:
- existing gdd.md paints (JSON fields)
- SYS directory without spec.md is a Production card (JSON)
- two in_progress cards stay in Production (JSON)
- Limit — missing narrative-bible is not a placeholder
- Limit — 65th production card omitted; no has_more
- Limit — status ready maps to In progress (JSON)
- Limit — present false still emits empty arrays
- Limit — spec-board still has no gamedev field
- Error — unknown project is still 404
- Error — missing project query is still 400
Dependencies: None
Estimated Effort: 5h
Subagent: yes
Path: full
Implementation Notes: SDD-ADR-048, SDD-ADR-049. Fixtures `/tmp` only. Inbox may stay count 0 this task. Empty `.gamedev/` dir still all `[]`. Sort production before applying the 64 cap. Replace 8192 snprintf.

### Task #2 — C Inbox walk + conversion + HTTP bytes
Definition of Done:
- [x] New `game_grill_*` walk in `game_board.c` only (index.md then slug-asc; epic-NNN numeric; name/summary/plan_title)
- [x] Inbox cards: `kind` epic; track/work_state empty → JSON null; owner `""`; continue `/gamedev-skill continue`; id/title/summary/plan_title per US-003
- [x] No `.grill/` → `inbox` `[]`. All plans (draft/closed). Cap 64 independent of phase caps
- [x] Converted omit: Companion-to exact path on scanned Game files OR (slug token in roadmap.md and/or game_context.md AND roadmap.md table cell `NNN` or `epic-NNN`)
- [x] Do not read `active.json`. Do not kebab-match. Plan-folder cite without cell does not convert
- [x] Companion-to token rule = spec-008; token never fopen'd; scan list = plan.md US-004
- [x] GET 200 leaves `.gamedev/`, `.sdd-skill/`, `.grill/` byte-identical and does not create missing trees
- [x] Do not edit `spec_board.c`. Do not share to_json
- [x] `test_game_board.c` + `test_httpd.c` cover Then clauses below
- [x] tests pass; breadcrumbs
User Stories Addressed: US-003, US-004, US-006 (C/HTTP half)
Gherkin covered:
- unconverted grill epic paints in Inbox (JSON)
- Companion-to exact path omits the epic (+ epic.md bytes)
- roadmap slug plus table NNN omits the epic
- Limit — closed grill plan leftover stays in Inbox
- Limit — missing conversion link sits beside a SYS card
- Limit — no grill directory yields empty Inbox (JSON)
- Limit — Inbox order is index.md then epic-NNN
- Error — GET does not write skill trees (C/HTTP bytes)
- Error — plan-folder cite without NNN does not convert
- Error — kebab name does not convert
- Error — table NNN without slug cite does not convert
Dependencies: Task #1
Estimated Effort: 3.5h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-046, SDD-ADR-047. Copy catalog algorithm, not spec_board symbols. Roadmap table: GFM `|` data rows only; skip header/separator. Slug token boundaries `[A-Za-z0-9-]`.

### Task #3 — GameBoardTab four columns + cards + i18n
Definition of Done:
- [x] Four headers always visible, document order Inbox | Pre-production | Production | Post-production & Launch (EN exact)
- [x] Empty column: header only; 0 cards; no "no artifacts in this phase"; not dimmed/hidden
- [x] `aria-current="true"` on the phase column matching `phase`; Inbox never; phase null → none
- [x] spec-010 chrome remains (phase label, focus, `/gamedev-skill continue` as text)
- [x] Artifact card: id, title, track A/B/H, work-state EN, owner, continue text; no letter E
- [x] Inbox card: letter E via `--color-epic-mark`; title, summary, plan_title, id, continue without @role; no track; no Pending
- [x] i18n en+zh: Inbox header + work-state Pending / In progress / Done / Blocked; reuse existing EN phase strings for phase headers
- [x] `GameBoard` / `GameBoardCard` types match GET keys; `useGameBoard` may still pass arrays through if #4 owns parse
- [x] `SpecBoardTab.tsx` / `useSddSkillPresent` / `useSpecBoard` not edited
- [x] Invert spec-010 GameBoardTab tests that lock no columns/cards
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001, US-002, US-003 (paint)
Gherkin covered:
- four column headers and current-phase highlight
- existing gdd.md paints (pane Thens)
- SYS directory without spec.md (pane)
- two in_progress cards (pane)
- unconverted grill epic paints (pane)
- Limit — empty column has header and no placeholder
- Limit — phase null highlights no phase column
- Limit — 65th production: no control named "Has more"
- Limit — status ready → card shows "In progress"
- Limit — no grill directory: Inbox shows 0 cards
Dependencies: Task #1
Estimated Effort: 3.5h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-038 token already exists. Do not import EpicCard from SpecBoardTab. Clipboard/drag Gherkin is Task #4. Layout: chrome row + 4-col grid (drop `max-w-2xl` on columns).

### Task #4 — Clipboard, no-drag, typed card parse
Definition of Done:
- [x] `parseGameBoard` types arrays as `GameBoardCard[]`; skip objects without `kind` `artifact`|`epic`
- [x] Artifact continue control copies `/gamedev-skill continue @role` via `navigator.clipboard.writeText`
- [x] Inbox continue control copies `/gamedev-skill continue`
- [x] Clipboard denied/missing: select-text on the control; no toast either path
- [x] Activating the card (not the continue control) does not show Archive/Unarchive/"No tasks planned yet"; no POST
- [x] Cards `draggable={false}`; drop on another column does not move the card (no local reorder)
- [x] `useGameBoard` stays one-shot (no interval)
- [x] tests pass; breadcrumbs
User Stories Addressed: US-006
Gherkin covered:
- click artifact copies continue at-role
- click inbox copies continue without at-role
- Error — card activate does not expand or archive
- Error — cards are not draggable
Dependencies: Task #3
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: SDD-ADR-050, SDD-ADR-051. Stub `writeText` in Vitest. Chrome continue `<p>` stays spec-010 (not a button). Continue **control** is a button named with the continue string.

### Task #5 — Remaining Vitest Gherkin mapping
Definition of Done:
- [x] Every UI Gherkin Then not already asserted in #3/#4 has a Vitest
- [x] spec-010 silent-win / Enter Graph / leftover `tab=specs`→game / no `/api/skill-presence` stay green
- [x] App fetch log: no request path contains `/api/skill-presence`
- [x] GET zero-write UI half (no skill-presence)
- [x] `colorForLabel("Function") === "#06b6d4"` still locked if that test is in the suite run
- [x] tests pass; breadcrumbs
User Stories Addressed: US-001–US-006 (remaining UI Thens)
Gherkin covered:
- Error — GET does not write skill trees (graph-ui no skill-presence half)
- Any #3/#4 Then still missing after implementation
Dependencies: Task #4
Estimated Effort: 2h
Subagent: no
Path: full
Implementation Notes: Test-only if #3/#4 product is complete. Do not add Playwright unless @tester later requires CERTIFICATION. Do not weaken silent win.

## Task Summary Table
| Task # | Name | Effort | Dependencies | Status |
| 1 | C artifact walk + widen card JSON | 5h | None | done |
| 2 | C Inbox walk + conversion + HTTP bytes | 3.5h | #1 | done |
| 3 | GameBoardTab four columns + cards + i18n | 3.5h | #1 | done |
| 4 | Clipboard, no-drag, typed card parse | 2h | #3 | done |
| 5 | Remaining Vitest Gherkin mapping | 2h | #4 | done |
Total: 16h

## Critical Path
#1 → #2 → #3 → #4 → #5 (16h). #3 may run after #1 in parallel with #2 once card JSON keys are stable.

## Implementation Guidance for @implementer
Read order: spec.md → plan.md → this file → docs/constitution.md
Per task: DoD completely → implement to spec → follow constitution.md → tests validating DoD → clear commit (held unless user asks)
Blocked: document in state.md Notes, switch task, inform @architect

## Test Coverage Requirements
happy path + limit + error from spec Gherkin, target >80% on touched `game_board` + httpd Gherkin + `GameBoardTab` + `useGameBoard`. C owns GET objects, conversion, cap 64, 400/404, zero writes. Vitest owns columns, highlight, cards, Inbox paint, copy, no toast/drag/expand, silent-win leftovers.

## Success Criteria for All Tasks
- [x] all DoD complete [x] tests>80% [x] passes @review [x] @tester approves [x] human docs complete
