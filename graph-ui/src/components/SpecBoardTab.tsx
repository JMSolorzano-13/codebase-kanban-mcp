/**
 * @sdd-task: Task #3 - SpecBoardTab strip + EpicCard wrap + i18n
 * @sdd-spec: specs/spec-015-s5k-specs-debt-and-path/spec.md
 * @sdd-decision: SDD-ADR-067 - Specs-only debt strip; aria-label; dead text; SDD-ADR-068 - EpicCard id wraps
 * @sdd-why: Open-debt region above 3 columns when (board.debt ?? []).length > 0; EpicCard id break-all; SpecCard id truncate locked
 * @human-debug: If strip missing with debt[] → ?? [] or length gate; if strip on Graph → painted outside SpecBoardTab; if epic id ellipsis → truncate still on id line; if spec id wraps → SpecCard id lost truncate
 */
import { useRef, useState } from "react";
import { ScrollArea } from "@/components/ui/scroll-area";
import { useProjects } from "../hooks/useProjects";
import { useSpecBoard } from "../hooks/useSpecBoard";
import { useUiMessages } from "../lib/i18n";
import type { SpecBoardDebt, SpecBoardEntry, SpecBoardEpic, SpecColumn } from "../lib/types";

interface SpecBoardTabProps {
  project: string | null;
  onSelectProject: (project: string) => void;
}

function isArchived(entry: SpecBoardEntry): boolean {
  return entry.archived === true;
}

