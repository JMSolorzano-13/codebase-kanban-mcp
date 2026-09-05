epic: 001
plan: executive-ui-ia
name: executive-dashboard
status: detailed

summary: Account home is a grayscale Dashboard — project list + Control on one screen, no node/edge dumps, last-indexed visible, create-index without alias field.
delivery-rationale: Operator opens localhost:9749 and sees the new executive home end-to-end without entering a project.

tech:
  facts:
    - App.tsx: global tabs specs|graph|stats|control; default specs; ?tab=&project=
    - StatsTab = current project list; fetches get_graph_schema per project via useProjects
    - ControlTab polls /api/processes 3s and /api/logs 2s; not project-scoped
    - CreateIndexModal POST /api/index {root_path, project_name?}; IndexProgress polls /api/index-status
    - Project.indexed_at already on list_projects payload
    - globals.css primary #1DA27E; header bg #0b1920; i18n en+zh
    - SpecBoard ProjectPicker also uses useProjects
  open-qs:
    1. Route: dashboard as default URL vs keep ?tab=stats alias for old bookmarks?
    2. Stop per-project get_graph_schema on Dashboard — split useProjects so Specs picker does not regress?
    3. Chrome token rewrite: replace --color-primary globally or scope graph-ui chrome so GraphTab CSS vars do not desaturate?
    4. Control embed: same ScrollArea page (stack) vs two-pane; how do 3s/2s polls interact with project-list refresh?
    5. POST /api/index without project_name — confirm C path already derives name when omitted (cbm_project_name_from_path)?
    6. HealthDot stays (semantic green/amber/red exception per ADR-002) or grayscale + text?

product:
  facts:
    - User: first screen = projects + Control; no Nodes/Edges sections
    - User: last update/reindex datetime on each Dashboard row
    - User: executive dark grayscale chrome, contrast for buttons/links
    - Create flow stays folder-browse + Index This Folder
    - Delete-with-confirm already on StatsTab
  open-qs:
    1. Zero-project empty state: show Control + CTA to index, or hide Control until first project?
    2. Indexing-in-progress: keep IndexProgress banner on Dashboard?
    3. Card actions this epic: Enter + Delete + New Index only — any other detail besides name/path/date/health?
    4. Control density: full current gauges/process-grid/logs, or compact summary with expand?
    5. After successful first index: stay on Dashboard or auto-enter Graph (ADR-008 is enter-click, not post-create)?

deps: [chrome grayscale tokens — introduced here, reused by 002/003]
adrs: [ADR-002, ADR-003, ADR-010, ADR-011, ADR-012]
glossary-refs: [Dashboard, Control, Chrome, Last indexed, Nodes/Edges sections]
