/**
 * @sdd-task: Task #4 - Create modal path_exists redirect + notice
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-015 - 409 path_exists opens Graph; name_exists stays in the modal
 * @sdd-why: US-001 redirect + status notice; notice clears on the next navigate
 * @human-debug: If 409 path_exists stays on Dashboard → line 142 (onPathExists); if notice survives leave → line 44 (navigate did not clear pathNotice)
 */
import { useCallback, useEffect, useState } from "react";
import { AdrTab } from "./components/AdrTab";
import { Dashboard } from "./components/Dashboard";
import { GraphTab } from "./components/GraphTab";
import { SpecBoardTab } from "./components/SpecBoardTab";
import { WorkspaceHeader } from "./components/WorkspaceHeader";
import { WorkspaceTabStrip } from "./components/WorkspaceTabStrip";
import { useSddSkillPresent } from "./hooks/useSddSkillPresent";
import { useUiMessages } from "./lib/i18n";
import { fallbackSpecsToGraph, readRoute, routeUrl, type RouteState } from "./lib/route";
import { isWorkspaceTab, type TabId, type WorkspaceTabId } from "./lib/types";

export function App() {
  const t = useUiMessages();
  const [route, setRoute] = useState<RouteState>(readRoute);
  const [adrDirty, setAdrDirty] = useState(false);
  const [pathNotice, setPathNotice] = useState<string | null>(null);
  const { tab: activeTab, project: selectedProject } = route;
  const inWorkspace = Boolean(selectedProject) && isWorkspaceTab(activeTab);
  const { present } = useSddSkillPresent(inWorkspace ? selectedProject : null);

  useEffect(() => {
    const initial = readRoute();
    window.history.replaceState(null, "", routeUrl(initial.tab, initial.project));
  }, []);

  useEffect(() => {
    const onPopState = () => {
      setPathNotice(null);
      setRoute(readRoute());
    };
    window.addEventListener("popstate", onPopState);
    return () => window.removeEventListener("popstate", onPopState);
  }, []);

  const navigate = useCallback((tab: TabId, project: string | null) => {
    setPathNotice(null);
    const url = routeUrl(tab, project);
    const current = `${window.location.pathname}${window.location.search}${window.location.hash}`;
    if (url === current) {
      setRoute({ tab, project });
      return;
    }
    window.history.pushState(null, "", url);
    setRoute({ tab, project });
  }, []);

  const openExistingProject = useCallback((name: string) => {
    setPathNotice(name);
    const url = routeUrl("graph", name);
    const current = `${window.location.pathname}${window.location.search}${window.location.hash}`;
    if (url !== current) {
      window.history.pushState(null, "", url);
    }
    setRoute({ tab: "graph", project: name });
  }, []);

  const replaceRoute = useCallback((tab: TabId, project: string | null) => {
    window.history.replaceState(null, "", routeUrl(tab, project));
    setRoute({ tab, project });
  }, []);

  /* Omit-until-true: specs URL becomes graph immediately, including while loading. */
  useEffect(() => {
    if (!selectedProject || !isWorkspaceTab(activeTab)) return;
    const next = fallbackSpecsToGraph(activeTab, present);
    if (next === activeTab) return;
    replaceRoute(next, selectedProject);
  }, [activeTab, present, selectedProject, replaceRoute]);

  const requestNavigate = useCallback(
    (tab: TabId, project: string | null) => {
      if (adrDirty && !window.confirm(t.adr.unsavedConfirm)) return;
      navigate(tab, project);
    },
    [adrDirty, navigate, t.adr.unsavedConfirm],
  );

  useEffect(() => {
    if (activeTab !== "adr") setAdrDirty(false);
  }, [activeTab]);

  const paneTab: WorkspaceTabId =
    selectedProject && isWorkspaceTab(activeTab)
      ? (fallbackSpecsToGraph(activeTab, present) as WorkspaceTabId)
      : "graph";

  return (
    <div className="h-screen flex flex-col bg-background text-foreground">
      <header className="flex items-center justify-between px-5 h-12 border-b border-border bg-card/80 backdrop-blur-md shrink-0">
        <div className="flex items-center gap-2.5">
          <div className="w-[7px] h-[7px] rounded-full bg-primary" />
          <span className="text-[13px] font-semibold text-foreground/90 tracking-tight">
            Codebase Memory
          </span>
        </div>

        {inWorkspace && selectedProject ? (
          <WorkspaceHeader
            projectName={selectedProject}
            onLeave={() => requestNavigate("dashboard", null)}
          />
        ) : null}
      </header>

      {inWorkspace && selectedProject ? (
        <WorkspaceTabStrip
          selected={paneTab}
          showSpecs={present}
          onSelect={(tab) => requestNavigate(tab, selectedProject)}
        />
      ) : null}

      {pathNotice ? (
        <div
          role="status"
          className="px-5 py-2 border-b border-border bg-card text-[12px] text-foreground/70"
        >
          {t.index.pathExistsNotice(pathNotice)}
        </div>
      ) : null}

      <main className="flex-1 min-h-0">
        {inWorkspace && selectedProject ? (
          paneTab === "specs" && present ? (
            <SpecBoardTab project={selectedProject} onSelectProject={() => undefined} />
          ) : paneTab === "adr" ? (
            <AdrTab project={selectedProject} onDirtyChange={setAdrDirty} />
          ) : (
            <GraphTab project={selectedProject} />
          )
        ) : (
          <Dashboard
            onSelectProject={(p) => navigate("graph", p)}
            onPathExists={openExistingProject}
          />
        )}
      </main>
    </div>
  );
}
