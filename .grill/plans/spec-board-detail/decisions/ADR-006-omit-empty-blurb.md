adr: 006
plan: spec-board-detail
date: 2026-08-29
status: accepted

context: spec.md may lack ## Executive Summary; expand still needs a defined empty behavior.
decision: Omit the blurb when Executive Summary is missing or empty. Do not invent fallback text. Card still expands (tasks and Done Archive/Unarchive remain).
why: User 2026-08-29 accepted recommendation. "Poca información"; invented copy would lie about the spec.
alternatives: [KPI as fallback, H1 as blurb, placeholder copy]
irreversible-because: Parser contract is "best-effort omit", same class as today's missing checklist_percent=-1.
epics: [epic-001]
