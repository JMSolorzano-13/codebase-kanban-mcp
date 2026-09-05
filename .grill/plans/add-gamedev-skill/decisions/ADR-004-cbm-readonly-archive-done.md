adr: 004
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: User: nothing moves by hand; only gamedev-skill advances the cycle; CBM may archive done. Constitution I.2 already forbids indexer/graph-ui writing skill cycle files. Specs archive is CBM-owned (prior plan ADR-001). Track B manuals have no Gherkin; done is an agent stamp (`status: draft|in_review|approved` on the owned doc).
decision: CBM never writes, moves, or renames anything under `.gamedev/`. No drag between columns, no mark-done, no edit of tasks/specs/docs from Game. Status and downstream work change only when the operator runs gamedev-skill as the owning agent (or @director for phase/focus/gates). Sole Game-tab mutation = archive of already-done cards, CBM-owned flag (same split as spec_archive), zero skill writes.
why: Same hygiene-vs-lifecycle split as Specs archive. Track B/manuals wait for the indicated agent to confirm done; that stamp can change required tasks/specs/activities. CBM is a map, not the skill.
alternatives: [drag-to-done in CBM, write status into .gamedev/, no archive]
irreversible-because: A later “mark done in UI” would be a second source of truth against gamedev-skill. Skill-tree archive would fight I.2.
epics: [003]
