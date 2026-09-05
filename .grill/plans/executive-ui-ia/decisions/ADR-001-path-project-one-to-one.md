adr: 001
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: Today a Project is keyed by name; optional project_name lets the same Path be indexed twice under different IDs.
decision: Path ↔ Project is always 1:1. Creating a Path or Project that already exists is blocked; UI redirects to the existing project.
why: User: a project IS a path; duplicate indexes are invalid; Dashboard must open the existing one instead of creating another.
alternatives: [keep name-override aliases, merge/repoint on conflict, warn-but-allow]
irreversible-because: Changes identity of indexes, /api/index + index_repository contract. Existing-row handling = ADR-009.
epics: []
