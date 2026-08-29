/**
 * @sdd-task: Task #4 - Create modal path_exists redirect + notice
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-016 - list canonical_root is the skip/group key
 * @sdd-why: Optional create skip uses the same newest pick as Dashboard conflict
 * @human-debug: If skip misses a listed Path → foldPathKey vs canonical_root mismatch
 */
import type { Project } from "./types";

export interface PathGroup {
  key: string;
  members: Project[];
  newest: Project;
}

/** Match cbm_identity_cmp_newest: strcmp indexed_at, then strcmp name. */
export function cmpNewest(a: Project, b: Project): number {
  const timeA = a.indexed_at ?? "";
  const timeB = b.indexed_at ?? "";
  if (timeA !== timeB) {
    return timeA < timeB ? -1 : 1;
  }
  const nameA = a.name ?? "";
  const nameB = b.name ?? "";
  if (nameA === nameB) return 0;
  return nameA < nameB ? -1 : 1;
}

export function pickNewest(members: readonly Project[]): Project {
  if (members.length === 0) {
    throw new Error("pickNewest: empty group");
  }
  return members.reduce((best, p) => (cmpNewest(p, best) > 0 ? p : best));
}

export function groupKey(p: Project): string {
  return p.canonical_root || p.root_path;
}

/** Trailing-slash fold only — not a JS realpath (server owns canonical_root). */
export function foldPathKey(path: string): string {
  const trimmed = path.replace(/[\\/]+$/, "");
  return trimmed.length > 0 ? trimmed : path;
}

/** Newest list row whose canonical_root or root_path matches Path. */
export function findNewestForPath(projects: readonly Project[], path: string): Project | undefined {
  if (!path) return undefined;
  const folded = foldPathKey(path);
  const matches = projects.filter((p) => {
    const keys = [p.canonical_root, p.root_path].filter((k): k is string => Boolean(k));
    return keys.some((k) => k === path || foldPathKey(k) === folded);
  });
  if (matches.length === 0) return undefined;
  return pickNewest(matches);
}

/** First-seen key order so Dashboard list order stays stable. */
export function groupProjects(projects: readonly Project[]): PathGroup[] {
  const buckets = new Map<string, Project[]>();
  const order: string[] = [];
  for (const p of projects) {
    const key = groupKey(p);
    const existing = buckets.get(key);
    if (existing) {
      existing.push(p);
    } else {
      buckets.set(key, [p]);
      order.push(key);
    }
  }
  return order.map((key) => {
    const members = buckets.get(key) ?? [];
    return { key, members, newest: pickNewest(members) };
  });
}
