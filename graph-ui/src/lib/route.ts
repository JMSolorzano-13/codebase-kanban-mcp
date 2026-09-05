/**
 * @sdd-task: Task #2 - TabId, route kernels, strip, i18n
 * @sdd-spec: specs/spec-010-c4h-game-tab-silent-win/spec.md
 * @sdd-decision: SDD-ADR-043
 * @sdd-why: closed set +game; dual fallback; strip showGame
 * @human-debug: If tab=game ignored → WORKSPACE_TABS / isWorkspaceTab
 */
import { isWorkspaceTab, type TabId } from "./types";

export interface RouteState {
  tab: TabId;
  project: string | null;
}

/* Workspace tab + non-empty project keeps that tab. stats/control/dashboard/
 * unknown/missing and any workspace tab without project are Dashboard with
 * project cleared so old bookmarks cannot keep a stale selection.
 * Specs-without-skill URL rewrite uses fallbackSpecsToGraph (presence).
 * Game-without-skill uses fallbackGameToGraph; leftover tab=specs on a
 * gamedev path uses resolveWorkspaceTab (specs → game, not Graph). */
export function readRoute(): RouteState {
  const params = new URLSearchParams(window.location.search);
  const rawTab = params.get("tab");
  const project = params.get("project");
  if (isWorkspaceTab(rawTab) && project) {
    return { tab: rawTab, project };
  }
  return { tab: "dashboard", project: null };
}

export function routeUrl(tab: TabId, project: string | null): string {
  const params = new URLSearchParams();
  params.set("tab", tab);
  if (project) params.set("project", project);
  return `${window.location.pathname}?${params.toString()}${window.location.hash}`;
}

/* Immediate Graph fallback while Specs is omitted (loading / false / error).
 * App #5 must replaceState; this helper is the testable kernel. */
export function fallbackSpecsToGraph(tab: TabId, present: boolean): TabId {
  if (tab === "specs" && !present) return "graph";
  return tab;
}

/* Immediate Graph fallback while Game is omitted (loading / false / error).
 * Same shape as fallbackSpecsToGraph; do not fold the two kernels together. */
export function fallbackGameToGraph(tab: TabId, present: boolean): TabId {
  if (tab === "game" && !present) return "graph";
  return tab;
}

/* Leftover tab=specs on a gamedev path becomes game (not Graph), then the
 * two presence kernels. App passes settled-aware showSpecs / showGame. */
export function resolveWorkspaceTab(
  tab: TabId,
  specsPresent: boolean,
  gamePresent: boolean,
): TabId {
  const afterSilentWin = tab === "specs" && gamePresent ? "game" : tab;
  return fallbackSpecsToGraph(fallbackGameToGraph(afterSilentWin, gamePresent), specsPresent);
}
