# Task #1 — Presence predicate sdd OR grill
Last updated: 2026-08-30 — spec-009-t4x-specs-tab-grill-presence | Path=compact | Patterns: ✓

`useSddSkillPresent` still one-shots GET `/api/spec-board`. `present` is now true on HTTP 200 when `sdd_skill_present === true` OR `grill_skill_present === true` (missing or non-boolean grill is false). File: `graph-ui/src/hooks/useSddSkillPresent.ts` (`bodyHasSkill`). Why: a grill-only repo can join the Specs strip without renaming the hook or adding a poll. App still consumes `present`; the SpecBoardTab host gate is Task #2.
