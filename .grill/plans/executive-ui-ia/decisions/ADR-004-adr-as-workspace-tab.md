adr: 004
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: ADR is agent markdown, not ops. User first said hide if docs-only; then added it as a first-class project tab.
decision: Phase 1 project workspace tabs = Graph + Specs(if .sdd-skill/) + ADR. ADR always present inside a project (empty/editor ok). Remove +ADR from Dashboard cards.
why: User: three tabs now; Specs only if skill exists; ADR is the place to read/edit the record.
alternatives: [ADR only Phase 2, ADR hidden until content exists, keep modal on cards]
irreversible-because: New TabId + routing; Dashboard no longer owns ADR UX.
epics: []
