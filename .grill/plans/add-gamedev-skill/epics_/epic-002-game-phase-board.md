epic: 002
plan: add-gamedev-skill
name: game-phase-board
status: detailed

summary: Game tab paints four columns (Inbox | Pre-prod | Prod | Post-prod+Launch) with existing artifact cards (track, work-state, owning agent, copy continue @role) plus unconverted grill epics; converted omitted.
delivery-rationale: Operator on a live `.gamedev/` tree sees the cycle map (phases, tracks, parallel cards) and leftover grill inbox in one board.

tech:
  facts:
    - spec-board must not grow gamedev JSON; new reader + new GET (epic 001)
    - Grill walk already in spec_board.c (Companion-to + source.grill_epic, cap 64); gamedev conversion is Companion-to OR roadmap slug+NNN (ADR-006)
    - Artifact set: phase docs; systems/SYS-*; levels/LVL-*; art/audio/ui/animation folders; playtest-log; post-prod docs if present; no missing-file placeholders (ADR-005)
    - Owner map: skill/project agents.md (role → owns)
    - Doc header status: draft|in_review|approved (_core.agent); state.md agent line: in_progress|blocked|needs_review|done; bevy uses qa-lead:ready (not in template)
    - Track independent of phase; current phase = state.md phase=
    - Caps on spec-board are 64/64; bevy production has many SYS-* plus art folders
  open-qs:
    1. New C module vs fork of spec_board.c patterns (fopen rb, kv_extract) without sharing JSON?
    2. Reuse grill walk from spec_board.c for Inbox or duplicate under gamedev reader?
    3. Roadmap epic-map parse: heading/table contract — what if source: line and table disagree?
    4. Work-state: header only vs overlay non-idle state.md lines vs both, and who wins?
    5. Status `ready` and other non-enum tokens: treat as in_progress, ignore, or blocked?
    6. Owner resolution if project docs/agents.md missing: ship skill table or unlabeled cards?
    7. SYS dir without spec.md / asset dir without context.md: still a card?
    8. Cap: global vs per-column; overflow omit vs has_more (Specs omitted has_more)?
    9. Poll: reuse spec-board interval or slower (larger tree walk)?
    10. Copy-continue payload: exact `/gamedev-skill continue @gameplay-engineer` vs `agent @role`?
    11. Inbox conversion Companion-to: which files scanned (all artifacts vs spec.md only)?
    12. Heap bound for bevy-sized art+systems listing?

product:
  facts:
    - ADR-003 columns + track badge + work-state on card + parallel in_progress OK
    - ADR-002/006 Inbox + conversion; sit-beside if missing link
    - ADR-005 one card per existing artifact; owning agent visible
    - ADR-009 card copies continue @role; inbox copies continue (no fake specialist)
    - ADR-004 no drag / no mark-done
  open-qs:
    1. Empty column copy: none vs “no artifacts in this phase”?
    2. Current-phase highlight only vs dim other columns vs hide future empty?
    3. Card order inside a column (SYS numeric, assets alpha, docs fixed list)?
    4. Inbox order: same as Specs (index.md then epic-NNN)?
    5. Kind marks: letter E on grill; letters/icons for A/B/H artifacts?
    6. Production density: flatten all art folders or group “art” as one card (would fight ADR-005)?
    7. Click-to-copy feedback: none vs toast vs selected text?
    8. Closed grill plans: still list unconverted (Specs ADR-003 analog) on Game Inbox?
    9. Work-state labels in UI language vs raw draft/in_review/approved?

deps: [epic-001 Game tab + GET]
adrs: [ADR-002, ADR-003, ADR-005, ADR-006, ADR-009]
glossary-refs: [Game columns, Artifact card, Game inbox, Unconverted (gamedev), Work-state, Track A, Track B, Hybrid]
