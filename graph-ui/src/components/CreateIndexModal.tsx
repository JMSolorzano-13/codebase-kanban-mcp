/**
 * @sdd-task: Task #4 - Create modal path_exists redirect + notice
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-015 - Create POST is {root_path} only; 409 path_exists is not a reindex
 * @sdd-why: Owned Path must close the modal and hand existing_project to App — never 202-then-redirect
 * @human-debug: If path_exists starts IndexProgress → line 133 (onCreated); if name_exists leaves modal → line 127 (code treated as path_exists)
 */
import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import { ScrollArea } from "@/components/ui/scroll-area";
import { useUiMessages } from "../lib/i18n";
import { findNewestForPath } from "../lib/pathGroups";
import type { Project } from "../lib/types";

interface BrowsePayload {
  error?: string;
  path?: string;
  dirs?: string[];
  roots?: string[];
  parent?: string;
}

interface IndexErrorPayload {
  error?: string;
  code?: string;
  existing_project?: string;
}

interface CreateIndexModalProps {
  onClose: () => void;
  onCreated: () => void;
  onPathExists: (project: string) => void;
  existingProjects?: readonly Project[];
}

function joinPath(base: string, dir: string): string {
  if (!base || base === "/") return `/${dir}`;
  if (/^[A-Za-z]:[\\/]?$/.test(base)) return `${base[0]}:/${dir}`;
  const slash = base.includes("\\") && !base.includes("/") ? "\\" : "/";
  return `${base.replace(/[\\/]+$/, "")}${slash}${dir}`;
}

