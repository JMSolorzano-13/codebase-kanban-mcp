/**
 * @sdd-task: Task #5 - Archive UI + refresh + remaining Vitest
 * @sdd-spec: specs/spec-012-m2k-game-expand-archive-deps/spec.md
 * @sdd-decision: SDD-ADR-054
 * @sdd-why: pass project + refresh into GameBoardTab; keep one-shot settled gate
 * @human-debug: If Archive POST has empty project → GameBoardTab not given selectedProject; if Game unmounts after Archive → refresh unset settled
 */
import { useCallback, useEffect, useRef, useState } from "react";
import { AdrTab } from "./components/AdrTab";
import { Dashboard } from "./components/Dashboard";
import { GameBoardTab } from "./components/GameBoardTab";
import { GraphTab } from "./components/GraphTab";
import { SpecBoardTab } from "./components/SpecBoardTab";
import { WorkspaceHeader } from "./components/WorkspaceHeader";
import { WorkspaceTabStrip } from "./components/WorkspaceTabStrip";
import { useGameBoard } from "./hooks/useGameBoard";
import { useSddSkillPresent } from "./hooks/useSddSkillPresent";
import { useUiMessages } from "./lib/i18n";
import { readRoute, resolveWorkspaceTab, routeUrl, type RouteState } from "./lib/route";
import { isWorkspaceTab, type TabId, type WorkspaceTabId } from "./lib/types";

export function App() {
  const t = useUiMessages();
  const [route, setRoute] = useState<RouteState>(readRoute);
  const [adrDirty, setAdrDirty] = useState(false);
  const [pathNotice, setPathNotice] = useState<string | null>(null);
  const { tab: activeTab, project: selectedProject } = route;
  const inWorkspace = Boolean(selectedProject) && isWorkspaceTab(activeTab);
  const { present: specsPresent } = useSddSkillPresent(inWorkspace ? selectedProject : null);
  const {
    settled: gameSettled,
    present: gamePresent,
    board: gameBoard,
    refresh: refreshGameBoard,
  } = useGameBoard(inWorkspace ? selectedProject : null);
  const showGame = gameSettled && gamePresent;
  const showSpecs = gameSettled && !showGame && specsPresent;
  /* Capture inbound tab=game / tab=specs before omit-until-true rewrites the URL. */
  const pendingGameDeepLink = useRef(readRoute().tab === "game");
  const pendingSpecsDeepLink = useRef(readRoute().tab === "specs");

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

  /* Omit-until-true uses settled-aware flags. Inbound tab=game restores like specs.
   * Inbound tab=specs + showGame replaces to game (not Specs). */
  useEffect(() => {
    if (!selectedProject || !isWorkspaceTab(activeTab)) return;
    if (showGame && pendingGameDeepLink.current) {
      pendingGameDeepLink.current = false;
      pendingSpecsDeepLink.current = false;
      if (activeTab !== "game") {
        replaceRoute("game", selectedProject);
        return;
      }
    }
    if (showGame && pendingSpecsDeepLink.current) {
      pendingSpecsDeepLink.current = false;
      if (activeTab !== "game") {
        replaceRoute("game", selectedProject);
        return;
      }
    }
    if (showSpecs && pendingSpecsDeepLink.current) {
      pendingSpecsDeepLink.current = false;
      if (activeTab !== "specs") {
        replaceRoute("specs", selectedProject);
        return;
      }
    }
    const next = resolveWorkspaceTab(activeTab, showSpecs, showGame);
    if (next === activeTab) return;
    replaceRoute(next, selectedProject);
  }, [activeTab, showSpecs, showGame, selectedProject, replaceRoute]);

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
      ? (resolveWorkspaceTab(activeTab, showSpecs, showGame) as WorkspaceTabId)
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
          showSpecs={showSpecs}
          showGame={showGame}
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
          paneTab === "game" && showGame && gameBoard ? (
            <GameBoardTab board={gameBoard} project={selectedProject} refresh={refreshGameBoard} />
          ) : paneTab === "specs" && showSpecs ? (
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
