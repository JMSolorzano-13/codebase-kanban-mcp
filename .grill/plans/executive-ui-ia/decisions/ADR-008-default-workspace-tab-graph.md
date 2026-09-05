adr: 008
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: Entering a project must pick one tab. Today App.tsx defaults to specs.
decision: Enter project → Graph tab. Specs/ADR reachable from workspace tabs after that.
why: User Q1=Graph. Graph is the product surface; Specs is conditional; ADR is a document.
alternatives: [Specs first, last-used tab, ADR first]
irreversible-because: Changes default route and the current "Kanban first on open" comment in App.tsx.
epics: []
