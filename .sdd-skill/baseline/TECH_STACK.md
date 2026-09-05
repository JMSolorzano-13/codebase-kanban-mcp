# Tech Stack
Updated: 2026-08-30

## Engine
| Package | Version | Purpose | Why Chosen |
| C | C11 (`-std=c11`) | indexer, daemon, HTTP, MCP | existing product |
| Make | Makefile.cbm | build | `-Wall -Wextra -Werror` |
| SQLite | vendored | per-project graph + `project_summaries` | existing |
| tree-sitter | vendored grammars | parse | existing |
| yyjson | vendored | JSON | existing |

## Frontend (graph-ui)
| Package | Version | Purpose | Why Chosen |
| react / react-dom | ^19.0.0 | UI | existing |
| vite | ^6.4.3 | build | existing |
| typescript | ^5.7.0 | types | existing |
| tailwindcss | ^4.1.0 | tokens | existing |
| @react-three/fiber | ^9.5.0 | 3D graph | existing |
| three | ~0.183.0 | 3D | existing |
| vitest | ^4.1.0 | unit/component | existing |
| @testing-library/react | ^16.1.0 | DOM tests | existing |

## Environment Setup
### Development
```bash
# engine
scripts/build.sh --with-ui
# UI only
cd graph-ui && npm test
# UI bind (daemon): default port 9749
```
### Testing
```bash
cd graph-ui && npm test
cd graph-ui && npm run test:coverage
# C suite via project make/scripts (do not invent new runners)
```
### Production
```bash
scripts/build.sh --with-ui --version <tag>
```

## Environment Variables
`CBM_NO_CCACHE=1` — disable ccache. UI port persisted via daemon config (`ui_port`, default 9749). No graph-ui `.env` required for this spec.

## Database Schema
Engine+SQLite, one file per project name under `~/.cache/codebase-memory-mcp/`. No global projects table. Path 1:1 is admission-time (spec-003), not UNIQUE SQL. `list_projects` rows: `name`, `root_path`, `indexed_at`, `canonical_root`. ADR text in `project_summaries` (Phase 2 markers in-document). POST `/api/adr` body max 32768 (was 16384).

## Index admission (spec-003)
POST `/api/index` `{root_path}` = create (409 `path_exists` / `name_exists` if blocked). `{root_path, project}` = reindex of that existing name. MCP `index_repository` without `name` on a single-owner Path = reindex.

## Configuration Files
| File | What |
| Makefile.cbm | C flags, link |
| scripts/build.sh | release/dev binary |
| graph-ui/package.json | UI deps |
| graph-ui/src/styles/globals.css | chrome + (today) teal primary |
| src/cli/cli.c / src/ui/config.c | UI port 9749 |

## UI routing (graph-ui)
Query: `?tab=` + optional `?project=`. Account home: `dashboard` (aliases: `stats`, `control`, unknown, workspace tab without project). Workspace (requires project): `graph` | `game` | `specs` | `adr`. Game tab when one-shot GET `/api/game-board` is 200 and `gamedev_skill_present === true` (Specs omitted). Specs tab when Game is not shown and GET `/api/spec-board` is 200 and (`sdd_skill_present` OR `grill_skill_present`). Leftover `?tab=specs` on a gamedev path becomes `tab=game`.

## Performance Targets
Dashboard: zero `get_graph_schema` calls to render the list. Control polls stay 3s processes / 2s logs. Workspace header: same list cache; no index-status. Specs strip: one-shot spec-board, not the 4s Kanban poll. Game strip: one-shot game-board (dual with spec-board).

## Monitoring & Logs
ControlTab: `/api/processes`, `/api/logs`. Daemon logs via existing Control panel.
