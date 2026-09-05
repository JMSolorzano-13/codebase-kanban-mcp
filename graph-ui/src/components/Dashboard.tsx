/**
 * @sdd-task: Task #5 - Dashboard Reindex + i18n + remaining Gherkin
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-015 - Dashboard Reindex POSTs {root_path, project}; never project_name
 * @sdd-why: US-002/007 — refresh the existing row without the create modal or workspace navigation
 * @human-debug: If Reindex opens Graph → onSelectProject called; if 202 silent → setIndexing not set; if 500 drops row → refresh on error
 */
import { useCallback, useState } from "react";
import { ScrollArea } from "@/components/ui/scroll-area";
import { useProjects } from "../hooks/useProjects";
import { formatIndexedAt } from "../lib/formatIndexedAt";
import { useUiLanguage, useUiMessages } from "../lib/i18n";
import { groupProjects } from "../lib/pathGroups";
import type { Project } from "../lib/types";
import { ControlTab } from "./ControlTab";
import { CreateIndexModal } from "./CreateIndexModal";
import { HealthDot } from "./HealthDot";
import { IndexProgress } from "./IndexProgress";

interface DashboardProps {
  onSelectProject: (project: string) => void;
  onPathExists?: (project: string) => void;
}

interface IndexErrorPayload {
  error?: string;
}

export function Dashboard({ onSelectProject, onPathExists }: DashboardProps) {
  const t = useUiMessages();
  const lang = useUiLanguage();
  const { projects, loading, error, refresh } = useProjects();
  const [showModal, setShowModal] = useState(false);
  const [indexing, setIndexing] = useState(false);
  const [reindexError, setReindexError] = useState<string | null>(null);

  const deleteProject = useCallback(async (name: string) => {
    if (!confirm(t.projects.deleteConfirm(name))) return;
    try {
      await fetch(`/api/project?name=${encodeURIComponent(name)}`, { method: "DELETE" });
      refresh();
    } catch {
      /* keep row; operator can Refresh */
    }
  }, [refresh, t.projects]);

  const reindexProject = useCallback(async (p: Project) => {
    setReindexError(null);
    try {
      const res = await fetch("/api/index", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ root_path: p.root_path, project: p.name }),
      });
      if (res.status === 202) {
        setIndexing(true);
        refresh();
        return;
      }
      let message: string = t.projects.reindexError;
      try {
        const data = (await res.json()) as IndexErrorPayload;
        if (typeof data.error === "string" && data.error.length > 0) {
          message = data.error;
        }
      } catch {
        /* keep i18n fallback — 500 may have empty/non-JSON body */
      }
      setReindexError(message);
    } catch {
      setReindexError(t.projects.reindexError);
    }
  }, [refresh, t.projects.reindexError]);

  const groups = groupProjects(projects);
  const listError = error || reindexError;

  return (
    <ScrollArea className="h-full">
      <div className="p-8 max-w-4xl mx-auto">
        {indexing && (
          <IndexProgress onDone={() => { setIndexing(false); refresh(); }} />
        )}

        <div className="flex items-center justify-between mb-6">
          <h2 className="text-[15px] font-semibold text-foreground/80">{t.projects.indexedProjects}</h2>
          <div className="flex items-center gap-2">
            <button
              onClick={() => setShowModal(true)}
              className="px-3 py-1.5 rounded-lg bg-primary/15 hover:bg-primary/25 text-primary text-[12px] font-medium transition-all"
            >
              + {t.index.newIndex}
            </button>
            <button
              onClick={refresh}
              disabled={loading}
              className="px-3 py-1.5 rounded-lg bg-white/[0.04] hover:bg-white/[0.07] text-[12px] text-foreground/40 font-medium transition-all disabled:opacity-30"
            >
              {loading ? "..." : t.common.refresh}
            </button>
          </div>
        </div>

        {listError && (
          <div role="alert" className="rounded-xl border border-destructive/20 bg-destructive/5 p-4 mb-6">
            <p className="text-destructive text-[13px]">{listError}</p>
          </div>
        )}

        {!loading && projects.length === 0 && !error && (
          <div className="text-center py-20">
            <p className="text-foreground/25 text-[13px] mb-2">{t.projects.noIndexedProjects}</p>
            <button
              onClick={() => setShowModal(true)}
              className="px-4 py-2 rounded-lg bg-primary/15 hover:bg-primary/25 text-primary text-[12px] font-medium transition-all"
            >
              {t.projects.indexFirstRepository}
            </button>
          </div>
        )}

        <div className="space-y-3">
          {groups.map((group) =>
            group.members.length >= 2 ? (
              <section
                key={group.key}
                role="region"
                aria-label={t.projects.conflict}
                className="rounded-xl border border-border/50 bg-card p-5"
              >
                <div className="flex items-start justify-between gap-3 mb-3">
                  <div className="min-w-0">
                    <p className="text-[11px] uppercase tracking-wide text-foreground/40 mb-1">
                      {t.projects.conflict}
                    </p>
                    <p className="text-[11px] text-foreground/20 font-mono truncate">
                      {group.newest.root_path}
                    </p>
                  </div>
                  <button
                    onClick={() => onSelectProject(group.newest.name)}
                    className="px-3 py-1.5 rounded-lg bg-primary/15 hover:bg-primary/25 text-primary text-[12px] font-medium transition-all shrink-0"
                  >
                    {t.projects.enter}
                  </button>
                </div>
                <ul className="space-y-3">
                  {group.members.map((p) => (
                    <li key={p.name} className="flex items-start justify-between gap-3">
                      <div className="min-w-0">
                        <p className="text-[14px] font-semibold text-foreground/90 mb-0.5">{p.name}</p>
                        <p className="text-[11px] text-foreground/35">
                          <span className="text-foreground/25 mr-1.5">{t.projects.lastIndexed}</span>
                          <time dateTime={p.indexed_at} title={p.indexed_at}>
                            {formatIndexedAt(p.indexed_at, lang)}
                          </time>
                        </p>
                      </div>
                      <div className="flex items-center gap-1.5 shrink-0">
                        <button
                          type="button"
                          aria-label={t.projects.reindex}
                          onClick={() => { void reindexProject(p); }}
                          className="px-3 py-1.5 rounded-lg bg-white/[0.04] hover:bg-white/[0.07] text-[12px] text-foreground/50 font-medium transition-all"
                        >
                          {t.projects.reindex}
                        </button>
                        <button
                          onClick={() => { void deleteProject(p.name); }}
                          className="px-2 py-1.5 rounded-lg hover:bg-destructive/10 text-foreground/20 hover:text-destructive text-[12px] transition-all shrink-0"
                          title={t.projects.deleteNamed(p.name)}
                          aria-label={t.projects.deleteNamed(p.name)}
                        >
                          ✕
                        </button>
                      </div>
                    </li>
                  ))}
                </ul>
              </section>
            ) : (
              <SoloProjectCard
                key={group.newest.name}
                project={group.newest}
                lang={lang}
                lastIndexedLabel={t.projects.lastIndexed}
                enterLabel={t.projects.enter}
                reindexLabel={t.projects.reindex}
                deleteTitle={t.projects.deleteTitle}
                onEnter={onSelectProject}
                onReindex={reindexProject}
                onDelete={deleteProject}
              />
            ),
          )}
        </div>

        <div className="border-t border-border/30 pt-8 mt-8">
          <ControlTab embedded />
        </div>
      </div>
      {showModal && (
        <CreateIndexModal
          onClose={() => setShowModal(false)}
          onCreated={() => { setIndexing(true); refresh(); }}
          onPathExists={(name) => {
            setShowModal(false);
            onPathExists?.(name);
          }}
          existingProjects={projects}
        />
      )}
    </ScrollArea>
  );
}

