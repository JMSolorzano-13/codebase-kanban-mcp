/**
 * @sdd-task: Task #2 - Workspace header
 * @sdd-spec: specs/spec-002-p8w-project-workspace/spec.md
 * @sdd-decision: SDD-ADR-013 - Workspace last-indexed from useProjects only
 * @sdd-why: Freshness in the workspace chrome uses the same list cache as Dashboard; ghost names omit time
 * @human-debug: If ghost shows a time → listed lookup used the wrong name or invented a second freshness field
 */
import { formatIndexedAt } from "../lib/formatIndexedAt";
import { useUiLanguage, useUiMessages } from "../lib/i18n";
import { useProjects } from "../hooks/useProjects";

export interface WorkspaceHeaderProps {
  projectName: string;
  onLeave: () => void;
}

export function WorkspaceHeader({ projectName, onLeave }: WorkspaceHeaderProps) {
  const t = useUiMessages();
  const lang = useUiLanguage();
  const { projects } = useProjects();
  const listed = projects.find((p) => p.name === projectName);

  return (
    <div className="flex items-center gap-2 px-3 py-1 rounded-lg bg-white/[0.04] border border-border/30">
      <span className="text-[11px] text-primary font-mono truncate max-w-[300px]">
        {projectName}
      </span>
      {listed ? (
        <p className="text-[11px] text-foreground/35 truncate">
          <span className="text-foreground/25 mr-1.5">{t.projects.lastIndexed}</span>
          <time dateTime={listed.indexed_at} title={listed.indexed_at}>
            {formatIndexedAt(listed.indexed_at, lang)}
          </time>
        </p>
      ) : null}
      <button
        type="button"
        aria-label={t.graph.backToDashboard}
        onClick={onLeave}
        className="text-foreground/20 hover:text-foreground/50 text-[12px] ml-1 transition-colors"
      >
        ×
      </button>
    </div>
  );
}
