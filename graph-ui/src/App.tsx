/**
 * @sdd-task: Task #5 - App routing + TabBar delete
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-008 - TabId dashboard|graph; stats/control/specs alias home
 * @sdd-why: Account home is Dashboard; Graph is Enter/deep-link only; no lab tab strip
 * @human-debug: If Specs opens on load → readRoute still defaults to specs; if Graph without project → missing project guard
 */
import { useCallback, useEffect, useState } from "react";
import { GraphTab } from "./components/GraphTab";
import { Dashboard } from "./components/Dashboard";
import type { TabId } from "./lib/types";
import { useUiMessages } from "./lib/i18n";

interface RouteState {
  tab: TabId;
  project: string | null;
}

/* Graph only when both tab=graph and a non-empty project; every other
 * query (missing, unknown, stats, control, specs) is Dashboard with
 * project cleared so old bookmarks cannot keep a stale selection. */
function readRoute(): RouteState {
  const params = new URLSearchParams(window.location.search);
  const rawTab = params.get("tab");
  const project = params.get("project");
  if (rawTab === "graph" && project) {
    return { tab: "graph", project };
  }
  return { tab: "dashboard", project: null };
}

function routeUrl(tab: TabId, project: string | null): string {
  const params = new URLSearchParams();
  params.set("tab", tab);
  if (project) params.set("project", project);
  return `${window.location.pathname}?${params.toString()}${window.location.hash}`;
}

export function App() {
  const t = useUiMessages();
  const [route, setRoute] = useState<RouteState>(readRoute);
  const { tab: activeTab, project: selectedProject } = route;

  /* First load writes the canonical query. Dashboard never keeps project=. */
  useEffect(() => {
    const initial = readRoute();
    window.history.replaceState(null, "", routeUrl(initial.tab, initial.project));
  }, []);

  useEffect(() => {
    const onPopState = () => setRoute(readRoute());
    window.addEventListener("popstate", onPopState);
    return () => window.removeEventListener("popstate", onPopState);
  }, []);

  const navigate = useCallback((tab: TabId, project: string | null) => {
    const url = routeUrl(tab, project);
    const current = `${window.location.pathname}${window.location.search}${window.location.hash}`;
    if (url === current) return;
    window.history.pushState(null, "", url);
    setRoute({ tab, project });
  }, []);

  const showGraph = activeTab === "graph" && Boolean(selectedProject);

  return (
    <div className="h-screen flex flex-col bg-background text-foreground">
      <header className="flex items-center justify-between px-5 h-12 border-b border-border bg-card/80 backdrop-blur-md shrink-0">
        <div className="flex items-center gap-2.5">
          <div className="w-[7px] h-[7px] rounded-full bg-primary" />
          <span className="text-[13px] font-semibold text-foreground/90 tracking-tight">
            Codebase Memory
          </span>
        </div>

        {showGraph && (
          <div className="flex items-center gap-2 px-3 py-1 rounded-lg bg-white/[0.04] border border-border/30">
            <span className="text-[10px] text-foreground/30 uppercase tracking-wider">
              {t.graph.selectedLabel}
            </span>
            <span className="text-[11px] text-primary font-mono truncate max-w-[300px]">
              {selectedProject}
            </span>
            <button
              type="button"
              aria-label={t.graph.backToDashboard}
              onClick={() => navigate("dashboard", null)}
              className="text-foreground/20 hover:text-foreground/50 text-[12px] ml-1 transition-colors"
            >
              ×
            </button>
          </div>
        )}
      </header>

      <main className="flex-1 min-h-0">
        {showGraph ? (
          <GraphTab project={selectedProject} />
        ) : (
          <Dashboard onSelectProject={(p) => navigate("graph", p)} />
        )}
      </main>
    </div>
  );
}
