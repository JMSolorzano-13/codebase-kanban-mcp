adr: 003
plan: tech-debt-and-epics-registry
date: 2026-09-01
status: accepted

context: User wants Game Inbox = grill epics still pending. gamedev v1.12.0 epics_registry Status = not_started|in_progress|closed|parked|evergreen. Inbox today hides via Companion-to / roadmap map (prior ADR-006). Parked is a human pause, not leftover work.
decision: When matching a grill epic to a registry row (Plan = grill slug AND Epic = NNN; no kebab/path fuzzy): hide Inbox iff Status ∈ {in_progress, closed, parked}. not_started stays. No row → stay (faltan). evergreen is Epic 0 only — never a grill Inbox card; ignore. Origin column unused for hide.
why: in_progress/closed = already picked up or done. parked = director parked it; showing it as “falta” would re-nudge paused work. Unlisted = not tracked yet = Inbox.
alternatives: [hide only closed, hide parked+closed keep in_progress, treat no-row as hidden, fuzzy name match]
irreversible-because: Hide set is Inbox truth; adding/removing a status later re-floods or hides leftover epics.
epics: [002]
