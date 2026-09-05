adr: 002
plan: add-epics-plans-kanban
date: 2026-08-30
status: accepted

context: User asked how sdd knows which epic to drop from Todo when it becomes a spec. Conversion is filesystem (ADR-001); board needs a read-side match.
decision: An epic is converted (omit from Todo) iff some listed spec.md has `Companion to: .grill/plans/<plan>/epics/epic-NNN-<name>.md` matching that epic's path, OR active.json `source.grill_epic` equals that path. Exact path match only. No fuzzy name. Missing link → epic stays in Todo (may sit beside its spec).
why: Companion-to is the durable per-spec pointer in this repo; active.json source is current-cycle only and covers drafts that may not have Companion-to yet. Fuzzy kebab match false-positives across plans. CBM map would duplicate spec.md.
alternatives: [fuzzy name match, CBM-owned epic↔spec table, active.json source only]
irreversible-because: Board truth depends on planner writing Companion-to; changing match later re-floods or hides Todo.
epics: [epic-001]
