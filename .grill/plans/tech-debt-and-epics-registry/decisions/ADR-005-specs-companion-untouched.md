adr: 005
plan: tech-debt-and-epics-registry
date: 2026-09-01
status: accepted

context: User: the new grill→skill link is gamedev-only. Specs already hides via Companion-to / active.json source.grill_epic (add-epics-plans-kanban ADR-002). sdd-skill has no epics_registry.
decision: Specs conversion matcher stays. Do not read epics_registry.md (or backlog.md) for Specs Todo hide. This plan does not add a Specs inbox-equivalent.
why: Registry is a gamedev-skill artifact; applying it to Specs would hide sdd work that never wrote a registry row.
alternatives: [mirror registry on Specs, TECH_DEBT.md hides Todo epics]
irreversible-because: Coupling Specs hide to a gamedev file would break XOR paths and sdd-only projects.
epics: [001]
