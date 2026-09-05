adr: 009
plan: tech-debt-and-epics-registry
date: 2026-09-01
status: accepted
context: Game already has chrome (phase/focus/continue, filter row, BlockedStrip). Mixing debt with Show Dones/Track would imply filter-scoped debt. Owner/target from the backlog template would densify chrome vs Specs ADR-008. User confirmed mirror.
decision: Game open-debt list is a board strip after BlockedStrip, before the 4 columns. Each row = debt:* tag + description, file/entry order. No owner, target, color, or severity. Not on the filter row. Not above the phase line.
why: Same density as Specs (id + text, grayscale). After BlockedStrip keeps “chrome not axis” without implying debt is a Track/Show-Dones filter.
alternatives: [filter row, above phase line, raw full line, +owner/target, Game-specific denser strip]
irreversible-because: Operators will look under BlockedStrip for debt; adding fields or moving into the filter row later re-densifies chrome and conflates debt with Track.
epics: [003]
