/**
 * @sdd-task: Task #2 - TabId, route kernels, strip, i18n
 * @sdd-spec: specs/spec-010-c4h-game-tab-silent-win/spec.md
 * @sdd-decision: SDD-ADR-043
 * @sdd-why: closed set +game; dual fallback; strip showGame
 * @human-debug: If tab=game ignored → WORKSPACE_TABS / isWorkspaceTab
 */
/* @vitest-environment jsdom */
import { afterEach, describe, expect, it } from "vitest";
import {
  fallbackGameToGraph,
  fallbackSpecsToGraph,
  readRoute,
  resolveWorkspaceTab,
  routeUrl,
} from "./route";
import { WORKSPACE_TABS } from "./types";

describe("route kernel", () => {
  afterEach(() => {
    window.history.replaceState(null, "", "/");
  });

  it("exports WORKSPACE_TABS as graph, specs, adr, game", () => {
    expect(WORKSPACE_TABS).toEqual(["graph", "specs", "adr", "game"]);
  });

  it("keeps graph, specs, adr, and game when project is present", () => {
    window.history.replaceState(null, "", "/?tab=graph&project=alpha");
    expect(readRoute()).toEqual({ tab: "graph", project: "alpha" });

    window.history.replaceState(null, "", "/?tab=specs&project=alpha");
    expect(readRoute()).toEqual({ tab: "specs", project: "alpha" });

    window.history.replaceState(null, "", "/?tab=adr&project=alpha");
    expect(readRoute()).toEqual({ tab: "adr", project: "alpha" });

    window.history.replaceState(null, "", "/?tab=game&project=alpha");
    expect(readRoute()).toEqual({ tab: "game", project: "alpha" });
  });

  it("maps a workspace tab without project to Dashboard", () => {
    for (const search of [
      "/?tab=graph",
      "/?tab=specs",
      "/?tab=adr",
      "/?tab=game",
      "/?tab=adr&project=",
    ]) {
      window.history.replaceState(null, "", search);
      expect(readRoute()).toEqual({ tab: "dashboard", project: null });
    }
  });

  it("aliases stats, control, dashboard, unknown, and missing tab to Dashboard", () => {
    for (const search of ["/?tab=stats", "/?tab=control", "/?tab=dashboard", "/?tab=unknown", "/"]) {
      window.history.replaceState(null, "", search);
      expect(readRoute()).toEqual({ tab: "dashboard", project: null });
    }
  });

  it("clears project on Dashboard aliases even if project is in the query", () => {
    window.history.replaceState(null, "", "/?tab=stats&project=alpha");
    expect(readRoute()).toEqual({ tab: "dashboard", project: null });
  });

  it("builds routeUrl with tab and optional project", () => {
    window.history.replaceState(null, "", "/ui#x");
    expect(routeUrl("dashboard", null)).toBe("/ui?tab=dashboard#x");
    expect(routeUrl("graph", "alpha")).toBe("/ui?tab=graph&project=alpha#x");
    expect(routeUrl("adr", "alpha")).toBe("/ui?tab=adr&project=alpha#x");
    expect(routeUrl("game", "alpha")).toBe("/ui?tab=game&project=alpha#x");
  });

  it("falls specs back to graph when sdd-skill is not present", () => {
    expect(fallbackSpecsToGraph("specs", false)).toBe("graph");
    expect(fallbackSpecsToGraph("specs", true)).toBe("specs");
    expect(fallbackSpecsToGraph("graph", false)).toBe("graph");
    expect(fallbackSpecsToGraph("adr", false)).toBe("adr");
    expect(fallbackSpecsToGraph("dashboard", false)).toBe("dashboard");
    expect(fallbackSpecsToGraph("game", false)).toBe("game");
  });

  it("falls game back to graph when gamedev is not present", () => {
    expect(fallbackGameToGraph("game", false)).toBe("graph");
    expect(fallbackGameToGraph("game", true)).toBe("game");
    expect(fallbackGameToGraph("graph", false)).toBe("graph");
    expect(fallbackGameToGraph("adr", false)).toBe("adr");
    expect(fallbackGameToGraph("specs", false)).toBe("specs");
    expect(fallbackGameToGraph("dashboard", false)).toBe("dashboard");
  });

  it("resolveWorkspaceTab sends leftover specs to game when gamedev is present", () => {
    expect(resolveWorkspaceTab("specs", true, true)).toBe("game");
    expect(resolveWorkspaceTab("specs", false, true)).toBe("game");
    expect(resolveWorkspaceTab("game", false, true)).toBe("game");
    expect(resolveWorkspaceTab("game", true, false)).toBe("graph");
    expect(resolveWorkspaceTab("specs", false, false)).toBe("graph");
    expect(resolveWorkspaceTab("specs", true, false)).toBe("specs");
    expect(resolveWorkspaceTab("graph", true, true)).toBe("graph");
    expect(resolveWorkspaceTab("adr", true, true)).toBe("adr");
  });
});
