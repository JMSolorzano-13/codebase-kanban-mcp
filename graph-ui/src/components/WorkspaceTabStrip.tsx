/**
 * @sdd-task: Task #2 - TabId, route kernels, strip, i18n
 * @sdd-spec: specs/spec-010-c4h-game-tab-silent-win/spec.md
 * @sdd-decision: SDD-ADR-043
 * @sdd-why: closed set +game; dual fallback; strip showGame
 * @human-debug: If tab=game ignored → WORKSPACE_TABS / isWorkspaceTab
 */
import { useUiMessages } from "../lib/i18n";
import type { WorkspaceTabId } from "../lib/types";

export interface WorkspaceTabStripProps {
  selected: WorkspaceTabId;
  showSpecs: boolean;
  showGame?: boolean;
  onSelect: (tab: WorkspaceTabId) => void;
}

function workspaceTabLabel(
  id: WorkspaceTabId,
  t: ReturnType<typeof useUiMessages>,
): string {
  if (id === "graph") return t.tabs.graph;
  if (id === "specs") return t.tabs.specs;
  if (id === "game") return t.tabs.game;
  return t.tabs.adr;
}

export function WorkspaceTabStrip({
  selected,
  showSpecs,
  showGame = false,
  onSelect,
}: WorkspaceTabStripProps) {
  const t = useUiMessages();
  /* Const order is graph, specs, adr, game. Display when Game is shown is
   * Graph | Game | ADR. Game wins if both flags are true (App must not). */
  const tabs: WorkspaceTabId[] = showGame
    ? ["graph", "game", "adr"]
    : showSpecs
      ? ["graph", "specs", "adr"]
      : ["graph", "adr"];

  return (
    <nav
      role="tablist"
      className="flex items-center gap-1 px-5 h-9 border-b border-border bg-card/60 shrink-0"
    >
      {tabs.map((id) => {
        const isSelected = selected === id;
        const label = workspaceTabLabel(id, t);
        return (
          <button
            key={id}
            type="button"
            role="tab"
            aria-selected={isSelected}
            aria-current={isSelected ? "page" : undefined}
            onClick={() => onSelect(id)}
            className={`px-3 h-full text-[12px] transition-colors ${
              isSelected
                ? "text-foreground/90 border-b border-foreground/50"
                : "text-foreground/40 hover:text-foreground/70"
            }`}
          >
            {label}
          </button>
        );
      })}
    </nav>
  );
}
