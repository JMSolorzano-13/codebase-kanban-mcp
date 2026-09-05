adr: 012
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: CreateIndexModal and index_repository accept optional project_name. That alias is how one Path becomes two Projects.
decision: Remove the optional Project ID field from the UI. Project name is derived from Path only. MCP/API name override, if it remains for agents, must still obey ADR-001 (reject if Path or derived name already exists) — do not offer a second identity in the Dashboard create flow.
why: User Q2=quitarlo. Path = Project. Alias field recreates duplicates.
alternatives: [keep override with uniqueness checks, alias-as-display-only]
irreversible-because: Removes a public create-index input and the "permanent cannot be renamed" copy. Existing custom-named DBs still exist; they are not renamed here (ADR-009 handles clashes).
epics: []
