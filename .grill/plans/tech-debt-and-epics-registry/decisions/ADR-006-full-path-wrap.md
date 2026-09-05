adr: 006
plan: tech-debt-and-epics-registry
date: 2026-09-01
status: accepted

context: EpicCard + InboxCard already paint short name (name:) then id = repo-relative `.grill/plans/<slug>/epics/epic-NNN-<name>.md`. CSS truncate clips the filename — operator loses which epic it is. User: keep short name; show complete path.
decision: Specs Todo epic cards and Game Inbox epic cards: keep title = short name; path line under it wraps (no truncate). Same id field; no second identifier. Does not change conversion keys. Non-epic cards out of this ADR unless they reuse the same line.
why: Path already carries the epic filename; wrapping is the fix. Truncate was a chrome choice, not a data limit.
alternatives: [filename only, tooltip-on-hover, truncate middle, drop path]
irreversible-because: Operators will rely on reading the filename on the card; re-truncating hides identity again.
epics: [001, 002]
