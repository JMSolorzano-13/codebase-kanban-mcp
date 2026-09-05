adr: 002
plan: tech-debt-and-epics-registry
date: 2026-09-01
status: accepted

context: User confirmed “open” debt only. sdd Status = identified→deferred→in_progress→resolved. gamedev closes a backlog line with `resolved-by: SYS-…` on the same entry. This repo’s TECH_DEBT.md is all resolved today — a resolved filter must yield empty/omit.
decision: Specs list = TD-NNN whose Status ≠ resolved. Game list = backlog lines tagged debt:* (debt:gate-* / debt:adopt-gap-* / debt:<slug>) that do not contain resolved-by. Design/tech tags without debt: are out. Parse-table vs heading details = spec-level.
why: Mirrors how each skill already marks done. Showing resolved would be a graveyard the operator already has in the file.
alternatives: [all statuses, only identified, only critical/high, debt:gate-* only]
irreversible-because: Filter is the product meaning of “deuda en el tablero”; widening later re-floods chrome.
epics: [001, 003]