function seedActiveIds(specs: SpecBoardEntry[]): Set<string> {
  const next = new Set<string>();
  for (const spec of specs) {
    if (spec.active) next.add(spec.id);
  }
  return next;
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
          {projects.map((p) => (
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
  const visible = entry.column === "todo" ? entry.tasks.filter((task) => !task.done) : entry.tasks;
  if (visible.length === 0) {
    return <p className="text-[10px] text-foreground/20 italic mt-2">{t.specBoard.noTasksYet}</p>;
  }
  return (
    <ul className="mt-2 flex flex-col gap-1">
      {visible.map((task) => (
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

/* ── Epic card (display-only; never expand / archive / POST) ──────── */

function EpicCard({ epic }: { epic: SpecBoardEpic }) {
  return (
    <div className="rounded-lg border border-white/10 bg-white/[0.02] p-3">
      <div className="flex items-baseline gap-2 min-w-0">
        <span className="text-[var(--color-epic-mark)] text-[12px] font-medium shrink-0">E</span>
        <p className="text-[12px] font-medium text-foreground/90 truncate">{epic.title}</p>
      </div>
      <p className="text-[11px] text-foreground/55 mt-1">{epic.summary}</p>
      <p className="text-[10px] text-foreground/40 mt-0.5">{epic.plan_title}</p>
      <p className="text-[10px] text-foreground/30 font-mono whitespace-normal break-all mt-0.5">{epic.id}</p>
    </div>
  );
}

/* ── Spec card ────────────────────────────────────────────────────── */

function SpecCard({
  entry,
  expanded,
  onToggle,
  onSetArchived,
}: {
  entry: SpecBoardEntry;
  expanded: boolean;
  onToggle: (id: string) => void;
  onSetArchived: (id: string, archived: boolean) => void;
}) {
  const t = useUiMessages();
  const canExpand = true;
  const blurb = entry.blurb ?? "";
  const archived = isArchived(entry);
  const showArchive = expanded && entry.column === "done" && !archived;
  const showUnarchive = expanded && entry.column === "done" && archived;

  return (
    <div
      className={`rounded-lg border p-3 transition-colors ${
        entry.active ? "border-primary/30 bg-primary/[0.04]" : "border-white/10 bg-white/[0.02]"
      }`}
    >
      <button
        type="button"
        aria-expanded={expanded}
        onClick={() => {
          if (canExpand) onToggle(entry.id);
        }}
        className="w-full text-left cursor-pointer"
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

      {expanded && (
        <>
          {blurb !== "" ? (
            <p data-region="blurb" className="text-[11px] text-foreground/55 mt-2 whitespace-pre-wrap">
              {blurb}
            </p>
          ) : null}
          <TaskList entry={entry} />
          {showArchive ? (
            <button
              type="button"
              className="mt-2 text-[10px] text-foreground/40 hover:text-foreground/70"
              onClick={() => {
                onSetArchived(entry.id, true);
              }}
            >
              {t.specBoard.archive}
            </button>
          ) : null}
          {showUnarchive ? (
            <button
              type="button"
              className="mt-2 text-[10px] text-foreground/40 hover:text-foreground/70"
              onClick={() => {
                onSetArchived(entry.id, false);
              }}
            >
              {t.specBoard.unarchive}
            </button>
          ) : null}
        </>
      )}
    </div>
  );
}

/* ── Open-debt chrome (dead text; not a column) ───────────────────── */

function DebtStrip({ rows, label }: { rows: SpecBoardDebt[]; label: string }) {
  if (rows.length === 0) return null;
  return (
    <div role="region" aria-label={label} className="flex flex-col gap-1">
      {rows.map((row) => (
        <p key={row.id} className="text-[11px] text-foreground/55 px-1 whitespace-normal break-words">
          {row.id} {row.title}
        </p>
      ))}
    </div>
  );
}

/* ── Column ───────────────────────────────────────────────────────── */

function Column({
  id,
  title,
  entries,
  epics = [],
  expandedIds,
  onToggle,
  showArchived,
  onToggleShowArchived,
  onSetArchived,
}: {
  id: SpecColumn;
  title: string;
  entries: SpecBoardEntry[];
  epics?: SpecBoardEpic[];
  expandedIds: Set<string>;
  onToggle: (id: string) => void;
  showArchived: boolean;
  onToggleShowArchived: () => void;
  onSetArchived: (id: string, archived: boolean) => void;
}) {
  const t = useUiMessages();
  const todoEpics = id === "todo" ? epics : [];
  const pending = todoEpics.length + entries.length;
  return (
    <div className="flex-1 min-w-0 flex flex-col">
      <div className="flex items-center justify-between mb-3 px-1">
        <h3 className="text-[11px] font-semibold uppercase tracking-wider text-foreground/40">{title}</h3>
        <div className="flex items-center gap-2">
          {id === "done" ? (
            <button
              type="button"
              aria-pressed={showArchived}
              onClick={onToggleShowArchived}
              className={`text-[10px] ${
                showArchived ? "text-foreground/70" : "text-foreground/35"
              } hover:text-foreground/70`}
            >
              {t.specBoard.showArchived}
            </button>
          ) : null}
          <span className="text-[10px] text-foreground/25">
            {id === "todo" ? t.specBoard.pendingCount(pending) : entries.length}
          </span>
        </div>
      </div>
      <div className="flex flex-col gap-2">
        {pending === 0 ? (
          <p className="text-[11px] text-foreground/15 italic px-1">{t.specBoard.noSpecs}</p>
        ) : (
          <>
            {todoEpics.map((epic) => (
              <EpicCard key={epic.id} epic={epic} />
            ))}
            {entries.map((entry) => (
              <SpecCard
                key={entry.id}
                entry={entry}
                expanded={expandedIds.has(entry.id)}
                onToggle={onToggle}
                onSetArchived={onSetArchived}
              />
            ))}
          </>
        )}
      </div>
    </div>
  );
}

/* ── Tab root ─────────────────────────────────────────────────────── */

export function SpecBoardTab({ project, onSelectProject }: SpecBoardTabProps) {
  const t = useUiMessages();
  const { board, loading, refresh } = useSpecBoard(project);
  const [expandedIds, setExpandedIds] = useState<Set<string>>(() => new Set());
  const [showArchived, setShowArchived] = useState(false);
  const lastProjectRef = useRef(project);
  const seededRef = useRef(false);
  const prevBoardRef = useRef(board);
  const staleBoardRef = useRef<typeof board>(null);

  if (lastProjectRef.current !== project) {
    lastProjectRef.current = project;
    seededRef.current = false;
    staleBoardRef.current = prevBoardRef.current;
    setExpandedIds(new Set());
    setShowArchived(false);
  }
  prevBoardRef.current = board;

  if (board && !seededRef.current && board !== staleBoardRef.current) {
    seededRef.current = true;
    setExpandedIds(seedActiveIds(board.specs));
  }

  const onToggle = (id: string) => {
    setExpandedIds((prev) => {
      const next = new Set(prev);
      if (next.has(id)) next.delete(id);
      else next.add(id);
      return next;
    });
  };

  const persistArchive = async (specId: string, archived: boolean): Promise<void> => {
    if (!project) return;
    try {
      const res = await fetch("/api/spec-board", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ project, spec_id: specId, archived }),
      });
      if (!res.ok) return;
      await refresh();
    } catch {
      /* GET board stays the source of truth */
    }
  };

  if (project === null) {
    return <ProjectPicker onSelectProject={onSelectProject} />;
  }

  if (loading && !board) {
    return <p className="p-8 text-foreground/20 text-[12px]">{t.common.loading}</p>;
  }

  /* Last-resort: stale poll / direct mount can reach here with both flags false. Grill-only must paint. */
  if (!board || !(board.sdd_skill_present === true || board.grill_skill_present === true)) {
    return <p className="p-8 text-foreground/30 text-[12px]">{t.specBoard.notSddSkill}</p>;
  }

  const byColumn = (id: SpecColumn) =>
    board.specs.filter((s) => {
      if (s.column !== id) return false;
      if (id === "done" && isArchived(s) && !showArchived) return false;
      return true;
    });

  return (
    <ScrollArea className="h-full">
      <div className="p-6 max-w-6xl mx-auto flex flex-col gap-6">
        <DebtStrip rows={board.debt ?? []} label={t.specBoard.openTechDebt} />
        <div className="flex gap-6">
          <Column
            id="todo"
            title={t.specBoard.columnTodo}
            entries={byColumn("todo")}
            epics={board.epics ?? []}
            expandedIds={expandedIds}
            onToggle={onToggle}
            showArchived={showArchived}
            onToggleShowArchived={() => setShowArchived((v) => !v)}
            onSetArchived={(id, archived) => {
              void persistArchive(id, archived);
            }}
          />
          <Column
            id="in_progress"
            title={t.specBoard.columnInProgress}
            entries={byColumn("in_progress")}
            expandedIds={expandedIds}
            onToggle={onToggle}
            showArchived={showArchived}
            onToggleShowArchived={() => setShowArchived((v) => !v)}
            onSetArchived={(id, archived) => {
              void persistArchive(id, archived);
            }}
          />
          <Column
            id="done"
            title={t.specBoard.columnDone}
            entries={byColumn("done")}
            expandedIds={expandedIds}
            onToggle={onToggle}
            showArchived={showArchived}
            onToggleShowArchived={() => setShowArchived((v) => !v)}
            onSetArchived={(id, archived) => {
              void persistArchive(id, archived);
            }}
          />
        </div>
      </div>
    </ScrollArea>
  );
}
