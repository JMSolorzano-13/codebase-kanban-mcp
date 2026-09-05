epic: 001
plan: tech-debt-and-epics-registry
name: specs-debt-and-path
status: detailed

summary: Specs paints open TECH_DEBT.md items in board chrome and wraps the full epic path on Todo cards.
delivery-rationale: Operator on an sdd/grill path sees leftover debt and which grill epic is which without opening Game or leaving Specs.

tech:
  facts:
    - GET /api/spec-board today: sdd_skill_present, grill_skill_present, specs[], epics[]; no debt field (spec_board.c to_json)
    - CBM has 0 reads of TECH_DEBT.md
    - this-repo TECH_DEBT.md: ## TD-NNN blocks + Debt Summary table; all 4 Status=resolved (omit list today)
    - grill-only Specs (no .sdd-skill): file absent → omit (ADR-001)
    - conversion matcher Companion-to / active.json unchanged (ADR-005)
    - same GET additive; hide not this epic (ADR-007)
    - EpicCard id line: `text-[10px] font-mono truncate` — ADR-006 drops truncate, wrap; title truncate stays
    - last-indexed lives in WorkspaceHeader (all tabs); Specs strip is new, above 3 cols (ADR-008)
    - caps today: specs 64, epics 64; poll ~4s via useSpecBoard
    - I.2 zero-write; POST archive + epic id still 404
  open-qs:
    1. Parse ## TD-NNN Status vs Debt Summary table — who wins on disagree?
    2. Status typo / unknown token: treat as open (≠ resolved) or skip row?
    3. Open-item cap: reuse 64, smaller chrome cap, overflow omit vs has_more?
    4. Additive JSON field names/shape — spec-level (no new route)
    5. Extra md read on every spec-board poll — sync in cbm_spec_board_read vs cached?

product:
  facts:
    - debt-chrome not column / not Todo card / not kind E / not expand/archive (ADR-001)
    - open-sdd-debt = Status ≠ resolved (ADR-002)
    - missing file or 0 open → omit list, no empty header (ADR-001)
    - path-wrap under short name on Todo epic cards only (ADR-006); Artifact/Spec cards out
    - specs-debt-strip above 3 cols; not WorkspaceHeader; not column header (ADR-008)
    - specs-debt-row = TD-NNN + title; heading/file order; no severity/category/status/color (ADR-008)
    - grayscale chrome locked (executive-ui / SDD-ADR-005)
  open-qs:
    1. Long debt title: wrap or truncate (path-wrap is epics only)?
    2. Click: dead text vs copy TD-NNN (ADR-001 forbids expand/archive only)?

deps: [Specs tab + GET /api/spec-board already host Kanban]
adrs: [ADR-001, ADR-002, ADR-005, ADR-006, ADR-007, ADR-008]
glossary-refs: [sdd-debt-file, debt-chrome, open-sdd-debt, unconverted-specs, epic-card-id, epic-short-name, path-wrap, same-get-additive, specs-debt-strip, specs-debt-row]
