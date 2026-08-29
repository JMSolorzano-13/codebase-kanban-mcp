/**
 * @sdd-task: Task #4 - Dashboard page: list + Control + create-index
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-002 - Stacked Dashboard: list then Control, one ScrollArea
 * @sdd-why: Extract ADR chrome so Dashboard rows omit it (US-002); keep isolated tests
 * @human-debug: If Dashboard shows ADR → imported by mistake; GET/POST /api/adr is this file only
 */
import { useCallback, useEffect, useState } from "react";
import { useUiMessages } from "../lib/i18n";

interface AdrPayload {
  has_adr?: boolean;
  content?: string;
  updated_at?: string;
}

export function AdrButton({ project }: { project: string }) {
  const t = useUiMessages();
  const [hasAdr, setHasAdr] = useState<boolean | null>(null);
  const [open, setOpen] = useState(false);
  const [content, setContent] = useState("");
  const [saving, setSaving] = useState(false);
  const [updatedAt, setUpdatedAt] = useState("");

  const fetchAdr = useCallback(async () => {
    try {
      const res = await fetch(`/api/adr?project=${encodeURIComponent(project)}`);
      const data = (await res.json()) as AdrPayload;
      setHasAdr(data.has_adr ?? false);
      if (data.content) setContent(data.content);
      if (data.updated_at) setUpdatedAt(data.updated_at);
    } catch {
      setHasAdr(false);
    }
  }, [project]);

  useEffect(() => {
    void fetchAdr();
  }, [fetchAdr]);

  const save = async (nextContent = content) => {
    setSaving(true);
    try {
      await fetch("/api/adr", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ project, content: nextContent }),
      });
      await fetchAdr();
      setOpen(false);
    } catch {
      /* keep editor open; operator can retry */
    } finally {
      setSaving(false);
    }
  };

  if (hasAdr === null) return null;

  return (
    <>
      <button
        onClick={() => { setOpen(true); void fetchAdr(); }}
        className={`px-2.5 py-1 rounded-lg text-[10px] font-medium transition-all ${
          hasAdr
            ? "bg-accent/15 text-accent hover:bg-accent/25"
            : "bg-white/[0.03] text-foreground/25 hover:text-foreground/40 hover:bg-white/[0.06]"
        }`}
      >
        {hasAdr ? "ADR" : "+ ADR"}
      </button>

      {open && (
        <div className="fixed inset-0 z-50 flex items-center justify-center" onClick={() => setOpen(false)}>
          <div className="absolute inset-0 bg-black/60 backdrop-blur-sm" />
          <div className="relative bg-card border border-border/40 rounded-2xl p-6 w-full max-w-2xl shadow-2xl max-h-[80vh] flex flex-col" onClick={(e) => e.stopPropagation()}>
            <div className="flex items-center justify-between mb-4">
              <div>
                <h3 className="text-[15px] font-semibold text-foreground/90">{t.adr.title}</h3>
                <p className="text-[11px] text-foreground/30 font-mono mt-0.5">{project}</p>
              </div>
              <button onClick={() => setOpen(false)} className="text-foreground/20 hover:text-foreground/50 text-[16px] p-1">×</button>
            </div>
            {updatedAt && (
              <p className="text-[10px] text-foreground/20 mb-3">{t.adr.lastUpdated}: {updatedAt}</p>
            )}
            <textarea
              value={content}
              onChange={(e) => setContent(e.target.value)}
              placeholder={"# Architecture Decision Record\n\n## Context\n...\n\n## Decision\n...\n\n## Consequences\n..."}
              className="flex-1 min-h-[300px] bg-white/[0.03] border border-white/[0.06] rounded-xl px-4 py-3 text-[12px] text-foreground font-mono placeholder-foreground/15 outline-none focus:border-primary/30 resize-none leading-relaxed"
            />
            <div className="flex justify-end gap-2 mt-4">
              {hasAdr && (
                <button
                  onClick={() => { void (async () => { setContent(""); await save(""); })(); }}
                  className="px-3 py-2 rounded-lg text-[12px] text-destructive/60 hover:text-destructive hover:bg-destructive/10 font-medium transition-all"
                >
                  {t.common.delete}
                </button>
              )}
              <button onClick={() => setOpen(false)} className="px-4 py-2 rounded-lg text-[12px] text-foreground/40 hover:bg-white/[0.04] font-medium transition-all">{t.common.cancel}</button>
              <button onClick={() => { void save(); }} disabled={saving} className="px-4 py-2 rounded-lg bg-primary/20 hover:bg-primary/30 text-primary text-[12px] font-medium transition-all disabled:opacity-30">
                {saving ? t.common.saving : t.common.save}
              </button>
            </div>
          </div>
        </div>
      )}
    </>
  );
}
