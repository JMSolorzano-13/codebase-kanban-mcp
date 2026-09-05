adr: 007
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: Specs expand is in-card grow (prior spec-board-detail ADR-002/003/008). User wants sdd-feature parity on Game cards. Track B has no tasks.md; _core.agent header is status/last_decision/open.
decision: Game cards expand in-place, same gesture as Specs; multiple open at once. Track A SYS: 1–2 sentence blurb from spec.md (header `open`/`last_decision` or first “What it does” block) + tasks.md list. Track B / phase docs: header only (status / last_decision / open); no invented task list. Levels: level.md header + recent changelog. playtest-log: open / last round. Grill inbox: epic summary; no archive. Done artifact cards include Archive (ADR-004). Do not dump full GDD/spec body into the expand.
why: Same interaction operators already know. Track B protocol has no Gherkin/tasks.md. Agents already skim the header to avoid full-doc reads. Full-doc expand is noise on localhost:9749.
alternatives: [modal/side panel, fake tasks for Track B, full-doc body, no expand]
irreversible-because: Locks Game board interaction to Specs’ in-card grow. A later overlay would be a second pattern. Blurb source is a parse contract per artifact kind.
epics: [003]
