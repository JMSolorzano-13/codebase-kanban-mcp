epic: 001
plan: add-gamedev-skill
name: game-tab-silent-win
status: detailed

summary: Path with `.gamedev/` shows a Game workspace tab and omits Specs; chrome shows phase/focus and copyable `/gamedev-skill continue`; new GET (board may be empty).
delivery-rationale: Operator opens bevy-tetris (or any `.gamedev/` project) and sees Game-not-Specs without needing cards yet.

tech:
  facts:
    - WORKSPACE_TABS = graph|specs|adr; comment already says add gamedev id; no plugin runtime
    - Specs present = spec-board 200 AND (sdd_skill_present OR grill_skill_present); gamedev-only already omits Specs; silent win must omit Specs even when sdd/grill true
    - GET /api/skill-presence → {sdd_skill,gamedev_skill}; graph-ui never calls it
    - cbm_spec_board_gamedev_skill_present = root/.gamedev is dir; spec-board to_json must not emit gamedev
    - fallbackSpecsToGraph(?tab=specs) while omitted; default enter Graph (executive ADR-008)
    - constitution I.2 zero skill writes; Path=Project 1:1
  open-qs:
    1. Game presence source: /api/skill-presence vs new board GET 200 vs dir check only in C behind a new route?
    2. New GET path name and 404 vs 200-empty when project exists but `.gamedev/` missing?
    3. ?tab=game deep-link without `.gamedev/` — fallback to graph like specs?
    4. ?tab=specs on a gamedev path — replaceState to graph or to game?
    5. Omit-until-true: hide Game while presence in flight (Specs pattern)?
    6. Epic 001 ships empty board JSON vs chrome-only with no card arrays yet?
    7. Dual fetch race: spec-board says sdd/grill true before gamedev presence lands — Specs flash?
    8. Heap: allocate full board struct in 001 or defer until 002?
    9. Tab id closed-set string: game vs gamedev (types.ts + readRoute + tests lock the set)?
    10. i18n: new tab key en+zh; reuse WorkspaceTabStrip or fork?

product:
  facts:
    - ADR-001 silent win; no conflict banner; other paths sdd+grill unchanged
    - ADR-009 chrome: phase + focus from state.md + `/gamedev-skill continue`; not a launcher
    - User: Game is a new tab, not Specs renamed
  open-qs:
    1. Visible tab label: Game vs Gamedev vs Play?
    2. Empty `.gamedev/` dir (no state.md): show tab + “run continue” or omit tab?
    3. Enter project: still land on Graph (existing default) even when Game exists?
    4. Missing/unreadable state.md: blank chrome, error line, or only the continue command?
    5. Gate-review hint in chrome this epic or wait until 003?
    6. Grill-only / sdd-only regression: Specs still appears; Game absent — copy for empty Game never shown there?
    7. Bookmark ?tab=specs on bevy after this ships: what does the operator see?

deps: [WORKSPACE_TABS extensible; skill-presence + gamedev dir check already exist]
adrs: [ADR-001, ADR-009]
glossary-refs: [Path-lifecycle, Silent win, Game tab, Game how-to, skill-presence]
