adr: 006
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: ADR-002 Game inbox needs a hide rule. Specs uses exact Companion-to / source.grill_epic. gamedev-skill never mentions grill. Living bevy-tetris conversion is roadmap.md epic map (plan slug + 001…008), not full paths. game_context.md citing the plan folder is plan-level, not per-epic.
decision: Omit a grill epic from Game Inbox iff (1) some Game artifact has `Companion to:` with that epic’s exact `.grill/plans/<plan>/epics/epic-NNN-<name>.md` path (same token rule as Specs), OR (2) roadmap.md’s grill→roadmap epic map includes that epic’s NNN for the same plan slug cited as grill source in roadmap.md and/or game_context.md. Citing the plan directory alone does not convert every epic. No kebab/name fuzzy match. Missing both links → epic stays in Inbox (may sit beside SYS/doc cards). Table-parse details are spec-level.
why: Companion-to alone would never empty Inbox on the real grill→gamedev path. The epic map is what @director already writes. Plan-folder-only would hide leftover unconverted epics (closed ≠ shipped). Fuzzy names false-positive across plans; NNN is unique inside the cited slug.
alternatives: [Companion-to only, plan-folder converts all, kebab fuzzy, CBM-owned epic table]
irreversible-because: Inbox truth depends on @director writing the map or Companion-to; changing match later re-floods or hides leftover epics.
epics: [002]
