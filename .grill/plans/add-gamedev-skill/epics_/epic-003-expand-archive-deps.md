epic: 003
plan: add-gamedev-skill
name: expand-archive-deps
status: detailed

summary: Game cards expand in-place (Track A blurb+tasks; Track B header; grill summary); done artifacts archive in CBM; blocked-by on cards plus a board blocked strip; Track A expand shows spec Inputs.
delivery-rationale: Operator inspects one card without leaving the board, hides finished work, and sees live parallel blocks — Specs-parity detail on the Game map.

tech:
  facts:
    - Specs expand: in-card grow, multi Set keyed by spec id; blurb 512 B; tasks cap 48; dual done matcher
    - Specs archive: spec_archive in project .db; POST /api/spec-board {spec_id,archived} returns flag object; GET merge; session show-archived; no confirm; 404 if epic id
    - Game archive must not write .gamedev/ (ADR-004); spec-board POST must not become Game archive (different ids)
    - state.md blocked line: agent:blocked:"task":"blocked-by"
    - Track A spec.md often has ## Inputs; heading text not a skill schema
    - ADR-007 forbids dumping full GDD/spec body
  open-qs:
    1. Archive store: new game_archive table vs reuse spec_archive with namespaced ids?
    2. POST path: new /api/game-board vs overload spec-board (would mix boards)?
    3. Card id stability: rel path vs SYS-id vs hash — archive key across reindex?
    4. Track A blurb source when both header `open` and “What it does” exist?
    5. tasks.md parse: reuse Specs task-line matcher or gamedev checkbox dialect?
    6. Level changelog “recent”: N lines vs since last playtest date?
    7. Blocked overlay: all artifacts owned by that agent vs only those named in the task string?
    8. Strip source: only status=blocked lines vs also needs_review?
    9. Inputs extract: first ## Inputs vs English/Spanish variants vs skip if missing?
    10. Expand Set: keyed how when grill epic and SYS share similar names?
    11. Unarchive + show-archived: copy Specs session toggle or persist?
    12. POST 200 body: flag object only (Specs) vs return board?

product:
  facts:
    - ADR-007 expand gesture = Specs; Track B no fake tasks; inbox no archive
    - ADR-004 archive done only; no confirm was Specs ADR-009 — not re-decided here
    - ADR-010 blocked-by + strip; no roadmap graph
    - User: parallel pending/in-progress/done; confirm done only in gamedev-skill
  open-qs:
    1. Archive eligible: header approved only vs work-state done including state.md done?
    2. Archive a blocked card: forbid (not done) or allow?
    3. Inbox epic expand: summary only or also plan title like Specs EpicCard?
    4. Blocked strip placement: above columns vs inside current-phase column?
    5. Click strip row: scroll/highlight that agent’s cards?
    6. Show archived: same session-only toggle as Specs?
    7. Empty expand (no tasks, no open): existing Specs no-tasks copy analog?
    8. playtest-log expand: last round only vs last N — operator scanning Round 6 on bevy?

deps: [epic-002 cards exist to expand/archive]
adrs: [ADR-004, ADR-007, ADR-010]
glossary-refs: [Game expand, Game archive, Game deps, Skill confirmation, Work-state]