export function CreateIndexModal({
  onClose,
  onCreated,
  onPathExists,
  existingProjects = [],
}: CreateIndexModalProps) {
  const t = useUiMessages();
  const [currentPath, setCurrentPath] = useState("");
  const [dirs, setDirs] = useState<string[]>([]);
  const [roots, setRoots] = useState<string[]>(["/"]);
  const [parentPath, setParentPath] = useState("");
  const [filter, setFilter] = useState("");
  const [activeIndex, setActiveIndex] = useState(0);
  const [loading, setLoading] = useState(false);
  const [submitting, setSubmitting] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const filterRef = useRef<HTMLInputElement>(null);
  /* Path whose listing is currently shown. Lets the typed-path effect skip a
   * redundant re-fetch after browse() sets currentPath itself. */
  const lastBrowsedRef = useRef<string>("");

  const browse = useCallback(async (path?: string, opts?: { silent?: boolean }) => {
    const silent = opts?.silent ?? false;
    if (!silent) setLoading(true);
    setError(null);
    try {
      const q = path ? `?path=${encodeURIComponent(path)}` : "";
      const res = await fetch(`/api/browse${q}`);
      const data = (await res.json()) as BrowsePayload;
      if (data.error) throw new Error(data.error);
      lastBrowsedRef.current = data.path ?? "";
      setCurrentPath(data.path ?? "");
      setDirs((data.dirs ?? []).sort());
      setRoots(data.roots ?? ["/"]);
      setParentPath(data.parent ?? "/");
    } catch (e) {
      /* Silent (typed-path) refreshes keep the last good listing instead of
       * flashing an error while the user is still typing a path. */
      if (!silent) setError(e instanceof Error ? e.message : "Browse failed");
    } finally {
      if (!silent) setLoading(false);
    }
  }, []);

  useEffect(() => { void browse(); }, [browse]);
  useEffect(() => { filterRef.current?.focus(); }, []);

  /* Windows only: when the user types a drive path into the Repository path
   * field, refresh the folder listing to match (debounced). On Windows, typing
   * is the way to switch drives, and without this the breadcrumb and path box
   * updated but the directory list stayed stale (e.g. typing "D:/" still showed
   * the previous drive's folders). POSIX navigation is left unchanged. */
  useEffect(() => {
    if (!currentPath || currentPath === lastBrowsedRef.current) return;
    if (!/^[A-Za-z]:/.test(currentPath.replace(/\\/g, "/"))) return;
    const id = setTimeout(() => { void browse(currentPath, { silent: true }); }, 350);
    return () => clearTimeout(id);
  }, [currentPath, browse]);

  const filteredDirs = useMemo(() => {
    const q = filter.trim().toLowerCase();
    if (!q) return dirs;
    return dirs.filter((d) => d.toLowerCase().includes(q));
  }, [dirs, filter]);

  useEffect(() => { setActiveIndex(0); }, [filter, currentPath]);

  const submit = async (path = currentPath) => {
    if (!path) return;
    /* List already owns this Path: same Graph + notice as 409, without a 202. */
    const listed = findNewestForPath(existingProjects, path);
    if (listed) {
      onPathExists(listed.name);
      onClose();
      return;
    }
    setSubmitting(true);
    setError(null);
    try {
      const res = await fetch("/api/index", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ root_path: path }),
      });
      const data = (await res.json()) as IndexErrorPayload;
      if (res.status === 409 && data.code === "path_exists" && data.existing_project) {
        onPathExists(data.existing_project);
        onClose();
        return;
      }
      if (!res.ok) throw new Error(data.error ?? "Failed");
      onCreated();
      onClose();
    } catch (e) {
      setError(e instanceof Error ? e.message : "Failed");
    } finally {
      setSubmitting(false);
    }
  };

  const onFilterKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === "ArrowDown") {
      e.preventDefault();
      setActiveIndex((i) => Math.min(i + 1, Math.max(filteredDirs.length - 1, 0)));
    } else if (e.key === "ArrowUp") {
      e.preventDefault();
      setActiveIndex((i) => Math.max(i - 1, 0));
    } else if (e.key === "Enter" && filteredDirs.length > 0) {
      e.preventDefault();
      const dir = filteredDirs.length === 1 ? filteredDirs[0] : filteredDirs[activeIndex];
      if (filteredDirs.length === 1) void submit(joinPath(currentPath, dir));
      else void browse(joinPath(currentPath, dir));
    }
  };

  const displayPath = currentPath.replace(/\\/g, "/");
  const segments = displayPath.split("/").filter(Boolean);
  /* A Windows drive path ("C:/Users/rap") has no unified "/" root — its first
   * segment is the drive letter. Build crumb targets accordingly so clicking a
   * segment navigates to a real directory instead of a bogus "/C:/..." path
   * that the backend rejects as "not a directory". */
  const isWinPath = /^[A-Za-z]:$/.test(segments[0] ?? "");
  const crumbPath = (i: number): string => {
    const parts = segments.slice(0, i + 1);
    if (isWinPath) return parts.length === 1 ? `${parts[0]}/` : parts.join("/");
    return "/" + parts.join("/");
  };

  /* Root/drive quick-jump buttons. On Windows the POSIX "/" root is meaningless
   * — browsing it returns an empty listing — so drop it and offer drive roots
   * instead. An older backend may not enumerate drives, so always include the
   * current drive; other drives stay reachable by typing a path. */
  const displayRoots = (() => {
    if (!isWinPath) return roots;
    const drives = Array.from(new Set(
      roots.filter((r) => /^[A-Za-z]:[\\/]?$/.test(r)).map((r) => `${r[0].toUpperCase()}:/`),
    ));
    const curRoot = `${displayPath[0].toUpperCase()}:/`;
    if (!drives.includes(curRoot)) drives.unshift(curRoot);
    return drives;
  })();

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center" onClick={onClose}>
      <div className="absolute inset-0 bg-black/60 backdrop-blur-sm" />
      <div className="relative bg-card border border-border/40 rounded-2xl w-full max-w-2xl shadow-2xl flex flex-col overflow-hidden" style={{ height: "min(82vh, 680px)" }} onClick={(e) => e.stopPropagation()}>
        <div className="px-5 pt-5 pb-3 shrink-0">
          <h3 className="text-[15px] font-semibold text-foreground/90 mb-1">{t.index.selectRepositoryFolder}</h3>
          <p className="text-[12px] text-foreground/30">{t.index.instructions}</p>
        </div>

        <div className="px-5 pb-3 shrink-0">
          <label className="block">
            <span className="block text-[10px] uppercase tracking-widest text-foreground/25 mb-1">{t.index.repositoryPath}</span>
            <input
              aria-label={t.index.repositoryPath}
              value={currentPath}
              onChange={(e) => setCurrentPath(e.target.value)}
              onKeyDown={(e) => { if (e.key === "Enter" && /^[A-Za-z]:/.test(currentPath.replace(/\\/g, "/"))) { e.preventDefault(); void browse(currentPath); } }}
              className="w-full bg-white/[0.04] border border-white/[0.06] rounded-lg px-3 py-2 text-[12px] text-foreground font-mono outline-none focus:border-primary/40"
            />
          </label>
        </div>

        <div className="px-5 pb-3 flex items-center gap-2 shrink-0">
          <input
            ref={filterRef}
            value={filter}
            placeholder={t.index.filterFolders}
            onChange={(e) => setFilter(e.target.value)}
            onKeyDown={onFilterKeyDown}
            className="flex-1 bg-white/[0.04] border border-white/[0.06] rounded-lg px-3 py-2 text-[12px] text-foreground outline-none focus:border-primary/40 placeholder:text-foreground/20"
          />
          <div className="flex items-center gap-1">
            {displayRoots.map((root) => (
              <button
                key={root}
                aria-label={t.index.browseRoot(root)}
                onClick={() => { void browse(root); }}
                className="px-2.5 py-2 rounded-lg bg-white/[0.04] hover:bg-white/[0.07] text-[11px] text-foreground/45 font-mono transition-all"
              >
                {root}
              </button>
            ))}
          </div>
        </div>

        <div className="px-5 py-2 border-y border-border/20 flex items-center gap-0.5 overflow-x-auto text-[11px] shrink-0">
          {!isWinPath && (
            <button onClick={() => { void browse("/"); }} className="text-primary/60 hover:text-primary shrink-0 transition-colors">/</button>
          )}
          {segments.map((seg, i) => (
            <span key={i} className="flex items-center gap-0.5 shrink-0">
              {(i > 0 || !isWinPath) && <span className="text-foreground/15">/</span>}
              <button
                onClick={() => { void browse(crumbPath(i)); }}
                className={`transition-colors ${i === segments.length - 1 ? "text-foreground/70 font-medium" : "text-primary/50 hover:text-primary"}`}
              >
                {seg}
              </button>
            </span>
          ))}
        </div>

        <ScrollArea className="flex-1 min-h-0">
          <div className="px-2 py-1">
            {currentPath !== "/" && (
              <button
                onClick={() => { void browse(parentPath); }}
                className="flex items-center gap-2 w-full text-left px-3 py-2 rounded-lg hover:bg-white/[0.04] text-[12px] text-foreground/40 transition-colors"
              >
                <span className="text-foreground/20">↑</span>
                <span>..</span>
              </button>
            )}
            {loading ? (
              <p className="text-foreground/20 text-[12px] text-center py-8">{t.common.loading}</p>
            ) : filteredDirs.length === 0 ? (
              <p className="text-foreground/15 text-[12px] text-center py-8">{t.index.noSubdirectories}</p>
            ) : (
              filteredDirs.map((d, i) => (
                <div
                  key={d}
                  className={`flex items-center gap-2 rounded-lg px-3 py-1.5 text-[12px] transition-colors group ${
                    i === activeIndex ? "bg-white/[0.05]" : "hover:bg-white/[0.04]"
                  }`}
                >
                  <button
                    aria-label={t.index.browseRoot(d)}
                    onClick={() => { void browse(joinPath(currentPath, d)); }}
                    className="flex min-w-0 flex-1 items-center gap-2 text-left text-foreground/60"
                  >
                    <span className="text-foreground/20 group-hover:text-foreground/40">/</span>
                    <span className="truncate">{d}</span>
                  </button>
                  <button
                    aria-label={t.index.indexDirectory(d)}
                    onClick={() => { void submit(joinPath(currentPath, d)); }}
                    disabled={submitting}
                    className="opacity-100 sm:opacity-0 sm:group-hover:opacity-100 px-2 py-1 rounded-md bg-primary/15 hover:bg-primary/25 text-primary text-[10px] font-medium transition-all disabled:opacity-30"
                  >
                    {t.index.indexThisFolder}
                  </button>
                </div>
              ))
            )}
          </div>
        </ScrollArea>

        <div className="px-5 py-4 border-t border-border/20 shrink-0">
          {error && <div className="rounded-lg bg-destructive/10 border border-destructive/20 px-3 py-2 mb-3"><p className="text-destructive text-[11px]">{error}</p></div>}
          <div className="flex items-center justify-between">
            <p className="text-[11px] text-foreground/25 font-mono truncate max-w-[250px]">{currentPath}</p>
            <div className="flex gap-2 shrink-0">
              <button onClick={onClose} className="px-3 py-2 rounded-lg text-[12px] text-foreground/40 hover:bg-white/[0.04] font-medium transition-all">{t.common.cancel}</button>
              <button onClick={() => { void submit(); }} disabled={submitting || !currentPath} className="px-4 py-2 rounded-lg bg-primary/20 hover:bg-primary/30 text-primary text-[12px] font-medium transition-all disabled:opacity-30">
                {submitting ? t.index.starting : t.index.indexThisFolder}
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
