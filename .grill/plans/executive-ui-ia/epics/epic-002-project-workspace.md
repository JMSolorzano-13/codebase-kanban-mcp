epic: 002
plan: executive-ui-ia
name: project-workspace
status: detailed

summary: Entering a project opens a scalable workspace: default Graph, Specs tab only if .sdd-skill/, ADR tab always (manual editor), last-indexed in the header.
delivery-rationale: One project interior is usable and extensible; Graph/Specs/ADR are reachable without the old global-tab IA.

tech:
  facts:
    - GraphTab requires selectedProject; currently disabled in header without it
    - SpecBoardTab has ProjectPicker when no project; GET /api/spec-board hides when no .sdd-skill/ (http_server comment)
    - spec_board.h: cbm_spec_board_sdd_skill_present; gamedev presence stub always false (future tab)
    - ADR today: AdrButton modal on StatsTab; GET/POST /api/adr; store project_summaries; manage_adr MCP
    - TabBar.tsx is empty stub; tabs inlined in App.tsx
    - TabId = specs|graph|stats|control — must become dashboard vs workspace-tab model
    - Header already shows selected project + X → stats
  open-qs:
    1. Tab registry: hardcoded array now (Graph, Specs?, ADR) with a documented extension point, or more?
    2. Presence of Specs: one GET on enter vs reuse spec-board payload; flicker if late hide?
    3. ADR tab storage: same /api/adr blob; Phase 1 may write whole document — must not paint into a "generated" region that Phase 2 later needs (ADR-007 marker still unspecified)?
    4. Deep links ?project=&tab=graph|specs|adr — map old tab=stats/control to Dashboard?
    5. indexed_at in workspace header: from list cache or extra health/status call?
    6. Graph 3D: do not retouch colorForLabel / layout colors (ADR-011) — any chrome around GraphTab still retokened?

product:
  facts:
    - Default tab Graph (ADR-008)
    - Specs only if .sdd-skill/ exists (hide, not disabled placeholder)
    - ADR always available; manual create/edit; not on Dashboard
    - Last indexed in a workspace header
    - Future tabs must fit without IA rewrite
    - X / leave project returns to Dashboard
  open-qs:
    1. Empty ADR: blank page + save, or starter headings (PURPOSE/STACK/… current textarea placeholder)?
    2. Switch to another project from inside workspace, or only via Dashboard?
    3. Specs missing: simply omit tab — any hint that sdd-skill is absent?
    4. ADR save feedback and unsaved-navigation guard?
    5. Order of tabs: Graph | Specs | ADR or Graph | ADR | Specs?

deps: [epic-001 chrome tokens + Dashboard enter/leave]
adrs: [ADR-004, ADR-007, ADR-008, ADR-010, ADR-011]
glossary-refs: [Project workspace, Graph tab, Specs tab, ADR tab, Last indexed]
