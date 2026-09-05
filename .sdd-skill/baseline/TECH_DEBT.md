# Tech Debt Register
Updated: 2026-08-30T00:17:00Z

## TD-001: useProjects N+1 get_graph_schema
ID: TD-001 | Category: performance | Severity: medium | Status: resolved | Identified: 2026-08-29 | Origin: adopt analysis | Spec: spec-001-w3q-executive-dashboard
Description: `useProjects` called `list_projects` then `get_graph_schema` per project. Dashboard no longer shows nodes/edges.
Resolution: Task #1 — hook is `list_projects` only. SpecBoard ProjectPicker uses name+path. SDD-ADR-006.
Acceptance Criteria:
- [x] Dashboard list render issues zero `get_graph_schema` RPC
- [x] Specs ProjectPicker still lists name+path without requiring schema

## TD-002: Project identity is name not path
ID: TD-002 | Category: architecture | Severity: high | Status: resolved | Identified: 2026-08-29 | Origin: adopt analysis | Spec: spec-003-h7q-path-project-identity
Description: Store key is project name. Same path can have two names via `project_name` override. No unique `root_path`.
Resolution: Admission-only Path 1:1 (`cbm_identity_admit` + catalog). No UNIQUE SQL. Create 409 `path_exists`; Reindex `{root_path, project}`; Dashboard conflict enter-newest + confirm-delete older. SDD-ADR-014..018.
Acceptance Criteria:
- [x] Path↔Project 1:1 enforced on create
- [x] Legacy duplicates have a resolution UI

## TD-003: TabBar.tsx is an empty stub
ID: TD-003 | Category: code-quality | Severity: low | Status: resolved | Identified: 2026-08-29 | Origin: adopt analysis | Spec: spec-001-w3q-executive-dashboard
Description: Tabs were inlined in App.tsx; TabBar.tsx unused.
Resolution: Task #5 — TabBar.tsx deleted. Workspace tabs in spec-002 are new code (SDD-ADR-004).
Acceptance Criteria:
- [x] No dead TabBar export, or it is the real tab component

## TD-004: Teal chrome tokens are global
ID: TD-004 | Category: ux | Severity: medium | Status: resolved | Identified: 2026-08-29 | Origin: adopt analysis | Spec: spec-001-w3q-executive-dashboard
Description: `globals.css` primary #1DA27E and header #0b1920 painted all chrome.
Resolution: Task #2 — grayscale chrome tokens; `colorForLabel` / EdgeLines hex locked. SDD-ADR-005.
Acceptance Criteria:
- [x] Chrome grayscale; GraphTab node colors unchanged

## Debt Summary
| ID | Title | Category | Severity | Status |
| TD-001 | useProjects N+1 schema | performance | medium | resolved |
| TD-002 | name-not-path identity | architecture | high | resolved |
| TD-003 | TabBar stub | code-quality | low | resolved |
| TD-004 | global teal tokens | ux | medium | resolved |
Total: 4 — 0 identified, 0 in_progress, 4 resolved

## Notes
Store files remain name-keyed. Path 1:1 is admission-only. Do not add UNIQUE `root_path` without a new spec.
