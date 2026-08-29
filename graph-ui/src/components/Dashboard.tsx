/**
 * @sdd-task: Task #4 - Dashboard page: list + Control + create-index
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-002 - Stacked Dashboard: list then Control, one ScrollArea
 * @sdd-why: Account home is identity + freshness + daemon Control; no schema cards
 * @human-debug: If Graph opens after 202 → onSelectProject called from create; if no Control → embedded mount missing
 */
import { useCallback, useState } from "react";
import { ScrollArea } from "@/components/ui/scroll-area";
import { useProjects } from "../hooks/useProjects";
import { formatIndexedAt } from "../lib/formatIndexedAt";
import { useUiLanguage, useUiMessages } from "../lib/i18n";
import { ControlTab } from "./ControlTab";
import { CreateIndexModal } from "./CreateIndexModal";
import { HealthDot } from "./HealthDot";
import { IndexProgress } from "./IndexProgress";

interface DashboardProps {
  onSelectProject: (project: string) => void;
}

export function Dashboard({ onSelectProject }: DashboardProps) {
  const t = useUiMessages();
  const lang = useUiLanguage();
  const { projects, loading, error, refresh } = useProjects();
  const [showModal, setShowModal] = useState(false);
  const [indexing, setIndexing] = useState(false);

  const deleteProject = useCallback(async (name: string) => {
    if (!confirm(t.projects.deleteConfirm(name))) return;
    try {
      await fetch(`/api/project?name=${encodeURIComponent(name)}`, { method: "DELETE" });
      refresh();
    } catch {
      /* keep row; operator can Refresh */
    }
  }, [refresh, t.projects]);

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

        {error && (
          <div role="alert" className="rounded-xl border border-destructive/20 bg-destructive/5 p-4 mb-6">
            <p className="text-destructive text-[13px]">{error}</p>
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
          {projects.map((p) => (
            <div key={p.name} className="rounded-xl border border-border/30 bg-card hover:bg-hover transition-all p-5">
              <div className="flex items-start justify-between gap-3">
                <div className="min-w-0 flex items-start gap-2.5">
                  <div className="mt-1.5"><HealthDot name={p.name} /></div>
                  <div className="min-w-0">
                    <h3 className="text-[14px] font-semibold text-foreground/90 mb-0.5">{p.name}</h3>
                    <p className="text-[11px] text-foreground/20 font-mono truncate">{p.root_path}</p>
                    <p className="text-[11px] text-foreground/35 mt-1">
                      <span className="text-foreground/25 mr-1.5">{t.projects.lastIndexed}</span>
                      <time dateTime={p.indexed_at} title={p.indexed_at}>
                        {formatIndexedAt(p.indexed_at, lang)}
                      </time>
                    </p>
                  </div>
                </div>
                <div className="flex items-center gap-1.5 shrink-0">
                  <button
                    onClick={() => onSelectProject(p.name)}
                    className="px-3 py-1.5 rounded-lg bg-primary/15 hover:bg-primary/25 text-primary text-[12px] font-medium transition-all"
                  >
                    {t.projects.enter}
                  </button>
                  <button
                    onClick={() => { void deleteProject(p.name); }}
                    className="px-2 py-1.5 rounded-lg hover:bg-destructive/10 text-foreground/20 hover:text-destructive text-[12px] transition-all"
                    title={t.projects.deleteTitle}
                    aria-label={t.projects.deleteTitle}
                  >
                    ✕
                  </button>
                </div>
              </div>
            </div>
          ))}
        </div>

        <div className="border-t border-border/30 pt-8 mt-8">
          <ControlTab embedded />
        </div>
      </div>
      {showModal && (
        <CreateIndexModal
          onClose={() => setShowModal(false)}
          onCreated={() => { setIndexing(true); refresh(); }}
        />
      )}
    </ScrollArea>
  );
}
