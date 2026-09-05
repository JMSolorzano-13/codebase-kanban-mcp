epic: 002
plan: tech-debt-and-epics-registry
name: game-inbox-registry
status: detailed

summary: Game Inbox hides grill epics via epics_registry when the file exists; otherwise prior Companion-to/roadmap hide; Inbox cards wrap full path.
delivery-rationale: Operator on a gamedev path sees only leftover grill epics as “faltan”, using the skill’s own tracking file when it exists.

tech:
  facts:
    - GET /api/game-board inbox[] today; hide in game_grill_epic_converted: Companion-to exact OR (slug token in roadmap.md|game_context.md AND nnn cell in roadmap) — prior ADR-006
    - epics_registry.md: 0 CBM reads; lazy; CBM never creates (ADR-004)
    - match: Plan = grill slug AND Epic = NNN; no kebab/path fuzzy (ADR-003)
    - file present (even 0 matching rows or 0 parsed rows) → ADR-003 sole hide; no ADR-006 fallthrough (ADR-004)
    - file absent → prior ADR-006 unchanged (ADR-004)
    - game_grill_fill_inbox already walks all `.grill/plans/*/epics/` — no plan-status filter (Specs ADR-003 analog; this plan does not add one)
    - evergreen = Epic 0 only — never a grill Inbox card; ignore
    - Origin unused for hide; native Plan never matches a grill slug
    - hide server-side; inbox[] already omitted (ADR-007)
    - InboxCard id: same truncate class as EpicCard — ADR-006 wrap
    - backlog.md / assets_registry.md / roadmap / game_context / state.md stay non-cards
    - spec-014: Inbox always visible; Show Dones + Track filter phase cols only
  open-qs:
    1. Duplicate Plan+NNN rows with different Status — which wins?
    2. Epic cell formats (`001` vs `1` vs `epic-001`) — normalize or exact?
    3. Plan cell path leftover vs slug-only — exact slug or basename?
    4. Reuse game_grill walk + swap predicate, or second pass after current convert?

product:
  facts:
    - hide Inbox iff Status ∈ {in_progress, closed, parked}; not_started + no-row stay (ADR-003)
    - “faltan” = visible Inbox cards after that rule
    - closed grill plans still eligible (existing walk; no new status filter)
    - Specs Todo never reads registry (ADR-005)
    - path-wrap under short name on Inbox epic cards (ADR-006)
    - no new Inbox column; no debt cards here (003)
    - parked = Inbox-absent only; no other surface this plan
  open-qs:
    1. Empty Inbox after hide: existing empty-column copy vs new “all tracked” copy?

deps: [Game tab + GET /api/game-board + Inbox already exist]
adrs: [ADR-003, ADR-004, ADR-005, ADR-006, ADR-007]
glossary-refs: [epics_registry, registry-status, registry-match, registry-hide-set, registry-else-adr006, unconverted-game-absent-registry, epic-card-id, epic-short-name, path-wrap, same-get-additive]