function SoloProjectCard({
  project: p,
  lang,
  lastIndexedLabel,
  enterLabel,
  reindexLabel,
  deleteTitle,
  onEnter,
  onReindex,
  onDelete,
}: {
  project: Project;
  lang: "en" | "zh";
  lastIndexedLabel: string;
  enterLabel: string;
  reindexLabel: string;
  deleteTitle: string;
  onEnter: (name: string) => void;
  onReindex: (p: Project) => void;
  onDelete: (name: string) => void;
}) {
  return (
    <div className="rounded-xl border border-border/30 bg-card hover:bg-hover transition-all p-5">
      <div className="flex items-start justify-between gap-3">
        <div className="min-w-0 flex items-start gap-2.5">
          <div className="mt-1.5"><HealthDot name={p.name} /></div>
          <div className="min-w-0">
            <h3 className="text-[14px] font-semibold text-foreground/90 mb-0.5">{p.name}</h3>
            <p className="text-[11px] text-foreground/20 font-mono truncate">{p.root_path}</p>
            <p className="text-[11px] text-foreground/35 mt-1">
              <span className="text-foreground/25 mr-1.5">{lastIndexedLabel}</span>
              <time dateTime={p.indexed_at} title={p.indexed_at}>
                {formatIndexedAt(p.indexed_at, lang)}
              </time>
            </p>
          </div>
        </div>
        <div className="flex items-center gap-1.5 shrink-0">
          <button
            onClick={() => onEnter(p.name)}
            className="px-3 py-1.5 rounded-lg bg-primary/15 hover:bg-primary/25 text-primary text-[12px] font-medium transition-all"
          >
            {enterLabel}
          </button>
          <button
            type="button"
            aria-label={reindexLabel}
            onClick={() => { void onReindex(p); }}
            className="px-3 py-1.5 rounded-lg bg-white/[0.04] hover:bg-white/[0.07] text-[12px] text-foreground/50 font-medium transition-all"
          >
            {reindexLabel}
          </button>
          <button
            onClick={() => { void onDelete(p.name); }}
            className="px-2 py-1.5 rounded-lg hover:bg-destructive/10 text-foreground/20 hover:text-destructive text-[12px] transition-all"
            title={deleteTitle}
            aria-label={deleteTitle}
          >
            ✕
          </button>
        </div>
      </div>
    </div>
  );
}
