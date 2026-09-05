plan: tech-debt-and-epics-registry
date: 2026-09-01
entry: mixed (phrase + CBM + sdd-skill docs + gamedev-skill 1.12.0 docs)
prior-plans: add-epics-plans-kanban (closed), add-gamedev-skill (closed), spec-014 filters (shipped)

specs-board-today:
- GET /api/spec-board; cols todo / in_progress / done
- grill epics only in Todo; kind mark E
- hide iff Companion-to exact `.grill/plans/<plan>/epics/epic-NNN-<name>.md` on a spec OR active.json source.grill_epic equals that path (ADR-002 prior)
- EpicCard: title (epic name:), summary, plan_title, then id with CSS truncate
- id = `.grill/plans/<slug>/epics/<filename>` (grill_append_epic)
- no TECH_DEBT.md read; no debt cards

game-board-today:
- GET /api/game-board; cols inbox / pre / prod / post
- inbox = unconverted grill epics only
- hide iff Companion-to exact path token on a Game artifact OR (plan slug in roadmap.md|game_context.md AND nnn cell in roadmap epic-map) — ADR-006 prior
- InboxCard: E + title + optional summary/plan_title + id truncate
- backlog.md / assets_registry.md / roadmap.md / game_context.md / state.md are explicit non-cards (test_game_board_non_cards)
- epics_registry.md: 0 CBM reads
- spec-014: Show Dones + Track A/B/All filter phase cols only; Inbox always visible

constitution:
- I.2: CBM indexer/graph-ui must not write .sdd-skill/ .grill/ .gamedev/ cycle files
- archive stays CBM-owned hide of done artifacts (not this plan unless asked)

path-bug (user point 3, observed):
- short name already at top (name: / card.title) — keep
- path line uses truncate → filename (epic name in path) clips; operator loses which epic it is
- user wants full path visible on epic cards in Specs Todo and Game Inbox
