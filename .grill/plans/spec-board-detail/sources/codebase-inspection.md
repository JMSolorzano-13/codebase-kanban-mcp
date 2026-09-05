# source: codebase inspection — spec-board-detail
date: 2026-08-29
entry: mixed (phrase + existing graph-ui + spec_board.c)

prior-plan: executive-ui-ia (draft) — Kanban redesign and write-back to .sdd-skill/ were explicit OUT
this-repo-sdd: .sdd-skill/ present; active.json status=completed; 4 specs in completed_specs
linked_gamedev: none

kanban-ui:
  host: graph-ui SpecBoardTab.tsx — 3 cols todo / in_progress / done
  poll: useSpecBoard GET /api/spec-board ~4s
  card-expand: SpecCard click toggles TaskList ONLY if entry.active && task_count>0
  todo+done cards: cursor-default, no expand, title || id
  TaskList already: #N name + done/current dots (inline, not floating overlay)
  no archive column, no show/hide control

spec-board-c:
  file: src/ui/spec_board.c + .h — documented zero-write, best-effort
  columns from active.json: planned_specs+draft_specs→todo; active_spec→in_progress; completed_specs→done
  title+tasks+checklist+agent+blocker: parsed ONLY for the active spec (loop after active.json)
  planned/draft/done entries: id+column only (title empty, tasks empty)
  spec.md: H1 title only (read_spec_title); Executive Summary NOT parsed
  no "archived" field anywhere

sdd-skill-contract:
  spec.md Status enum: draft|approved|in_progress|completed — no archived
  active.json keys: active_spec, planned_specs, draft_specs, completed_specs — no archived_specs
  this project spec.md has "## Executive Summary" (2-3 paragraphs template)

last-indexed:
  helper: graph-ui/src/lib/formatIndexedAt.ts
  SDD-ADR-003: Intl timeZone:"UTC" + timeZoneName:short; <time dateTime={iso}>
  surfaces: Dashboard rows, WorkspaceHeader, AdrTab generated stamp
  ISO stored as-is (Z); display forced UTC — operator at UTC-6 sees wall-clock UTC
