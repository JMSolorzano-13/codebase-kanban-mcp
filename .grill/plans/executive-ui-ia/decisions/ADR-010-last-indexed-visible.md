adr: 010
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: User needs to tell which duplicate is newer and to see freshness of each index.
decision: Show last update / last reindex datetime on every Dashboard project row and again in a project-workspace header. Field already exists on Project as indexed_at (graph-ui types + list_projects) — must be visible, not only used internally for ADR-009.
why: User: show last update or reindex on Dashboard per project and inside the project in a header.
alternatives: [Dashboard only, hover-only, omit]
irreversible-because: Becomes part of the executive Dashboard contract and the conflict "newest" definition.
epics: []
