import { useEffect, useState } from "react";
import { ScrollArea } from "@/components/ui/scroll-area";
import { useProjects } from "../hooks/useProjects";
import { useSpecBoard } from "../hooks/useSpecBoard";
import { useUiMessages } from "../lib/i18n";
import type { SpecBoardEntry, SpecColumn } from "../lib/types";

interface SpecBoardTabProps {
  project: string | null;
  onSelectProject: (project: string) => void;
}

/* ── Project picker (shown when no project is selected yet) ─────────── */

function ProjectPicker({ onSelectProject }: { onSelectProject: (project: string) => void }) {
  const t = useUiMessages();
  const { projects, loading } = useProjects();

  return (
    <div className="p-8 max-w-2xl mx-auto">
      <p className="text-[13px] text-foreground/40 mb-6">{t.specBoard.selectProject}</p>
      {loading ? (
        <p className="text-foreground/20 text-[12px]">{t.common.loading}</p>
      ) : projects.length === 0 ? (
        <p className="text-foreground/20 text-[12px]">{t.projects.noIndexedProjects}</p>
      ) : (
        <div className="flex flex-col gap-2">
          {projects.map(({ project: p }) => (
            <button
              key={p.name}
              onClick={() => onSelectProject(p.name)}
              className="text-left border border-white/10 hover:border-white/20 rounded-lg px-4 py-3 transition-colors"
            >
              <p className="text-foreground/90 text-[13px] font-medium">{p.name}</p>
              <p className="text-foreground/30 text-[11px] font-mono truncate">{p.root_path}</p>
            </button>
          ))}
        </div>
      )}
    </div>
  );
}

/* ── Task mini-list (expanded detail of a spec card) ─────────────────── */

function TaskList({ entry }: { entry: SpecBoardEntry }) {
  const t = useUiMessages();
  if (entry.task_count === 0) {
    return <p className="text-[10px] text-foreground/20 italic mt-2">{t.specBoard.noTasksYet}</p>;
  }
  return (
    <ul className="mt-2 flex flex-col gap-1">
      {entry.tasks.map((task) => (
        <li
          key={task.number}
          className={`flex items-center gap-2 text-[11px] ${
            task.current
              ? "text-primary font-medium"
              : task.done
                ? "text-foreground/30 line-through"
                : "text-foreground/50"
          }`}
        >
          <span
            className={`w-1.5 h-1.5 rounded-full shrink-0 ${
              task.current ? "bg-primary animate-pulse" : task.done ? "bg-emerald-400/60" : "bg-white/15"
            }`}
          />
          <span className="truncate">
            #{task.number} {task.name || ""}
          </span>
        </li>
      ))}
    </ul>
  );
}

/* ── Spec card ────────────────────────────────────────────────────── */

function SpecCard({ entry }: { entry: SpecBoardEntry }) {
  const t = useUiMessages();
  const [expanded, setExpanded] = useState(entry.active);

  useEffect(() => setExpanded(entry.active), [entry.active]);

  const canExpand = entry.active && entry.task_count > 0;

  return (
    <div
      className={`rounded-lg border p-3 transition-colors ${
        entry.active ? "border-primary/30 bg-primary/[0.04]" : "border-white/10 bg-white/[0.02]"
      }`}
    >
      <button
        onClick={() => canExpand && setExpanded((e) => !e)}
        className={`w-full text-left ${canExpand ? "cursor-pointer" : "cursor-default"}`}
      >
        <p className="text-[12px] font-medium text-foreground/90 truncate">
          {entry.title || entry.id}
        </p>
        <p className="text-[10px] text-foreground/30 font-mono truncate mt-0.5">{entry.id}</p>
      </button>

      {entry.active && (
        <div className="flex flex-wrap items-center gap-x-3 gap-y-1 mt-2">
          {entry.current_agent && (
            <span className="text-[10px] text-primary/80" title={t.specBoard.currentAgent}>
              {entry.current_agent}
            </span>
          )}
          {entry.task_count > 0 && (
            <span className="text-[10px] text-foreground/40">
              {t.specBoard.tasksDone(entry.tasks_done, entry.task_count)}
            </span>
          )}
          {entry.checklist_percent >= 0 && (
            <span className="text-[10px] text-foreground/40">
              {t.specBoard.checklistLabel} {entry.checklist_percent.toFixed(0)}%
            </span>
          )}
        </div>
      )}

      {entry.blocked_note && (
        <p className="text-[10px] text-amber-400/80 mt-1.5">⚠ {t.specBoard.blocked}: {entry.blocked_note}</p>
      )}

      {expanded && canExpand && <TaskList entry={entry} />}
    </div>
  );
}

/* ── Column ───────────────────────────────────────────────────────── */

function Column({
  id,
  title,
  entries,
}: {
  id: SpecColumn;
  title: string;
  entries: SpecBoardEntry[];
}) {
  const t = useUiMessages();
  return (
    <div className="flex-1 min-w-0 flex flex-col">
      <div className="flex items-center justify-between mb-3 px-1">
        <h3 className="text-[11px] font-semibold uppercase tracking-wider text-foreground/40">{title}</h3>
        <span className="text-[10px] text-foreground/25">
          {id === "todo" ? t.specBoard.pendingCount(entries.length) : entries.length}
        </span>
      </div>
      <div className="flex flex-col gap-2">
        {entries.length === 0 ? (
          <p className="text-[11px] text-foreground/15 italic px-1">{t.specBoard.noSpecs}</p>
        ) : (
          entries.map((entry) => <SpecCard key={entry.id} entry={entry} />)
        )}
      </div>
    </div>
  );
}

/* ── Tab root ─────────────────────────────────────────────────────── */

export function SpecBoardTab({ project, onSelectProject }: SpecBoardTabProps) {
  const t = useUiMessages();
  const { board, loading } = useSpecBoard(project);

  if (!project) {
    return <ProjectPicker onSelectProject={onSelectProject} />;
  }

  if (loading && !board) {
    return <p className="p-8 text-foreground/20 text-[12px]">{t.common.loading}</p>;
  }

  if (!board || !board.sdd_skill_present) {
    return <p className="p-8 text-foreground/30 text-[12px]">{t.specBoard.notSddSkill}</p>;
  }

  const byColumn = (id: SpecColumn) => board.specs.filter((s) => s.column === id);

  return (
    <ScrollArea className="h-full">
      <div className="p-6 flex gap-6 max-w-6xl mx-auto">
        <Column id="todo" title={t.specBoard.columnTodo} entries={byColumn("todo")} />
        <Column id="in_progress" title={t.specBoard.columnInProgress} entries={byColumn("in_progress")} />
        <Column id="done" title={t.specBoard.columnDone} entries={byColumn("done")} />
      </div>
    </ScrollArea>
  );
}
