# source: codebase inspection — executive-ui-ia
date: 2026-08-29
entry: mixed (phrase + existing graph-ui)

product:
  name: codebase-memory-mcp
  kind: native C MCP server + embedded 3D graph UI
  ui-bind: localhost:9749, served from binary, daemon-owned HTTP
  mcp-tools: 15 (index, list, delete, status, search_graph, trace_path, detect_changes, query_graph, get_graph_schema, get_code_snippet, get_architecture, search_code, manage_adr, ingest_traces, check_index_coverage)
  persistence: ~/.cache/codebase-memory-mcp/ per-project SQLite .db
  linked_sdd: none
  linked_gamedev: none

current-ui-ia:
  shell: App.tsx global header tabs
  tab-ids: specs | graph | stats | control
  default-tab: specs (comment: "Kanban first thing on open")
  routing: ?tab=&project= querystring, history push/pop
  graph-tab: disabled until selectedProject
  stats-tab-click: navigates to graph
  specs-tab: own ProjectPicker when no project
  control-tab: global process/logs, not project-scoped
  TabBar.tsx: empty stub, tabs inlined in App.tsx

current-theme:
  file: graph-ui/src/styles/globals.css
  bg: #0a161a
  primary: #1DA27E (teal-green)
  accent: #1C8585
  header: bg-[#0b1920]
  gauges/health: emerald/teal + yellow/red status
  user-ask: replace with dark grayscale, contrast between surfaces for buttons/links

current-dashboard (StatsTab):
  aggregate-cards: project-count + total-nodes + total-edges
  per-card: HealthDot, name, root_path, AdrButton, View Graph, delete
  per-card-stats: node count, edge count, node-label chips (Function/Class/…)
  create: CreateIndexModal (browse FS + optional project_name override)
  schema-fetch: useProjects calls get_graph_schema per project (drives node/edge UI)

adr-investigation:
  what: Architecture Decision Record — markdown doc per project
  storage: SQLite table project_summaries (project, summary, created_at, updated_at)
  legacy: <root>/.codebase-memory/adr.md migrated on read (#256)
  mcp: manage_adr modes get|update|store|sections; writes serialized behind project mutation guard
  ui: AdrButton modal on StatsTab project cards; GET/POST /api/adr
  graph-hooks: get_architecture + get_graph_schema emit adr_present + hint to write one
  sections-hint: PURPOSE, STACK, ARCHITECTURE, PATTERNS, TRADEOFFS, PHILOSOPHY
  operational?: NO — does not drive index, graph queries, watchers, processes, or control
  graph-understanding?: optional agent-facing notes, not required to read/use the graph
  verdict: documentation / agent memory across sessions. Not control, not schema, not a graph primitive.

path-vs-project-today:
  identity-key: project NAME (filename of .db + query prefix)
  name-source: cbm_project_name_from_path(repo_path) OR optional project_name override
  ui-copy: "Project ID (optional — permanent, cannot be renamed)"
  uniqueness: keyed by name, NOT by canonical root_path
  same-path-two-names: possible via name override
  same-name-two-paths: reindex of that name replaces/owns that .db
  1-1-enforced: NO — this plan must add it
  index-api: POST /api/index { root_path, project_name? }

surfaces-to-touch (inspected, not a solution):
  graph-ui: App.tsx, StatsTab.tsx, ControlTab.tsx, SpecBoardTab.tsx, GraphTab.tsx, globals.css, i18n.ts, types.TabId, useProjects.ts
  backend-likely: /api/index, index_repository name/path resolve, list_projects
  keep: 3D GraphTab internals, SpecBoard read of .sdd-skill, Control process/logs data
