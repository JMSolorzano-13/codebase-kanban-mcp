/**
 * @sdd-task: Task #3 - GameBoardTab strip + leftover Vitest
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-072 always-emit debt; SDD-ADR-074 Game-only strip
 * @sdd-why: GameBoard.debt?: GameBoardDebt[] (id, title); missing treated as []
 * @human-debug: If Game strip throws on old mocks → debt not optional; if Specs shows Game ids → wrong board
 */
/* Graph data types matching the C layout3d.c JSON output */

export interface GraphNode {
  id: number;
  x: number;
  y: number;
  z: number;
  label: string;
  name: string;
  file_path?: string;
  qualified_name?: string;
  start_line?: number;
  end_line?: number;
  size: number;
  color: string;
  /* Dead-code classification from the backend layout (layout3d.c). */
  status?: NodeStatus;
  in_calls?: number;
}

export type NodeStatus =
  | "dead"
  | "single"
  | "entry"
  | "test"
  | "exported"
  | "normal"
  | "structural";

/* Git remote metadata for building GitHub deep-links (/api/repo-info). */
export interface RepoInfo {
  root_path: string;
  branch: string;
  remote_url: string;
  web_base: string; /* e.g. github.com/<org>/<repo> */
  blob_base: string; /* e.g. github.com/<org>/<repo>/blob/<branch> */
}

export interface GraphEdge {
  source: number;
  target: number;
  type: string;
}

export interface LinkedProject {
  project: string;
  nodes: GraphNode[];
  edges: GraphEdge[];
  offset: { x: number; y: number; z: number };
  cross_edges: GraphEdge[];
}

/* Missed-graph skeleton (#963): the file structure of files the indexer
 * could not fully cover, laid out as a satellite cluster beside the code
 * galaxy (server-computed offset, same shape as LinkedProject's). */
export interface MissedGraph {
  nodes: GraphNode[];
  edges: GraphEdge[];
  offset: { x: number; y: number; z: number };
}

export interface GraphData {
  nodes: GraphNode[];
  edges: GraphEdge[];
  total_nodes: number;
  linked_projects?: LinkedProject[];
  missed_graph?: MissedGraph;
}

export interface Project {
  name: string;
  root_path: string;
  indexed_at: string;
  /** Server realpath of root_path; absent on pre-spec-003 daemons. */
  canonical_root?: string;
}

export interface SchemaInfo {
  node_labels: { label: string; count: number }[];
  edge_types: { type: string; count: number }[];
  total_nodes: number;
  total_edges: number;
}

/** Closed set. Const order is not strip display order (Game shown: Graph | Game | ADR). */
export const WORKSPACE_TABS = ["graph", "specs", "adr", "game"] as const;
export type WorkspaceTabId = (typeof WORKSPACE_TABS)[number];
export type TabId = "dashboard" | WorkspaceTabId;

export function isWorkspaceTab(tab: string | null): tab is WorkspaceTabId {
  return tab !== null && (WORKSPACE_TABS as readonly string[]).includes(tab);
}

/* Spec board (sdd-skill Kanban) — matches spec_board.c's JSON output.
 * Zero-write / best-effort: everything here is derived from files sdd-skill's
 * own cycle already produces (active.json, tasks.md, state.md,
 * history/test_results.log, checklist.md). checklist_percent is -1 when the
 * spec has no checklist.md "TOTAL" row yet (nothing to show, not an error). */
export interface SpecTask {
  number: number;
  name: string;
  done: boolean;
  current: boolean;
}

export type SpecColumn = "todo" | "in_progress" | "done";

export interface SpecBoardEntry {
  id: string;
  title: string;
  blurb: string;
  column: SpecColumn;
  archived?: boolean;
  active: boolean;
  current_agent: string;
  blocked_note: string;
  task_count: number;
  tasks_done: number;
  checklist_percent: number;
  tasks: SpecTask[];
}

export interface SpecBoardEpic {
  kind: "epic";
  id: string;
  title: string;
  summary: string;
  plan_title: string;
  column: "todo";
}

export interface SpecBoardDebt {
  id: string;
  title: string;
}

export interface SpecBoard {
  sdd_skill_present: boolean;
  grill_skill_present?: boolean;
  specs: SpecBoardEntry[];
  epics?: SpecBoardEpic[];
  debt?: SpecBoardDebt[];
}

/** GET /api/game-board JSON. Arrays are card objects; pane paints them. */
export type GamePhase = "01-preproduction" | "02-production" | "03-postproduction";

export type GameBoardTrack = "A" | "B" | "H";

export type GameBoardWorkState = "pending" | "in_progress" | "done" | "blocked";

export interface GameBoardTask {
  number: number;
  name: string;
  done: boolean;
}

export interface GameBoardBlocked {
  owner: string;
  task: string;
  blocked_by: string;
}

export interface GameBoardDebt {
  id: string;
  title: string;
}

export interface GameBoardCard {
  kind: "artifact" | "epic";
  id: string;
  title: string;
  track: GameBoardTrack | null;
  work_state: GameBoardWorkState | null;
  owner: string;
  continue: string;
  summary: string;
  plan_title: string;
  blurb: string;
  tasks: GameBoardTask[];
  inputs: string;
  last_decision: string;
  open: string;
  recent: string;
  blocked_by: string | null;
  archived: boolean;
}

export interface GameBoard {
  gamedev_skill_present: boolean;
  phase: GamePhase | null;
  focus: string | null;
  continue: string;
  blocked: GameBoardBlocked[];
  inbox: GameBoardCard[];
  preproduction: GameBoardCard[];
  production: GameBoardCard[];
  postproduction: GameBoardCard[];
  debt?: GameBoardDebt[];
}

export interface ProcessInfo {
  pid: number;
  cpu: number;
  rss_mb: number;
  elapsed: string;
  command: string;
  is_self: boolean;
}
