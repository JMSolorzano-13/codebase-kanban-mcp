/**
 * @sdd-task: Task #4 - AdrTab generated-at + replace warning
 * @sdd-spec: specs/spec-004-j8k-adr-parse-on-reindex/spec.md
 * @sdd-decision: SDD-ADR-016 - indexed_at from useProjects list cache; stamp is chrome not blob
 * @sdd-why: US-006 show generated-at + replace warning when CBM-GENERATED-START is present
 * @human-debug: If stamp missing with markers → list cache miss or lastClean lacks CBM-GENERATED-START; if ISO appears in textarea → wrote generated-at into content
 */
import { useCallback, useEffect, useState } from "react";
import { useProjects } from "../hooks/useProjects";
import { formatIndexedAt } from "../lib/formatIndexedAt";
import { useUiLanguage, useUiMessages } from "../lib/i18n";

/** Gherkin asserts this English heading string even when UI language is zh. */
export const ADR_PLACEHOLDER =
  "# Architecture Decision Record\n\n## Context\n...\n\n## Decision\n...\n\n## Consequences\n...";

const GENERATED_MARKER = "CBM-GENERATED-START";

interface AdrPayload {
  has_adr?: boolean;
  content?: string;
  updated_at?: string;
}

export interface AdrTabProps {
  project: string;
  onDirtyChange?: (dirty: boolean) => void;
}

export function AdrTab({ project, onDirtyChange }: AdrTabProps) {
  const t = useUiMessages();
  const lang = useUiLanguage();
  const { projects } = useProjects();
  const listed = projects.find((p) => p.name === project);
  const [hasAdr, setHasAdr] = useState(false);
  const [content, setContent] = useState("");
  const [lastClean, setLastClean] = useState("");
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState("");
  const [status, setStatus] = useState("");

  const fetchAdr = useCallback(async () => {
    try {
      const res = await fetch(`/api/adr?project=${encodeURIComponent(project)}`);
      if (!res.ok) {
        setHasAdr(false);
        setContent("");
        setLastClean("");
        return;
      }
      const data = (await res.json()) as AdrPayload;
      const has = Boolean(data.has_adr);
      // has_adr false / missing content → empty value; do not persist the placeholder
      const next = has && typeof data.content === "string" ? data.content : "";
      setHasAdr(has);
      setContent(next);
      setLastClean(next);
    } catch {
      setHasAdr(false);
      setContent("");
      setLastClean("");
    }
  }, [project]);

  useEffect(() => {
    void fetchAdr();
  }, [fetchAdr]);

  useEffect(() => {
    onDirtyChange?.(content !== lastClean);
  }, [content, lastClean, onDirtyChange]);

  const persist = async (nextContent: string) => {
    setSaving(true);
    setError("");
    try {
      const res = await fetch("/api/adr", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ project, content: nextContent }),
      });
      if (!res.ok) {
        setStatus("");
        setError(t.adr.saveError);
        return;
      }
      setContent(nextContent);
      setLastClean(nextContent);
      setStatus(t.adr.saveSuccess);
      if (nextContent !== "") setHasAdr(true);
      try {
        const getRes = await fetch(`/api/adr?project=${encodeURIComponent(project)}`);
        if (getRes.ok) {
          const data = (await getRes.json()) as AdrPayload;
          setHasAdr(Boolean(data.has_adr));
        }
      } catch {
        /* keep hasAdr from the POST outcome; C upserts empty so do not force false */
      }
    } catch {
      setStatus("");
      setError(t.adr.saveError);
    } finally {
      setSaving(false);
    }
  };

  const onEdit = (value: string) => {
    setContent(value);
    if (status) setStatus("");
  };

  const hasGenerated = lastClean.includes(GENERATED_MARKER);

  return (
    <div className="flex flex-col h-full min-h-0 p-6 gap-3">
      <div>
        <h2 className="text-[15px] font-semibold text-foreground/90">{t.adr.title}</h2>
        <p className="text-[11px] text-foreground/30 font-mono mt-0.5">{project}</p>
        {hasGenerated && listed ? (
          <p className="text-[11px] text-foreground/35 mt-1">
            <span className="text-foreground/25 mr-1.5">{t.adr.generatedAt}</span>
            <time dateTime={listed.indexed_at} title={listed.indexed_at}>
              {formatIndexedAt(listed.indexed_at, lang)}
            </time>
          </p>
        ) : null}
      </div>
      {hasGenerated ? (
        <p role="note" className="text-[12px] text-foreground/45">
          {t.adr.replaceWarning}
        </p>
      ) : null}
      {error ? (
        <p role="alert" className="text-[12px] text-destructive">
          {error}
        </p>
      ) : null}
      {status ? (
        <p role="status" className="text-[12px] text-foreground/50">
          {status}
        </p>
      ) : null}
      <textarea
        aria-label={t.adr.title}
        value={content}
        onChange={(e) => onEdit(e.target.value)}
        placeholder={ADR_PLACEHOLDER}
        className="flex-1 min-h-[300px] bg-white/[0.03] border border-white/[0.06] rounded-xl px-4 py-3 text-[12px] text-foreground font-mono placeholder-foreground/15 outline-none focus:border-border resize-none leading-relaxed"
      />
      <div className="flex justify-end gap-2">
        {hasAdr ? (
          <button
            type="button"
            onClick={() => { void persist(""); }}
            disabled={saving}
            className="px-3 py-2 rounded-lg text-[12px] text-destructive/60 hover:text-destructive hover:bg-destructive/10 font-medium transition-all disabled:opacity-30"
          >
            {t.common.delete}
          </button>
        ) : null}
        <button
          type="button"
          onClick={() => { void persist(content); }}
          disabled={saving}
          className="px-4 py-2 rounded-lg bg-primary/20 hover:bg-primary/30 text-primary text-[12px] font-medium transition-all disabled:opacity-30"
        >
          {saving ? t.common.saving : t.common.save}
        </button>
      </div>
    </div>
  );
}
