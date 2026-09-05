epic: 003
plan: tech-debt-and-epics-registry
name: game-debt-chrome
status: detailed

summary: Game paints open backlog.md debt:* lines in board chrome (not a column, not Inbox cards).
delivery-rationale: Operator on a gamedev path sees leftover debt the same way Specs shows TECH_DEBT.md, without mixing Inbox conversion.

tech:
  facts:
    - GET /api/game-board today: gamedev_skill_present, phase, focus, continue, blocked[], inbox, pre/prod/post; no debt field
    - backlog.md is an explicit non-card today (test_game_board_non_cards)
    - gamedev has no TECH_DEBT.md; tags debt:gate-* / debt:adopt-gap-* / debt:<slug>
    - open-gamedev-debt = line with debt:* and no resolved-by (ADR-002)
    - design/tech tags without debt: are out
    - Game chrome today: phase, focus, continue, Show archived | Show Dones | Track, BlockedStrip, then 4 cols
    - same GET additive (ADR-007); 003 does not wait on 002
    - I.2 zero-write; do not create backlog.md
    - strip after BlockedStrip, before 4 cols (ADR-009)
  open-qs:
    1. Comment vs list-item vs heading — which lines count as an “entry”?
    2. resolved-by anywhere in the entry vs same-line only?
    3. Cap / overflow same question as 001 (chrome flood on large backlog)
    4. Parse shared helper with 001 or separate (different file contracts)
    5. Additive JSON field names/shape — spec-level (no new route)

product:
  facts:
    - debt-chrome not column / not Inbox card / not kind E / not expand/archive (ADR-001)
    - missing backlog or 0 open debt:* → omit list (ADR-001)
    - Inbox hide stays 002; this epic only chrome
    - game-debt-strip after BlockedStrip, before 4 cols; not filter row; not above phase line (ADR-009)
    - game-debt-row = debt:* tag + description; file/entry order; no owner/target/color/severity (ADR-009)
    - mirrors Specs ADR-008 density; grayscale locked
  open-qs:
    1. Long description: wrap or truncate (path-wrap is epics only)?
    2. Click: dead text vs copy debt tag (ADR-001 forbids expand/archive only)?

deps: [Game tab + GET /api/game-board; chrome contract mirrors 001 / ADR-008]
adrs: [ADR-001, ADR-002, ADR-007, ADR-009]
glossary-refs: [gamedev-debt-file, debt-chrome, open-gamedev-debt, same-get-additive, game-debt-strip, game-debt-row]
